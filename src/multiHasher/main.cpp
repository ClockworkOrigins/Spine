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

#include "utils/Compression.h"
#include "utils/Hashing.h"

#include <QDirIterator>
#include <QFile>

int main(int, char ** argv) {
	// first entry: ModID
	const int modID = std::stoi(argv[1]);
	// second entry: language
	const QString language(argv[2]);
	const QString statement("INSERT INTO modfiles (ModID, Path, Language, Hash) VALUES (%1, '%2', '%3', '%4');\n");
	// third entry: directory to prepare
	QFile outFile(QString(argv[3]) + "/../addToDatabase.txt");
	if (!outFile.open(QIODevice::WriteOnly)) {
		return 1;
	}
	QDirIterator it(argv[3], QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QDir dir(argv[3]);
		const QString path = it.filePath().replace(it.path(), "");
		QString hash;
		const bool b = spine::utils::Hashing::hash(it.filePath(), hash);
		if (b) {
			QString line = statement.arg(modID).arg(path + ".z").arg(language).arg(hash);
			outFile.write(line.toLatin1());
		} else {
			return 1;
		}
		spine::utils::Compression::compress(it.filePath(), true);
	}
	return 0;
}
