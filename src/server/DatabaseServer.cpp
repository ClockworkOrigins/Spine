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

#include "MariaDBWrapper.h"
#include "ServerCommon.h"
#include "SpineLevel.h"
#include "SpineServerConfig.h"

#define BOOST_SPIRIT_THREADSAFE
#include "boost/property_tree/json_parser.hpp"

using namespace boost::property_tree;

using namespace spine::server;

DatabaseServer::DatabaseServer() : _server(nullptr), _runner(nullptr) {
}

DatabaseServer::~DatabaseServer() {
	delete _server;
	delete _runner;
}

int DatabaseServer::run() {
	_server = new HttpsServer(SSLCHAINPATH, SSLPRIVKEYNPATH);
	_server->config.port = DATABASESERVER_PORT;
	_server->config.thread_pool_size = 4;
	
	_server->resource["^/getModnameForIDs"]["POST"] = std::bind(&DatabaseServer::getModnameForIDs, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getUserID"]["POST"] = std::bind(&DatabaseServer::getUserID, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getOwnRating"]["POST"] = std::bind(&DatabaseServer::getOwnRating, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getWeightedRating"]["POST"] = std::bind(&DatabaseServer::getWeightedRating, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getRatings"]["POST"] = std::bind(&DatabaseServer::getRatings, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/unlockAchievement"]["POST"] = std::bind(&DatabaseServer::unlockAchievement, this, std::placeholders::_1, std::placeholders::_2);
	_server->resource["^/getUserIDForDiscordID"]["POST"] = std::bind(&DatabaseServer::getUserIDForDiscordID, this, std::placeholders::_1, std::placeholders::_2);

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
				return;
			}

			ptree nameNodes;
			for (const auto & v : pt.get_child("ModIDs")) {
				if (!database.query("SET @paramModID=" + v.second.data() + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					return;
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
		
		const int32_t projectID = pt.get<int32_t>("ProjectID");

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
		
		const int32_t projectID = pt.get<int32_t>("ProjectID");

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
		
		const int32_t projectID = pt.get<int32_t>("ProjectID");

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

void DatabaseServer::unlockAchievement(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const {
	try {
		const std::string content = ServerCommon::convertString(request->content.string());

		std::stringstream ss(content);

		ptree pt;
		read_json(ss, pt);
		
		const int32_t projectID = pt.get<int32_t>("ProjectID");
		const int32_t userID = pt.get<int32_t>("UserID");
		const int32_t achievementID = pt.get<int32_t>("AchievementID");

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

		const int64_t discordAPI = pt.get<int64_t>("DiscordAPI");

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
