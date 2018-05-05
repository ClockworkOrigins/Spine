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

#include "api/Multiplayer.h"

#include <thread>

#include "SpineConfig.h"

#include "api/APIMessage.h"

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

#include "clockUtils/container/LockFreeQueue.h"
#include "clockUtils/sockets/TcpSocket.h"

namespace spine {
namespace api {
namespace {
	static bool inMatch = false;
	static bool lookingForMatch = false;
	static std::vector<std::string> mpUsernames;
	static clockUtils::container::LockFreeQueue<APIMessage *, 100> queue;
	static clockUtils::sockets::TcpSocket * mpSock = nullptr;
	static std::string usedHostname = "clockwork-origins.de";

	const size_t NAMELENGTH = 100;

	bool onlineAndLoggedIn = false;
}

	bool initializeMultiplayer() {
		clockUtils::sockets::TcpSocket tmpSock;
		onlineAndLoggedIn = !username.empty() && tmpSock.connectToHostname("clockwork-origins.de", SPINE_MP_PORT, 5000) == clockUtils::ClockError::SUCCESS;
		return true;
	}

	void setHostname(const char * hostname) {
		usedHostname = hostname;
	}

	void searchMatch(int32_t numPlayers, int32_t identifier) {
		if (initialized && (activatedModules & common::SpineModules::Multiplayer) && onlineAndLoggedIn) {
			if (lookingForMatch) {
				return;
			}
			mpSock = new clockUtils::sockets::TcpSocket();
			std::thread([numPlayers, identifier]() {
				if (clockUtils::ClockError::SUCCESS == mpSock->connectToHostname(usedHostname, SPINE_MP_PORT, 10000)) {
					lookingForMatch = true;
					inMatch = false;
					common::SearchMatchMessage smm;
					smm.numPlayers = numPlayers;
					smm.identifier = identifier;
					smm.modID = modID;
					smm.username = username;
					std::string serialized = smm.SerializeBlank();
					if (!mpSock || mpSock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
						lookingForMatch = false;
						searchMatch(numPlayers, identifier);
						return;
					}
					if (lookingForMatch && mpSock && mpSock->receivePacket(serialized) == clockUtils::ClockError::SUCCESS) {
						try {
							common::Message * msg = common::Message::DeserializeBlank(serialized);
							if (msg) {
								common::FoundMatchMessage * fmm = dynamic_cast<common::FoundMatchMessage *>(msg);
								if (fmm) {
									mpUsernames = fmm->users;
									inMatch = true;
								}
							}
							delete msg;
						} catch (...) {
						}
					}
					lookingForMatch = false;
					while (mpSock && mpSock->receivePacket(serialized) == clockUtils::ClockError::SUCCESS) {
						try {
							APIMessage * msg = APIMessage::deserialize(serialized);
							if (msg) {
								queue.push(msg);
							}
						} catch (...) {
							break;
						}
					}
				}
			}).detach();
		}
	}

	void searchMatchWithFriend(int32_t identifier, const char * friendName) {
		if (initialized && (activatedModules & common::SpineModules::Multiplayer) && onlineAndLoggedIn) {
			if (lookingForMatch) {
				return;
			}
			std::string friendNameStr = friendName;
			mpSock = new clockUtils::sockets::TcpSocket();
			std::thread([identifier, friendNameStr]() {
				if (clockUtils::ClockError::SUCCESS == mpSock->connectToHostname(usedHostname, SPINE_MP_PORT, 10000)) {
					lookingForMatch = true;
					inMatch = false;
					common::SearchMatchMessage smm;
					smm.numPlayers = 2;
					smm.identifier = identifier;
					smm.friendName = friendNameStr;
					smm.modID = modID;
					smm.username = username;
					std::string serialized = smm.SerializeBlank();
					if (mpSock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
						lookingForMatch = false;
						searchMatchWithFriend(identifier, friendNameStr.c_str());
						return;
					}
					if (lookingForMatch && mpSock && mpSock->receivePacket(serialized) == clockUtils::ClockError::SUCCESS) {
						try {
							common::Message * msg = common::Message::DeserializeBlank(serialized);
							if (msg) {
								common::FoundMatchMessage * fmm = dynamic_cast<common::FoundMatchMessage *>(msg);
								if (fmm) {
									mpUsernames = fmm->users;
									inMatch = true;
								}
							}
							delete msg;
						} catch (...) {
						}
					}
					lookingForMatch = false;
					while (mpSock && mpSock->receivePacket(serialized) == clockUtils::ClockError::SUCCESS) {
						try {
							APIMessage * msg = APIMessage::deserialize(serialized);
							if (msg) {
								queue.push(msg);
							}
						} catch (...) {
							break;
						}
					}
				}
			}).detach();
		}
	}

	void stopSearchMatch() {
		if (initialized && (activatedModules & common::SpineModules::Multiplayer)) {
			if (!lookingForMatch) {
				return;
			}
			lookingForMatch = false;
			std::thread([]() {
				delete mpSock;
				mpSock = nullptr;
			}).detach();
		}
	}

	int32_t isInMatch() {
		return initialized && (activatedModules & common::SpineModules::Multiplayer) && inMatch;
	}

	int32_t getPlayerCount() {
		if (initialized && (activatedModules & common::SpineModules::Multiplayer)) {
			return int32_t(mpUsernames.size());
		} else {
			return 0;
		}
	}

	void getPlayerUsername(int32_t player, char * str) {
		if (initialized && player < int32_t(mpUsernames.size()) && (activatedModules & common::SpineModules::Multiplayer)) {
			strcpy(str, mpUsernames[player].c_str());
			str[mpUsernames[player].size()] = '\0';
			for (size_t i = mpUsernames[player].size(); i < NAMELENGTH; i++) {
				str[i] = '\0';
			}
		} else {
			for (size_t i = 0; i < NAMELENGTH; i++) {
				str[i] = '\0';
			}
		}
	}

	APIMessage * createMessage(int32_t messageType) {
		APIMessage * msg = nullptr;
		switch (messageType) {
		case APIMessageType::BASE: {
			msg = new APIMessage();
			break;
		}
		case APIMessageType::INT: {
			msg = new APIMessage_I();
			break;
		}
		case APIMessageType::STRING: {
			msg = new APIMessage_S();
			break;
		}
		case APIMessageType::INT4: {
			msg = new APIMessage_I4();
			break;
		}
		case APIMessageType::INT3: {
			msg = new APIMessage_I3();
			break;
		}
		default: {
			break;
		}
		}
		return msg;
	}

	void deleteMessage(APIMessage * message) {
		delete message;
	}

	void sendMessage(APIMessage * message) {
		if (initialized && message && mpSock) {
			try {
				message->username = username;
				const std::string serialized = message->serialize();
				mpSock->writePacket(serialized);
			} catch (...) {
			}
			delete message;
		}
	}

	APIMessage * receiveMessage() {
		if (!queue.empty()) {
			APIMessage * msg;
			if (clockUtils::ClockError::SUCCESS == queue.poll(msg)) {
				return msg;
			} else {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	}

	int32_t isOnline() {
		return onlineAndLoggedIn ? 1 : 0;
	}

} /* namespace api */
} /* namespace spine */
