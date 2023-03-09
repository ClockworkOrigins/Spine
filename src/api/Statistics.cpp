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

#include "api/Statistics.h"

#include "clockUtils/sockets/TcpSocket.h"

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

namespace spine {
namespace api {

	bool initializeStatistics() {
		const bool success = true;
		return success;
	}

	void updateStatistic(int32_t identifier, int32_t guild, const char * statName, int32_t statValue) {
		if (initialized && (activatedModules & common::SpineModules::Statistics)) {
			common::UpdateChapterStatsMessage ucsm;
			ucsm.identifier = identifier;
			ucsm.guild = guild;
			ucsm.statName = statName;
			ucsm.statValue = statValue;
			const std::string serialized = ucsm.SerializeBlank();
			if (sock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
				reconnect();
				updateStatistic(identifier, guild, statName, statValue);
				return;
			}
		}
	}

} /* namespace api */
} /* namespace spine */
