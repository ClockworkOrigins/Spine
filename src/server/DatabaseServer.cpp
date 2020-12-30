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

#include "DatabaseServer.h"

#include <set>
#include <sstream>

#include "LanguageConverter.h"
#include "MariaDBWrapper.h"
#include "ServerCommon.h"
#include "SpineLevel.h"
#include "SpineServerConfig.h"

#include "common/ScoreOrder.h"

#define BOOST_SPIRIT_THREADSAFE
#include "boost/filesystem.hpp"
#include "boost/property_tree/json_parser.hpp"

using namespace boost::property_tree;

using namespace spine::server;

DatabaseServer::DatabaseServer() : _server(nullptr), _runner(nullptr) {}

DatabaseServer::~DatabaseServer() {
	delete _server;
	delete _runner;
}

int DatabaseServer::run() {
	_server = new HttpsServer(SSLCHAINPATH, SSLPRIVKEYNPATH);
	_server->config.port = DATABASESERVER_PORT;
	_server->config.thread_pool_size = 16;
	
	_server->resource["^/getModnameForIDs"]["POST"] = std::bind(&DatabaseServer::getModnameForIDs, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getUserID"]["POST"] = std::bind(&DatabaseServer::getUserID, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getOwnRating"]["POST"] = std::bind(&DatabaseServer::getOwnRating, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getWeightedRating"]["POST"] = std::bind(&DatabaseServer::getWeightedRating, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getRatings"]["POST"] = std::bind(&DatabaseServer::getRatings, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/unlockAchievementServer"]["POST"] = std::bind(&DatabaseServer::unlockAchievementServer, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getUserIDForDiscordID"]["POST"] = std::bind(&DatabaseServer::getUserIDForDiscordID, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getSpineLevelRanking"]["POST"] = std::bind(&DatabaseServer::getSpineLevelRanking, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/sendUserInfos"]["POST"] = std::bind(&DatabaseServer::sendUserInfos, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestSingleProjectStats"]["POST"] = std::bind(&DatabaseServer::requestSingleProjectStats, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestCompatibilityList"]["POST"] = std::bind(&DatabaseServer::requestCompatibilityList, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updatePlayTime"]["POST"] = std::bind(&DatabaseServer::updatePlayTime, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestScores"]["POST"] = std::bind(&DatabaseServer::requestScores, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updateScore"]["POST"] = std::bind(&DatabaseServer::updateScore, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getReviews"]["POST"] = std::bind(&DatabaseServer::getReviews, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updateReview"]["POST"] = std::bind(&DatabaseServer::updateReview, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestAchievements"]["POST"] = std::bind(&DatabaseServer::requestAchievements, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/unlockAchievement"]["POST"] = std::bind(&DatabaseServer::unlockAchievement, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updateAchievementProgress"]["POST"] = std::bind(&DatabaseServer::updateAchievementProgress, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestOverallSaveData"]["POST"] = std::bind(&DatabaseServer::requestOverallSaveData, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestAllFriends"]["POST"] = std::bind(&DatabaseServer::requestAllFriends, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updateChapterStats"]["POST"] = std::bind(&DatabaseServer::updateChapterStats, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/isAchievementUnlocked"]["POST"] = std::bind(&DatabaseServer::isAchievementUnlocked, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updateOfflineData"]["POST"] = std::bind(&DatabaseServer::updateOfflineData, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestOfflineData"]["POST"] = std::bind(&DatabaseServer::requestOfflineData, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/friendRequest"]["POST"] = std::bind(&DatabaseServer::friendRequest, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/feedback"]["POST"] = std::bind(&DatabaseServer::feedback, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/acceptFriend"]["POST"] = std::bind(&DatabaseServer::acceptFriend, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/declineFriend"]["POST"] = std::bind(&DatabaseServer::declineFriend, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/updateLoginTime"]["POST"] = std::bind(&DatabaseServer::updateLoginTime, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestRandomPage"]["POST"] = std::bind(&DatabaseServer::requestRandomPage, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/requestInfoPage"]["POST"] = std::bind(&DatabaseServer::requestInfoPage, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/submitInfoPage"]["POST"] = std::bind(&DatabaseServer::submitInfoPage, this, std::placeholders::_1, std::placeholders::_2);

	_runner = new std::thread([this]() {
		_server->start();
	});
	
	return 0;
}

void DatabaseServer::stop() {
	_server->stop();
	_runner->join();
}

