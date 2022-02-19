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
#include <QFileInfo>
#include <QFutureSynchronizer>
#include <QMap>
#include <QMutex>
#include <QTextStream>
#include <QtConcurrentRun>

int main(const int argc, char ** argv) {
	if (argc != 4 && argc != 3) {
		std::cout << "Usage: " << argv[0] << " <g1|g2> <path to your mod files (_work folder) or a vdf/mod file> (<path for results>)" << std::endl;
		std::cout << "\t\twithout path for results it will replace the input file with the optimized one" << std::endl;
		return 1;
	}

	const QString gothicVersion(argv[1]);
	QString modPath(argv[2]);
	QString resultPath(argc == 4 ? argv[3] : argv[2]);

	QMap<QString, QString> modkitFiles;

	if (gothicVersion == "g1") {
		modkitFiles = spine::utils::GothicVdf::parseResource(":/g1.txt");
	} else if (gothicVersion == "g2") {
		modkitFiles = spine::utils::GothicVdf::parseResource(":/g2.txt");
	} else {
		std::cout << "First parameter must be either g1 or g2 depending on the Gothic version your modification is made for." << std::endl;
		return 1;
	}
	
	modPath.replace("\\", "/");
	resultPath.replace("\\", "/");

	quint64 counter = 0;
	quint64 strippedCounter = 0;
	quint64 strippedSizeCounter = 0;

	QMutex lock;

	QFile logFile("ModChecker.log");
	if (!logFile.open(QIODevice::WriteOnly)) {
		std::cout << "Couldn't open log file" << std::endl;
	}
	QTextStream logStream(&logFile);

	QElapsedTimer timer;
	timer.start();

	if (modPath.endsWith("vdf", Qt::CaseInsensitive) || modPath.endsWith("mod", Qt::CaseInsensitive)) {
		const auto result = modPath == resultPath ? spine::utils::GothicVdf::optimize(modPath, gothicVersion == "g1" ? ":/g1.txt" : ":/g2.txt") : spine::utils::GothicVdf::optimize(modPath, resultPath, gothicVersion == "g1" ? ":/g1.txt" : ":/g2.txt");

		if (result.status == spine::utils::GothicVdf::Result::Status::Fail) {
			std::cout << "File couldn't be parsed" << std::endl;
			return 1;
		}

		counter = result.fileCount;
		strippedCounter = result.strippedFileCount;
		logStream << result.log;
		strippedSizeCounter = result.strippedSize;
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
