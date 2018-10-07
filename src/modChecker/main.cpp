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

#include <QCryptographicHash>
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
			QDir(resultPath).mkpath(resultDir);
			QFile::copy(file, resultPath + "/" + strippedPath);
		} else {
			QFile modFile(file);
			modFile.open(QIODevice::ReadOnly);
			QString modFileHash;
			QCryptographicHash hash(QCryptographicHash::Sha512);
			if (hash.addData(&modFile)) {
				 modFileHash = QString::fromLatin1(hash.result().toHex());
			} else {
				std::cout << "Hash Generation not possible for mod file" << std::endl;
				return 1;
			}
			modFile.close();

			QFile modkitFile(modkitPath + "/" + strippedPath);
			modkitFile.open(QIODevice::ReadOnly);
			QString modkitFileHash;
			QCryptographicHash modkitHash(QCryptographicHash::Sha512);
			if (modkitHash.addData(&modkitFile)) {
				 modkitFileHash = QString::fromLatin1(modkitHash.result().toHex());
			} else {
				std::cout << "Hash Generation not possible for modkit file" << std::endl;
				return 1;
			}
			if (modFileHash != modkitFileHash) {
				const QString resultDir = resultPath + "/" + absolutePath;
				QDir(resultPath).mkpath(resultDir);
				QFile::copy(file, resultPath + "/" + strippedPath);
			}
		}
		counter++;
	}

	std::cout << "Processed files: " << counter << std::endl;

	return 0;
}
