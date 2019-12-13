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
// Copyright 2019 Clockwork Origins

#include "ManagementServer.h"

#include "MariaDBWrapper.h"
#include "ServerCommon.h"
#include "SpineServerConfig.h"

#include "common/MessageStructs.h"

#define BOOST_SPIRIT_THREADSAFE
#include "boost/filesystem/operations.hpp"
#include "boost/property_tree/json_parser.hpp"

using namespace boost::property_tree;

namespace spine {
namespace server {

	ManagementServer::ManagementServer() : _server(nullptr), _runner(nullptr) {
	}

	ManagementServer::~ManagementServer() {
		delete _server;
		delete _runner;
	}
	
	int ManagementServer::run() {
		_server = new HttpsServer(SSLCHAINPATH, SSLPRIVKEYNPATH);
		_server->config.port = MANAGEMENTSERVER_PORT;
		
		_server->resource["^/getMods"]["POST"] = std::bind(&ManagementServer::getMods, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getAchievements"]["POST"] = std::bind(&ManagementServer::getAchievements, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/updateAchievements"]["POST"] = std::bind(&ManagementServer::updateAchievements, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getGeneralConfiguration"]["POST"] = std::bind(&ManagementServer::getGeneralConfiguration, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/updateGeneralConfiguration"]["POST"] = std::bind(&ManagementServer::updateGeneralConfiguration, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getCustomStatistics"]["POST"] = std::bind(&ManagementServer::getCustomStatistics, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getModFiles"]["POST"] = std::bind(&ManagementServer::getModFiles, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/updateModVersion"]["POST"] = std::bind(&ManagementServer::updateModVersion, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getStatistics"]["POST"] = std::bind(&ManagementServer::getStatistics, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getScores"]["POST"] = std::bind(&ManagementServer::getScores, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/updateScores"]["POST"] = std::bind(&ManagementServer::updateScores, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/getUsers"]["POST"] = std::bind(&ManagementServer::getUsers, this, std::placeholders::_1, std::placeholders::_2);
		_server->resource["^/changeUserAccess"]["POST"] = std::bind(&ManagementServer::changeUserAccess, this, std::placeholders::_1, std::placeholders::_2);

		_runner = new std::thread([this]() {
			_server->start();
		});
		
		return 0;
	}

	void ManagementServer::stop() {
		_server->stop();
		_runner->join();
	}

	void ManagementServer::getMods(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}
			
			const std::string language = pt.get<std::string>("Language");

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
					
				if (!database.query("PREPARE selectTeamsStmt FROM \"SELECT TeamID FROM teammembers WHERE UserID = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectModStmt FROM \"SELECT ModID FROM mods WHERE TeamID = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectModNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM modnames WHERE ModID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramLanguage='" + language + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectTeamsStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto lastResults = database.getResults<std::vector<std::string>>();
				
				ptree modNodes;
				for (const auto & vec : lastResults) {
					if (!database.query("SET @paramTeamID=" + vec[0] + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE selectModStmt USING @paramTeamID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					auto results = database.getResults<std::vector<std::string>>();
					for (auto mod : results) {
						if (!database.query("SET @paramModID=" + mod[0] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE selectModNameStmt USING @paramModID, @paramLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						auto nameResults = database.getResults<std::vector<std::string>>();

						if (nameResults.empty()) continue;

						ptree modNode;
						modNode.put("Name", nameResults[0][0]);
						modNode.put("ID", std::stoi(mod[0]));
						modNodes.push_back(std::make_pair("", modNode));
					}
				}
				
				responseTree.add_child("Mods", modNodes);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectAchievementsStmt FROM \"SELECT Identifier FROM modAchievementList WHERE ModID = ? ORDER BY Identifier ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectAchievementNameStmt FROM \"SELECT CAST(Name AS BINARY), CAST(Language AS BINARY) FROM modAchievementNames WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectAchievementDescriptionStmt FROM \"SELECT CAST(Description AS BINARY), CAST(Language AS BINARY) FROM modAchievementDescriptions WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectAchievementProgressStmt FROM \"SELECT Max FROM modAchievementProgressMax WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectAchievementHiddenStmt FROM \"SELECT ModID FROM modAchievementHidden WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectAchievementIconsStmt FROM \"SELECT LockedIcon, LockedHash, UnlockedIcon, UnlockedHash FROM modAchievementIcons WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectAchievementsStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto results = database.getResults<std::vector<std::string>>();

				ptree achievementNodes;
				for (const auto & achievement : results) {
					ptree achievementNode;
					
					const int currentID = std::stoi(achievement[0]);

					if (!database.query("SET @paramIdentifier=" + std::to_string(currentID) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					
					if (!database.query("EXECUTE selectAchievementNameStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					auto results2 = database.getResults<std::vector<std::string>>();
					if (!results2.empty()) {
						ptree nameNodes;
						for (const auto & vec : results2) {
							ptree nameNode;
							nameNode.put("Text", vec[0]);
							nameNode.put("Language", vec[1]);
							nameNodes.push_back(std::make_pair("", nameNode));
						}
						achievementNode.add_child("Names", nameNodes);
					}
					
					if (!database.query("EXECUTE selectAchievementDescriptionStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					results2 = database.getResults<std::vector<std::string>>();
					if (!results2.empty()) {
						ptree descriptionNodes;
						for (const auto & vec : results2) {
							ptree descriptionNode;
							descriptionNode.put("Text", vec[0]);
							descriptionNode.put("Language", vec[1]);
							descriptionNodes.push_back(std::make_pair("", descriptionNode));
						}
						achievementNode.add_child("Descriptions", descriptionNodes);
					}

					if (!database.query("EXECUTE selectAchievementProgressStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					results2 = database.getResults<std::vector<std::string>>();
					achievementNode.put("MaxProgress", results2.empty() ? 0 : std::stoi(results2[0][0]));

					if (!database.query("EXECUTE selectAchievementHiddenStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					results2 = database.getResults<std::vector<std::string>>();
					achievementNode.put("Hidden", results2.empty());

					if (!database.query("EXECUTE selectAchievementIconsStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					results2 = database.getResults<std::vector<std::string>>();
					if (!results2.empty()) {
						achievementNode.put("LockedImageName", results2[0][0]);
						achievementNode.put("LockedImageHash", results2[0][1]);
						achievementNode.put("UnlockedImageName", results2[0][2]);
						achievementNode.put("UnlockedImageHash", results2[0][3]);
					} else {
						achievementNode.put("LockedImageName", "");
						achievementNode.put("LockedImageHash", "");
						achievementNode.put("UnlockedImageName", "");
						achievementNode.put("UnlockedImageHash", "");
					}
					achievementNodes.push_back(std::make_pair("", achievementNode));
				}
				
				responseTree.add_child("Achievements", achievementNodes);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::updateAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			do {
				CONNECTTODATABASE(__LINE__)

				if (!database.query("PREPARE insertAchievementStmt FROM \"INSERT IGNORE INTO modAchievementList (ModID, Identifier) VALUES (?, ?)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateAchievementNameStmt FROM \"INSERT INTO modAchievementNames (ModID, Identifier, Language, Name) VALUES (?, ?, CONVERT(? USING BINARY), CONVERT(? USING BINARY)) ON DUPLICATE KEY UPDATE Name = CONVERT(? USING BINARY)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateAchievementDescriptionStmt FROM \"INSERT INTO modAchievementDescriptions (ModID, Identifier, Language, Description) VALUES (?, ?, CONVERT(? USING BINARY), CONVERT(? USING BINARY)) ON DUPLICATE KEY UPDATE Description = CONVERT(? USING BINARY)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateAchievementProgressStmt FROM \"INSERT INTO modAchievementProgressMax (ModID, Identifier, Max) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE Max = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE deleteAchievementProgressStmt FROM \"DELETE FROM modAchievementProgressMax WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateAchievementHiddenStmt FROM \"INSERT IGNORE INTO modAchievementHidden (ModID, Identifier) VALUES (?, ?)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE deleteAchievementHiddenStmt FROM \"DELETE FROM modAchievementHidden WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateAchievementIconStmt FROM \"INSERT INTO modAchievementIcons (ModID, Identifier, LockedIcon, LockedHash, UnlockedIcon, UnlockedHash) VALUES (?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE LockedIcon = ?, LockedHash = ?, UnlockedIcon = ?, UnlockedHash = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto achievements = pt.get_child("Achievements");

				int id = 0;
				for (const auto & achievement : achievements) {
					if (!database.query("SET @paramIdentifier=" + std::to_string(id) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE insertAchievementStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					for (const auto & tt : achievement.second.get_child("Names")) {
						if (!database.query("SET @paramLanguage='" + tt.second.get<std::string>("Language") + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramName='" + tt.second.get<std::string>("Text") + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE updateAchievementNameStmt USING @paramModID, @paramIdentifier, @paramLanguage, @paramName, @paramName;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					}
					for (const auto & tt : achievement.second.get_child("Descriptions")) {
						if (!database.query("SET @paramLanguage='" + tt.second.get<std::string>("Language") + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramDescription='" + tt.second.get<std::string>("Text") + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE updateAchievementDescriptionStmt USING @paramModID, @paramIdentifier, @paramLanguage, @paramDescription, @paramDescription;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					}
					const int32_t maxProgress = achievement.second.get<int32_t>("MaxProgress");
					if (maxProgress > 0) {
						if (!database.query("SET @paramProgress=" + std::to_string(maxProgress) + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE updateAchievementProgressStmt USING @paramModID, @paramIdentifier, @paramProgress, @paramProgress;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					} else {
						if (!database.query("EXECUTE deleteAchievementProgressStmt USING @paramModID, @paramIdentifier;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					}
					const bool hidden = achievement.second.get<bool>("Hidden");
					if (hidden) {
						if (!database.query("EXECUTE updateAchievementHiddenStmt USING @paramModID, @paramIdentifier;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					} else {
						if (!database.query("EXECUTE deleteAchievementHiddenStmt USING @paramModID, @paramIdentifier;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					}
					
					const std::string lockedImageName = achievement.second.get<std::string>("LockedImageName");
					const std::string lockedImageHash = achievement.second.get<std::string>("LockedImageHash");
					const std::string unlockedImageName = achievement.second.get<std::string>("UnlockedImageName");
					const std::string unlockedImageHash = achievement.second.get<std::string>("UnlockedImageHash");
					
					if (!lockedImageName.empty() || !unlockedImageName.empty()) {
						if (!database.query("SET @paramLockedIcon='" + lockedImageName + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramLockedHash='" + lockedImageHash + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramUnlockedIcon='" + unlockedImageName + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramUnlockedHash='" + unlockedImageHash + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE updateAchievementIconStmt USING @paramModID, @paramIdentifier, @paramLockedIcon, @paramLockedHash, @paramUnlockedIcon, @paramUnlockedHash, @paramLockedIcon, @paramLockedHash, @paramUnlockedIcon, @paramUnlockedHash;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					}
					
					id++;
				}
			} while (false);

			response->write(code);
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getGeneralConfiguration(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectModStmt FROM \"SELECT Enabled, Gothic, Type, ReleaseDate FROM mods WHERE ModID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectDevDurationStmt FROM \"SELECT Duration FROM devtimes WHERE ModID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectModStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				if (results.empty()) {
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				
				responseTree.put("Enabled", results[0][0] == "1");
				responseTree.put("GothicVersion", std::stoi(results[0][1]));
				responseTree.put("ModType", std::stoi(results[0][2]));
				responseTree.put("ReleaseDate", std::stoi(results[0][3]));
				
				if (!database.query("EXECUTE selectDevDurationStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				responseTree.put("Duration", results.empty() ? 0 : std::stoi(results[0][0]));
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::updateGeneralConfiguration(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t enabled = pt.get<bool>("Enabled") ? 1 : 0;
			const int32_t gothicVersion = pt.get<int32_t>("GothicVersion");
			const int32_t modType = pt.get<int32_t>("ModType");
			const int32_t duration = pt.get<int32_t>("Duration");
			const int32_t releaseDate = pt.get<int32_t>("ReleaseDate");

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			do {
				CONNECTTODATABASE(__LINE__)

				if (!database.query("PREPARE updateStmt FROM \"UPDATE mods SET Enabled = ?, Gothic = ?, Type = ?, ReleaseDate = ? WHERE ModID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateDevDurationStmt FROM \"INSERT INTO devtimes (ModID, Duration) VALUES (?, ?) ON DUPLICATE KEY UPDATE Duration = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramEnabled=" + std::to_string(enabled) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramGothicVersion=" + std::to_string(gothicVersion) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModType=" + std::to_string(modType) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramReleaseDate=" + std::to_string(releaseDate) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramDuration=" + std::to_string(duration) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE updateStmt USING @paramEnabled, @paramGothicVersion, @paramModType, @paramReleaseDate, @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE updateDevDurationStmt USING @paramModID, @paramDuration, @paramDuration;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} while (false);

			response->write(code);
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getCustomStatistics(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectChapterStatsStmt FROM \"SELECT Identifier, Guild, StatName, StatValue FROM chapterStats WHERE ModID = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectChapterStatsStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				if (results.empty()) {
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}

				std::map<std::pair<int32_t, int32_t>, std::vector<std::pair<std::string, int32_t>>> m;

				for (auto vec : results) {
					int32_t identifier = std::stoi(vec[0]);
					int32_t guild = std::stoi(vec[1]);
					const std::string statName = vec[2];
					const int32_t statValue = std::stoi(vec[3]);

					m[std::make_pair(identifier, guild)].push_back(std::make_pair(statName, statValue));
				}

				ptree statsNode;
				for (auto & it : m) {
					ptree statNode;

					statNode.put("ID", it.first.first);
					statNode.put("Guild", it.first.second);

					ptree entriesNode;
					for (auto it2 = it.second.begin(); it2 != it.second.end(); ++it2) {
						ptree entryNode;

						entryNode.put("Name", it2->first);
						entryNode.put("Value", it2->second);
						
						entriesNode.push_back(std::make_pair("", entryNode));
					}
					statNode.add_child("Entries", entriesNode);
					
					statsNode.push_back(std::make_pair("", statNode));
				}
				responseTree.add_child("Stats", statsNode);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getModFiles(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("PREPARE selectModFilesStmt FROM \"SELECT Path, Hash, Language FROM modfiles WHERE ModID = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectVersionStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				if (results.empty()) {
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}

				responseTree.put("VersionMajor", std::stoi(results[0][0]));
				responseTree.put("VersionMinor", std::stoi(results[0][1]));
				responseTree.put("VersionPatch", std::stoi(results[0][2]));
				
				if (!database.query("EXECUTE selectModFilesStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				ptree filesNode;
				for (const auto & vec : results) {
					ptree fileNode;

					fileNode.put("Name", vec[0]);
					fileNode.put("Hash", vec[1]);
					fileNode.put("Language", vec[2]);
					
					filesNode.push_back(std::make_pair("", fileNode));
				}
				responseTree.add_child("Files", filesNode);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::updateModVersion(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t versionMajor = pt.get<int32_t>("VersionMajor");
			const int32_t versionMinor = pt.get<int32_t>("VersionMinor");
			const int32_t versionPatch = pt.get<int32_t>("VersionPatch");

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			do {
				CONNECTTODATABASE(__LINE__)

				if (!database.query("PREPARE updateStmt FROM \"UPDATE mods SET MajorVersion = ?, MinorVersion = ?, PatchVersion = ? WHERE ModID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramVersionMajor=" + std::to_string(versionMajor) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramVersionMinor=" + std::to_string(versionMinor) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramVersionPatch=" + std::to_string(versionPatch) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE updateStmt USING @paramVersionMajor, @paramVersionMinor, @paramVersionPatch, @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} while (false);

			response->write(code);
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getStatistics(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}
			
			const std::string language = pt.get<std::string>("Language");

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectAchievementNamesStmt FROM \"SELECT Identifier, CAST(Name AS BINARY) FROM modAchievementNames WHERE ModID = ? AND Language = ? ORDER BY Identifier ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectOverallDownloadsStmt FROM \"SELECT Counter FROM downloads WHERE ModID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectDownloadsPerVersionStmt FROM \"SELECT Version, Counter FROM downloadsPerVersion WHERE ModID = ? ORDER BY Version ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectPlaytimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID != -1 ORDER BY Duration ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectSessiontimeStmt FROM \"SELECT Duration FROM sessionTimes WHERE ModID = ? ORDER BY Duration ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectAchievementtimeStmt FROM \"SELECT Duration FROM achievementTimes WHERE ModID = ? AND Identifier = ? ORDER BY Duration ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE selectIntervalPlayersStmt FROM \"SELECT COUNT(*) FROM lastPlayTimes WHERE ModID = ? AND UserID != -1 AND Timestamp > ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramLanguage='" + language + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectOverallDownloadsStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				responseTree.put("OverallDownloads", results.empty() ? 0 : std::stoi(results[0][0]));
				
				if (!database.query("EXECUTE selectDownloadsPerVersionStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				ptree downloadsPerVersionNodes;
				for (const auto & vec : results) {
					ptree downloadsPerVersionNode;

					downloadsPerVersionNode.put("Version", vec[0]);
					downloadsPerVersionNode.put("Count", std::stoi(vec[1]));
					
					downloadsPerVersionNodes.push_back(std::make_pair("", downloadsPerVersionNode));
				}
				responseTree.add_child("DownloadsPerVersion", downloadsPerVersionNodes);
				
				if (!database.query("EXECUTE selectPlaytimeStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				uint32_t min = std::numeric_limits<uint32_t>::max();
				uint32_t max = 0;
				uint64_t count = 0;
				uint32_t medianPlaytime = 0;
				for (size_t i = 0; i < results.size(); i++) {
					const auto vec = results[i];
					const uint32_t current = static_cast<uint32_t>(std::stoi(vec[0]));
					if (current < min) {
						min = current;
					}
					if (current > max) {
						max = current;
					}
					count += current;
					if (i == results.size() / 2) {
						medianPlaytime = current;
					}
				}
				ptree playTimeNode;
				{
					playTimeNode.put("Minimum", min);
					playTimeNode.put("Maximum", max);
					playTimeNode.put("Median", medianPlaytime);
					playTimeNode.put("Average", results.empty() ? 0 : static_cast<int32_t>(count / results.size()));
				}
				responseTree.add_child("PlayTime", playTimeNode);
				responseTree.put("OverallPlayerCount", results.size());
				
				if (!database.query("EXECUTE selectSessiontimeStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				min = std::numeric_limits<uint32_t>::max();
				max = 0;
				count = 0;
				medianPlaytime = 0;
				for (size_t i = 0; i < results.size(); i++) {
					const auto vec = results[i];
					const uint32_t current = static_cast<uint32_t>(std::stoi(vec[0]));
					if (current < min) {
						min = current;
					}
					if (current > max) {
						max = current;
					}
					count += current;
					if (i == results.size() / 2) {
						medianPlaytime = current;
					}
				}
				ptree sessionTimeNode;
				{
					playTimeNode.put("Minimum", min);
					playTimeNode.put("Maximum", max);
					playTimeNode.put("Median", medianPlaytime);
					playTimeNode.put("Average", results.empty() ? 0 : static_cast<int32_t>(count / results.size()));
				}
				responseTree.add_child("SessionTime", sessionTimeNode);

				const int timestamp = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
				if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp - 24) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectIntervalPlayersStmt USING @paramModID, @paramTimestamp;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();
				responseTree.put("Last24HoursPlayerCount", static_cast<uint32_t>(std::stoi(results[0][0])));

				if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp - 7 * 24) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectIntervalPlayersStmt USING @paramModID, @paramTimestamp;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();
				responseTree.put("Last7DaysPlayerCount", static_cast<uint32_t>(std::stoi(results[0][0])));

				if (!database.query("EXECUTE selectAchievementNamesStmt USING @paramModID, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				ptree achievementStatisticNodes;
				for (const auto & vec : results) {
					const std::string id = vec[0];
					const std::string name = vec[1];

					ptree achievementStatisticNode;

					achievementStatisticNode.put("Name", name);

					if (!database.query("SET @paramIdentifier=" + id + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE selectAchievementtimeStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					const auto results2 = database.getResults<std::vector<std::string>>();

					min = std::numeric_limits<uint32_t>::max();
					max = 0;
					count = 0;
					medianPlaytime = 0;
					for (size_t i = 0; i < results2.size(); i++) {
						const auto vec2 = results2[i];
						const uint32_t current = static_cast<uint32_t>(std::stoi(vec2[0]));
						if (current < min) {
							min = current;
						}
						if (current > max) {
							max = current;
						}
						count += current;
						if (i == results2.size() / 2) {
							medianPlaytime = current;
						}
					}
					achievementStatisticNode.put("Minimum", min);
					achievementStatisticNode.put("Maximum", max);
					achievementStatisticNode.put("Median", medianPlaytime);
					achievementStatisticNode.put("Average", results2.empty() ? 0 : static_cast<int32_t>(count / results2.size()));
					
					achievementStatisticNodes.push_back(std::make_pair("", achievementStatisticNode));
				}
				responseTree.add_child("Achievements", achievementStatisticNodes);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getScores(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}
			
			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectScoreNamesStmt FROM \"SELECT Identifier, CAST(Name AS BINARY), CAST(Language AS BINARY) FROM modScoreNames WHERE ModID = ? ORDER BY Identifier ASC\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectScoreNamesStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				std::map<int32_t, std::vector<std::pair<std::string, std::string>>> map;

				for (const auto & vec : results) {
					const int32_t id = std::stoi(vec[0]);
					const std::string name = vec[1];
					const std::string language = vec[2];

					map[id].push_back(std::make_pair(name, language));
				}

				ptree scoreNodes;
				for (auto & it : map) {
					ptree scoreNode;

					ptree nameNodes;
					for (auto & it2 : it.second) {
						ptree nameNode;
						
						nameNode.put("Language", it2.second);
						nameNode.put("Text", it2.first);
						
						nameNodes.push_back(std::make_pair("", nameNode));
					}
					scoreNode.add_child("Names", nameNodes);
					
					scoreNodes.push_back(std::make_pair("", scoreNode));
				}
				responseTree.add_child("Scores", scoreNodes);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::updateScores(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			do {
				CONNECTTODATABASE(__LINE__)
					
				if (!database.query("PREPARE insertScoreStmt FROM \"INSERT IGNORE INTO modScoreList (ModID, Identifier) VALUES (?, ?)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE updateScoreNameStmt FROM \"INSERT INTO modScoreNames (ModID, Identifier, Language, Name) VALUES (?, ?, CONVERT(? USING BINARY), CONVERT(? USING BINARY)) ON DUPLICATE KEY UPDATE Name = CONVERT(? USING BINARY)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto scores = pt.get_child("Scores");

				int id = 0;
				for (const auto & score : scores) {
					if (!database.query("SET @paramIdentifier=" + std::to_string(id) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE insertScoreStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					for (const auto & tt : score.second.get_child("Names")) {
						if (!database.query("SET @paramLanguage='" + tt.second.get<std::string>("Language") + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramName='" + tt.second.get<std::string>("Text") + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE updateScoreNameStmt USING @paramModID, @paramIdentifier, @paramLanguage, @paramName, @paramName;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
					}
					id++;
				}
			} while (false);

			response->write(code);
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::getUsers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}
			
			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			std::stringstream responseStream;
			ptree responseTree;

			do {
				CONNECTTODATABASE(__LINE__)
				
				if (!database.query("PREPARE selectEarlyAccessorsStmt FROM \"SELECT UserID FROM earlyUnlocks WHERE ModID = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectEarlyAccessorsStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				ptree unlockedUserNodes;
				for (const auto & vec : results) {
					const int32_t id = std::stoi(vec[0]);
					const std::string name = ServerCommon::getUsername(id);
					ptree userNode;
					userNode.put("Name", name);
					unlockedUserNodes.push_back(std::make_pair("", userNode));
				}
				responseTree.add_child("UnlockedUsers", unlockedUserNodes);

				const auto userList = ServerCommon::getUserList();
				ptree userNodes;
				for (const auto & name : userList) {
					ptree userNode;
					userNode.put("Name", name);
					userNodes.push_back(std::make_pair("", userNode));
				}
				responseTree.add_child("Users", userNodes);
			} while (false);

			write_json(responseStream, responseTree);

			response->write(code, responseStream.str());
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::changeUserAccess(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
		try {
			const std::string content = ServerCommon::convertString(request->content.string());

			std::stringstream ss(content);

			ptree pt;
			read_json(ss, pt);
		
			const std::string username = pt.get<std::string>("Username");
			const std::string password = pt.get<std::string>("Password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const int32_t modID = pt.get<int32_t>("ModID");

			if (!hasAdminAccessToMod(userID, modID)) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			const std::string user = pt.get<std::string>("User");
			const bool enabled = pt.get<bool>("Enabled");

			const int accessUserID = ServerCommon::getUserID(user);

			if (accessUserID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}

			SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

			do {
				CONNECTTODATABASE(__LINE__)
					
				if (!database.query("PREPARE insertStmt FROM \"INSERT INTO earlyUnlocks (ModID, UserID) VALUES (?, ?)\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM earlyUnlocks WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramUserID=" + std::to_string(accessUserID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (enabled) {
					if (!database.query("EXECUTE insertStmt USING @paramModID, @paramUserID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				} else {
					if (!database.query("EXECUTE deleteStmt USING @paramModID, @paramUserID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				}
			} while (false);

			response->write(code);
		} catch (...) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
		}
	}

	void ManagementServer::uploadAchievementIcons(common::UploadAchievementIconsMessage * msg) const {
		boost::filesystem::create_directories("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(msg->modID) + "/achievements");
		
		for (const auto & icon : msg->icons) {
			if (icon.data.empty()) continue; // shouldn't be possible, but we want to be on the save side
			
			std::ofstream out;
			out.open("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(msg->modID) + "/achievements/" + icon.name, std::ios::out | std::ios::binary);
			out.write(reinterpret_cast<const char *>(&icon.data[0]), icon.data.size());
			out.close();
		}
	}

	bool ManagementServer::hasAdminAccessToMod(int userID, int modID) const {
		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE selectModStmt FROM \"SELECT TeamID FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectMemberStmt FROM \"SELECT UserID FROM teammembers WHERE TeamID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectModStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			if (results.empty()) break;
			
			if (!database.query("SET @paramTeamID=" + results[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			
			if (!database.query("EXECUTE selectMemberStmt USING @paramTeamID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			
			return !results.empty();
		} while (false);
		return false;
	}

} /* namespace server */
} /* namespace spine */
