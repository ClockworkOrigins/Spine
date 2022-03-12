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
// Copyright 2022 Clockwork Origins

#include <iostream>

#include <QDirIterator>
#include <QFileInfo>
#include <QFutureSynchronizer>
#include <QMutex>
#include <QRegularExpression>
#include <QTextStream>
#include <QtConcurrentRun>

int main(const int argc, char ** argv) {
	if (argc != 2 && argc != 3) {
		std::cout << "Usage: " << argv[0] << " <path to your scripts folder> (cleanup)" << std::endl;
		std::cout << "\t\twithout cleanup you will just get a report of the variables that can get deleted" << std::endl;
		return 1;
	}

	const QString scriptsPath(argv[1]);

	const auto cleanup = argc == 3 && QString(argv[2]) == "cleanup";

	QMap<QString, int> variables;

	{
		QDirIterator it(scriptsPath, { "Story_Globals.d" }, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (it.hasNext()) {
			const QString file = it.next();

			QFile f(file);
			if (!f.open(QIODevice::ReadOnly))
				continue;

			QTextStream ts(&f);

			while (!ts.atEnd()) {
				const auto line = ts.readLine();

				QRegularExpression regex("var\\s+int\\s+([^;]+)\\s*;", QRegularExpression::CaseInsensitiveOption);

				const auto match = regex.match(line);

				if (!match.hasMatch())
					continue;

				variables.insert(match.captured(1), 1);
			}
		}
	}

	QMutex lock;

	{
		std::atomic<int> fileCount { 0 };
		QFutureSynchronizer<void> syncer;
		QDirIterator it(scriptsPath, { "*.d" }, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (it.hasNext()) {
			const QString file = it.next();

			if (file.contains("Story_Globals.d", Qt::CaseInsensitive))
				continue;

			++fileCount;

			auto f = QtConcurrent::run([file, &variables, &lock, &fileCount] {
				lock.lock();
				const auto v = variables;
				lock.unlock();

				QFile scriptFile(file);
				if (!scriptFile.open(QIODevice::ReadOnly))
					return;

				QTextStream ts(&scriptFile);

				const auto content = ts.readAll();

				for (auto iterator = v.begin(); iterator != v.end(); ++iterator) {
					if (iterator.value() > 1)
						continue;

					const auto & key = iterator.key();

					QRegularExpression regex("[^a-zA-Z0-9_]+" + key + "[^a-zA-Z0-9_]+", QRegularExpression::CaseInsensitiveOption);

					const auto match = regex.match(content);

					if (!match.hasMatch())
						continue;

					QMutexLocker ml(&lock);
					if (!variables.contains(key))
						continue;

					variables[key]++;

					if (variables[key] == 1)
						continue;

					variables.remove(key);
				}

				--fileCount;

				QMutexLocker ml(&lock);
				std::cout << "Files pending: " << fileCount << std::endl;
				std::cout << "\tVariables: " << variables.count() << std::endl;
			});
			syncer.addFuture(f);
		}

		syncer.waitForFinished();
	}

	{
		QFile f("out.txt");
		if (!f.open(QIODevice::WriteOnly))
			return 1;

		QTextStream ts(&f);

		for (auto it = variables.begin(); it != variables.end(); ++it) {
			ts << it.key() << "\n";
		}
	}

	if (cleanup) {
		QDirIterator it(scriptsPath, { "Story_Globals.d" }, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (it.hasNext()) {
			const QString file = it.next();

			QStringList split;

			{
				QFile f(file);
				if (!f.open(QIODevice::ReadOnly))
					continue;

				QTextStream ts(&f);

				const QString content = ts.readAll();
				split = content.split('\n');

				for (int i = 0; i < split.count(); i++) {
					for (auto iterator = variables.begin(); iterator != variables.end(); ++iterator) {
						QRegularExpression regex("var\\s+int\\s+" + iterator.key() + "\\s*;", QRegularExpression::CaseInsensitiveOption);

						const auto match = regex.match(split[i]);

						if (!match.hasMatch())
							continue;

						split.removeAt(i);
						i--;
					}
				}
			}

			{
				QFile f(file);
				if (!f.open(QIODevice::WriteOnly))
					continue;

				QTextStream ts(&f);

				for (const auto & l : split) {
					ts << l << "\n";
				}
			}
		}
	}

	return 0;
}
