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
// Copyright 2019 Clockwork Origins

#include "ManagementServer.h"

#include <set>

#include "DownloadSizeChecker.h"
#include "FileSynchronizer.h"
#include "LanguageConverter.h"
#include "MariaDBWrapper.h"
#include "ServerCommon.h"
#include "SpineServerConfig.h"

#include "common/MessageStructs.h"
#include "common/NewsTickerTypes.h"
#include "common/ProjectPrivileges.h"
#include "common/ScoreOrder.h"

#define BOOST_SPIRIT_THREADSAFE
#include "boost/filesystem/operations.hpp"
#include "boost/property_tree/json_parser.hpp"

#ifdef WIN32
#pragma warning(disable: 4996 4457 4267 4456 4101)
#endif
#include "simple-web-server/client_https.hpp"
#ifdef WIN32
#pragma warning(default: 4996 4457 4267 4456 4101)
#endif

using namespace boost::property_tree;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

using namespace spine::common;
using namespace spine::server;

ManagementServer::ManagementServer() : _server(nullptr), _runner(nullptr) {
}

ManagementServer::~ManagementServer() {
	delete _server;
	delete _runner;
}

int ManagementServer::run() {
	_server = new HttpsServer(SSLCHAINPATH, SSLPRIVKEYNPATH);
	_server->config.port = MANAGEMENTSERVER_PORT;
	_server->config.thread_pool_size = 4;
	
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
	_server->resource["^/createPlayTestSurvey"]["POST"] = std::bind(&ManagementServer::createPlayTestSurvey, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/enablePlayTestSurvey"]["POST"] = std::bind(&ManagementServer::enablePlayTestSurvey, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updatePlayTestSurvey"]["POST"] = std::bind(&ManagementServer::updatePlayTestSurvey, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getPlayTestSurvey"]["POST"] = std::bind(&ManagementServer::getPlayTestSurvey, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getPlayTestSurveys"]["POST"] = std::bind(&ManagementServer::getPlayTestSurveys, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/submitPlayTestSurveyAnswers"]["POST"] = std::bind(&ManagementServer::submitPlayTestSurveyAnswers, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getOwnPlayTestSurveyAnswers"]["POST"] = std::bind(&ManagementServer::getOwnPlayTestSurveyAnswers, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getAllPlayTestSurveyAnswers"]["POST"] = std::bind(&ManagementServer::getAllPlayTestSurveyAnswers, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/deletePlayTestSurvey"]["POST"] = std::bind(&ManagementServer::deletePlayTestSurvey, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/clearAchievementProgress"]["POST"] = std::bind(&ManagementServer::clearAchievementProgress, this, std::placeholders::_1, std::placeholders::_2);

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
	
		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}
		
		const auto language = pt.get<std::string>("Language");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE selectTeamsStmt FROM \"SELECT TeamID FROM teammembers WHERE UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAllTeamsStmt FROM \"SELECT TeamID FROM teams\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectModStmt FROM \"SELECT ModID FROM mods WHERE TeamID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAdminModsStmt FROM \"SELECT ProjectID FROM projectPrivileges WHERE UserID = ? AND Privileges & " + std::to_string(static_cast<int>(ProjectPrivilege::Manage)) + "\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectPackagesStmt FROM \"SELECT PackageID FROM optionalpackages WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (userID == 3) {
				if (!database.query("EXECUTE selectAllTeamsStmt;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} else {
				if (!database.query("EXECUTE selectTeamsStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			}
			auto lastResults = database.getResults<std::vector<std::string>>();

			std::set<int> addedMods;
			
			ptree modNodes;
			for (const auto & vec : lastResults) {
				if (!database.query("SET @paramTeamID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectModStmt USING @paramTeamID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();
				for (const auto & mod : results) {
					ptree modNode;

					const auto projectID = std::stoi(mod[0]);

					if (!addAccessibleMod(projectID, language, database, modNode))
						continue;

					addedMods.insert(projectID);

					modNodes.push_back(std::make_pair("", modNode));
				}
			}

			if (!database.query("EXECUTE selectAdminModsStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			for (const auto & vec : lastResults) {
				ptree modNode;

				const auto projectID = std::stoi(vec[0]);

				if (addedMods.count(projectID) > 0)
					continue;

				if (!addAccessibleMod(projectID, language, database, modNode))
					continue;

				addedMods.insert(projectID);

				modNodes.push_back(std::make_pair("", modNode));
			}
			
			responseTree.add_child("Mods", modNodes);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

bool ManagementServer::addAccessibleMod(int projectID, const std::string& language, MariaDBWrapper & database, ptree & modNode) const {
	const auto projectName = ServerCommon::getProjectName(projectID, LanguageConverter::convert(language));

	if (projectName.empty())
		return false;

	modNode.put("Name", projectName);
	modNode.put("ID", projectID);

	if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
		return false;
	}
	if (!database.query("EXECUTE selectPackagesStmt USING @paramModID;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
		return false;
	}

	const auto packageResults = database.getResults<std::vector<std::string>>();

	ptree packageNodes;

	for (const auto& vec2 : packageResults) {
		const auto packageName = ServerCommon::getPackageName(std::stoi(vec2[0]), LanguageConverter::convert(language));

		if (packageName.empty())
			continue;

		ptree packageNode;
		packageNode.put("Name", packageName);
		packageNode.put("ID", std::stoi(vec2[0]));

		packageNodes.push_back(std::make_pair("", packageNode));
	}

	if (!packageNodes.empty())
		modNode.add_child("Packages", packageNodes);

	return true;
}

void ManagementServer::getAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		const auto modID = pt.get<int32_t>("ModID");

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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAchievementNameStmt FROM \"SELECT CAST(Name AS BINARY), CAST(Language AS BINARY) FROM modAchievementNames WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAchievementDescriptionStmt FROM \"SELECT CAST(Description AS BINARY), CAST(Language AS BINARY) FROM modAchievementDescriptions WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAchievementProgressStmt FROM \"SELECT Max FROM modAchievementProgressMax WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAchievementHiddenStmt FROM \"SELECT ModID FROM modAchievementHidden WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAchievementIconsStmt FROM \"SELECT LockedIcon, LockedHash, UnlockedIcon, UnlockedHash FROM modAchievementIcons WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectEnabledStmt FROM \"SELECT Enabled FROM mods WHERE ModID = ? AND Enabled = 0 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectAchievementsStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			ptree achievementNodes;
			for (const auto & achievement : results) {
				ptree achievementNode;
				
				const int currentID = std::stoi(achievement[0]);

				if (!database.query("SET @paramIdentifier=" + std::to_string(currentID) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				
				if (!database.query("EXECUTE selectAchievementNameStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results2 = database.getResults<std::vector<std::string>>();
				achievementNode.put("MaxProgress", results2.empty() ? 0 : std::stoi(results2[0][0]));

				if (!database.query("EXECUTE selectAchievementHiddenStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results2 = database.getResults<std::vector<std::string>>();
				achievementNode.put("Hidden", !results2.empty());

				if (!database.query("EXECUTE selectAchievementIconsStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
			
			if (!database.query("EXECUTE selectEnabledStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			if (results.empty()) break;

			responseTree.put("Clearable", 1);
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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateAchievementNameStmt FROM \"INSERT INTO modAchievementNames (ModID, Identifier, Language, Name) VALUES (?, ?, CONVERT(? USING BINARY), CONVERT(? USING BINARY)) ON DUPLICATE KEY UPDATE Name = CONVERT(? USING BINARY)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateAchievementDescriptionStmt FROM \"INSERT INTO modAchievementDescriptions (ModID, Identifier, Language, Description) VALUES (?, ?, CONVERT(? USING BINARY), CONVERT(? USING BINARY)) ON DUPLICATE KEY UPDATE Description = CONVERT(? USING BINARY)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateAchievementProgressStmt FROM \"INSERT INTO modAchievementProgressMax (ModID, Identifier, Max) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE Max = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteAchievementProgressStmt FROM \"DELETE FROM modAchievementProgressMax WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateAchievementHiddenStmt FROM \"INSERT IGNORE INTO modAchievementHidden (ModID, Identifier) VALUES (?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteAchievementHiddenStmt FROM \"DELETE FROM modAchievementHidden WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateAchievementIconStmt FROM \"INSERT INTO modAchievementIcons (ModID, Identifier, LockedIcon, LockedHash, UnlockedIcon, UnlockedHash) VALUES (?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE LockedIcon = ?, LockedHash = ?, UnlockedIcon = ?, UnlockedHash = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto achievements = pt.get_child("Achievements");

			int id = 0;
			for (const auto & achievement : achievements) {
				if (!database.query("SET @paramIdentifier=" + std::to_string(id) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE insertAchievementStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				for (const auto & tt : achievement.second.get_child("Names")) {
					if (!database.query("SET @paramLanguage='" + tt.second.get<std::string>("Language") + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramName='" + tt.second.get<std::string>("Text") + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE updateAchievementNameStmt USING @paramModID, @paramIdentifier, @paramLanguage, @paramName, @paramName;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				}
				for (const auto & tt : achievement.second.get_child("Descriptions")) {
					if (!database.query("SET @paramLanguage='" + tt.second.get<std::string>("Language") + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramDescription='" + tt.second.get<std::string>("Text") + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE updateAchievementDescriptionStmt USING @paramModID, @paramIdentifier, @paramLanguage, @paramDescription, @paramDescription;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				}
				const int32_t maxProgress = achievement.second.get<int32_t>("MaxProgress");
				if (maxProgress > 0) {
					if (!database.query("SET @paramProgress=" + std::to_string(maxProgress) + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE updateAchievementProgressStmt USING @paramModID, @paramIdentifier, @paramProgress, @paramProgress;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				} else {
					if (!database.query("EXECUTE deleteAchievementProgressStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				}
				const bool hidden = achievement.second.get<bool>("Hidden");
				if (hidden) {
					if (!database.query("EXECUTE updateAchievementHiddenStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				} else {
					if (!database.query("EXECUTE deleteAchievementHiddenStmt USING @paramModID, @paramIdentifier;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramLockedHash='" + lockedImageHash + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramUnlockedIcon='" + unlockedImageName + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramUnlockedHash='" + unlockedImageHash + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE updateAchievementIconStmt USING @paramModID, @paramIdentifier, @paramLockedIcon, @paramLockedHash, @paramUnlockedIcon, @paramUnlockedHash, @paramLockedIcon, @paramLockedHash, @paramUnlockedIcon, @paramUnlockedHash;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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

		const auto modID = pt.get<int32_t>("ModID");

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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectDevDurationStmt FROM \"SELECT Duration FROM devtimes WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectFeedbackMailStmt FROM \"SELECT Mail FROM feedbackMails WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectDiscussionUrlStmt FROM \"SELECT Url FROM discussionUrls WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectKeywordsStmt FROM \"SELECT Keywords FROM keywordsPerProject WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectModStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			responseTree.put("Duration", results.empty() ? 0 : std::stoi(results[0][0]));
			
			if (!database.query("EXECUTE selectFeedbackMailStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			if (!results.empty()) {
				responseTree.put("FeedbackMail", results[0][0]);
			}
			
			if (!database.query("EXECUTE selectDiscussionUrlStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			if (!results.empty()) {
				responseTree.put("DiscussionUrl", results[0][0]);
			}

			if (!database.query("EXECUTE selectKeywordsStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			responseTree.put("Keywords", results.empty() ? "" : results[0][0]);
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

		const auto modID = pt.get<int32_t>("ModID");

		if (!hasAdminAccessToMod(userID, modID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		const int32_t enabled = pt.get<bool>("Enabled") ? 1 : 0;
		const auto gothicVersion = pt.get<int32_t>("GothicVersion");
		const auto modType = pt.get<int32_t>("ModType");
		const auto duration = pt.get<int32_t>("Duration");
		const auto releaseDate = pt.get<int32_t>("ReleaseDate");
		const auto keywords = pt.get<std::string>("Keywords");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE updateStmt FROM \"UPDATE mods SET Gothic = ?, Type = ?, ReleaseDate = ? WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateEnabledStmt FROM \"UPDATE mods SET Enabled = ? WHERE ModID = ? AND Enabled = 0 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateDevDurationStmt FROM \"INSERT INTO devtimes (ModID, Duration) VALUES (?, ?) ON DUPLICATE KEY UPDATE Duration = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateFeedbackMailStmt FROM \"INSERT INTO feedbackMails (ProjectID, Mail) VALUES (?, ?) ON DUPLICATE KEY UPDATE Mail = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateKeywordsStmt FROM \"INSERT INTO keywordsPerProject (ProjectID, Keywords) VALUES (?, ?) ON DUPLICATE KEY UPDATE Keywords = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteFeedbackMailStmt FROM \"DELETE FROM feedbackMails WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateDiscussionUrlStmt FROM \"INSERT INTO discussionUrls (ProjectID, Url) VALUES (?, ?) ON DUPLICATE KEY UPDATE Url = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteDiscussionUrlStmt FROM \"DELETE FROM discussionUrls WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectEnabledStateStmt FROM \"SELECT Enabled FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE insertReleaseNewsStmt FROM \"INSERT INTO newsticker (ProjectID, Type, Date) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramEnabled=" + std::to_string(enabled) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramGothicVersion=" + std::to_string(gothicVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModType=" + std::to_string(modType) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramReleaseDate=" + std::to_string(releaseDate) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramDuration=" + std::to_string(duration) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramKeywords='" + keywords + "';")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectEnabledStateStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();

			const bool wasEnabled = results[0][0] == "1";

			if (enabled == 1 && !wasEnabled) {
				if (!database.query("SET @paramNewsType=" + std::to_string(static_cast<int>(NewsTickerType::Release)) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE insertReleaseNewsStmt USING @paramModID, @paramNewsType, @paramReleaseDate;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}

				auto message = "[RELEASE] " + ServerCommon::getProjectName(modID, English);

				std::thread([message] {
					pushToDiscord(message);
				}).detach();
			}
			
			if (!database.query("EXECUTE updateStmt USING @paramGothicVersion, @paramModType, @paramReleaseDate, @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateEnabledStmt USING @paramEnabled, @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateDevDurationStmt USING @paramModID, @paramDuration, @paramDuration;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateKeywordsStmt USING @paramModID, @paramKeywords, @paramKeywords;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}

			if (pt.count("FeedbackMail") == 0) {
				if (!database.query("EXECUTE deleteFeedbackMailStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} else {
				if (!database.query("SET @paramMail='" + pt.get<std::string>("FeedbackMail") + "';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE updateFeedbackMailStmt USING @paramModID, @paramMail, @paramMail;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			}

			if (pt.count("DiscussionUrl") == 0) {
				if (!database.query("EXECUTE deleteDiscussionUrlStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} else {
				if (!database.query("SET @paramUrl='" + pt.get<std::string>("DiscussionUrl") + "';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE updateDiscussionUrlStmt USING @paramModID, @paramUrl, @paramUrl;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectChapterStatsStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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

		const int32_t packageID = pt.get<int32_t>("PackageID");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion, SpineVersion, Gothic FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectModFilesStmt FROM \"SELECT Path, Hash, Language FROM modfiles WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectPackageFilesStmt FROM \"SELECT Path, Hash, Language FROM optionalpackagefiles WHERE PackageID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramPackageID=" + std::to_string(packageID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectVersionStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
			responseTree.put("VersionSpine", std::stoi(results[0][3]));
			responseTree.put("GameType", std::stoi(results[0][4]));

			if (packageID >= 0) {
				if (!database.query("EXECUTE selectPackageFilesStmt USING @paramPackageID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} else {
				if (!database.query("EXECUTE selectModFilesStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
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

		const auto projectID = pt.get<int32_t>("ModID");

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		const auto versionMajor = pt.get<int32_t>("VersionMajor");
		const auto versionMinor = pt.get<int32_t>("VersionMinor");
		const auto versionPatch = pt.get<int32_t>("VersionPatch");
		
		int32_t date = 0;

		try {
			date = pt.get<int32_t>("Date");
		} catch (...) {}

		int32_t savegameCompatible = 0;

		try {
			savegameCompatible = pt.get<int32_t>("SavegameCompatible");
		} catch (...) {}

		int32_t versionSpine = 0;

		try {
			versionSpine = pt.get<int32_t>("VersionSpine");
		} catch (...) {}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE updateStmt FROM \"UPDATE mods SET MajorVersion = ?, MinorVersion = ?, PatchVersion = ?, SpineVersion = ? WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE isEnabledStmt FROM \"SELECT Enabled FROM mods WHERE ModID = ? AND Enabled = 1 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateDateStmt FROM \"INSERT INTO lastUpdated (ProjectID, Date) VALUES (?, ?) ON DUPLICATE KEY UPDATE Date = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE insertNewsTickerStmt FROM \"INSERT INTO newsticker (ProjectID, Date, Type) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectNewsIDStmt FROM \"SELECT MAX(NewsID) FROM newsticker WHERE ProjectID = ? AND Date = ? AND Type = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE insertUpdateNewsStmt FROM \"INSERT INTO updateNews (NewsID, ProjectID, MajorVersion, MinorVersion, PatchVersion, SavegameCompatible) VALUES (?, ?, ?, ?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE insertChangelogStmt FROM \"INSERT INTO changelogs (NewsID, Language, Changelog) VALUES (?, CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramVersionMajor=" + std::to_string(versionMajor) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramVersionMinor=" + std::to_string(versionMinor) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramVersionPatch=" + std::to_string(versionPatch) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramVersionSpine=" + std::to_string(versionSpine) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramDate=" + std::to_string(date) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramType=" + std::to_string(static_cast<int>(NewsTickerType::Update)) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateStmt USING @paramVersionMajor, @paramVersionMinor, @paramVersionPatch, @paramVersionSpine, @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}

			FileSynchronizer::AddJob job;
			job.operation = FileSynchronizer::Operation::RaiseVersion;
			job.majorVersion = versionMajor;
			job.minorVersion = versionMinor;
			job.patchVersion = versionPatch;
			job.spineVersion = versionSpine;
			job.projectID = projectID;
			
			FileSynchronizer::addJob(job);

			DownloadSizeChecker::refresh();
			
			if (!database.query("EXECUTE isEnabledStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			if (results.empty())
				break;
			
			if (!database.query("EXECUTE updateDateStmt USING @paramModID, @paramDate, @paramDate;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE insertNewsTickerStmt USING @paramModID, @paramDate, @paramType;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectNewsIDStmt USING @paramModID, @paramDate, @paramType;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			if (!database.query("SET @paramNewsID=" + results[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSavegameCompatible=" + std::to_string(savegameCompatible) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE insertUpdateNewsStmt USING @paramNewsID, @paramModID, @paramVersionMajor, @paramVersionMinor, @paramVersionPatch, @paramSavegameCompatible;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}

			for (const auto & v : pt.get_child("Changelogs")) {
				const std::string language = v.second.get<std::string>("Language");
				const std::string changelog = v.second.get<std::string>("Changelog");
				
				if (!database.query("SET @paramLanguage='" + language + "';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					return;
				}
				if (!database.query("SET @paramChangelog='" + changelog + "';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					return;
				}
				if (!database.query("EXECUTE insertChangelogStmt USING @paramNewsID, @paramLanguage, @paramChangelog;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			}

			auto message = "[UPDATE] " + ServerCommon::getProjectName(projectID, English) + " " + std::to_string(versionMajor) + "." + std::to_string(versionMinor) + "." + std::to_string(versionPatch) + "." + std::to_string(versionSpine);

			std::thread([message] {
				pushToDiscord(message);
			}).detach();
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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectOverallDownloadsStmt FROM \"SELECT Counter FROM downloads WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectDownloadsPerVersionStmt FROM \"SELECT Version, Counter FROM downloadsPerVersion WHERE ModID = ? ORDER BY Version ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectPlaytimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID != -1 ORDER BY Duration ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectSessiontimeStmt FROM \"SELECT Duration FROM sessionTimes WHERE ModID = ? ORDER BY Duration ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAchievementtimeStmt FROM \"SELECT Duration FROM achievementTimes WHERE ModID = ? AND Identifier = ? ORDER BY Duration ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectIntervalPlayersStmt FROM \"SELECT COUNT(*) FROM lastPlayTimes WHERE ModID = ? AND UserID != -1 AND Timestamp > ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectOverallDownloadsStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			responseTree.put("OverallDownloads", results.empty() ? 0 : std::stoi(results[0][0]));
			
			if (!database.query("EXECUTE selectDownloadsPerVersionStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			uint32_t max = 0;
			uint64_t count = 0;
			uint32_t medianPlaytime = 0;
			for (size_t i = 0; i < results.size(); i++) {
				const auto vec = results[i];
				const auto current = static_cast<uint32_t>(std::stoi(vec[0]));
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
				playTimeNode.put("Maximum", max);
				playTimeNode.put("Median", medianPlaytime);
				playTimeNode.put("Average", results.empty() ? 0 : static_cast<int32_t>(count / results.size()));
			}
			responseTree.add_child("PlayTime", playTimeNode);
			responseTree.put("OverallPlayerCount", results.size());
			
			if (!database.query("EXECUTE selectSessiontimeStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			max = 0;
			count = 0;
			medianPlaytime = 0;
			for (size_t i = 0; i < results.size(); i++) {
				const auto vec = results[i];
				const auto current = static_cast<uint32_t>(std::stoi(vec[0]));
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
				sessionTimeNode.put("Maximum", max);
				sessionTimeNode.put("Median", medianPlaytime);
				sessionTimeNode.put("Average", results.empty() ? 0 : static_cast<int32_t>(count / results.size()));
			}
			responseTree.add_child("SessionTime", sessionTimeNode);

			const int timestamp = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
			if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp - 24) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectIntervalPlayersStmt USING @paramModID, @paramTimestamp;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			responseTree.put("Last24HoursPlayerCount", static_cast<uint32_t>(std::stoi(results[0][0])));

			if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp - 7 * 24) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectIntervalPlayersStmt USING @paramModID, @paramTimestamp;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			responseTree.put("Last7DaysPlayerCount", static_cast<uint32_t>(std::stoi(results[0][0])));

			if (!database.query("EXECUTE selectAchievementNamesStmt USING @paramModID, @paramLanguage;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectAchievementtimeStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto results2 = database.getResults<std::vector<std::string>>();

				max = 0;
				count = 0;
				medianPlaytime = 0;
				for (size_t i = 0; i < results2.size(); i++) {
					const auto vec2 = results2[i];
					const uint32_t current = static_cast<uint32_t>(std::stoi(vec2[0]));
					if (current > max) {
						max = current;
					}
					count += current;
					if (i == results2.size() / 2) {
						medianPlaytime = current;
					}
				}
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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectScoreOrdersStmt FROM \"SELECT Identifier, ScoreOrder FROM scoreOrders WHERE ProjectID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectScoreOrdersStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			std::map<int32_t, ScoreOrder> scoreOrders;

			for (const auto & vec : results) {
				scoreOrders.insert(std::make_pair(std::stoi(vec[0]), static_cast<ScoreOrder>(std::stoi(vec[1]))));
			}
			
			if (!database.query("EXECUTE selectScoreNamesStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

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

				const auto it2 = scoreOrders.find(it.first);

				const auto scoreOrder = it2 != scoreOrders.end() ? it2->second : ScoreOrder::Descending;
				
				scoreNode.put("Order", static_cast<int>(scoreOrder));
				
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

		const auto modID = pt.get<int32_t>("ModID");

		if (!hasAdminAccessToMod(userID, modID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE insertScoreStmt FROM \"INSERT IGNORE INTO modScoreList (ModID, Identifier) VALUES (?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateScoreOrderStmt FROM \"INSERT INTO scoreOrders (ProjectID, Identifier, ScoreOrder) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE ScoreOrder = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateScoreNameStmt FROM \"INSERT INTO modScoreNames (ModID, Identifier, Language, Name) VALUES (?, ?, CONVERT(? USING BINARY), CONVERT(? USING BINARY)) ON DUPLICATE KEY UPDATE Name = CONVERT(? USING BINARY)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto scores = pt.get_child("Scores");

			int id = 0;
			for (const auto & score : scores) {
				if (!database.query("SET @paramIdentifier=" + std::to_string(id) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE insertScoreStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				for (const auto & tt : score.second.get_child("Names")) {
					if (!database.query("SET @paramLanguage='" + tt.second.get<std::string>("Language") + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramName='" + tt.second.get<std::string>("Text") + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE updateScoreNameStmt USING @paramModID, @paramIdentifier, @paramLanguage, @paramName, @paramName;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
				}

				const auto orderNode = score.second.count("Order") == 0 ? static_cast<int32_t>(ScoreOrder::Descending) : score.second.get<int32_t>("Order");
				if (!database.query("SET @paramScoreOrder=" + std::to_string(orderNode) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE updateScoreOrderStmt USING @paramModID, @paramIdentifier, @paramScoreOrder, @paramScoreOrder;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectEarlyAccessorsStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM earlyUnlocks WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(accessUserID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (enabled) {
				if (!database.query("EXECUTE insertStmt USING @paramModID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			} else {
				if (!database.query("EXECUTE deleteStmt USING @paramModID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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

void ManagementServer::createPlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t projectID = pt.get<int32_t>("ProjectID");
		const std::string language = pt.get<std::string>("Language");
		const int32_t majorVersion = pt.get<int32_t>("MajorVersion");
		const int32_t minorVersion = pt.get<int32_t>("MinorVersion");
		const int32_t patchVersion = pt.get<int32_t>("PatchVersion");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO playTestSurveys (ProjectID, Language, Enabled, MajorVersion, MinorVersion, PatchVersion) VALUES (?, ?, 0, ?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramMajorVersion=" + std::to_string(majorVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramMinorVersion=" + std::to_string(minorVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramPatchVersion=" + std::to_string(patchVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE insertStmt USING @paramProjectID, @paramLanguage, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::enablePlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t projectID = pt.get<int32_t>("ProjectID");
		const int32_t surveyID = pt.get<int32_t>("SurveyID");
		const int32_t enabled = pt.get<int32_t>("Enabled");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE updateStmt FROM \"UPDATE playTestSurveys SET Enabled = ? WHERE SurveyID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSurveyID=" + std::to_string(surveyID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramEnabled=" + std::to_string(enabled) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateStmt USING @paramEnabled, @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::updatePlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t projectID = pt.get<int32_t>("ProjectID");
		const int32_t surveyID = pt.get<int32_t>("SurveyID");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO playTestSurveyQuestions (SurveyID, Question) VALUES (?, CONVERT(? USING BINARY))\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}	
			if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM playTestSurveyQuestions WHERE SurveyID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSurveyID=" + std::to_string(surveyID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateStmt USING @paramEnabled, @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			for (const auto & v : pt.get_child("Questions")) {
				const auto question = v.second.data();
				if (!database.query("SET @paramQuestion='" + question + "';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE insertStmt USING @paramSurveyID, @paramQuestion;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
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

void ManagementServer::getPlayTestSurveys(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t projectID = pt.get<int32_t>("ProjectID");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectStmt FROM \"SELECT SurveyID, Enabled, Language, MajorVersion, MinorVersion, PatchVersion FROM playTestSurveys WHERE ProjectID = ? ORDER BY MajorVersion, MinorVersion, PatchVersion DESC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectQuestionCountStmt FROM \"SELECT COUNT(*) FROM playTestSurveyQuestions WHERE SurveyID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAnswerCountStmt FROM \"SELECT DISTINCT UserID FROM playTestSurveyAnswers WHERE SurveyID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();
			
			ptree surveysNode;
			for (const auto & vec : results) {
				ptree surveyNode;

				surveyNode.put("SurveyID", vec[0]);
				surveyNode.put("Enabled", vec[1]);
				surveyNode.put("Language", vec[2]);
				surveyNode.put("MajorVersion", vec[3]);
				surveyNode.put("MinorVersion", vec[4]);
				surveyNode.put("PatchVersion", vec[5]);
				
				if (!database.query("SET @paramSurveyID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectQuestionCountStmt USING @paramSurveyID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto results2 = database.getResults<std::vector<std::string>>();

				surveyNode.put("QuestionCount", results2.empty() ? "0" : results2[0][0]);
				
				if (!database.query("EXECUTE selectAnswerCountStmt USING @paramSurveyID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results2 = database.getResults<std::vector<std::string>>();

				surveyNode.put("AnswerCount", results2.size());
				
				surveysNode.push_back(std::make_pair("", surveyNode));
			}
			responseTree.add_child("Surveys", surveysNode);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::getPlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t surveyID = pt.get<int32_t>("SurveyID");
		const int32_t projectID = pt.get<int32_t>("ProjectID");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectQuestionsStmt FROM \"SELECT CAST(Question AS BINARY) FROM playTestSurveyQuestions WHERE SurveyID = ? ORDER BY QuestionID ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSurveyID=" + std::to_string(surveyID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectQuestionsStmt USING @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();
			
			ptree questionsNode;
			for (const auto & vec : results) {
				ptree questionNode;

				questionNode.put("Question", vec[0]);
				
				questionsNode.push_back(std::make_pair("", questionNode));
			}
			responseTree.add_child("Questions", questionsNode);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::submitPlayTestSurveyAnswers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t surveyID = pt.get<int32_t>("SurveyID");
		const int32_t majorVersion = pt.get<int32_t>("MajorVersion");
		const int32_t minorVersion = pt.get<int32_t>("MinorVersion");
		const int32_t patchVersion = pt.get<int32_t>("PatchVersion");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO playTestSurveyAnswers (SurveyID, UserID, QuestionID, Answer, MajorVersion, MinorVersion, PatchVersion) VALUES (?, ?, ?, CONVERT(? USING BINARY), ?, ?, ?) ON DUPLICATE KEY UPDATE Answer = CONVERT(? USING BINARY), MajorVersion = ?, MinorVersion = ?, PatchVersion = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSurveyID=" + std::to_string(surveyID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramMajorVersion=" + std::to_string(majorVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramMinorVersion=" + std::to_string(minorVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramPatchVersion=" + std::to_string(patchVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			for (const auto & v : pt.get_child("Answers")) {
				const auto questionID = v.second.get<int32_t>("QuestionID");
				const auto answer = v.second.get<std::string>("Answer");
				
				if (!database.query("SET @paramQuestionID=" + std::to_string(questionID) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramAnswer='" + answer + "';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE insertStmt USING @paramSurveyID, @paramUserID, @paramQuestionID, @paramAnswer, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion, @paramAnswer, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
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

void ManagementServer::getOwnPlayTestSurveyAnswers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const std::string language = pt.get<std::string>("Language");
		const int32_t projectID = pt.get<int32_t>("ProjectID");
		const int32_t majorVersion = pt.get<int32_t>("MajorVersion");
		const int32_t minorVersion = pt.get<int32_t>("MinorVersion");
		const int32_t patchVersion = pt.get<int32_t>("PatchVersion");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectPlayTestStmt FROM \"SELECT ModID FROM mods WHERE ModID = ? AND Type = 8 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectSurveyIDStmt FROM \"SELECT SurveyID FROM playTestSurveys WHERE ProjectID = ? AND Language = CONVERT(? USING BINARY) AND MajorVersion = ? AND MinorVersion = ? AND PatchVersion = ? AND Enabled = 1 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectQuestionsStmt FROM \"SELECT QuestionID, CAST(Question AS BINARY) FROM playTestSurveyQuestions WHERE SurveyID = ? ORDER BY QuestionID ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAnswersStmt FROM \"SELECT CAST(Answer AS BINARY) FROM playTestSurveyAnswers WHERE SurveyID = ? AND UserID = ? AND QuestionID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramMajorVersion=" + std::to_string(majorVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramMinorVersion=" + std::to_string(minorVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramPatchVersion=" + std::to_string(patchVersion) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectPlayTestStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			if (results.empty()) {
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			
			if (!database.query("EXECUTE selectSurveyIDStmt USING @paramProjectID, @paramLanguage, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			if (results.empty() && language != "English") {
				if (!database.query("SET @paramLanguage='English';")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectSurveyIDStmt USING @paramProjectID, @paramLanguage, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				results = database.getResults<std::vector<std::string>>();
			}

			if (results.empty()) break;
			
			if (!database.query("SET @paramSurveyID=" + results[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			
			responseTree.put("SurveyID", results[0][0]);
			
			if (!database.query("EXECUTE selectQuestionsStmt USING @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			
			ptree questionsNode;
			for (const auto & vec : results) {
				if (!database.query("SET @paramQuestionID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			
				if (!database.query("EXECUTE selectAnswersStmt USING @paramSurveyID, @paramUserID, @paramQuestionID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto results2 = database.getResults<std::vector<std::string>>();
			
				ptree answerNode;

				answerNode.put("QuestionID", vec[0]);
				answerNode.put("Question", vec[1]);
				answerNode.put("Answer", results2.empty() ? "" : results2[0][0]);
				
				questionsNode.push_back(std::make_pair("", answerNode));
			}
			responseTree.add_child("Questions", questionsNode);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::getAllPlayTestSurveyAnswers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t surveyID = pt.get<int32_t>("SurveyID");
		const int32_t projectID = pt.get<int32_t>("ProjectID");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectQuestionsStmt FROM \"SELECT QuestionID, CAST(Question AS BINARY) FROM playTestSurveyQuestions WHERE SurveyID = ? ORDER BY QuestionID ASC\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectAnswersStmt FROM \"SELECT CAST(Answer AS BINARY) FROM playTestSurveyAnswers WHERE QuestionID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSurveyID=" + std::to_string(surveyID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectQuestionsStmt USING @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();
			
			ptree questionsNode;
			for (const auto & vec : results) {
				ptree questionNode;

				questionNode.put("Question", vec[1]);
				
				if (!database.query("SET @paramQuestionID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectAnswersStmt USING @paramQuestionID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto results2 = database.getResults<std::vector<std::string>>();

				ptree answersNode;

				for (const auto & vec2 : results2) {
					ptree answerNode;

					answerNode.put("Answer", vec2[0]);

					answersNode.push_back(std::make_pair("", answerNode));
				}

				questionNode.add_child("Answers", answersNode);
				
				questionsNode.push_back(std::make_pair("", questionNode));
			}
			responseTree.add_child("Questions", questionsNode);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::deletePlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t projectID = pt.get<int32_t>("ProjectID");
		const int32_t surveyID = pt.get<int32_t>("SurveyID");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)
				
			if (!database.query("PREPARE deleteSurveyStmt FROM \"DELETE FROM playTestSurveys WHERE SurveyID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteSurveyQuestionsStmt FROM \"DELETE FROM playTestSurveyQuestions WHERE SurveyID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteSurveyAnswersStmt FROM \"DELETE FROM playTestSurveyAnswers WHERE SurveyID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramSurveyID=" + std::to_string(surveyID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE deleteSurveyStmt USING @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE deleteSurveyQuestionsStmt USING @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE deleteSurveyAnswersStmt USING @paramSurveyID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::clearAchievementProgress(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const int32_t projectID = pt.get<int32_t>("ProjectID");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		if (!hasAdminAccessToMod(userID, projectID)) {
			response->write(SimpleWeb::StatusCode::client_error_unauthorized);
			return;
		}

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE deleteAchievementProgressStmt FROM \"DELETE FROM modAchievements WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectEnabledStmt FROM \"SELECT Enabled FROM mods WHERE ModID = ? AND Enabled = 0 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectEnabledStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();

			if (results.empty()) break;
			
			if (!database.query("EXECUTE deleteAchievementProgressStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void ManagementServer::uploadAchievementIcons(UploadAchievementIconsMessage * msg) const {
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
	if (userID == 3) return true;
	
	do {
		CONNECTTODATABASE(__LINE__)
			
		if (!database.query("PREPARE selectModStmt FROM \"SELECT TeamID FROM mods WHERE ModID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectMemberStmt FROM \"SELECT UserID FROM teammembers WHERE TeamID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectModStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();

		if (results.empty()) break;
		
		if (!database.query("SET @paramTeamID=" + results[0][0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ":" << __LINE__ << std::endl;
			break;
		}
		
		if (!database.query("EXECUTE selectMemberStmt USING @paramTeamID, @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		
		return !results.empty();
	} while (false);
	return false;
}

void ManagementServer::pushToDiscord(const std::string & message) {
	try {
		const static std::string url = "discordapp.com";
		HttpsClient client(url, false);

		ptree requestData;

		std::string msg = message;
		msg = std::regex_replace(msg, std::regex("&apos;"), "'");

		requestData.put("content", msg);

		std::stringstream responseStream;
		write_json(responseStream, requestData);
		const std::string content = responseStream.str();

		SimpleWeb::CaseInsensitiveMultimap header;
		header.emplace("Content-Type", "application/json");
		header.emplace("Content-length", std::to_string(content.length()));
		header.emplace("Charset", "UTF-8");

		const auto response = client.request("POST", DISCORDWEBAPIURL, content, header);

		if (response->status_code.size() >= 2 && response->status_code[0] == '2' && response->status_code[1] == '0') {
			std::cout << "Successfully pushed to Discord" << std::endl;
		} else {
			std::cout << "Failed to push to Discord: " << response->status_code << " - " << response->content.string() << std::endl;
		}
	} catch (const boost::system::system_error & ex) {
		std::cout << "Failed to push to Discord: " << ex.code() << " - " << ex.what() << std::endl;
	}
}
