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
#include "ManagementServer.h"
#include "MariaDBWrapper.h"
#include "MatchmakingServer.h"
#include "ServerCommon.h"
#include "SpineLevel.h"
#include "StatsCollector.h"
#include "TranslatorServer.h"
#include "UploadServer.h"

#include "common/MessageStructs.h"

#include "boost/filesystem.hpp"

#include "clockUtils/sockets/TcpSocket.h"

#include "tinyxml2.h"

using namespace spine::common;
using namespace spine::server;

Server::Server() : _listenClient(new clockUtils::sockets::TcpSocket()), _listenMPServer(new clockUtils::sockets::TcpSocket()), _matchmakingServer(new MatchmakingServer()), _gmpServer(new GMPServer()), _uploadServer(new UploadServer()), _databaseServer(new DatabaseServer()), _managementServer(new ManagementServer()), _translatorServer(new TranslatorServer()) {
	DatabaseCreator::createTables();

	DatabaseMigrator::migrate();

	SpineLevel::init();

	Cleanup::init();

#ifndef TEST_CONFIG
	FileSynchronizer::init();
#endif

	StatsCollector::init();

	DownloadSizeChecker::init();
}

Server::~Server() {
	_databaseServer->stop();
	_managementServer->stop();
	
	delete _listenClient;
	delete _listenMPServer;
	delete _matchmakingServer;
	delete _gmpServer;
	delete _uploadServer;
	delete _databaseServer;
	delete _managementServer;
	delete _translatorServer;
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
	_translatorServer->run();

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
			DownloadSizeChecker::clear();
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
			std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTeamIDStmt FROM \"SELECT TeamID FROM mods WHERE ModID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectMemberStmt FROM \"SELECT UserID FROM teammembers WHERE TeamID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectTeamIDStmt USING @paramModID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		if (lastResults.empty()) break;

		if (!database.query("SET @paramTeamID=" + lastResults[0][0] + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectMemberStmt USING @paramTeamID, @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
		std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("PREPARE selectScoreStmt FROM \"SELECT Score, Identifier, UserID FROM scores WHERE Version = ? ORDER BY Score DESC\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("PREPARE selectMaxVersionStmt FROM \"SELECT MAX(Version) FROM scores\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("EXECUTE selectMaxVersionStmt;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
		return;
	}
	auto lastResults = database.getResults<std::vector<std::string>>();

	const auto version = lastResults.empty() ? "0" : lastResults[0][0];
	
	if (!database.query("SET @paramVersion=" + version + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	
	if (!database.query("EXECUTE selectScoreStmt USING @paramVersion;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
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
