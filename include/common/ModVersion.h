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

#pragma once

#include <cstdint>

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"

namespace spine {
namespace common {

	struct ModVersion {
		int32_t modID = -1;
		int8_t majorVersion = 0;
		int8_t minorVersion = 0;
		int8_t patchVersion = 0;
		int8_t spineVersion = 0;
		int language = 0;

		ModVersion() {
		}

		ModVersion(int i1, int i2, int i3, int i4, int i5) : modID(static_cast<int32_t>(i1)), majorVersion(static_cast<int8_t>(i2)), minorVersion(static_cast<int8_t>(i3)), patchVersion(static_cast<int8_t>(i4)), spineVersion(static_cast<int8_t>(i5)) {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & modID;
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
			ar & spineVersion;
			ar & language;
		}
	};

} /* namespace common */
} /* namespace spine */
