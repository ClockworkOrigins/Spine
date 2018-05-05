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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#include <iostream>

#include <QDirIterator>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

int main(int, char ** argv) {
	QStringList xardasTakes;
	QStringList heroTakes;
	QRegularExpression heroRegex("AI_Output[\\s]*\\([^,]+,[^,]+,[^\"]*\"[^\"]+_15_\\d+\"[\\s]*\\); //([^\\\n]+)");
	QRegularExpression xardasRegex(R"(AI_Output[\s]*\([^,]+,[^,]+,[^"]*"[^"]+)" + QString::fromStdString(argv[2]) + "[^\"]+_(\\d+)_\\d+\"[\\s]*\\); //([^\\\n]+)");
	QDirIterator it(argv[1], QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	QSet<QString> files;
	while (it.hasNext()) {
		it.next();
		QString filePath = it.filePath();
		QFile f(filePath);
		if (f.open(QIODevice::ReadOnly)) {
			QTextStream ts(&f);
			while (!ts.atEnd()) {
				QString line = ts.readLine();
				if (line.contains(xardasRegex)) {
					QRegularExpressionMatch match = xardasRegex.match(line);
					QString voiceNumber = match.captured(1);
					const int number = voiceNumber.toInt();
					if (number != 15) {
						xardasTakes.append(match.captured(2));
						if (!files.contains(filePath)) {
							std::cout << filePath.toStdString() << std::endl;
							files.insert(filePath);
						}
					}
				} else if (line.contains(heroRegex)) {
					QRegularExpressionMatch match = heroRegex.match(line);
					heroTakes.append(match.captured(1));
				}
			}
		}
	}
	std::cout << xardasTakes.count() << std::endl;
	int wordCount = 0;
	for (const QString & s : xardasTakes) {
		wordCount += s.split(" ").count();
		//std::cout << s.toStdString() << std::endl;
	}
	std::cout << wordCount << std::endl;
	std::cout << heroTakes.count() << std::endl;
	wordCount = 0;
	for (const QString & s : heroTakes) {
		wordCount += s.split(" ").count();
		//std::cout << s.toStdString() << std::endl;
	}
	std::cout << wordCount << std::endl;
	return 0;
}
