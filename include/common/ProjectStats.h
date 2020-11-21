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

	struct ProjectStats {
		int32_t projectID = -1;
		std::string name;
		ModType type = ModType::TOTALCONVERSION;
		int32_t lastTimePlayed = 0;
		int32_t duration = 0;
		int32_t achievedAchievements = 0;
		int32_t allAchievements = 0;
		std::string bestScoreName;
		int32_t bestScoreRank = 0;
		int32_t bestScore = 0;
		bool feedbackMailAvailable = false;
		std::string discussionUrl;

		ProjectStats() {}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & projectID;
			ar & name;
			ar & type;
			ar & lastTimePlayed;
			ar & duration;
			ar & achievedAchievements;
			ar & allAchievements;
			ar & bestScoreName;
			ar & bestScoreRank;
			ar & bestScore;
			ar & feedbackMailAvailable;
			ar & discussionUrl;
		}
	};

} /* namespace common */
} /* namespace spine */
