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
// Copyright 2020 Clockwork Origins

#include "utils/Conversion.h"
#include "utils/Hashing.h"

#include <QDirIterator>
#include <QFile>
#include <QTextStream>

#include "zipper/zipper.h"

int main(int argc, char ** argv) {
	if (argc != 3) {
		std::cout << "No path specified" << std::endl;
		return -1;
	}

	QString manifest;

	const QString folder(argv[1]);
	QDir dir(folder);

	{
		QDirIterator it(folder, QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			const QString path = it.filePath().replace(it.path(), "");

			if (path == ".manifest") continue;
			
			QString hash;
			spine::utils::Hashing::hash(it.filePath(), hash);
			manifest.append(QString("%1;%2\n").arg(path).arg(hash));
		}
	}

	{
		QFile manifestFile(folder + "/.manifest");
		manifestFile.open(QIODevice::WriteOnly);

		QTextStream ts(&manifestFile);
		ts << manifest;
	}

	{
		const std::string archiveName = argv[2];
		zipper::Zipper zipper(q2s(folder) + "/" + archiveName);
		
		QDirIterator it(folder, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
		while (it.hasNext()) {
			it.next();
			const QString path = it.filePath().replace(it.path(), "");

			if (path == s2q(archiveName)) continue;
			
			zipper.add(q2s(it.filePath()));
		}
	}
	
	return 0;
}
