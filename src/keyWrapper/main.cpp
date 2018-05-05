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

int main(int, char ** argv) {
	std::ofstream out("RSAKey.h");
	std::ifstream in(argv[1]);
	out << "#ifndef __SPINE_COMMON_RSAKEY_H__\n";
	out << "#define __SPINE_COMMON_RSAKEY_H__\n";
	out << "\n";
	out << "namespace spine {\n";
	out << "namespace common {\n";
	out << "\n";
	out << "\tstatic char RSA_PUB_KEY[] = { ";
	while (in.good()) {
		const int i = in.get();
		std::cout << i << std::endl;
		out << "0x" << std::hex << i;
		if (in.good()) {
			out << ", ";
		}
	}
	out << " };\n";
	out << "\n";
	out << "} /* namespace common */\n";
	out << "} /* namespace spine */\n";
	out << "\n";
	out << "#endif /* __SPINE_COMMON_RSAKEY_H__ */\n";
	return 0;
}
