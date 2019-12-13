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

#include "ServerCommon.h"

#include <iostream>
#include <regex>
#include <thread>

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"
#include "Smtp.h"

namespace spine {
namespace server {

	std::string ServerCommon::convertString(const std::string & str) {
		std::string result = str;
		result = std::regex_replace(result, std::regex("%20"), " ");
		result = std::regex_replace(result, std::regex("%21"), "!");
		result = std::regex_replace(result, std::regex("%22"), "\"");
		result = std::regex_replace(result, std::regex("%23"), "#");
		result = std::regex_replace(result, std::regex("%24"), "$");
		result = std::regex_replace(result, std::regex("%25"), "%");
		result = std::regex_replace(result, std::regex("%26"), "&");
		result = std::regex_replace(result, std::regex("%27"), "'");
		result = std::regex_replace(result, std::regex("%28"), "(");
		result = std::regex_replace(result, std::regex("%29"), ")");
		result = std::regex_replace(result, std::regex("%2a"), "*");
		result = std::regex_replace(result, std::regex("%2b"), "+");
		result = std::regex_replace(result, std::regex("%2c"), ",");
		result = std::regex_replace(result, std::regex("%2d"), "-");
		result = std::regex_replace(result, std::regex("%2e"), ".");
		result = std::regex_replace(result, std::regex("%2f"), "/");
		result = std::regex_replace(result, std::regex("%3a"), ":");
		result = std::regex_replace(result, std::regex("%3b"), ";");
		result = std::regex_replace(result, std::regex("%3c"), "<");
		result = std::regex_replace(result, std::regex("%3d"), "=");
		result = std::regex_replace(result, std::regex("%3e"), ">");
		result = std::regex_replace(result, std::regex("%3f"), "?");
		result = std::regex_replace(result, std::regex("%40"), "@");
		result = std::regex_replace(result, std::regex("%5b"), "[");
		result = std::regex_replace(result, std::regex("%5c"), "\\");
		result = std::regex_replace(result, std::regex("%5d"), "]");
		result = std::regex_replace(result, std::regex("%5e"), "^");
		result = std::regex_replace(result, std::regex("%5f"), "_");
		result = std::regex_replace(result, std::regex("%7b"), "{");
		result = std::regex_replace(result, std::regex("%7d"), "}");
		return result;
	}

	int ServerCommon::getUserID(const std::string & username, const std::string & password) {
		MariaDBWrapper accountDatabase;
		if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
			std::cout << "Couldn't connect to database" << std::endl;
			return -1;
		}

		if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT ID FROM accounts WHERE Username = ? AND Password = PASSWORD(?) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return -1;
		}
		if (!accountDatabase.query("SET @paramUsername='" + username + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return -1;
		}
		if (!accountDatabase.query("SET @paramPassword='" + password + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return -1;
		}
		if (!accountDatabase.query("EXECUTE selectStmt USING @paramUsername, @paramPassword;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		}
		const auto results = accountDatabase.getResults<std::vector<std::string>>();

		return results.empty() ? -1 : std::stoi(results[0][0]);
	}

	int ServerCommon::getUserID(const std::string & username) {
		MariaDBWrapper accountDatabase;
		if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
			std::cout << "Couldn't connect to database" << std::endl;
			return -1;
		}

		if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT ID FROM accounts WHERE Username = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return -1;
		}
		if (!accountDatabase.query("SET @paramUsername='" + username + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return -1;
		}
		if (!accountDatabase.query("EXECUTE selectStmt USING @paramUsername;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		}
		const auto results = accountDatabase.getResults<std::vector<std::string>>();

		return results.empty() ? -1 : std::stoi(results[0][0]);
	}

	std::string ServerCommon::getUsername(const int id) {
		MariaDBWrapper accountDatabase;
		if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
			return "";
		}

		if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT Username FROM accounts WHERE ID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return "";
		}
		if (!accountDatabase.query("SET @paramID=" + std::to_string(id) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return "";
		}
		if (!accountDatabase.query("EXECUTE selectStmt USING @paramID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		}
		const auto results = accountDatabase.getResults<std::vector<std::string>>();

		return results.empty() ? "" : results[0][0];
	}

	std::vector<std::string> ServerCommon::getUserList() {
		MariaDBWrapper accountDatabase;
		if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
			return {};
		}

		if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT Username FROM accounts ORDER BY Username ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			return {};
		}
		if (!accountDatabase.query("EXECUTE selectStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		}
		const auto results = accountDatabase.getResults<std::vector<std::string>>();
		std::vector<std::string> users;
		users.reserve(results.size());
		for (auto vec : results) {
			users.push_back(vec[0]);
		}
		return users;
	}

	void ServerCommon::sendMail(const std::string & subject, const std::string & body, const std::string & replyTo) {
		std::thread([subject, body, replyTo]() {
			const Smtp s("127.0.0.1");
			const bool b = s.sendMail("contact@clockwork-origins.de", "bonne@clockwork-origins.de", subject, body, replyTo);
			static_cast<void>(b);
		}).detach();
	}
	
} /* namespace server */
} /* namespace spine */