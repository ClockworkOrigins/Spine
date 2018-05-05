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

#include <fstream>

#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filter/zlib.hpp"

#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>

int main(int argc, char ** argv) {
	// first entry: ModID
	const int modID = std::stoi(argv[1]);
	// second entry: language
	const QString language(argv[2]);
	QString statement("INSERT INTO modfiles (ModID, Path, Language, Hash) VALUES (%1, '%2', '%3', '%4');\n");
	// third entry: directory to prepare
	QFile outFile(QString(argv[3]) + "/../addToDatabase.txt");
	if (!outFile.open(QIODevice::WriteOnly)) {
		return 1;
	}
	QDirIterator it(argv[3], QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QFile f(it.filePath());
		QDir dir(argv[3]);
		const QString path = it.filePath().replace(it.path(), "");
		if (f.open(QFile::ReadOnly)) {
			QCryptographicHash hash(QCryptographicHash::Sha512);
			if (hash.addData(&f)) {
				QString line = statement.arg(modID).arg(path + ".z").arg(language).arg(QString::fromLatin1(hash.result().toHex()));
				outFile.write(line.toLatin1());
			} else {
				return 1;
			}
			f.close();
			{
				std::ifstream uncompressedFile(it.filePath().toStdString(), std::ios_base::in | std::ios_base::binary);
				boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
				in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
				in.push(uncompressedFile);
				std::ofstream compressedFile(it.filePath().toStdString() + ".z", std::ios_base::out | std::ios_base::binary);
				boost::iostreams::copy(in, compressedFile);
			}
			QFile(it.filePath()).remove();
		} else {
			return 1;
		}
	}
	return 0;
}
