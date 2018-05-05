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

#ifndef __SPINE_COMMON_MODUPDATE_H__
#define __SPINE_COMMON_MODUPDATE_H__

#include <cstdint>
#include <string>

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"

namespace spine {
namespace common {

	struct ModUpdate {
		int32_t modID;
		std::string name;
		int8_t majorVersion;
		int8_t minorVersion;
		int8_t patchVersion;
		std::vector<std::pair<std::string, std::string>> files;
		std::vector<std::pair<int32_t, std::vector<std::pair<std::string, std::string>>>> packageFiles;
		std::string fileserver;

		ModUpdate() {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & modID;
			ar & name;
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
			ar & files;
			ar & packageFiles;
			ar & fileserver;
		}
	};

} /* namespace common */
} /* namespace spine */

#endif /* __SPINE_COMMON_MODUPDATE_H__ */

