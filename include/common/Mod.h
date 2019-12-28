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

#ifndef __SPINE_COMMON_MOD_H__
#define __SPINE_COMMON_MOD_H__

#include <cstdint>
#include <string>

#include "common/GothicVersion.h"
#include "common/ModType.h"

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"

namespace spine {
namespace common {

	struct Mod {
		int32_t id;
		std::string name;
		int32_t teamID;
		std::string teamName;
		GothicVersion gothic;
		uint32_t releaseDate;
		ModType type;
		int8_t majorVersion;
		int8_t minorVersion;
		int8_t patchVersion;
		int32_t devDuration;
		int32_t avgDuration;
		uint64_t downloadSize;

		Mod() {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & id;
			ar & name;
			ar & teamID;
			ar & teamName;
			ar & gothic;
			ar & releaseDate;
			ar & type;
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
			ar & devDuration;
			ar & avgDuration;
			ar & downloadSize;
		}
	};

} /* namespace common */
} /* namespace spine */

#endif /* __SPINE_COMMON_MOD_H__ */

