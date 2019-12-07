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

#define BOOST_SPIRIT_THREADSAFE
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
		
			const std::string username = pt.get<std::string>("username");
			const std::string password = pt.get<std::string>("password");

			const int userID = ServerCommon::getUserID(username, password);

			if (userID == -1) {
				response->write(SimpleWeb::StatusCode::client_error_unauthorized);
				return;
			}
			
			const std::string language = pt.get<std::string>("language");

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
				if (!database.query("PREPARE selectStmt FROM \"SELECT Name FROM modnames WHERE ModID = ? AND Language = ? LIMIT 1\";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					code = SimpleWeb::StatusCode::client_error_failed_dependency;
					break;
				}
				if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramLanguage='" + language + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE selectTeamsStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
					break;
				}
				auto lastResults = database.getResults<std::vector<std::string>>();
				
				ptree modNodes;
				for (auto vec : lastResults) {
					if (!database.query("SET @paramTeamID=" + vec[0] + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectModStmt USING @paramTeamID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
						break;
					}
					auto results = database.getResults<std::vector<std::string>>();
					for (auto mod : results) {
						if (!database.query("SET @paramModID=" + mod[0] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE selectModNameStmt USING @paramModID, @paramLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
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

} /* namespace server */
} /* namespace spine */
