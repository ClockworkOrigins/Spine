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
#include <iostream>

#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filter/zlib.hpp"

#include <QCryptographicHash>
#include <QFileInfo>

int main(const int argc, char ** argv) {
	if (argc != 2) {
		return 1;
	}
	QString fileName(argv[1]);
	if (QFileInfo(fileName).suffix() == "z") {
		{
			std::ifstream compressedFile(fileName.toStdString(), std::ios_base::in | std::ios_base::binary);
			boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
			in.push(boost::iostreams::zlib_decompressor());
			in.push(compressedFile);
			fileName = fileName.mid(0, fileName.size() - 2);
			std::ofstream uncompressedFile(fileName.toStdString(), std::ios_base::out | std::ios_base::binary);
			boost::iostreams::copy(in, uncompressedFile);
		}
	} else {
		QFile f(argv[1]);
		if (f.open(QFile::ReadOnly)) {
			QCryptographicHash hash(QCryptographicHash::Sha512);
			if (hash.addData(&f)) {
				std::cout << QString::fromLatin1(hash.result().toHex()).toStdString() << std::endl;
			} else {
				return 1;
			}
			f.close();
			std::ifstream uncompressedFile(argv[1], std::ios_base::in | std::ios_base::binary);
			boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
			in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
			in.push(uncompressedFile);
			std::ofstream compressedFile(std::string(argv[1]) + ".z", std::ios_base::out | std::ios_base::binary);
			boost::iostreams::copy(in, compressedFile);
		} else {
			return 1;
		}
	}
	return 0;
}
