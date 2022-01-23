/*
	This file is part of Spine.

    Spine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Spine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Spine.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#include <iostream>

#include "utils/GothicVdf.h"
#include "utils/Hashing.h"

#include <QDirIterator>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFutureSynchronizer>
#include <QMap>
#include <QMutex>
#include <QTextStream>
#include <QtConcurrentRun>

QMap<QString, QString> parse(const QString & path) {
	QFile f(path);

	QMap<QString, QString> results;

	if (!f.open(QIODevice::ReadOnly)) return results;

	QTextStream ts(&f);

	while (!ts.atEnd()) {
		const auto line = ts.readLine();

		const auto split = line.split(";");

		Q_ASSERT(split.count() == 2);
		if (split.count() != 2) continue;

		results.insert(split[0].toLower(), split[1]);
	}

	return results;
}

int main(const int argc, char ** argv) {
	if (argc != 4) {
		std::cout << "Usage: " << argv[0] << " <g1|g2> <path to your mod files (_work folder) or a vdf/mod file> <path for results>" << std::endl;
		return 1;
	}

	const QString gothicVersion(argv[1]);
	QString modPath(argv[2]);
	QString resultPath(argv[3]);

	QMap<QString, QString> modkitFiles;

	if (gothicVersion == "g1") {
		modkitFiles = parse(":/g1.txt");
	} else if (gothicVersion == "g2") {
		modkitFiles = parse(":/g2.txt");
	} else {
		std::cout << "First parameter must be either g1 or g2 depending on the Gothic version your modification is made for." << std::endl;
		return 1;
	}
	
	modPath.replace("\\", "/");
	resultPath.replace("\\", "/");

	uint32_t counter = 0;
	uint32_t strippedCounter = 0;
	uint64_t strippedSizeCounter = 0;

	QMutex lock;

	QFile logFile("ModChecker.log");
	if (!logFile.open(QIODevice::WriteOnly)) {
		std::cout << "Couldn't open log file" << std::endl;
	}
	QTextStream logStream(&logFile);


	QElapsedTimer timer;
	timer.start();

	if (modPath.endsWith("vdf", Qt::CaseInsensitive) || modPath.endsWith("mod", Qt::CaseInsensitive)) {
		spine::utils::GothicVdf vdf(modPath);
		const auto b = vdf.parse();
		if (!b) {
			std::cout << "File couldn't be parsed" << std::endl;
			return 1;
		}

		const auto files = vdf.getFiles();

		for (int i = 0; i < files.count(); i++) {
			auto f = files[i];

			if (!f.startsWith("_WORK")) {
				strippedCounter++;
				vdf.remove(f);

				logStream << f << "\n";
			} else {
				auto copyF = f;
				const auto it2 = modkitFiles.find(copyF.replace("_WORK/", "").toLower());

				if (it2 != modkitFiles.end()) {
					QString modFileHash = vdf.getHash(i - strippedCounter);

					if (modFileHash == it2.value()) {
						strippedCounter++;
						vdf.remove(f);

						logStream << f << "\n";
					}
				}
			}

			counter++;
		}

		const auto strippedPath = resultPath + "/" + modPath.split("/").back();
		if (!QDir(resultPath).exists()) {
			QDir(resultPath).mkpath(resultPath);
		}
		if (strippedCounter > 0) {
			vdf.write(strippedPath);
			vdf.close();
		} else {
			vdf.close();
			QFile::copy(modPath, strippedPath);
		}
		strippedSizeCounter = QFileInfo(modPath).size() - QFileInfo(strippedPath).size();
	} else {
		QFutureSynchronizer<void> syncer;

		QDirIterator it(modPath, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (it.hasNext()) {
			const QString file = it.next();
			const auto future = QtConcurrent::run([file, &modPath, &resultPath, &modkitFiles, &counter, &strippedCounter, &strippedSizeCounter, &logStream, &lock] {
				QString absolutePath = QFileInfo(file).absolutePath();
				absolutePath.replace(modPath, "");
				QString strippedPath = file;
				strippedPath.replace(modPath, "");
				while (strippedPath.at(0) == '/') {
					strippedPath.remove(0, 1);
				}

				strippedPath = strippedPath.toLower();

				const auto it2 = modkitFiles.find(strippedPath);

				if (it2 == modkitFiles.end()) {
					const QString resultDir = QFileInfo(resultPath + "/" + strippedPath).absolutePath();
					const bool b = QDir(resultPath).mkpath(resultDir);
					Q_UNUSED(b)
					QFile::copy(file, resultPath + "/" + strippedPath);
				} else {
					QString modFileHash;
					bool b = spine::utils::Hashing::hash(file, modFileHash);
					if (!b) {
						std::cout << "Hash Generation not possible for mod file" << std::endl;
						return;
					}

					if (modFileHash != it2.value()) {
						const QString resultDir = resultPath + "/" + absolutePath;
						b = QDir(resultPath).mkpath(resultDir);
						Q_UNUSED(b)
						QFile::copy(file, resultPath + "/" + strippedPath);
					} else {
						QMutexLocker ml(&lock);
						strippedCounter++;
						strippedSizeCounter += QFileInfo(file).size();

						logStream << file << "\n";
					}
				}
				QMutexLocker ml(&lock);
				counter++;
			});
			syncer.addFuture(future);
		}

		syncer.waitForFinished();
	}

	std::cout << "Duration: " << timer.elapsed() << "ms" << std::endl;
	std::cout << "Processed files: " << counter << std::endl;
	std::cout << "Stripped files: " << strippedCounter << std::endl;
	std::cout << "Saved size: " << strippedSizeCounter << std::endl;
	std::cout << "Find the stripped files in ModChecker.log next to the executable" << std::endl;

	return 0;
}
