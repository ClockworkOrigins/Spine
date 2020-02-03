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
// Copyright 2019 Clockwork Origins

#include "Compression.h"

#include <fstream>

#include "utils/Conversion.h"

#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filter/zlib.hpp"

#include <QFileInfo>

namespace spine {
namespace utils {

	bool Compression::compress(const QString & file, bool deleteSourceAfterCompression) {
		return compress(file, file + ".z", deleteSourceAfterCompression);
	}

	bool Compression::compress(const QString & file, const QString & targetFile, bool deleteSourceAfterCompression) {
#ifdef Q_OS_WIN
		const auto sfile = q2ws(file);
#else
		const auto sfile = q2s(file);
#endif

		{
			std::ifstream uncompressedFile(sfile, std::ios_base::in | std::ios_base::binary);
			boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
			in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
			in.push(uncompressedFile);
						
#ifdef Q_OS_WIN
		const auto sTargetFile = q2ws(targetFile);
#else
		const auto sTargetFile = q2s(targetFile);
#endif
			
			std::ofstream compressedFile(sTargetFile, std::ios_base::out | std::ios_base::binary);
			boost::iostreams::copy(in, compressedFile);
		}

		if (deleteSourceAfterCompression) {
			QFile::remove(file);
		}

		return true;
	}

	bool Compression::uncompress(const QString & file, bool deleteSourceAfterUncompression) {
		return uncompress(file, file.mid(0, file.size() - 2), deleteSourceAfterUncompression);
	}

	bool Compression::uncompress(const QString & file, const QString & targetFile, bool deleteSourceAfterUncompression) {
		if (QFileInfo(file).suffix() != "z") return false;

		{
#ifdef Q_OS_WIN
		const auto sfile = q2ws(file);
#else
		const auto sfile = q2s(file);
#endif
			
			std::ifstream compressedFile(sfile, std::ios_base::in | std::ios_base::binary);
			boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
			in.push(boost::iostreams::zlib_decompressor());
			in.push(compressedFile);
						
#ifdef Q_OS_WIN
		const auto sTargetFile = q2ws(targetFile);
#else
		const auto sTargetFile = q2s(targetFile);
#endif
			
			std::ofstream uncompressedFile(sTargetFile, std::ios_base::out | std::ios_base::binary);
			boost::iostreams::copy(in, uncompressedFile);
		}
		
		if (deleteSourceAfterUncompression) {
			QFile::remove(file);
		}

		return true;
	}

} /* namespace utils */
} /* namespace spine */
