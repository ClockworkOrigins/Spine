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

#include "utils/Hashing.h"

#include <QDirIterator>
#include <QFileInfo>

int main(const int argc, char ** argv) {
	if (argc != 4) {
		return 1;
	}
	
	QString modPath(argv[1]);
	QString modkitPath(argv[2]);
	QString resultPath(argv[3]);
	
	modPath.replace("\\", "/");
	modkitPath.replace("\\", "/");
	resultPath.replace("\\", "/");

	int counter = 0;

	QDirIterator it(modPath, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		const QString file = it.next();
		QString absolutePath = QFileInfo(file).absolutePath();
		absolutePath.replace(modPath, "");
		QString strippedPath = file;
		strippedPath.replace(modPath, "");
		while (strippedPath.at(0) == '/') {
			strippedPath.remove(0, 1);
		}

		if (!QFileInfo::exists(modkitPath + "/" + strippedPath)) {
			const QString resultDir = QFileInfo(resultPath + "/" + strippedPath).absolutePath();
			const bool b = QDir(resultPath).mkpath(resultDir);
			Q_UNUSED(b);
			QFile::copy(file, resultPath + "/" + strippedPath);
		} else {
			QString modFileHash;
			bool b = spine::utils::Hashing::hash(file, modFileHash);
			if (!b) {
				std::cout << "Hash Generation not possible for mod file" << std::endl;
				return 1;
			}

			QString modkitFileHash;
			b = spine::utils::Hashing::hash(modkitPath + "/" + strippedPath, modkitFileHash);
			if (!b) {
				std::cout << "Hash Generation not possible for modkit file" << std::endl;
				return 1;
			}
			if (modFileHash != modkitFileHash) {
				const QString resultDir = resultPath + "/" + absolutePath;
				b = QDir(resultPath).mkpath(resultDir);
				Q_UNUSED(b);
				QFile::copy(file, resultPath + "/" + strippedPath);
			}
		}
		counter++;
	}

	std::cout << "Processed files: " << counter << std::endl;

	return 0;
}
