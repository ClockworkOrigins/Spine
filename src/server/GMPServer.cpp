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

#include "GMPServer.h"

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

#include "common/MessageStructs.h"

#include "clockUtils/sockets/TcpSocket.h"

using namespace spine;

GMPServer::GMPServer() : _listenGMPServer(new clockUtils::sockets::TcpSocket()) {
}

GMPServer::~GMPServer() {
	delete _listenGMPServer;
}

int GMPServer::run() {
	if (_listenGMPServer->listen(MPSERVER_PORT, 10, true, std::bind(&GMPServer::handleRequestUserInfosMP, this, std::placeholders::_1)) != clockUtils::ClockError::SUCCESS) {
		return 1;
	}

	return 0;
}

void GMPServer::handleRequestUserInfosMP(clockUtils::sockets::TcpSocket * sock) const {
	{
		MariaDBWrapper spineDatabase;
		if (!spineDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
			delete sock;
			return;
		}

		if (!spineDatabase.query("PREPARE selectStmt FROM \"SELECT ModID FROM gmpWhitelist WHERE IP = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			delete sock;
			return;
		}
		if (!spineDatabase.query("SET @paramIP='" + sock->getRemoteIP() + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			delete sock;
			return;
		}
		if (!spineDatabase.query("EXECUTE selectStmt USING @paramIP;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		}
		std::vector<std::vector<std::string>> results = spineDatabase.getResults<std::vector<std::string>>();
		if (results.empty()) {
			delete sock;
			return;
		}
	}
	//std::cout << "Accepted connection with " << sock->getRemoteIP() << std::endl;
	std::string serialized;
	int ipLength = 0;
	int macLength = 0;
	std::string buffer;
	while (buffer.length() < size_t(8 + ipLength + macLength)) {
		if (clockUtils::ClockError::SUCCESS != sock->read(serialized)) {
			delete sock;
			return;
		}
		buffer += serialized;
		if (buffer.length() >= 4) {
			memcpy(&ipLength, buffer.c_str(), 4);
		}
		if (buffer.length() >= size_t(4 + 4 + ipLength)) {
			memcpy(&macLength, buffer.c_str() + 4 + ipLength, 4);
		}
		//std::cout << "Lengths: " << ipLength << " " << macLength << std::endl;
	}
	std::string ip = buffer.substr(4, ipLength);
	std::string mac = buffer.substr(8 + ipLength, macLength);
	for (char & i : mac) {
		if (i == '-') {
			i = ':';
		}
	}
	//std::cout << "Received IP: " << ip << std::endl;
	//std::cout << "Received MAC: " << mac << std::endl;
	enum MPErrorCode {
		Success,
		NotFound,
		Banned,
		GeneralError
	};
	enum MPCode {
		DontSendSettings = 0,
		SendSettings = 1
	};
	MPErrorCode errorCode = GeneralError;
	int userID = -1;
	std::string systemHash;
	std::string settingsHash;
	std::string settings;
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectUserIDStmt FROM \"SELECT UserID, Hash FROM userSessionInfos WHERE Mac = ? AND IP = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectUserIDReducedStmt FROM \"SELECT UserID, Hash FROM userSessionInfos WHERE Mac = ? AND IP LIKE ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectUserSettingsHashStmt FROM \"SELECT IFNULL(MD5(GROUP_CONCAT(Value)), '') FROM userSettings WHERE UserID = ? ORDER BY Value ASC LIMIT 1;\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectUserSettingsStmt FROM \"SELECT IFNULL(GROUP_CONCAT(CONCAT(Entry, ',', Value) SEPARATOR ';'), '') FROM userSettings WHERE UserID = ? ORDER BY Entry ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectHashBannedStmt FROM \"SELECT * FROM bannedHashes WHERE Hash = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectUserBannedStmt FROM \"SELECT * FROM bannedUsers WHERE UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("PREPARE selectProvisoricalBannedStmt FROM \"SELECT * FROM provisoricalBans WHERE UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("SET @paramMac='" + mac + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("SET @paramIP='" + ip + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		const std::string reducedIP = ip.substr(0, ip.find_last_of('.'));
		if (!database.query("SET @paramIPReduced='" + reducedIP + "%';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("EXECUTE selectUserIDStmt USING @paramMac, @paramIP;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			errorCode = GeneralError;
			break;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty()) {
			if (!database.query("EXECUTE selectUserIDReducedStmt USING @paramMac, @paramIPReduced;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				errorCode = GeneralError;
				break;
			}
			lastResults = database.getResults<std::vector<std::string>>();
			if (lastResults.empty()) {
				errorCode = NotFound;
				break;
			}
		}
		userID = std::stoi(lastResults[0][0]);
		systemHash = lastResults[0][1];
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("SET @paramHash='" + systemHash + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		if (!database.query("EXECUTE selectHashBannedStmt USING @paramHash;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) { // was banned
			break;
		}
		if (!database.query("EXECUTE selectUserBannedStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			errorCode = GeneralError;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) { // was banned
			errorCode = Banned;
			break;
		}
		/*if (!database.query("EXECUTE selectProvisoricalBannedStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			errorCode = GeneralError;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) { // was banned
			errorCode = Banned;
			break;
		}*/
		if (!database.query("EXECUTE selectUserSettingsHashStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			errorCode = GeneralError;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty() || lastResults[0][0].empty()) {
			errorCode = NotFound;
			break;
		}
		settingsHash = lastResults[0][0];
		if (!database.query("EXECUTE selectUserSettingsStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			errorCode = GeneralError;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty() || lastResults[0][0].empty()) {
			errorCode = NotFound;
			break;
		}
		settings = lastResults[0][0];
		settings += ";";
		errorCode = Success;
	} while (false);
	sock->write(&errorCode, 4);
	if (errorCode == Success) {
		sock->write(&userID, 4);
		int hashLength = systemHash.length();
		sock->write(&hashLength, 4);
		sock->write(systemHash);
		int settingsHashLength = settingsHash.length();
		sock->write(&settingsHashLength, 4);
		sock->write(settingsHash);
		buffer.clear();
		if (clockUtils::ClockError::SUCCESS != sock->read(serialized)) {
			delete sock;
			return;
		}
		MPCode mpCode = SendSettings;
		if (serialized.length() >= 4) {
			memcpy(&mpCode, serialized.c_str(), 4);
		}
		//std::cout << "MPCode: " << mpCode << std::endl;
		if (mpCode == MPCode::SendSettings) {
			int length = settings.length();
			sock->write(&length, 4);
			sock->write(settings);
		}
	}
	delete sock;
}
