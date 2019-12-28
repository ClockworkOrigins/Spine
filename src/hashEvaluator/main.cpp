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

#include "utils/Compression.h"
#include "utils/Hashing.h"

#include <QFileInfo>

int main(const int argc, char ** argv) {
	if (argc != 2) {
		return 1;
	}
	const QString fileName(argv[1]);
	if (QFileInfo(fileName).suffix() == "z") {
		spine::utils::Compression::uncompress(fileName, false);
	} else {
		QString hash;
		const bool b = spine::utils::Hashing::hash(argv[1], hash);
		
		if (!b) return 1;
		
		std::cout << hash.toStdString() << std::endl;

		spine::utils::Compression::compress(fileName, false);
	}
	return 0;
}
