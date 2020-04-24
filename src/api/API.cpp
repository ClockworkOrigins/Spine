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

#include "api/API.h"

#include <fstream>
#include <regex>
#include <set>
#include <thread>

#include "SpineConfig.h"

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

#include "api/Friends.h"
#include "api/Gamepad.h"
#include "api/Multiplayer.h"
#include "api/Statistics.h"

#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"

#include "clockUtils/sockets/TcpSocket.h"

namespace spine {
namespace api {
namespace {

	// id + vector of username | score
	std::map<int, std::vector<std::pair<std::string, int>>> scores;
	std::set<int32_t> achievements;
	std::map<int32_t, std::pair<int32_t, int32_t>> achievementProgress;

#ifdef WIN32
	std::wstring overallSaveName;
#else
	std::string overallSaveName;
#endif
	
	std::map<std::string, std::string> overallSaveEntries;
	bool showAchievements = true;
	std::mutex overallSaveMutex;
	bool testMode = true;

	std::map<std::pair<int, int>, bool> otherModsAchievements;

	int userID = -1;

	void saveOverallSave() {
		std::thread([]() {
			std::lock_guard<std::mutex> lg(overallSaveMutex);
			std::ofstream out(overallSaveName, std::ios::binary | std::ios::out);
			boost::archive::binary_oarchive arch(out);
			try {
				arch << overallSaveEntries;
			} catch (...) {
			}
		}).detach();
	}

	const size_t NAMELENGTH = 100;
}

	int32_t activatedModules = 0;
	bool initialized = false;
	std::string username;
	clockUtils::sockets::TcpSocket * sock = nullptr;
	int32_t modID = -1;

	void reconnect() {
		delete sock;
		sock = new clockUtils::sockets::TcpSocket();
		if (sock->connectToIP("127.0.0.1", LOCAL_PORT, 10000) != clockUtils::ClockError::SUCCESS) {
			initialized = false;
		}
	}

