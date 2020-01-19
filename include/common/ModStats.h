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

#include "common/ModType.h"

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"

namespace spine {
namespace common {

	struct ModStats {
		int32_t modID;
		std::string name;
		ModType type;
		int32_t lastTimePlayed;
		int32_t duration;
		int32_t achievedAchievements;
		int32_t allAchievements;
		std::string bestScoreName;
		int32_t bestScoreRank;
		int32_t bestScore;

		ModStats() {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & modID;
			ar & name;
			ar & type;
			ar & lastTimePlayed;
			ar & duration;
			ar & achievedAchievements;
			ar & allAchievements;
			ar & bestScoreName;
			ar & bestScoreRank;
			ar & bestScore;
		}
	};

} /* namespace common */
} /* namespace spine */
