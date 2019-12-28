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

#include <QDirIterator>
#include <QRegularExpression>
#include <QTextStream>

int main(int, char ** argv) {
	QRegularExpression regEx(R"(INSTANCE[\s\S]+id[\s]*=[\s]*(\d+)[^\d+][\s\S]+)", QRegularExpression::PatternOption::CaseInsensitiveOption);

	QList<int> idList;

	QDirIterator it(argv[1], QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		it.next();
		const QString filePath = it.filePath();
		QFile f(filePath);
		if (f.open(QIODevice::ReadOnly)) {
			QTextStream ts(&f);
			QString fileContent = ts.readAll();
			if (fileContent.contains(regEx)) {
				QRegularExpressionMatch match = regEx.match(fileContent);
				idList << match.captured(1).toInt();
			}
		}
	}
	qSort(idList);
	const int minimumID = 500;
	int lastID = 0;
	for (int i : idList) {
		if (i > minimumID && i - lastID > 1) {
			std::cout << "Unused ID: " << (lastID + 1) << std::endl;
			break;
		}
		lastID = i;
	}
	return 0;
}
