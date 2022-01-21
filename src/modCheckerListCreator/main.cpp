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

#include "utils/Hashing.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QTextStream>

int main(const int argc, char ** argv) {
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <g1|g2> <path to modkit base folder (the _work folder)>" << std::endl;
		return 1;
	}

	const QString gothicVersion(argv[1]);
	QString modkitPath(argv[2]);

	if (gothicVersion != "g1" && gothicVersion != "g2") {
		std::cout << "First parameter must be either g1 or g2 depending on the Gothic version your modification is made for." << std::endl;
		return 1;
	}

	QFile f(gothicVersion + ".txt");

	if (!f.open(QIODevice::WriteOnly)) {
		std::cout << "Can't open output file" << std::endl;
		return 1;
	}

	QTextStream ts(&f);

	modkitPath.replace("\\", "/");
	
	QDirIterator it(modkitPath, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		const QString file = it.next();
		QString absolutePath = QFileInfo(file).absolutePath();
		absolutePath.replace(modkitPath, "");
		QString strippedPath = file;
		strippedPath.replace(modkitPath, "");
		while (strippedPath.at(0) == '/') {
			strippedPath.remove(0, 1);
		}

		strippedPath = strippedPath.toLower();

		QString modFileHash;
		const bool b = spine::utils::Hashing::hash(file, modFileHash);
		if (!b) {
			std::cout << "Hash Generation not possible for mod file" << std::endl;
			return 1;
		}

		ts << strippedPath << ";" << modFileHash << "\n";
	}

	return 0;
}
