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

#include <fstream>

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

int main(int, char ** argv) {
	spine::MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		return 1;
	}
	std::fstream fs;
	fs.open(argv[1], std::fstream::in);
	while (fs.good()) {
		std::string line;
		std::getline(fs, line);
		if (!line.empty()) {
			bool b = database.query(line);
			static_cast<void>(b);
		}
	}
	return 0;
}