	int32_t init(int32_t modules) {
		bool success = true;
		sock = new clockUtils::sockets::TcpSocket();
		if (sock->connectToIP("127.0.0.1", LOCAL_PORT, 10000) == clockUtils::ClockError::SUCCESS) {
			try {
				common::RequestUsernameMessage rum;
				std::string serialized = rum.SerializeBlank();
				if (clockUtils::ClockError::SUCCESS == sock->writePacket(serialized)) {
					if (clockUtils::ClockError::SUCCESS == sock->receivePacket(serialized)) {
						common::Message * msg = common::Message::DeserializeBlank(serialized);
						if (msg) {
							if (msg->type == common::MessageType::SENDUSERNAME) {
								username = dynamic_cast<common::SendUsernameMessage *>(msg)->username;
								modID = dynamic_cast<common::SendUsernameMessage *>(msg)->modID;
								testMode = modID == -1;
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
				if (success && (modules & common::SpineModules::Scores)) {
					common::RequestScoresMessage rsm;
					serialized = rsm.SerializeBlank();
					if (clockUtils::ClockError::SUCCESS == sock->writePacket(serialized)) {
						if (clockUtils::ClockError::SUCCESS == sock->receivePacket(serialized)) {
							common::Message * msg = common::Message::DeserializeBlank(serialized);
							if (msg) {
								if (msg->type == common::MessageType::SENDSCORES) {
									for (auto & p : dynamic_cast<common::SendScoresMessage *>(msg)->scores) {
										scores[p.first] = p.second;
									}
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
				}
				if (success && (modules & common::SpineModules::Achievements)) {
					common::RequestAchievementsMessage ram;
					serialized = ram.SerializeBlank();
					if (clockUtils::ClockError::SUCCESS == sock->writePacket(serialized)) {
						if (clockUtils::ClockError::SUCCESS == sock->receivePacket(serialized)) {
							common::Message * msg = common::Message::DeserializeBlank(serialized);
							if (msg) {
								if (msg->type == common::MessageType::SENDACHIEVEMENTS) {
									common::SendAchievementsMessage * sam = dynamic_cast<common::SendAchievementsMessage *>(msg);
									for (int32_t i : sam->achievements) {
										achievements.insert(i);
									}
									for (const auto & p : sam->achievementProgress) {
										achievementProgress[p.first] = p.second;
									}
									showAchievements = sam->showAchievements;
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
				}
				if (success && (modules & common::SpineModules::OverallSave)) {
					common::RequestOverallSavePathMessage rospm;
					serialized = rospm.SerializeBlank();
					if (clockUtils::ClockError::SUCCESS == sock->writePacket(serialized)) {
						if (clockUtils::ClockError::SUCCESS == sock->receivePacket(serialized)) {
							common::Message * msg = common::Message::DeserializeBlank(serialized);
							if (msg) {
								if (msg->type == common::MessageType::SENDOVERALLSAVEPATH) {
									common::SendOverallSavePathMessage * sospm = dynamic_cast<common::SendOverallSavePathMessage *>(msg);
									overallSaveName = sospm->path;
									std::ifstream in(overallSaveName, std::ios::binary | std::ios::in);
									try {
										boost::archive::binary_iarchive arch(in);
										arch >> overallSaveEntries;
									} catch (...) {
									}
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
					common::RequestOverallSaveDataMessage rosdm;
					serialized = rosdm.SerializeBlank();
					if (clockUtils::ClockError::SUCCESS == sock->writePacket(serialized)) {
						if (clockUtils::ClockError::SUCCESS == sock->receivePacket(serialized)) {
							common::Message * msg = common::Message::DeserializeBlank(serialized);
							if (msg) {
								if (msg->type == common::MessageType::SENDOVERALLSAVEDATA) {
									common::SendOverallSaveDataMessage * sosdm = dynamic_cast<common::SendOverallSaveDataMessage *>(msg);
									for (const auto & p : sosdm->data) {
										overallSaveEntries[p.first] = p.second;
									}
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
				}
				if (success && (modules & common::SpineModules::Gamepad)) {
					initializeGamepad();
				}
				if (success && (modules & common::SpineModules::Friends)) {
					success = initializeFriends();
				}
				if (success && modules & common::SpineModules::Multiplayer) {
					success = initializeMultiplayer();
				}
				if (success && modules & common::SpineModules::Statistics) {
					success = initializeStatistics();
				}
			} catch (boost::archive::archive_exception &) {
				success = false;
			}
		} else {
			success = false;
		}
		initialized = success;
		if (initialized) {
			activatedModules = modules;
		}
		return success;
	}

	void getUsername(char * str) {
		if (initialized && (activatedModules & common::SpineModules::GetCurrentUsername)) {
			strcpy(str, username.c_str());
			for (size_t i = username.size(); i < NAMELENGTH; i++) {
				str[i] = '\0';
			}
		} else {
			for (size_t i = 0; i < NAMELENGTH; i++) {
				str[i] = '\0';
			}
		}
	}

	void updateScore(int32_t id, int32_t score) {
		if (initialized && (activatedModules & common::SpineModules::Scores)) {
			common::UpdateScoreMessage usm;
			usm.identifier = id;
			usm.score = score;
			const std::string serialized = usm.SerializeBlank();
			if (sock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
				reconnect();
				updateScore(id, score);
				return;
			}
			auto vec = scores[id];
			bool found = false;
			for (auto & i : vec) {
				if (i.first == username) {
					found = true;
					i.second = score;
					break;
				}
			}
			if (!found) {
				vec.emplace_back(username, score);
			}
			std::sort(vec.begin(), vec.end(), [](const std::pair<std::string, int32_t> & a, const std::pair<std::string, int32_t> & b) {
				return a.second > b.second;
			});
			scores[id] = vec;
		}
	}

	int32_t getUserScore(int32_t id) {
		if (initialized && (activatedModules & common::SpineModules::Scores)) {
			auto & vec = scores[id];
			for (auto & p : vec) {
				if (p.first == username) {
					return p.second;
				}
			}
		}
		return -1;
	}

	int32_t getUserRank(int32_t id) {
		if (initialized && (activatedModules & common::SpineModules::Scores)) {
			int32_t rank = 0;
			auto & vec = scores[id];
			for (auto & p : vec) {
				rank++;
				if (p.first == username) {
					return rank;
				}
			}
		}
		return -1;
	}

	int32_t getScoreForRank(int32_t id, int32_t rank) {
		if (initialized && (activatedModules & common::SpineModules::Scores)) {
			auto & vec = scores[id];
			if (rank < int(vec.size())) {
				return vec[rank].second;
			}
		}
		return -1;
	}

	void getUsernameForRank(int32_t id, int32_t rank, char * str) {
		if (initialized && (activatedModules & common::SpineModules::Scores)) {
			auto & vec = scores[id];
			if (rank < int(vec.size())) {
				strcpy(str, vec[rank].first.c_str());
				for (size_t i = vec[rank].first.size(); i < NAMELENGTH; i++) {
					str[i] = '\0';
				}
			} else {
				for (size_t i = 0; i < NAMELENGTH; i++) {
					str[i] = '\0';
				}
			}
		} else {
			for (size_t i = 0; i < NAMELENGTH; i++) {
				str[i] = '\0';
			}
		}
	}

	int32_t getScoreForUsername(int32_t id, const char * user) {
		int32_t score = 0;
		if (initialized && (activatedModules & common::SpineModules::Scores)) {
			auto & vec = scores[id];
			for (const auto & p : vec) {
				if (p.first == user) {
					score = p.second;
					break;
				}
			}
		}
		return score;
	}

	void unlockAchievement(int32_t id) {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			if (!isAchievementUnlocked(id)) {
				common::UnlockAchievementMessage uam;
				uam.identifier = id;
				const std::string serialized = uam.SerializeBlank();
				if (sock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
					reconnect();
					unlockAchievement(id);
					return;
				}
			}
		}
		achievements.insert(id);
	}

	int32_t isAchievementUnlocked(int32_t id) {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			return achievements.find(id) != achievements.end();
		} else {
			return 1;
		}
	}

	int32_t updateAchievementProgress(int32_t id, int32_t progress) {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			if (!isAchievementUnlocked(id)) {
				if (progress >= achievementProgress[id].second && !getTestMode()) {
					unlockAchievement(id);
					achievementProgress[id].first = progress;
					return 1;
				} else {
					if (achievementProgress[id].first != progress) {
						common::UpdateAchievementProgressMessage uapm;
						uapm.progress = progress;
						uapm.identifier = id;
						const std::string serialized = uapm.SerializeBlank();
						if (sock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
							reconnect();
							return updateAchievementProgress(id, progress);
						}
						achievementProgress[id].first = progress;
					}
				}
			}
		}
		return 0;
	}

	int32_t getAchievementProgress(int32_t id) {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			return achievementProgress[id].first;
		} else {
			return 0;
		}
	}

	int32_t getAchievementMaxProgress(int32_t id) {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			return achievementProgress[id].second;
		} else {
			return 0;
		}
	}

	int32_t isAchievementOfOtherModUnlocked(int32_t id, int32_t achievementID) {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			const auto p = std::make_pair(id, achievementID);

			const auto it = otherModsAchievements.find(p);

			if (it != otherModsAchievements.end()) {
				return it->second;
			}

			common::IsAchievementUnlockedMessage iaum;
			iaum.modID = id;
			iaum.achievementID = achievementID;
			std::string serialized = iaum.SerializeBlank();
			if (sock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
				reconnect();
				sock->writePacket(serialized);
			}
			if (sock->receivePacket(serialized) != clockUtils::ClockError::SUCCESS) {
				return 0;
			}
			try {
				common::Message * msg = common::Message::DeserializeBlank(serialized);
				if (!msg) {
					return 0;
				}
				common::SendAchievementUnlockedMessage * saum = dynamic_cast<common::SendAchievementUnlockedMessage *>(msg);
				if (!saum) {
					return 0;
				}

				otherModsAchievements.insert(std::make_pair(p, saum->unlocked));

				return saum->unlocked;
			} catch (...) {
				return 0;
			}
		} else {
			return 0;
		}
	}

	void setOverallSaveValue(const char * key, const char * value) {
		if (initialized && (activatedModules & common::SpineModules::OverallSave)) {
			bool save = false;
			std::string keyStr(key);
			keyStr = std::regex_replace(keyStr, std::regex("'"), "&apos;");
			{
				std::lock_guard<std::mutex> lg(overallSaveMutex);
				auto it = overallSaveEntries.find(keyStr);
				if (it == overallSaveEntries.end()) {
					overallSaveEntries[keyStr] = value;
					save = true;
				} else {
					if (it->second != value) {
						it->second = value;
						save = true;
					}
				}
			}
			if (save) {
				common::UpdateOverallSaveDataMessage uom;
				uom.entry = keyStr;
				uom.value = value;
				const std::string serialized = uom.SerializeBlank();
				if (sock->writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
					reconnect();
					sock->writePacket(serialized);
				}
				saveOverallSave();
			}
		}
	}

	void getOverallSaveValue(const char * key, char * value) {
		if (initialized && (activatedModules & common::SpineModules::OverallSave)) {
			std::string keyStr(key);
			keyStr = std::regex_replace(keyStr, std::regex("'"), "&apos;");
			const auto it = overallSaveEntries.find(keyStr);
			if (it != overallSaveEntries.end() && !it->second.empty()) {
				strcpy(value, it->second.c_str());
				for (size_t i = it->second.size(); i < NAMELENGTH; i++) {
					value[i] = '\0';
				}
			} else {
				for (size_t i = 0; i < NAMELENGTH; i++) {
					value[i] = '\0';
				}
			}
		} else {
			for (size_t i = 0; i < NAMELENGTH; i++) {
				value[i] = '\0';
			}
		}
	}

	void setOverallSaveValueInt(const char * key, int value) {
		const std::string v = std::to_string(value);
		setOverallSaveValue(key, v.c_str());
	}

	int getOverallSaveValueInt(const char * key) {
		if (initialized && (activatedModules & common::SpineModules::OverallSave)) {
			std::string keyStr(key);
			keyStr = std::regex_replace(keyStr, std::regex("'"), "&apos;");
			const auto it = overallSaveEntries.find(keyStr);
			if (it != overallSaveEntries.end() && !it->second.empty()) {
				return std::stoi(it->second);
			}
			return -1;
		}
		return -1;
	}

	int32_t getShowAchievements() {
		if (initialized && (activatedModules & common::SpineModules::Achievements)) {
			return showAchievements;
		} else {
			return 1;
		}
	}

	int32_t getTestMode() {
		return testMode ? 1 : 0;
	}

	int32_t getUserID() {
		return userID;
	}

} /* namespace api */
} /* namespace spine */
