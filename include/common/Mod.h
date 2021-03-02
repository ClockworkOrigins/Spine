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
#include <string>

#include "common/GameType.h"
#include "common/Language.h"
#include "common/ModType.h"

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"

namespace spine {
namespace common {

	struct Mod {
		int32_t id = -1;
		std::string name;
		int32_t teamID = -1;
		std::string teamName;
		GameType gothic = GameType::Gothic;
		uint32_t releaseDate = 0;
		ModType type = ModType::TOTALCONVERSION;
		int8_t majorVersion = 0;
		int8_t minorVersion = 0;
		int8_t patchVersion = 0;
		int8_t spineVersion = 0;
		int32_t devDuration = 0;
		int32_t avgDuration = 0;
		uint64_t downloadSize = 0;
		uint32_t updateDate = 0;
		Language language = Language::None;
		uint16_t supportedLanguages = 0;

		Mod() {}

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
			ar & spineVersion;
			ar & devDuration;
			ar & avgDuration;
			ar & downloadSize;
			ar & updateDate;
			ar & language;
			ar & supportedLanguages;
		}
	};

} /* namespace common */
} /* namespace spine */
