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

#include "Server.h"

#include <fstream>
#include <map>
#include <set>
#include <thread>

#include "Cleanup.h"
#include "DatabaseCreator.h"
#include "DatabaseMigrator.h"
#include "DatabaseServer.h"
#include "DownloadSizeChecker.h"
#include "FileSynchronizer.h"
#include "GMPServer.h"
#include "LanguageConverter.h"
#include "ManagementServer.h"
#include "MariaDBWrapper.h"
#include "MatchmakingServer.h"
#include "ServerCommon.h"
#include "SpineLevel.h"
#include "StatsCollector.h"
#include "UploadServer.h"

#include "common/MessageStructs.h"
#include "common/NewsTickerTypes.h"

#include "boost/filesystem.hpp"

#include "clockUtils/sockets/TcpSocket.h"

#include "tinyxml2.h"

using namespace spine::common;
using namespace spine::server;

static const std::string DEFAULTURL = "https://clockwork-origins.de/Gothic/downloads/mods/";

Server::Server() : _listenClient(new clockUtils::sockets::TcpSocket()), _listenMPServer(new clockUtils::sockets::TcpSocket()), _downloadSizeChecker(new DownloadSizeChecker()), _matchmakingServer(new MatchmakingServer()), _gmpServer(new GMPServer()), _uploadServer(new UploadServer()), _databaseServer(new DatabaseServer(_downloadSizeChecker)), _managementServer(new ManagementServer()) {
	DatabaseCreator::createTables();

	DatabaseMigrator::migrate();

	SpineLevel::init();

	Cleanup::init();

	FileSynchronizer::init();

	StatsCollector::init();
}

Server::~Server() {
	_databaseServer->stop();
	_managementServer->stop();
	
	delete _listenClient;
	delete _listenMPServer;
	delete _downloadSizeChecker;
	delete _matchmakingServer;
	delete _gmpServer;
	delete _uploadServer;
	delete _databaseServer;
	delete _managementServer;
}

int Server::run() {
	if (_listenClient->listen(SERVER_PORT, 10, true, std::bind(&Server::accept, this, std::placeholders::_1)) != clockUtils::ClockError::SUCCESS) {
		return 1;
	}
	_uploadServer->run();
#ifndef TEST_CONFIG
	_gmpServer->run();
#endif
	_databaseServer->run();
	_managementServer->run();

	bool running = true;

	while (running) {
		std::cout << "Possible actions:" << std::endl;
		std::cout << "\tx:\t\tshutdown server" << std::endl;
		std::cout << "\tc:\t\tclear download sizes" << std::endl;

		const int c = getchar();

		switch (c) {
		case 'x': {
			running = false;
			break;
		}
		case 'c': {
			_downloadSizeChecker->clear();
			break;
		}
		default: {
			break;
		}
		}
	}

	return 0;
}

