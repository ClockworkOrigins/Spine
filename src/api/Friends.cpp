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

#include "api/Friends.h"

#include "api/APIMessage.h"

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

#include "clockUtils/sockets/TcpSocket.h"

namespace spine {
namespace api {
namespace {
	std::vector<common::Friend> friends;

	constexpr size_t FRIENDNAMELENGTH = 100;
}

	bool initializeFriends() {
		bool success = true;
		const common::RequestAllFriendsMessage rum;
		std::string serialized = rum.SerializeBlank();
		if (clockUtils::ClockError::SUCCESS == sock->writePacket(serialized)) {
			if (clockUtils::ClockError::SUCCESS == sock->receivePacket(serialized)) {
				common::Message * msg = common::Message::DeserializeBlank(serialized);
				if (msg) {
					if (msg->type == common::MessageType::SENDALLFRIENDS) {
						friends = dynamic_cast<common::SendAllFriendsMessage *>(msg)->friends;
					} else {
						success = false;
					}
				} else {
					success = false;
				}
				delete msg;
			} else {
				success = false;
			}
		} else {
			success = false;
		}
		return success;
	}

	int32_t getFriendCount() {
		if (initialized && (activatedModules & common::SpineModules::Friends)) {
			return static_cast<int32_t>(friends.size());
		}
		return 0;
	}

	void getFriendName(int32_t index, char * str) {
		if (initialized && (activatedModules & common::SpineModules::Friends) && index < int32_t(friends.size())) {
			strcpy(str, friends[index].name.c_str());
			for (size_t i = friends[index].name.size(); i < FRIENDNAMELENGTH; i++) {
				str[i] = '\0';
			}
		} else {
			for (size_t i = 0; i < FRIENDNAMELENGTH; i++) {
				str[i] = '\0';
			}
		}
	}

} /* namespace api */
} /* namespace spine */