void DatabaseServer::getModnameForIDs(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string language = pt.get<std::string>("Language");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectStmt FROM \"SELECT Name FROM modnames WHERE ModID = ? AND Language = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}

			ptree nameNodes;
			for (const auto & v : pt.get_child("ModIDs")) {
				if (!database.query("SET @paramModID=" + v.second.data() + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectStmt USING @paramModID, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				const auto results = database.getResults<std::vector<std::string>>();
				if (results.empty()) continue;

				ptree nameNode;
				nameNode.put("", results[0][0]);
				nameNodes.push_back(std::make_pair("", nameNode));
			}
			responseTree.add_child("Names", nameNodes);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getUserID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password);			

		responseTree.put("ID", userID);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getOwnRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
	
		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}
		
		const auto projectID = pt.get<int32_t>("ProjectID");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectOwnRatingStmt FROM \"SELECT Rating FROM ratings WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectOwnPlaytimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectOverallPlaytimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID != -1 ORDER BY Duration ASC\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectReviewStmt FROM \"SELECT Review FROM reviews WHERE ProjectID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			
			if (!database.query("EXECUTE selectOwnRatingStmt USING @paramProjectID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			responseTree.put("Rating", results.empty() ? 0 : std::stoi(results[0][0]));
			
			if (!database.query("EXECUTE selectOwnPlaytimeStmt USING @paramProjectID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			const int ownPlaytime = results.empty() ? 0 : std::stoi(results[0][0]);
			
			if (!database.query("EXECUTE selectOverallPlaytimeStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			const int medianPlaytime = results.size() < 2 ? std::numeric_limits<int>::max() : std::stoi(results[results.size() / 2][0]);

			responseTree.put("AllowedToRate", ownPlaytime > medianPlaytime ? 1 : 0);

			if (!database.query("EXECUTE selectReviewStmt USING @paramProjectID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			if (!results.empty()) {
				responseTree.put("Review", results[0][0]);
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getWeightedRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
		
		const auto projectID = pt.get<int32_t>("ProjectID");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectRatingsStmt FROM \"SELECT UserID, Rating FROM ratings WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectPlaytimesStmt FROM \"SELECT UserID, Duration FROM playtimes WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectOverallPlaytimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID != -1 ORDER BY Duration ASC\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			
			if (!database.query("EXECUTE selectOverallPlaytimeStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();

			const int quartile3Playtime = results.size() < 4 ? std::numeric_limits<int>::max() : std::stoi(results[results.size() * 3 / 4][0]);
			
			if (!database.query("EXECUTE selectPlaytimesStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			std::map<int, int> playTimes;

			for (const auto & vec : results) {
				const int u = std::stoi(vec[0]);
				const int t = std::stoi(vec[1]);

				playTimes.insert(std::make_pair(u, t));
			}
			
			if (!database.query("EXECUTE selectRatingsStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			results = database.getResults<std::vector<std::string>>();

			int rating = 0;
			int count = 0;
			
			for (const auto & vec : results) {
				const int u = std::stoi(vec[0]);
				const int r = std::stoi(vec[1]);
				
				rating += r;
				count++;

				const auto it = playTimes.find(u);

				const int time = it == playTimes.end() ? 0 : it->second;

				if (time > quartile3Playtime) {
					rating += r;
					count++;
				}

				const auto level = SpineLevel::getLevel(u);

				if (level.level > 5) {
					rating += r;
					count++;
				}
			}
			
			responseTree.put("Rating", rating);
			responseTree.put("Count", count);
			responseTree.put("RealCount", results.size());
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getRatings(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
		
		const auto projectID = pt.get<int32_t>("ProjectID");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectRatingsStmt FROM \"SELECT Rating FROM ratings WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			
			if (!database.query("EXECUTE selectRatingsStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();

			int rating1 = 0;
			int rating2 = 0;
			int rating3 = 0;
			int rating4 = 0;
			int rating5 = 0;
			
			for (const auto & vec : results) {
				const int r = std::stoi(vec[0]);
				
				if (r == 1) {
					rating1++;
				} else if (r == 2) {
					rating2++;
				} else if (r == 3) {
					rating3++;
				} else if (r == 4) {
					rating4++;
				} else if (r == 5) {
					rating5++;
				}
			}
			
			responseTree.put("Rating1", rating1);
			responseTree.put("Rating2", rating2);
			responseTree.put("Rating3", rating3);
			responseTree.put("Rating4", rating4);
			responseTree.put("Rating5", rating5);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::unlockAchievementServer(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
		
		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto userID = pt.get<int32_t>("UserID");
		const auto achievementID = pt.get<int32_t>("AchievementID");

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE updateStmt FROM \"INSERT IGNORE INTO modAchievements (ModID, UserID, Identifier) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE validateServerStmt FROM \"SELECT * FROM gmpWhitelist WHERE ModID = ? AND IP = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectStmt FROM \"SELECT * FROM modAchievementList WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIdentifier=" + std::to_string(achievementID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIP='" + request->remote_endpoint_address() + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE validateServerStmt USING @paramModID, @paramIP;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();

			if (results.empty()) {
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break; // invalid access
			}
			
			if (!database.query("EXECUTE updateStmt USING @paramModID, @paramUserID, @paramIdentifier;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}

			SpineLevel::updateLevel(userID);
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getUserIDForDiscordID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto discordAPI = pt.get<int64_t>("DiscordAPI");

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE validateServerStmt FROM \"SELECT * FROM gmpWhitelist WHERE IP = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIP='" + request->remote_endpoint_address() + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE validateServerStmt USING @paramIP;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();

			if (results.empty()) {
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break; // invalid access
			}
		} while (false);

		do {
			if (code != SimpleWeb::StatusCode::success_ok) break;
			
			MariaDBWrapper accountDatabase;
			if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}

			if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT ID FROM linkedDiscordAccounts WHERE DiscordID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!accountDatabase.query("SET @paramDiscordID=" + std::to_string(discordAPI) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!accountDatabase.query("EXECUTE selectStmt USING @paramDiscordID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			const auto results = accountDatabase.getResults<std::vector<std::string>>();

			responseTree.put("UserID", results.empty() ? -1 : std::stoi(results[0][0]));
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getSpineLevelRanking(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		std::stringstream responseStream;
		ptree responseTree;

		SpineLevel::addRanking(responseTree);

		write_json(responseStream, responseTree);

		response->write(SimpleWeb::StatusCode::success_ok, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::sendUserInfos(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const std::string hash = pt.get<std::string>("Hash");
		const std::string mac = pt.get<std::string>("Mac");
		const std::string language = pt.get<std::string>("Language");
		
		const int userID = ServerCommon::getUserID(username, password);

		if (userID == -1) {
			response->write(code);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE insertHashStmt FROM \"INSERT IGNORE INTO userHashes (UserID, Hash) VALUES (?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updateSessionInfosStmt FROM \"INSERT INTO userSessionInfos (UserID, Mac, IP, Hash) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Mac = ?, IP = ?, Hash = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updateUserSettingsStmt FROM \"INSERT INTO userSettings (UserID, Entry, Value) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE Value = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updateUserLanguageStmt FROM \"INSERT INTO userLanguages (UserID, Language) VALUES (?, ?) ON DUPLICATE KEY UPDATE Language = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE insertBanStmt FROM \"INSERT IGNORE INTO provisoricalBans (UserID) VALUES (?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectSessionInfoMatchStmt FROM \"SELECT * FROM userSessionInfos WHERE UserID = ? AND Mac = ? AND Hash = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramHash='" + hash + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramMac='" + mac + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIP='" + request->remote_endpoint_address() + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE updateUserLanguageStmt USING @paramUserID, @paramLanguage, @paramLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (pt.count("Launch") == 1) { // in this case settings have to be empty
				if (!database.query("EXECUTE selectSessionInfoMatchStmt USING @paramUserID, @paramMac, @paramHash;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				const auto lastResults = database.getResults<std::vector<std::string>>();
				if (lastResults.empty()) { // if current mac doesn't match the mac from startup => ban
					if (!database.query("EXECUTE insertBanStmt USING @paramUserID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					break;
				}
			}
			if (!database.query("EXECUTE insertHashStmt USING @paramUserID, @paramHash;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE updateSessionInfosStmt USING @paramUserID, @paramMac, @paramIP, @paramHash, @paramMac, @paramIP, @paramHash;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (pt.count("Settings") == 1) {
				for (const auto & v : pt.get_child("Settings")) {
					const auto data = v.second;
					const std::string entry = data.get<std::string>("Entry");
					const std::string value = data.get<std::string>("Value");
					
					if (!database.query("SET @paramEntry='" + entry + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramValue='" + value + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("EXECUTE updateUserSettingsStmt USING @paramUserID, @paramEntry, @paramValue, @paramValue;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
				}
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestSingleProjectStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const std::string username = pt.get<std::string>("Username");
		const std::string password = pt.get<std::string>("Password");
		const std::string language = pt.get<std::string>("Language");
		const auto projectID = pt.get<int64_t>("ProjectID");

		if (projectID == -1) {
			response->write(code);
			return;
		}

		const int userID = ServerCommon::getUserID(username, password);

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectLastTimePlayedStmt FROM \"SELECT Timestamp FROM lastPlayTimes WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectTimePlayedStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectAchievementsStmt FROM \"SELECT IFNULL(COUNT(*), 0) FROM modAchievements WHERE ModID = ? AND UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectAllAchievementsStmt FROM \"SELECT IFNULL(COUNT(*), 0) FROM modAchievementList WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectScoreStmt FROM \"SELECT Score, Identifier, UserID FROM modScores WHERE ModID = ? ORDER BY Score DESC\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectScoreOrdersStmt FROM \"SELECT Identifier, ScoreOrder FROM scoreOrders WHERE ProjectID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectScoreNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM modScoreNames WHERE ModID = ? AND Identifier = ? AND Language = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectFeedbackMailStmt FROM \"SELECT Mail FROM feedbackMails WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectDiscussionsUrlStmt FROM \"SELECT CAST(Url AS BINARY) FROM discussionUrls WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectTypeStmt FROM \"SELECT Type FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (userID != -1) {
				if (!database.query("EXECUTE selectLastTimePlayedStmt USING @paramModID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				auto lastResults = database.getResults<std::vector<std::string>>();
				if (lastResults.empty()) {
					responseTree.put("LastTimePlayed", -1);
				} else {
					responseTree.put("LastTimePlayed", std::stoi(lastResults[0][0]));
				}
			} else {
				responseTree.put("LastTimePlayed", -1);
			}
			// get mod name in current language
			responseTree.put("ProjectID", projectID);
			responseTree.put("Name", ServerCommon::getProjectName(projectID, LanguageConverter::convert(language)));

			if (!database.query("EXECUTE selectTimePlayedStmt USING @paramModID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
			}
			auto results = database.getResults<std::vector<std::string>>();
			if (!results.empty() && userID != -1) {
				responseTree.put("Duration", std::stoi(results[0][0]));
			} else {
				responseTree.put("Duration", 0);
			}

			if (!database.query("EXECUTE selectTypeStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
			}
			results = database.getResults<std::vector<std::string>>();

			responseTree.put("Type", results[0][0]);
			
			if (!database.query("EXECUTE selectAllAchievementsStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
			}
			results = database.getResults<std::vector<std::string>>();
			if (results.empty()) {
				responseTree.put("AchievedAchievements", 0);
				responseTree.put("AllAchievements", 0);
			} else {
				responseTree.put("AllAchievements", std::stoi(results[0][0]));
				if (!database.query("EXECUTE selectAchievementsStmt USING @paramModID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
				}
				results = database.getResults<std::vector<std::string>>();
				if (results.empty()) {
					responseTree.put("AchievedAchievements", 0);
				} else {
					responseTree.put("AchievedAchievements", std::stoi(results[0][0]));
				}
			}
			if (!database.query("EXECUTE selectScoreOrdersStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
			}
			results = database.getResults<std::vector<std::string>>();

			std::map<int32_t, common::ScoreOrder> scoreOrders;

			for (const auto & vec : results) {
				scoreOrders.insert(std::make_pair(std::stoi(vec[0]), static_cast<common::ScoreOrder>(std::stoi(vec[1]))));
			}
			
			if (projectID == 339) { // Tri6: Infinite Demo
				do {
					MariaDBWrapper tri6Database;
					if (!tri6Database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, TRI6DATABASE, 0)) {
						std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!tri6Database.query("PREPARE selectTri6ScoreStmt FROM \"SELECT Score, Identifier, UserID FROM scores WHERE Version = ? ORDER BY Score DESC\";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!tri6Database.query("PREPARE selectTri6MaxVersionStmt FROM \"SELECT MAX(Version) FROM scores\";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!tri6Database.query("EXECUTE selectTri6MaxVersionStmt;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					auto lastResults = tri6Database.getResults<std::vector<std::string>>();

					const auto version = lastResults.empty() ? "0" : lastResults[0][0];

					if (!tri6Database.query("SET @paramVersion=" + version + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!tri6Database.query("EXECUTE selectTri6ScoreStmt USING @paramVersion;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					results = tri6Database.getResults<std::vector<std::string>>();
				} while (false);
			}
			else {
				if (!database.query("EXECUTE selectScoreStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
				}
				results = database.getResults<std::vector<std::string>>();
			}
			if (results.empty()) {
				responseTree.put("BestScore", 0);
				responseTree.put("BestScoreName", "");
				responseTree.put("BestScoreRank", -1);
			}
			else {
				std::map<int, std::vector<std::pair<int, int>>> scores;
				for (const auto & s : results) {
					scores[std::stoi(s[1])].push_back(std::make_pair(std::stoi(s[0]), std::stoi(s[2])));
				}

				for (auto & score : scores) {
					const auto it = scoreOrders.find(score.first);
					if (it != scoreOrders.end() && it->second == common::ScoreOrder::Ascending) {
						std::reverse(score.second.begin(), score.second.end());
					}
				}
				
				int bestScore = 0;
				std::string bestScoreName;
				int bestScoreRank = 0;
				int identifier = -1;
				for (auto & score : scores) {
					int realRank = 1;
					int lastScore = 0;
					int rank = 1;
					for (const auto & p : score.second) {
						if (lastScore != p.first) {
							rank = realRank;
						}
						if (p.second == userID) {
							if (rank < bestScoreRank || bestScoreRank == 0) {
								bestScore = p.first;
								bestScoreRank = rank;
								identifier = score.first;
								break;
							}
						}
						lastScore = p.first;
						realRank++;
					}
				}
				if (identifier != -1) {
					if (!database.query("SET @paramIdentifier=" + std::to_string(identifier) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
					}
					if (!database.query("EXECUTE selectScoreNameStmt USING @paramModID, @paramIdentifier, @paramLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
					}
					results = database.getResults<std::vector<std::string>>();
					if (results.empty()) {
						bestScore = 0;
						bestScoreRank = 0;
						bestScoreName = "";
					} else {
						bestScoreName = results[0][0];
					}
				}
				responseTree.put("BestScore", bestScore);
				responseTree.put("BestScoreName", bestScoreName);
				responseTree.put("BestScoreRank", bestScoreRank);
			}
			if (!database.query("EXECUTE selectFeedbackMailStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
			}
			results = database.getResults<std::vector<std::string>>();

			responseTree.put("FeedbackMailAvailable", !results.empty());
			if (!database.query("EXECUTE selectDiscussionsUrlStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
			}
			results = database.getResults<std::vector<std::string>>();

			responseTree.put("DiscussionUrl", results.empty() ? "" : results[0][0]);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestCompatibilityList(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto projectID = pt.get<int64_t>("ProjectID");

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectForbiddenStmt FROM \"SELECT DISTINCT PatchID FROM forbiddenPatches WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectCompatibilitiesStmt FROM \"SELECT DISTINCT PatchID FROM compatibilityList WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectCompatibilitiesValuesStmt FROM \"SELECT SUM(Compatible) AS UpVotes, COUNT(Compatible) AS Amount FROM compatibilityList WHERE ModID = ? AND PatchID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectForbiddenStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();

			if (!lastResults.empty()) {
				ptree forbbidenPatchesNodes;
				for (const auto & v : lastResults) {
					ptree forbiddenPatchNode;
					forbiddenPatchNode.put("", v[0][0]);
					forbbidenPatchesNodes.push_back(std::make_pair("", forbiddenPatchNode));
				}
				responseTree.add_child("ForbiddenPatches", forbbidenPatchesNodes);
			}

			if (!database.query("EXECUTE selectCompatibilitiesStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();
			
			if (!lastResults.empty()) {
				bool added = false;
				ptree impossiblePatchNodes;
				for (const auto & vec : lastResults) {
					if (!database.query("SET @paramPatchID=" + vec[0] + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!database.query("EXECUTE selectCompatibilitiesValuesStmt USING @paramModID, @paramPatchID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}

					auto results = database.getResults<std::vector<std::string>>();
					const int upVotes = std::stoi(results[0][0]);
					const int amount = std::stoi(results[0][1]);
					if (upVotes < amount / 2) { // if less then 50% of all users rated combination as compatible it is assumed to be incompatible
						ptree impossiblePatchNode;
						impossiblePatchNode.put("", vec[0]);
						impossiblePatchNodes.push_back(std::make_pair("", impossiblePatchNode));
						added = true;
					}
				}
				if (added) {
					responseTree.add_child("ForbiddenPatches", impossiblePatchNodes);
				}
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updatePlayTime(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto duration = pt.get<int32_t>("Duration");

		if (duration > 60 * 24 || duration < 0) {
			response->write(code);
			return;
		}
		
		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO playtimes (ModID, UserID, Duration) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE Duration = Duration + ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE insertSessionTimeStmt FROM \"INSERT INTO sessionTimes (ModID, Duration) VALUES (?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE deleteSessionInfosStmt FROM \"DELETE FROM userSessionInfos WHERE UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE deleteSettingsStmt FROM \"DELETE FROM userSettings WHERE UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectProjectPlayersStmt FROM \"SELECT UserID FROM playtimes WHERE ModID = ? AND UserID != -1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramDuration=" + std::to_string(duration) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}

			std::set<int> userList;

			for (const auto & v : pt.get_child("Projects")) {
				const auto data = v.second.data();
				
				if (!database.query("SET @paramModID=" + data + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE insertStmt USING @paramModID, @paramUserID, @paramDuration, @paramDuration;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE insertSessionTimeStmt USING @paramModID, @paramDuration;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("PREPARE insertLastPlaytimeStmt FROM \"INSERT INTO lastPlayTimes (ModID, UserID, Timestamp) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE Timestamp = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				
				const int timestamp = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
				if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE insertLastPlaytimeStmt USING @paramModID, @paramUserID, @paramTimestamp, @paramTimestamp;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}

				if (!database.query("EXECUTE selectProjectPlayersStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				const auto lastResults = database.getResults<std::vector<std::string>>();

				for (const auto & vec : lastResults) {
					userList.insert(std::stoi(vec[0]));
				}
			}
			if (!database.query("EXECUTE deleteSessionInfosStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE deleteSettingsStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}

			SpineLevel::clear(std::vector<int>(userList.begin(), userList.end()));
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestScores(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto projectID = pt.get<int64_t>("ProjectID");

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectStmt FROM \"SELECT Identifier, UserID, Score FROM modScores WHERE ModID = ? ORDER BY Score DESC\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectScoreOrderStmt FROM \"SELECT Identifier, ScoreOrder FROM scoreOrders WHERE ProjectID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectScoreOrderStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();

			std::map<int32_t, common::ScoreOrder> scoreOrders;

			for (const auto & vec : lastResults) {
				scoreOrders.insert(std::make_pair(std::stoi(vec[0]), static_cast<common::ScoreOrder>(std::stoi(vec[1]))));
			}
			
			if (!database.query("EXECUTE selectStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();

			std::map<int, std::vector<std::pair<std::string, int32_t>>> scores;
			for (const auto & vec : lastResults) {
				const auto identifier = static_cast<int32_t>(std::stoi(vec[0]));
				const int userID = std::stoi(vec[1]);
				const auto score = static_cast<int32_t>(std::stoi(vec[2]));
				std::string username = ServerCommon::getUsername(userID);
				if (!username.empty()) {
					scores[identifier].push_back(std::make_pair(username, score));
				}
			}
			ptree scoreNodes;
			for (auto & score : scores) {
				ptree scoreNode;

				const auto it = scoreOrders.find(score.first);
				if (it != scoreOrders.end() && it->second == common::ScoreOrder::Ascending) {
					std::reverse(score.second.begin(), score.second.end());
				}
				
				for (const auto & p : score.second) {
					ptree scoreEntryNode;
					scoreEntryNode.put("Username", p.first);
					scoreEntryNode.put("Score", p.second);

					scoreNode.push_back(std::make_pair("", scoreEntryNode));
				}
				scoreNodes.put("ID", score.first);
				scoreNodes.add_child("Scores", scoreNode);
			}
			responseTree.add_child("Scores", scoreNodes);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updateScore(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto identifier = pt.get<int32_t>("Identifier");
		const auto score = pt.get<int32_t>("Score");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		do {
			CONNECTTODATABASE(__LINE__)

			if (userID != -1) {
				if (!database.query("PREPARE selectStmt FROM \"SELECT * FROM modScoreList WHERE ModID = ? AND Identifier = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("PREPARE updateStmt FROM \"INSERT INTO modScores (ModID, UserID, Identifier, Score) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Score = ?\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("PREPARE selectCheaterStmt FROM \"SELECT UserID FROM cheaters WHERE UserID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("SET @paramIdentifier=" + std::to_string(identifier) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE selectCheaterStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				auto lastResults = database.getResults<std::vector<std::string>>();
				if (!lastResults.empty()) {
					break;
				}
				if (!database.query("EXECUTE selectStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				if (!lastResults.empty()) {
					if (!database.query("SET @paramScore=" + std::to_string(score) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!database.query("EXECUTE updateStmt USING @paramModID, @paramUserID, @paramIdentifier, @paramScore, @paramScore;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
				}

				SpineLevel::updateLevel(userID);
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::getReviews(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto projectID = pt.get<int64_t>("ProjectID");

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectStmt FROM \"SELECT UserID, Review, ReviewDate, PlayTime FROM reviews WHERE ProjectID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectRatingStmt FROM \"SELECT Rating FROM ratings WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectPlayTimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			const auto lastResults = database.getResults<std::vector<std::string>>();

			ptree reviewNodes;
			for (const auto & vec : lastResults) {
				if (vec[1].empty()) continue;
				
				if (!database.query("SET @paramUserID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE selectRatingStmt USING @paramProjectID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();
				
				ptree reviewNode;

				reviewNode.put("Username", ServerCommon::getUsername(std::stoi(vec[0])));
				reviewNode.put("Review", vec[1]);
				reviewNode.put("Date", vec[2]);
				reviewNode.put("ReviewDuration", vec[3]);
				reviewNode.put("Rating", results[0][0]);
				
				if (!database.query("EXECUTE selectPlayTimeStmt USING @paramProjectID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				results = database.getResults<std::vector<std::string>>();
				
				reviewNode.put("Duration", results[0][0]);
				
				reviewNodes.push_back(std::make_pair("", reviewNode));
			}
			responseTree.add_child("Reviews", reviewNodes);
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updateReview(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto review = pt.get<std::string>("Review");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE updateStmt FROM \"INSERT INTO reviews (ProjectID, UserID, Review, ReviewDate, PlayTime) VALUES (?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE Review = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectPlayTimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectFeedbackMailStmt FROM \"SELECT Mail FROM feedbackMails WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramReview='" + review + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			const int timestamp = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
			if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectPlayTimeStmt USING @paramProjectID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();
			
			if (!database.query("SET @paramDuration=" + lastResults[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			
			if (!database.query("EXECUTE updateStmt USING @paramProjectID, @paramUserID, @paramReview, @paramTimestamp, @paramDuration, @paramReview;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}

			SpineLevel::updateLevel(userID);
			
			if (!database.query("EXECUTE selectFeedbackMailStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();

			if (lastResults.empty()) break;

			const auto feedbackMail = lastResults[0][0];

			const std::string body = "The review of " + username + " was added/changed for your project '" + ServerCommon::getProjectName(projectID, common::Language::English) + "'";
			ServerCommon::sendMail("[SPINE] Review added or updated", body, "noreply@clockwork-origins.com", feedbackMail);
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto projectID = pt.get<int64_t>("ProjectID");
		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectStmt FROM \"SELECT Identifier FROM modAchievements WHERE ModID = ? AND UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectProgressMaxStmt FROM \"SELECT Identifier, Max FROM modAchievementProgressMax WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectProgressStmt FROM \"SELECT Current FROM modAchievementProgress WHERE ModID = ? AND UserID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectStmt USING @paramProjectID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();
			ptree achievementsNode;
			for (const auto & vec : lastResults) {
				const auto identifier = static_cast<int32_t>(std::stoi(vec[0]));

				ptree achievementNode;
				achievementNode.put("", identifier);
				achievementsNode.push_back(std::make_pair("", achievementNode));
			}
			if (!lastResults.empty()) {
				responseTree.add_child("Achievements", achievementsNode);
			}
			
			if (!database.query("EXECUTE selectProgressMaxStmt USING @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();

			ptree achievementProgressesNode;
			
			for (const auto & vec : lastResults) {
				if (!database.query("SET @paramIdentifier=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE selectProgressStmt USING @paramProjectID, @paramUserID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				auto result = database.getResults<std::vector<std::string>>();

				ptree achievementProgressNode;
				achievementProgressNode.put("Identifier", std::stoi(vec[0]));
				achievementProgressNode.put("Maximum", std::stoi(vec[1]));
				
				if (!result.empty()) {
					achievementProgressNode.put("Current", std::stoi(result[0][0]));
				}

				achievementProgressesNode.push_back(std::make_pair("", achievementProgressNode));
			}

			if (!lastResults.empty()) {
				responseTree.add_child("AchievementProgress", achievementProgressesNode);
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::unlockAchievement(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto identifier = pt.get<int32_t>("Identifier");
		auto duration = pt.get<int32_t>("Duration");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE updateStmt FROM \"INSERT INTO modAchievements (ModID, UserID, Identifier) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectStmt FROM \"SELECT * FROM modAchievementList WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectPlaytimeStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE insertAchievementTimeStmt FROM \"INSERT INTO achievementTimes (ModID, Identifier, Duration) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIdentifier=" + std::to_string(identifier) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectStmt USING @paramProjectID, @paramIdentifier;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();
			if (!lastResults.empty()) {
				if (!database.query("EXECUTE updateStmt USING @paramProjectID, @paramUserID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
			}
			if (!database.query("EXECUTE selectPlaytimeStmt USING @paramModID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();
			if (!lastResults.empty()) {
				duration += static_cast<uint32_t>(std::stoi(lastResults[0][0]));
			}
			if (!database.query("SET @paramDuration=" + std::to_string(duration) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE insertAchievementTimeStmt USING @paramProjectID, @paramIdentifier, @paramDuration;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}

			SpineLevel::updateLevel(userID);
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updateAchievementProgress(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto identifier = pt.get<int32_t>("Identifier");
		const auto progress = pt.get<int32_t>("Progress");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE updateProgressStmt FROM \"INSERT INTO modAchievementProgress (ModID, UserID, Identifier, Current) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Current = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIdentifier=" + std::to_string(identifier) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProgress=" + std::to_string(progress) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE updateProgressStmt USING @paramProjectID, @paramUserID, @paramIdentifier, @paramProgress, @paramProgress;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestOverallSaveData(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto projectID = pt.get<int64_t>("ProjectID");
		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectStmt FROM \"SELECT Entry, Value FROM overallSaveData WHERE UserID = ? AND ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectStmt USING @paramUserID, @paramProjectID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();
			ptree dataNodes;
			for (const auto & vec : lastResults) {
				ptree dataNode;
				dataNode.put("Key", vec[0]);
				dataNode.put("Value", vec[1]);

				dataNodes.push_back(std::make_pair("", dataNode));
			}
			if (!lastResults.empty()) {
				responseTree.add_child("Data", dataNodes);
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestAllFriends(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto friendsOnly = pt.count("FriendsOnly") == 1;

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			auto allUsers = ServerCommon::getUserList();
			
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectOwnFriendsStmt FROM \"SELECT FriendID FROM friends WHERE UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE checkIfFriendStmt FROM \"SELECT UserID FROM friends WHERE UserID = ? AND FriendID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectRequestsStmt FROM \"SELECT UserID FROM friends WHERE FriendID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectOwnFriendsStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();

			std::vector<common::Friend> friends;
			std::vector<common::Friend> friendRequests;
			std::vector<common::Friend> pendingFriends;
			
			for (const auto & vec : lastResults) {
				if (!database.query("SET @paramFriendID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE checkIfFriendStmt USING @paramFriendID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				std::string friendName = ServerCommon::getUsername(std::stoi(vec[0]));
				auto results = database.getResults<std::vector<std::string>>();
				if (results.empty()) {
					int32_t friendID = std::stoi(vec[0]);
					const auto sulm = SpineLevel::getLevel(friendID);
					pendingFriends.emplace_back(friendName, sulm.level);
				} else {
					int32_t friendID = std::stoi(vec[0]);
					const auto sulm = SpineLevel::getLevel(friendID);
					friends.emplace_back(friendName, sulm.level);
				}
				allUsers.erase(std::remove_if(allUsers.begin(), allUsers.end(), [friendName](const std::string & o) { return o == friendName; }), allUsers.end());
			}

			if (!friendsOnly) {
				if (!database.query("EXECUTE selectRequestsStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				for (const auto & vec : lastResults) {
					const std::string friendName = ServerCommon::getUsername(std::stoi(vec[0]));
					if (std::find_if(friends.begin(), friends.end(), [friendName](const common::Friend & o) { return o.name == friendName; }) == friends.end()) {
						int32_t friendID = std::stoi(vec[0]);
						const auto sulm = SpineLevel::getLevel(friendID);
						friendRequests.emplace_back(friendName, sulm.level);
						allUsers.erase(std::remove_if(allUsers.begin(), allUsers.end(), [friendName](const std::string & o) { return o == friendName; }), allUsers.end());
					}
				}

				ptree friendRequestNodes;
				if (!friendRequests.empty()) {
					for (const auto & f : friendRequests) {
						ptree friendNode;
						friendNode.put("Name", f.name);
						friendNode.put("Level", f.level);

						friendRequestNodes.push_back(std::make_pair("", friendNode));
					}
					responseTree.add_child("FriendRequests", friendRequestNodes);
				}

				ptree pendingFriendNodes;
				if (!pendingFriends.empty()) {
					for (const auto & f : pendingFriends) {
						ptree friendNode;
						friendNode.put("Name", f.name);
						friendNode.put("Level", f.level);

						pendingFriendNodes.push_back(std::make_pair("", friendNode));
					}
					responseTree.add_child("PendingFriends", pendingFriendNodes);
				}

				ptree userNodes;
				if (!allUsers.empty()) {
					for (const auto & name : allUsers) {
						ptree friendNode;
						friendNode.put("", name);

						userNodes.push_back(std::make_pair("", friendNode));
					}
					responseTree.add_child("Users", userNodes);
				}
			}

			std::sort(friends.begin(), friends.end(), [](const common::Friend & a, const common::Friend & b) {
				return a.name < b.name;
			});

			ptree friendNodes;
			if (!friends.empty()) {
				for (const auto & f : friends) {
					ptree friendNode;
					friendNode.put("Name", f.name);
					friendNode.put("Level", f.level);

					friendNodes.push_back(std::make_pair("", friendNode));
				}
				responseTree.add_child("Friends", friendNodes);
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updateChapterStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto identifier = pt.get<int32_t>("Identifier");
		const auto guild = pt.get<int32_t>("Guild");
		const auto key = pt.get<std::string>("Key");
		const auto value = pt.get<int32_t>("Value");

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO chapterStats (ModID, Identifier, Guild, StatName, StatValue) VALUES (?, ?, ?, CONVERT(? USING BINARY), ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramIdentifier=" + std::to_string(identifier) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramGuild=" + std::to_string(guild) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramStatName='" + key + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramStatValue=" + std::to_string(value) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE insertStmt USING @paramProjectID, @paramIdentifier, @paramGuild, @paramStatName, @paramStatValue;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::isAchievementUnlocked(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto achievementID = pt.get<int32_t>("AchievementID");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectStmt FROM \"SELECT UserID FROM modAchievements WHERE UserID = ? AND ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramAchievementID=" + std::to_string(achievementID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectStmt USING @paramUserID, @paramProjectID, @paramAchievementID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			const auto lastResults = database.getResults<std::vector<std::string>>();

			if (!lastResults.empty()) {
				responseTree.put("Unlocked", true);
			}			
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updateOfflineData(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE insertAchievementStmt FROM \"INSERT IGNORE INTO modAchievements (ModID, Identifier, UserID) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updateAchievementProgress FROM \"INSERT INTO modAchievementProgress (ModID, Identifier, UserID, Current) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Current = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updateScoresStmt FROM \"INSERT INTO modScores (ModID, Identifier, UserID, Score) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Score = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updateOverallSaveStmt FROM \"INSERT INTO overallSaveData (ModID, UserID, Entry, Value) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Value = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE updatePlayTimeStmt FROM \"INSERT INTO playtimes (ModID, UserID, Duration) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE Duration = Duration + ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (pt.count("Achievements") > 0) {
				for (const auto & v : pt.get_child("Achievements")) {
					const auto data = v.second;

					const auto projectID = data.get<int32_t>("ProjectID");
					const auto identifier = data.get<int32_t>("Identifier");

					if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramAchievementID=" + std::to_string(identifier) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("EXECUTE insertAchievementStmt USING @paramProjectID, @paramAchievementID, @paramUserID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}

					if (data.count("Progress") == 0) continue;

					const auto progress = data.get<int32_t>("Progress");

					if (!database.query("SET @paramProgress=" + std::to_string(progress) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
					if (!database.query("EXECUTE updateAchievementProgress USING @paramProjectID, @paramAchievementID, @paramUserID, @paramProgress, @paramProgress;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
				}
			}
			if (pt.count("Scores") > 0) {
				for (const auto & v : pt.get_child("Scores")) {
					const auto data = v.second;

					const auto projectID = data.get<int32_t>("ProjectID");
					const auto identifier = data.get<int32_t>("Identifier");
					const auto score = data.get<int32_t>("Score");

					if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramScoreID=" + std::to_string(identifier) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramScore=" + std::to_string(score) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("EXECUTE updateScoresStmt USING @paramProjectID, @paramScoreID, @paramUserID, @paramScore, @paramScore;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
				}
			}
			if (pt.count("OverallSaveData") > 0) {
				for (const auto & v : pt.get_child("OverallSaveData")) {
					const auto data = v.second;

					const auto projectID = data.get<int32_t>("ProjectID");
					const auto key = data.get<std::string>("Key");
					const auto value = data.get<std::string>("Value");

					if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramKey='" + key + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramValue='" + value + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("EXECUTE updateOverallSaveStmt USING @paramProjectID, @paramUserID, @paramKey, @paramValue, @paramValue;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
				}
			}
			if (pt.count("PlayTimes") > 0) {
				for (const auto & v : pt.get_child("PlayTimes")) {
					const auto data = v.second;

					const auto projectID = data.get<int32_t>("ProjectID");
					const auto time = data.get<int32_t>("Time");

					if (time > 60 * 24 || time < 0) continue;

					if (!database.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("SET @paramTime=" + std::to_string(time) + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						continue;
					}
					if (!database.query("EXECUTE updatePlayTimeStmt USING @paramProjectID, @paramUserID, @paramTime, @paramTime;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_bad_request;
						break;
					}
				}
			}

			SpineLevel::updateLevel(userID);
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestOfflineData(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectAchievementsStmt FROM \"SELECT ModID, Identifier FROM modAchievementList\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectOwnAchievementsStmt FROM \"SELECT ModID FROM modAchievements WHERE ModID = ? AND Identifier = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectModAchievementProgressStmt FROM \"SELECT Current FROM modAchievementProgress WHERE ModID = ? AND Identifier = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectModAchievementProgressMaxStmt FROM \"SELECT Max FROM modAchievementProgressMax WHERE ModID = ? AND Identifier = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectScoresStmt FROM \"SELECT ModID, Identifier, UserID, Score FROM modScores\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectScoreOrderStmt FROM \"SELECT Identifier, ScoreOrder FROM scoreOrders WHERE ProjectID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("PREPARE selectOverallSavesStmt FROM \"SELECT ModID, Entry, Value FROM overallSaveData WHERE UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE selectAchievementsStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();

			ptree achievementsNode;
			
			for (const auto & vec : lastResults) {
				ptree achievementNode;
				achievementNode.put("ProjectID", vec[0]);
				achievementNode.put("Identifier", vec[1]);
				
				if (!database.query("SET @paramModID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("SET @paramIdentifier=" + vec[1] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				if (!database.query("EXECUTE selectOwnAchievementsStmt USING @paramModID, @paramIdentifier, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				auto results = database.getResults<std::vector<std::string>>();

				if (!results.empty()) {
					achievementNode.put("Unlocked", true);
				}
				
				if (!database.query("EXECUTE selectModAchievementProgressStmt USING @paramModID, @paramIdentifier, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				results = database.getResults<std::vector<std::string>>();
				
				if (!results.empty()) {
					achievementNode.put("Progress", results[0][0]);
				}
				
				if (!database.query("EXECUTE selectModAchievementProgressMaxStmt USING @paramModID, @paramIdentifier;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_bad_request;
					break;
				}
				results = database.getResults<std::vector<std::string>>();
				
				if (!results.empty()) {
					achievementNode.put("Max", results[0][0]);
				}

				achievementsNode.push_back(std::make_pair("", achievementNode));
			}

			if (!lastResults.empty()) {
				responseTree.add_child("Achievements", achievementsNode);
			}
			
			if (!database.query("EXECUTE selectScoreOrderStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();

			std::map<int32_t, common::ScoreOrder> scoreOrders;

			for (const auto & vec : lastResults) {
				scoreOrders.insert(std::make_pair(std::stoi(vec[0]), static_cast<common::ScoreOrder>(std::stoi(vec[1]))));
			}
			
			if (!database.query("EXECUTE selectScoresStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();

			ptree scoresNode;
			
			for (const auto & vec : lastResults) {
				ptree scoreNode;
				scoreNode.put("ProjectID", vec[0]);
				scoreNode.put("Identifier", vec[1]);
				scoreNode.put("Username", ServerCommon::getUsername(std::stoi(vec[2])));
				scoreNode.put("Score", vec[3]);

				const auto it = scoreOrders.find(std::stoi(vec[1]));
				const auto scoreOrder = it != scoreOrders.end() ? it->second : common::ScoreOrder::Descending;
				
				scoreNode.put("Order", static_cast<int>(scoreOrder));

				scoresNode.push_back(std::make_pair("", scoreNode));
			}

			if (!lastResults.empty()) {
				responseTree.add_child("Scores", scoresNode);
			}
			
			if (!database.query("EXECUTE selectOverallSavesStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();

			ptree overallSaveDataNodes;
			
			for (const auto & vec : lastResults) {				
				ptree overallSaveDataNode;
				overallSaveDataNode.put("ProjectID", vec[0]);
				overallSaveDataNode.put("Key", vec[1]);
				overallSaveDataNode.put("Value", vec[2]);

				scoresNode.push_back(std::make_pair("", overallSaveDataNode));
			}

			if (!lastResults.empty()) {
				responseTree.add_child("OverallSaveData", overallSaveDataNodes);
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::friendRequest(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto friendName = pt.get<std::string>("Friend");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}
		
		const int friendID = ServerCommon::getUserID(friendName);

		if (friendID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO friends (UserID, FriendID) VALUES (?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramFriendID=" + std::to_string(friendID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE insertStmt USING @paramUserID, @paramFriendID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::feedback(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto text = pt.get<std::string>("Text");
		const auto projectID = pt.get<int>("ProjectID");
		const auto majorVersion = pt.get<int>("MajorVersion");
		const auto minorVersion = pt.get<int>("MinorVersion");
		const auto patchVersion = pt.get<int>("PatchVersion");

		std::string replyMail = "noreply@clockwork-origins.de";

		if (!username.empty()) {
			const int userID = ServerCommon::getUserID(username);

			MariaDBWrapper accountDatabase;
			do {
				if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
					std::cout << "Couldn't connect to database" << std::endl;
					break;
				}

				if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT Mail FROM accounts WHERE ID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!accountDatabase.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!accountDatabase.query("EXECUTE selectStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				const auto results = accountDatabase.getResults<std::vector<std::string>>();
				
				if (results.empty()) break;

				replyMail = results[0][0];
			} while (false);
		}

		if (projectID == -1) {
			ServerCommon::sendMail("[Spine] New Feedback arrived", ss.str() + "\n" + text + "\n" + std::to_string(majorVersion) + "." + std::to_string(minorVersion) + "." + std::to_string(patchVersion) + "\n\n" + username, replyMail);
		} else {
			MariaDBWrapper accountDatabase;
			do {
				if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
					std::cout << "Couldn't connect to database" << std::endl;
					break;
				}

				if (!accountDatabase.query("PREPARE selectMailStmt FROM \"SELECT Mail FROM feedbackMails WHERE ProjectID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!accountDatabase.query("PREPARE selectProjectNameStmt FROM \"SELECT Name FROM projectNames WHERE ProjectID = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!accountDatabase.query("SET @paramProjectID=" + std::to_string(projectID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!accountDatabase.query("EXECUTE selectMailStmt USING @paramProjectID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto results = accountDatabase.getResults<std::vector<std::string>>();

				if (results.empty()) break;

				const auto receiver = results[0][0];

				if (!accountDatabase.query("EXECUTE selectProjectNameStmt USING @paramProjectID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				results = accountDatabase.getResults<std::vector<std::string>>();

				if (results.empty()) break;

				const auto projectName = results[0][0];

				const auto title = "[Spine] Feedback for '" + projectName + "'!";

				const auto message = text + "\n\nPlayed Version: " + std::to_string(majorVersion) + "." + std::to_string(minorVersion) + "." + std::to_string(patchVersion);

				ServerCommon::sendMail(title, text, replyMail, receiver);
			} while (false);
		}

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::acceptFriend(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto friendName = pt.get<std::string>("Friend");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		const int friendID = ServerCommon::getUserID(friendName);

		if (friendID == -1 || userID == friendID) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO friends (UserID, FriendID) VALUES (?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramFriendID=" + std::to_string(friendID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE insertStmt USING @paramUserID, @paramFriendID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::declineFriend(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto friendName = pt.get<std::string>("Friend");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		const int friendID = ServerCommon::getUserID(friendName);

		if (friendID == -1 || userID == friendID) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM friends WHERE UserID = ? AND FriendID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("SET @paramFriendID=" + std::to_string(friendID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
			if (!database.query("EXECUTE deleteStmt USING @paramFriendID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_bad_request;
				break;
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::updateLoginTime(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		if (userID == -1) {
			response->write(SimpleWeb::StatusCode::client_error_bad_request);
			return;
		}

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO lastLoginTimes (UserID, Timestamp) VALUES (?, ?) ON DUPLICATE KEY UPDATE Timestamp = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			const int timestamp = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
			if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE insertStmt USING @paramUserID, @paramTimestamp, @paramTimestamp;")) {
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

void DatabaseServer::requestRandomPage(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto language = pt.get<std::string>("Language");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		int projectID = -1;

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectSpineModStmt FROM \"SELECT ModID FROM spinefeatures WHERE Features != 0\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectLanguageStmt FROM \"SELECT ProjectID FROM projectNames WHERE ProjectID = ? AND Languages & ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectRandomModStmt FROM \"SELECT ProjectID FROM projectNames WHERE Languages & ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectPlaytimesStmt FROM \"SELECT ModID FROM playtimes WHERE UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectEnabledStmt FROM \"SELECT Enabled FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage=" + std::to_string(LanguageConverter::convert(language)) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectSpineModStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();
			std::vector<int> ids;
			for (const auto & vec : lastResults) {
				if (!database.query("SET @paramModID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					continue;
				}
				if (!database.query("EXECUTE selectLanguageStmt USING @paramModID, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					continue;
				}
				auto results = database.getResults<std::vector<std::string>>();

				if (results.empty()) continue;

				if (!database.query("EXECUTE selectEnabledStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					continue;
				}
				auto results2 = database.getResults<std::vector<std::string>>();

				if (results2.empty()) continue;

				ids.push_back(std::stoi(results[0][0]));
			}

			std::vector<int> filteredIds = ids;
			if (userID != -1) {
				if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectPlaytimesStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();

				for (const auto & vec : lastResults) {
					const auto it = std::find_if(ids.begin(), ids.end(), [&vec](int id) {
						return id == std::stoi(vec[0]);
					});

					if (it != ids.end()) {
						ids.erase(it);
					}
				}
			}

			if (!filteredIds.empty()) {
				projectID = filteredIds[std::rand() % filteredIds.size()];
			} else if (!ids.empty()) {
				projectID = ids[std::rand() % ids.size()];
			} else {
				if (!database.query("EXECUTE selectRandomModStmt USING @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				if (!lastResults.empty()) {
					projectID = std::stoi(lastResults[std::rand() % lastResults.size()][0]);
				}
			}
		} while (false);

		responseTree.put("ID", projectID);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::requestInfoPage(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		std::stringstream responseStream;
		ptree responseTree;

		const auto username = pt.get<std::string>("Username");
		const auto password = pt.get<std::string>("Password");
		const auto language = pt.get<std::string>("Language");
		const auto projectID = pt.get<int32_t>("ProjectID");

		const int userID = ServerCommon::getUserID(username, password); // if userID is -1 user is not in database, so it's the play time of all unregistered players summed up

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE selectScreensStmt FROM \"SELECT Image, Hash FROM screens WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectDescriptionStmt FROM \"SELECT CAST(Description AS BINARY) FROM descriptions WHERE ModID = ? AND Language = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectFeaturesStmt FROM \"SELECT DISTINCT CAST(Feature AS BINARY) FROM features WHERE ModID = ? AND Language = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectSpineFeaturesStmt FROM \"SELECT Features FROM spinefeatures WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectTeamIDAndReleaseDateStmt FROM \"SELECT TeamID, ReleaseDate FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectTeamStmt FROM \"SELECT * FROM teammembers WHERE TeamID = ? AND UserID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectEditRightsStmt FROM \"SELECT * FROM editrights WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectModEnabledStmt FROM \"SELECT Enabled FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectEarlyAccessStmt FROM \"SELECT * FROM earlyUnlocks WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectOptionalPackagesStmt FROM \"SELECT PackageID FROM optionalpackages WHERE ModID = ? AND Enabled = 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectOptionalPackageNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM optionalpackagenames WHERE PackageID = ? AND Language = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion, Gothic FROM mods WHERE ModID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectUpdateDateStmt FROM \"SELECT Date FROM lastUpdated WHERE ProjectID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectNewsTickerStmt FROM \"SELECT NewsID, Date FROM newsticker WHERE ProjectID = ? AND Type = ? ORDER BY NewsID DESC\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectUpdateNewsStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion, SavegameCompatible FROM updateNews WHERE NewsID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectChangelogStmt FROM \"SELECT CAST(Changelog AS BINARY) FROM changelogs WHERE NewsID = ? AND Language = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}

			const auto projectName = ServerCommon::getProjectName(projectID, LanguageConverter::convert(language));

			if (!projectName.empty()) {
				responseTree.put("Name", projectName);
				
				if (!database.query("EXECUTE selectScreensStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				auto lastResults = database.getResults<std::vector<std::string>>();
				ptree screenshotNodes;
				for (const auto & p : lastResults) {
					ptree screenshotNode;
					screenshotNode.put("File", p[0]);
					screenshotNode.put("Hash", p[1]);

					screenshotNodes.push_back(std::make_pair("", screenshotNode));
				}
				if (!lastResults.empty()) {
					responseTree.add_child("Screenshots", screenshotNodes);
				}
				if (!database.query("EXECUTE selectDescriptionStmt USING @paramModID, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				if (!lastResults.empty()) {
					responseTree.put("Description", lastResults[0][0]);
				}
				if (!database.query("EXECUTE selectFeaturesStmt USING @paramModID, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();

				ptree featureNodes;
				
				for (const auto & v : lastResults) {
					ptree featureNode;
					featureNode.put("", v[0]);
					
					featureNodes.push_back(std::make_pair("", featureNode));
				}

				if (!lastResults.empty()) {
					responseTree.add_child("Features", featureNodes);
				}
				
				if (!database.query("EXECUTE selectSpineFeaturesStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				responseTree.put("SpineFeatures", lastResults.empty() ? 0 : std::stoi(lastResults[0][0]));

				if (!database.query("EXECUTE selectTeamIDAndReleaseDateStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();

				const auto releaseDate = std::stoi(lastResults[0][1]);
				
				responseTree.put("ReleaseDate", releaseDate);

				if (!database.query("SET @paramTeamID=" + lastResults[0][0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectTeamStmt USING @paramTeamID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();

				bool hasEditRights = !lastResults.empty() || userID == 3;
				
				if (!database.query("EXECUTE selectEditRightsStmt USING @paramModID, @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				hasEditRights |= !lastResults.empty();

				if (hasEditRights) {
					responseTree.put("EditRights", 1);
				}
				
				if (!database.query("EXECUTE selectVersionStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				if (lastResults.empty()) {
					break;
				}
				responseTree.put("MajorVersion", std::stoi(lastResults[0][0]));
				responseTree.put("MinorVersion", std::stoi(lastResults[0][1]));
				responseTree.put("PatchVersion", std::stoi(lastResults[0][2]));
				responseTree.put("GameType", std::stoi(lastResults[0][3]));

				if (!database.query("EXECUTE selectUpdateDateStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();

				const auto updateDate = lastResults.empty() ? 0 : std::stoi(lastResults[0][0]);
				responseTree.put("UpdateDate", std::max(releaseDate, updateDate));

				if (!database.query("EXECUTE selectModEnabledStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();
				auto installAllowed = !lastResults.empty() && std::stoi(lastResults[0][0]) == 1;
				if (!installAllowed) {
					if (!database.query("EXECUTE selectEarlyAccessStmt USING @paramModID, @paramUserID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					lastResults = database.getResults<std::vector<std::string>>();
					installAllowed = !lastResults.empty();
				}
				if (installAllowed) {
					if (!database.query("EXECUTE selectOptionalPackagesStmt USING @paramModID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}

					ptree packageNodes;
					
					lastResults = database.getResults<std::vector<std::string>>();
					for (const auto & vec : lastResults) {
						if (!database.query("SET @paramPackageID=" + vec[0] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE selectOptionalPackageNameStmt USING @paramPackageID, @paramLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						auto results = database.getResults<std::vector<std::string>>();
						if (!results.empty()) {
							ptree packageNode;
							packageNode.put("ID", std::stoi(vec[0]));
							packageNode.put("Name", vec[1]);
							
							packageNodes.push_back(std::make_pair("", packageNode));
						}
					}

					if (!packageNodes.empty()) {
						responseTree.add_child("Packages", packageNodes);
					}
					
					responseTree.put("InstallAllowed", 1);
				}

				if (!database.query("SET @paramType=" + std::to_string(static_cast<int>(common::NewsTickerType::Update)) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE selectNewsTickerStmt USING @paramModID, @paramType;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				lastResults = database.getResults<std::vector<std::string>>();

				ptree historyNodes;

				for (const auto & vec : lastResults) {
					ptree historyNode;
					
					historyNode.put("Timestamp", std::stoi(vec[1]));

					if (!database.query("SET @paramNewsID=" + vec[0] + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE selectUpdateNewsStmt USING @paramNewsID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					auto r = database.getResults<std::vector<std::string>>();

					if (r.empty()) continue;

					responseTree.put("MajorVersion", std::stoi(r[0][0]));
					responseTree.put("MinorVersion", std::stoi(r[0][1]));
					responseTree.put("PatchVersion", std::stoi(r[0][2]));
					responseTree.put("SaveCompatible", r[0][3] == "1" ? 1 : 0);

					if (!database.query("EXECUTE selectChangelogStmt USING @paramNewsID, @paramLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					r = database.getResults<std::vector<std::string>>();

					if (r.empty() && language != "English") {
						if (!database.query("SET @paramLanguage='English';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE selectChangelogStmt USING @paramNewsID, @paramLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						r = database.getResults<std::vector<std::string>>();
					}

					responseTree.put("Changelog", r.empty() ? "" : r[0][0]);

					historyNodes.push_back(std::make_pair("", historyNode));
				}

				if (!historyNodes.empty()) {
					responseTree.add_child("History", historyNodes);
				}
			}
		} while (false);

		write_json(responseStream, responseTree);

		response->write(code, responseStream.str());
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}

void DatabaseServer::submitInfoPage(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);

		SimpleWeb::StatusCode code = SimpleWeb::StatusCode::success_ok;

		const auto projectID = pt.get<int32_t>("ProjectID");
		const auto language = pt.get<std::string>("Language");
		const auto description = pt.get<std::string>("Description");
		const auto spineFeatures = pt.get<int32_t>("SpineFeatures");

		do {
			CONNECTTODATABASE(__LINE__)

			if (!database.query("PREPARE updateDescriptionStmt FROM \"INSERT INTO descriptions (ModID, Description, Language) VALUES (?, CONVERT(? USING BINARY), ?) ON DUPLICATE KEY UPDATE Description = CONVERT(? USING BINARY)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE removeFeaturesStmt FROM \"DELETE FROM features WHERE ModID = ? AND Language = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE insertFeatureStmt FROM \"INSERT INTO features (ModID, Feature, Language) VALUES (?, CONVERT(? USING BINARY), ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE updateSpineFeaturesStmt FROM \"INSERT INTO spinefeatures (ModID, Features) VALUES (?, ?) ON DUPLICATE KEY UPDATE Features = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE selectScreenStmt FROM \"SELECT Image, Hash FROM screens WHERE ModID = ?\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE deleteScreenStmt FROM \"DELETE FROM screens WHERE ModID = ? AND Image = ? AND Hash = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("PREPARE insertScreenStmt FROM \"INSERT INTO screens (ModID, Image, Hash) VALUES (?, ?, ?)\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramLanguage='" + language + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramModID=" + std::to_string(projectID) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("SET @paramDescription='" + description + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateDescriptionStmt USING @paramModID, @paramDescription, @paramLanguage, @paramDescription;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE removeFeaturesStmt USING @paramModID, @paramLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			for (const auto & v : pt.get_child("Features")) {
				if (!database.query("SET @paramFeature='" + v.second.data() + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("EXECUTE insertFeatureStmt USING @paramModID, @paramFeature, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
			}
			if (!database.query("SET @paramModules=" + std::to_string(spineFeatures) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE updateSpineFeaturesStmt USING @paramModID, @paramModules, @paramModules;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			if (!database.query("EXECUTE selectScreenStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				code = SimpleWeb::StatusCode::client_error_failed_dependency;
				break;
			}
			auto lastResults = database.getResults<std::vector<std::string>>();
			std::vector<std::pair<std::string, std::string>> images;
			images.reserve(lastResults.size());
			for (const auto & v : lastResults) {
				images.emplace_back(v[0], v[1]);
			}
			// compare images with received ones to delete old screens
			for (const auto & p : images) {
				bool found = false;
				if (pt.count("Screenshots") > 0) {
					for (const auto & p2 : pt.get_child("Screenshots")) {
						const auto file = p2.second.get<std::string>("File");
						const auto hash = p2.second.get<std::string>("Hash");

						if (p.first == file && p.second == hash) {
							// identical image, so skip
							found = true;
							break;
						}
					}
				}
				if (!found) { // remove deleted image
					if (!database.query("SET @paramImage='" + p.first + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("SET @paramHash='" + p.second + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					if (!database.query("EXECUTE deleteScreenStmt USING @paramModID, @paramImage, @paramHash;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						code = SimpleWeb::StatusCode::client_error_failed_dependency;
						break;
					}
					boost::filesystem::remove("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(projectID) + "/screens/" + p.first);
				}
			}
			if (pt.count("Screenshots") > 0) {
				for (const auto & p : pt.get_child("Screenshots")) {
					const auto file = p.second.get<std::string>("File");
					const auto hash = p.second.get<std::string>("Hash");
					
					bool found = false;
					for (const auto & p2 : images) {
						if (p2.first == file && p2.second == hash) {
							// identical image, so skip
							found = true;
							break;
						}
					}
					if (!found) { // remove deleted image
						if (!database.query("SET @paramImage='" + file + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("SET @paramHash='" + hash + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						if (!database.query("EXECUTE insertScreenStmt USING @paramModID, @paramImage, @paramHash;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
							code = SimpleWeb::StatusCode::client_error_failed_dependency;
							break;
						}
						boost::filesystem::create_directories("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(projectID) + "/screens/"); // ensure folder exists
					}
				}
			}
			if (pt.count("Images") > 0) {
				for (const auto & p : pt.get_child("Images")) {
					const auto file = p.second.get<std::string>("File");
					const auto data = p.second.get<std::string>("Data");
					
					std::ofstream out;
					out.open("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(projectID) + "/screens/" + file, std::ios::out | std::ios::binary);
					out.write(reinterpret_cast<const char *>(&data[0]), data.size());
					out.close();
				}
			}
		} while (false);

		response->write(code);
	} catch (...) {
		response->write(SimpleWeb::StatusCode::client_error_bad_request);
	}
}