void Server::accept(clockUtils::sockets::TcpSocket * sock) {
	sock->receiveCallback(std::bind(&Server::receiveMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void Server::receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error) const {
	if (error != clockUtils::ClockError::SUCCESS) {
		std::thread([sock]() {
			sock->close();
			delete sock;
		}).detach();
	} else {
		try {
			Message * m = Message::DeserializeBlank(std::string(message.begin(), message.end()));
			if (!m) {
				m = Message::DeserializePrivate(std::string(message.begin(), message.end()));
				if (!m) {
					sock->writePacket("Crash"); // it's a hack to stop hanging threads
					return;
				}
			}
			if (m->type == MessageType::UPDATEREQUEST) {
				auto * msg = dynamic_cast<UpdateRequestMessage *>(m);
				handleAutoUpdate(sock, msg);
			} else if (m->type == MessageType::REQUESTALLMODS) {
				auto * msg = dynamic_cast<RequestAllModsMessage *>(m);
				handleModListRequest(sock, msg);
			} else if (m->type == MessageType::REQUESTMODFILES) {
				auto * msg = dynamic_cast<RequestModFilesMessage *>(m);
				handleModFilesListRequest(sock, msg);
			} else if (m->type == MessageType::DOWNLOADSUCCEEDED) {
				auto * msg = dynamic_cast<DownloadSucceededMessage *>(m);
				handleDownloadSucceeded(sock, msg);
			} else if (m->type == MessageType::REQUESTPACKAGEFILES) {
				auto * msg = dynamic_cast<RequestPackageFilesMessage *>(m);
				handleRequestPackageFiles(sock, msg);
			} else if (m->type == MessageType::PACKAGEDOWNLOADSUCCEEDED) {
				auto * msg = dynamic_cast<PackageDownloadSucceededMessage *>(m);
				handlePackageDownloadSucceeded(sock, msg);
			} else if (m->type == MessageType::REQUESTALLNEWS) {
				auto * msg = dynamic_cast<RequestAllNewsMessage *>(m);
				handleRequestAllNews(sock, msg);
			} else if (m->type == MessageType::UPLOADSCREENSHOTS) {
				auto * msg = dynamic_cast<UploadScreenshotsMessage *>(m);
				handleUploadScreenshots(sock, msg);
			} else if (m->type == MessageType::UPLOADACHIEVEMENTICONS) {
				auto * msg = dynamic_cast<UploadAchievementIconsMessage *>(m);
				_managementServer->uploadAchievementIcons(msg);
			} else {
				delete m;
				return;
			}

			delete m;
		} catch (const boost::archive::archive_exception &) {
			std::cerr << "deserialization not working" << std::endl;
			sock->writePacket("Crash"); // it's a hack to stop hanging threads
			return;
		} catch (std::system_error & err) {
			std::cout << "System Error: " << err.what() << std::endl;
		} catch (std::exception & err) {
			std::cout << "Exception: " << err.what() << std::endl;
		} catch (...) {
			std::cout << "Unhandled exception" << std::endl;
		}
	}
}

void Server::handleAutoUpdate(clockUtils::sockets::TcpSocket * sock, UpdateRequestMessage * msg) const {
	// 1. read xml
	tinyxml2::XMLDocument doc;

	const tinyxml2::XMLError e = doc.LoadFile("Spine_Version.xml");

	if (e) {
		std::cerr << "Couldn't open xml file!" << std::endl;
		UpdateFileCountMessage ufcm;
		ufcm.count = 0;

		const std::string serialized = ufcm.SerializeBlank();

		sock->writePacket(serialized);
		return;
	}

	uint32_t lastVersion = (msg->majorVersion << 16) + (msg->minorVersion << 8) + msg->patchVersion;

	std::map<uint32_t, std::vector<std::pair<std::string, std::string>>> versions;
	versions.insert(std::make_pair(lastVersion, std::vector<std::pair<std::string, std::string>>()));

	auto * const rootNode = doc.FirstChildElement("Versions");

	if (rootNode != nullptr) {
		for (tinyxml2::XMLElement * node = rootNode->FirstChildElement("Version"); node != nullptr; node = node->NextSiblingElement("Version")) {
			const uint8_t majorVersion = static_cast<uint8_t>(std::stoi(node->Attribute("majorVersion")));
			const uint8_t minorVersion = static_cast<uint8_t>(std::stoi(node->Attribute("minorVersion")));
			const uint8_t patchVersion = static_cast<uint8_t>(std::stoi(node->Attribute("patchVersion")));
			uint32_t version = (majorVersion << 16) + (minorVersion << 8) + patchVersion;

			versions.insert(std::make_pair(version, std::vector<std::pair<std::string, std::string>>()));

			auto it = versions.find(version);

			for (tinyxml2::XMLElement * file = node->FirstChildElement("File"); file != nullptr; file = file->NextSiblingElement("File")) {
				it->second.emplace_back(file->GetText(), file->Attribute("Hash"));
			}
		}
	}

	// 2. find all changed files
	std::set<std::pair<std::string, std::string>> files;

	auto it = versions.find(lastVersion);
	if (it != versions.end()) {
		++it;
		for (; it != versions.end(); ++it) {
			for (const std::pair<std::string, std::string> & file : it->second) {
				files.insert(file);
			}
			lastVersion = it->first;
		}
	}

	// 3. send file count
	UpdateFilesMessage ufm;
	for (const auto & p : files) {
		ufm.files.push_back(p);
	}
	const std::string serialized = ufm.SerializeBlank();
	sock->writePacket(serialized);
}

void Server::handleModListRequest(clockUtils::sockets::TcpSocket * sock, RequestAllModsMessage * msg) const {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	std::map<int32_t, std::string> teamNames;
	{
		if (!database.query(std::string("SELECT TeamID FROM teams;"))) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		if (!database.query("PREPARE selectTeamnameStmt FROM \"SELECT CAST(Name AS BINARY) FROM teamNames WHERE TeamID = ? AND Languages & ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramLanguage=" + std::to_string(LanguageConverter::convert(msg->language)) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramFallbackLanguage=" + std::to_string(English) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		for (const auto & vec : lastResults) {
			const auto id = static_cast<int32_t>(std::stoi(vec[0]));
			
			if (!database.query("SET @paramTeamID=" + std::to_string(id) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				return;
			}
			if (!database.query("EXECUTE selectTeamnameStmt USING @paramTeamID, @paramLanguage;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				return;
			}
			auto results = database.getResults<std::vector<std::string>>();
			if (!results.empty()) {
				teamNames.insert(std::make_pair(id, results[0][0]));
			} else {
				if (!database.query("EXECUTE selectTeamnameStmt USING @paramTeamID, @paramFallbackLanguage;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					return;
				}
				results = database.getResults<std::vector<std::string>>();
				if (!results.empty()) {
					teamNames.insert(std::make_pair(id, results[0][0]));
				}
			}
		}
	}
	
	if (!database.query("PREPARE selectUpdateDateStmt FROM \"SELECT Date FROM lastUpdated WHERE ProjectID = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectPlayedProjectsStmt FROM \"SELECT ModID FROM playtimes WHERE Duration > 0 AND UserID = ? AND UserID != -1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	
	// ModID | Name of the Mod shown in GUI | ID of the team | Enabled in GUI or only internal | 1 or 2 either if the mod is for Gothic 1 or 2 | Release date encoded as integer in days | one of Mod or Patch encoded as integer | major version | minor version | patch version
	if (!database.query(std::string("SELECT ModID, TeamID, Gothic, ReleaseDate, Type, MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods WHERE Enabled = 1;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	
	auto enabledResults = database.getResults<std::vector<std::string>>();
	const int userID = ServerCommon::getUserID(msg->username, msg->password);
	if (userID != -1) {
		if (!database.query(std::string("SELECT ModID, TeamID, Gothic, ReleaseDate, Type, MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods WHERE Enabled = 0;"))) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto disabledResults = database.getResults<std::vector<std::string>>();
		if (!database.query("PREPARE selectIsTeammemberStmt FROM \"SELECT UserID FROM teammembers WHERE TeamID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectEarlyUnlockStmt FROM \"SELECT * FROM earlyUnlocks WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		for (auto vec : disabledResults) {
			if (!database.query("SET @paramTeamID=" + vec[1] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				return;
			}
			if (!database.query("EXECUTE selectIsTeammemberStmt USING @paramTeamID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				return;
			}
			auto allowed = database.getResults<std::vector<std::string>>();
			if (!allowed.empty()) {
				enabledResults.push_back(vec);
				continue;
			}
			if (!database.query("SET @paramModID=" + vec[0] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				return;
			}
			if (!database.query("EXECUTE selectEarlyUnlockStmt USING @paramModID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				return;
			}
			allowed = database.getResults<std::vector<std::string>>();
			if (!allowed.empty()) {
				enabledResults.push_back(vec);
			}
		}
	}
	UpdateAllModsMessage uamm;
	if (!database.query("PREPARE selectModnameStmt FROM \"SELECT Languages, CAST(Name AS BINARY) FROM projectNames WHERE ProjectID = ?\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectDevtimeStmt FROM \"SELECT Duration FROM devtimes WHERE ModID = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectTimeStmt FROM \"SELECT IFNULL(SUM(Duration), 0), IFNULL(COUNT(Duration), 0) FROM playtimes WHERE ModID = ? AND UserID != -1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	std::map<int32_t, uint32_t> versions;

	const Language clientLanguage = LanguageConverter::convert(msg->language);
	const Language defaultLanguage = Language::English;
	
	for (auto vec : enabledResults) {
		Mod mod;
		mod.id = std::stoi(vec[0]);
		if (!database.query("SET @paramModID=" + vec[0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectModnameStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto results = database.getResults<std::vector<std::string>>();

		if (results.empty()) continue;

		std::map<int, std::string> names;

		mod.supportedLanguages = 0;

		for (const auto & vec2 : results) {
			const auto l = static_cast<Language>(std::stoi(vec2[0]));
			const auto n = vec2[1];

			names.insert(std::make_pair(l, n));

			mod.supportedLanguages |= l;
		}

		Language l = Language::None;
		
		auto nameIt = std::find_if(names.begin(), names.end(), [clientLanguage](const std::pair<int, std::string> & p) {
			return p.first & clientLanguage;
		});

		if (nameIt == names.end()) {
			nameIt = std::find_if(names.begin(), names.end(), [=](const std::pair<int, std::string> & p) {
				return p.first & defaultLanguage;
			});

			if (nameIt == names.end()) {
				nameIt = names.begin();

				l = nameIt->first & German ? German : nameIt->first & English ? English : nameIt->first & Polish ? Polish : nameIt->first & Russian ? Russian : None;
			} else {
				l = defaultLanguage;
			}
		} else {
			l = clientLanguage;
		}

		mod.name = nameIt->second;
		mod.teamID = std::stoi(vec[1]);
		const auto it = teamNames.find(mod.teamID);
		if (it != teamNames.end()) {
			mod.teamName = it->second;
		}
		mod.gothic = static_cast<GameType>(std::stoi(vec[2]));
		mod.releaseDate = std::stoi(vec[3]);
		mod.type = static_cast<ModType>(std::stoi(vec[4]));
		mod.majorVersion = static_cast<int8_t>(std::stoi(vec[5]));
		mod.minorVersion = static_cast<int8_t>(std::stoi(vec[6]));
		mod.patchVersion = static_cast<int8_t>(std::stoi(vec[7]));
		mod.spineVersion = static_cast<int8_t>(std::stoi(vec[8]));
		if (!database.query("EXECUTE selectDevtimeStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			continue;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			mod.devDuration = std::stoi(results[0][0]);
		} else {
			mod.devDuration = -1;
		}
		if (!database.query("EXECUTE selectTimeStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			continue;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			const int sumDurations = std::stoi(results[0][0]);
			const int countDurations = std::stoi(results[0][1]);
			if (countDurations >= 1) {
				mod.avgDuration = sumDurations / countDurations;
			} else {
				mod.avgDuration = -1;
			}
		} else {
			mod.avgDuration = -1;
		}
		uint32_t version = (mod.majorVersion << 24) + (mod.minorVersion << 16) + (mod.patchVersion << 8) + mod.spineVersion;
		mod.downloadSize = _downloadSizeChecker->getBytes(mod.id, LanguageConverter::convert(l), version);

		if (!database.query("EXECUTE selectUpdateDateStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		results = database.getResults<std::vector<std::string>>();

		const uint32_t updateDate = results.empty() ? 0 : std::stoi(results[0][0]);
		mod.updateDate = std::max(mod.releaseDate, updateDate);

		mod.language = l;
		
		uamm.mods.push_back(mod);
		versions.insert(std::make_pair(mod.id, version));
	}
	
	if (!database.query("EXECUTE selectPlayedProjectsStmt USING @paramUserID")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	const auto results = database.getResults<std::vector<std::string>>();

	for (const auto & vec : results) {
		const int32_t projectID = std::stoi(vec[0]);
		
		uamm.playedProjects.push_back(projectID);
	}
	
	std::string serialized = uamm.SerializePrivate();
	sock->writePacket(serialized);

	// optional packages

	// PackageID | ModID | Enabled
	if (!database.query(std::string("SELECT * FROM optionalpackages WHERE Enabled = 1;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	enabledResults = database.getResults<std::vector<std::string>>();
	if (userID != -1) {
		if (!database.query(std::string("SELECT * FROM optionalpackages WHERE Enabled = 0;"))) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto disabledResults = database.getResults<std::vector<std::string>>();
		if (!database.query("PREPARE selectEarlyUnlockStmt FROM \"SELECT * FROM earlyUnlocks WHERE ModID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		for (auto vec : disabledResults) {
			if (!database.query("SET @paramModID=" + vec[1] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				return;
			}
			if (!database.query("EXECUTE selectEarlyUnlockStmt USING @paramModID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				return;
			}
			auto allowed = database.getResults<std::vector<std::string>>();
			if (!allowed.empty()) {
				enabledResults.push_back(vec);
			}
		}
	}
	UpdatePackageListMessage uplm;
	if (!database.query("PREPARE selectOptionalNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM optionalpackagenames WHERE PackageID = ? AND Language = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	
	if (!database.query("SET @paramLanguage='" + msg->language + "';")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}

	for (auto vec : enabledResults) {
		UpdatePackageListMessage::Package package;
		package.packageID = std::stoi(vec[0]);
		package.modID = std::stoi(vec[1]);
		if (!database.query("SET @paramPackageID=" + vec[0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectOptionalNameStmt USING @paramPackageID, @paramLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		const auto nameResults = database.getResults<std::vector<std::string>>();
		if (nameResults.empty() || nameResults[0].empty()) {
			continue;
		}
		package.name = nameResults[0][0];
		package.downloadSize = _downloadSizeChecker->getBytesForPackage(package.modID, package.packageID, msg->language, versions[package.modID]);
		uplm.packages.push_back(package);
	}
	
	serialized = uplm.SerializePrivate();
	sock->writePacket(serialized);
}

void Server::handleModFilesListRequest(clockUtils::sockets::TcpSocket * sock, RequestModFilesMessage * msg) const {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	{
		if (!database.query("PREPARE selectStmt FROM \"SELECT Path, Hash FROM modfiles WHERE ModID = ? AND (Language = ? OR Language = 'All')\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectFileserverStmt FROM \"SELECT Url FROM fileserver WHERE ModID = ? AND MajorVersion = ? AND MinorVersion = ? AND PatchVersion = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramModID=" + std::to_string(msg->modID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramLanguage='" + msg->language + "';")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectStmt USING @paramModID, @paramLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		ListModFilesMessage lmfm;
		for (auto vec : lastResults) {
			lmfm.fileList.emplace_back(vec[0], vec[1]);
		}
		if (!database.query("EXECUTE selectVersionStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty()) {
			return;
		}
		const std::string majorVersion = lastResults[0][0];
		const std::string minorVersion = lastResults[0][1];
		const std::string patchVersion = lastResults[0][2];
		if (!database.query("SET @paramMajorVersion=" + majorVersion + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramMinorVersion=" + minorVersion + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramPatchVersion=" + patchVersion + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectFileserverStmt USING @paramModID, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		std::vector<std::string> possibilities = { DEFAULTURL };
		for (auto vec : lastResults) {
			possibilities.push_back(vec[0]);
		}
		lmfm.fileserver = possibilities[std::rand() % possibilities.size()];
		const std::string serialized = lmfm.SerializePrivate();
		sock->writePacket(serialized);
	}
}

void Server::handleDownloadSucceeded(clockUtils::sockets::TcpSocket *, DownloadSucceededMessage * msg) const {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	{
		if (!database.query("PREPARE insertStmt FROM \"INSERT INTO downloads (ModID, Counter) VALUES (?, 1) ON DUPLICATE KEY UPDATE Counter = Counter + 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramModID=" + std::to_string(msg->modID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE insertStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		if (!database.query("PREPARE insertPerVersionStmt FROM \"INSERT INTO downloadsPerVersion (ModID, Version, Counter) VALUES (?, CONVERT(? USING BINARY), 1) ON DUPLICATE KEY UPDATE Counter = Counter + 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectVersionStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			const std::string version = results[0][0] + "." + results[0][1] + "." + results[0][2];
			if (!database.query("SET @paramVersion='" + version + "';")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				return;
			}
			if (!database.query("EXECUTE insertPerVersionStmt USING @paramModID, @paramVersion;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				return;
			}
		}
	}
}

void Server::handleRequestPackageFiles(clockUtils::sockets::TcpSocket * sock, RequestPackageFilesMessage * msg) const {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	{
		if (!database.query("PREPARE selectStmt FROM \"SELECT Path, Hash FROM optionalpackagefiles WHERE PackageID = ? AND (Language = ? OR Language = 'All')\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectModIDStmt FROM \"SELECT ModID FROM optionalpackages WHERE PackageID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("PREPARE selectFileserverStmt FROM \"SELECT Url FROM fileserver WHERE ModID = ? AND MajorVersion = ? AND MinorVersion = ? AND PatchVersion = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramPackageID=" + std::to_string(msg->packageID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramLanguage='" + msg->language + "';")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectStmt USING @paramPackageID, @paramLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		ListModFilesMessage lmfm;
		for (auto vec : lastResults) {
			lmfm.fileList.emplace_back(vec[0], vec[1]);
		}
		if (!database.query("EXECUTE selectModIDStmt USING @paramPackageID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty()) {
			return;
		}
		if (!database.query("SET @paramModID=" + lastResults[0][0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectVersionStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty()) {
			return;
		}
		const std::string majorVersion = lastResults[0][0];
		const std::string minorVersion = lastResults[0][1];
		const std::string patchVersion = lastResults[0][2];
		if (!database.query("SET @paramMajorVersion=" + majorVersion + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramMinorVersion=" + minorVersion + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramPatchVersion=" + patchVersion + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE selectFileserverStmt USING @paramModID, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		std::vector<std::string> possibilities = { DEFAULTURL };
		for (auto vec : lastResults) {
			possibilities.push_back(vec[0]);
		}
		lmfm.fileserver = possibilities[std::rand() % possibilities.size()];
		const std::string serialized = lmfm.SerializePrivate();
		sock->writePacket(serialized);
	}
}

void Server::handlePackageDownloadSucceeded(clockUtils::sockets::TcpSocket *, PackageDownloadSucceededMessage * msg) const {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	{
		if (!database.query("PREPARE insertStmt FROM \"INSERT INTO packagedownloads (PackageID, Counter) VALUES (?, 1) ON DUPLICATE KEY UPDATE Counter = Counter + 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("SET @paramPackageID=" + std::to_string(msg->packageID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			return;
		}
		if (!database.query("EXECUTE insertStmt USING @paramPackageID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			return;
		}
	}
}

void Server::handleRequestAllNews(clockUtils::sockets::TcpSocket * sock, RequestAllNewsMessage * msg) const {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query("PREPARE selectNews FROM \"SELECT NewsID, CAST(Title AS BINARY), CAST(Body AS BINARY), Timestamp FROM news WHERE Language = ? AND NewsID > ?\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectModRefsStmt FROM \"SELECT ModID FROM newsModReferences WHERE NewsID = ?\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectImagesStmt FROM \"SELECT File, Hash FROM newsImageReferences WHERE NewsID = ?\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectNewsTicker FROM \"SELECT NewsID, Type, ProjectID, Date FROM newsticker ORDER BY NewsID DESC\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectUpdateNews FROM \"SELECT MajorVersion, MinorVersion, PatchVersion FROM updateNews WHERE NewsID = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("SET @paramLanguage='" + msg->language + "';")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("SET @paramLanguageI=" + std::to_string(LanguageConverter::convert(msg->language)) + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("SET @paramEnglishLanguageI=" + std::to_string(English) + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("SET @paramFallbackLanguageI=0;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("SET @paramFallbackLanguage='English';")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("SET @paramLastNewsID=" + std::to_string(msg->lastNewsID) + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("EXECUTE selectNews USING @paramLanguage, @paramLastNewsID;")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	auto lastResults = database.getResults<std::vector<std::string>>();
	SendAllNewsMessage sanm;
	for (auto vec : lastResults) {
		News news;
		news.id = std::stoi(vec[0]);
		news.title = vec[1];
		news.body = vec[2];
		news.timestamp = std::stoi(vec[3]);

		if (!database.query("SET @paramNewsID=" + vec[0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			continue;
		}
		if (!database.query("EXECUTE selectModRefsStmt USING @paramNewsID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			continue;
		}
		auto results = database.getResults<std::vector<std::string>>();
		for (auto modID : results) {
			news.referencedMods.emplace_back(std::stoi(modID[0]), ServerCommon::getProjectName(std::stoi(modID[0]), LanguageConverter::convert(msg->language)));
		}
		if (!database.query("EXECUTE selectImagesStmt USING @paramNewsID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			continue;
		}
		results = database.getResults<std::vector<std::string>>();
		for (auto p : results) {
			news.imageFiles.emplace_back(p[0], p[1]);
		}
		sanm.news.push_back(news);
	}
	
	if (!database.query("EXECUTE selectNewsTicker;")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	lastResults = database.getResults<std::vector<std::string>>();

	for (const auto & vec : lastResults) {
		const auto type = static_cast<NewsTickerType>(std::stoi(vec[1]));
		const auto projectID = std::stoi(vec[2]);
		const auto timestamp = std::stoi(vec[3]);

		NewsTicker nt;
		nt.type = type;
		nt.projectID = projectID;
		nt.timestamp = timestamp;

		nt.name = ServerCommon::getProjectName(std::stoi(vec[2]), LanguageConverter::convert(msg->language));

		if (nt.name.empty()) continue;

		if (type == NewsTickerType::Update) {		
			if (!database.query("SET @paramNewsID=" + vec[0] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				continue;
			}
			if (!database.query("EXECUTE selectUpdateNews USING @paramNewsID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				continue;
			}
			const auto r = database.getResults<std::vector<std::string>>();

			if (r.empty()) continue; // this mustn't happen, but let's be cautious

			nt.majorVersion = static_cast<int8_t>(std::stoi(r[0][0]));
			nt.minorVersion = static_cast<int8_t>(std::stoi(r[0][1]));
			nt.patchVersion = static_cast<int8_t>(std::stoi(r[0][2]));
		} else if (type == NewsTickerType::Release) {
			// nothing special here
		} else {
			continue;
		}

		sanm.newsTicker.push_back(nt);
	}

	const std::string serialized = sanm.SerializePrivate();
	sock->writePacket(serialized);
}

void Server::handleUploadScreenshots(clockUtils::sockets::TcpSocket *, UploadScreenshotsMessage * msg) const {
	do {
		const int userID = ServerCommon::getUserID(msg->username, msg->password);
		
		if (userID == -1) break;

		boost::filesystem::create_directories("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(msg->projectID) + "/screens/"); // ensure folder exists
		
		for (const auto & p : msg->screenshots) {
			std::ofstream out;
			out.open("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(msg->projectID) + "/screens/" + p.first, std::ios::out | std::ios::binary);
			out.write(reinterpret_cast<const char *>(&p.second[0]), p.second.size());
			out.close();
		}
	} while (false);
}

bool Server::isTeamMemberOfMod(int modID, int userID) {
	if (userID == 3) return true;
	
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectTeamIDStmt FROM \"SELECT TeamID FROM mods WHERE ModID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectMemberStmt FROM \"SELECT UserID FROM teammembers WHERE TeamID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectTeamIDStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty()) break;

		if (!database.query("SET @paramTeamID=" + lastResults[0][0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectMemberStmt USING @paramTeamID, @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();

		return !lastResults.empty();
	} while (false);

	return false;
}

void Server::getBestTri6Score(int userID, ProjectStats & projectStats) {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, TRI6DATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query("PREPARE selectScoreStmt FROM \"SELECT Score, Identifier, UserID FROM scores WHERE Version = ? ORDER BY Score DESC\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("PREPARE selectMaxVersionStmt FROM \"SELECT MAX(Version) FROM scores\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	if (!database.query("EXECUTE selectMaxVersionStmt;")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	auto lastResults = database.getResults<std::vector<std::string>>();

	const auto version = lastResults.empty() ? "0" : lastResults[0][0];
	
	if (!database.query("SET @paramVersion=" + version + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
		return;
	}
	
	if (!database.query("EXECUTE selectScoreStmt USING @paramVersion;")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	lastResults = database.getResults<std::vector<std::string>>();
	if (lastResults.empty()) {
		projectStats.bestScore = 0;
		projectStats.bestScoreName = "";
		projectStats.bestScoreRank = -1;
	} else {
		std::map<int, std::vector<std::pair<int, int>>> scores;
		for (auto s : lastResults) {
			scores[std::stoi(s[1])].push_back(std::make_pair(std::stoi(s[0]), std::stoi(s[2])));
		}
		projectStats.bestScore = 0;
		projectStats.bestScoreName = "";
		projectStats.bestScoreRank = 0;
		int identifier = -1;
		for (auto & score : scores) {
			int rank = 1;
			int lastRank = 1;
			int lastScore = 0;
			for (const auto & p : score.second) {
				if (lastScore != p.first) {
					rank = lastRank;
				}
				if (p.second == userID) {
					if (rank < projectStats.bestScoreRank || projectStats.bestScoreRank == 0 || (projectStats.bestScoreRank == rank && projectStats.bestScore < p.first)) {
						projectStats.bestScore = p.first;
						projectStats.bestScoreRank = rank;
						identifier = score.first;
						break;
					}
				}
				lastScore = p.first;
				lastRank++;
			}
		}
		if (identifier != -1) {
			static std::map<int, std::string> scoreNames = {
				{ 0, "Hyperion" },
				{ 1, "Hermes" },
				{ 2, "Helios" }
			};
			
			projectStats.bestScoreName = scoreNames[identifier];
		}
	}
}
