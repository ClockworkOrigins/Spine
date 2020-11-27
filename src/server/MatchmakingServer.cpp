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

#include "MatchmakingServer.h"

#include <thread>

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

#include "common/MessageStructs.h"

#include "clockUtils/sockets/TcpSocket.h"

using namespace spine;

MatchmakingServer::MatchmakingServer() : _listenClient(new clockUtils::sockets::TcpSocket()) {
	_listenClient->listen(SPINE_MP_PORT, 10, true, std::bind(&MatchmakingServer::accept, this, std::placeholders::_1));
}

MatchmakingServer::~MatchmakingServer() {
	delete _listenClient;
}

void MatchmakingServer::socketError(clockUtils::sockets::TcpSocket * sock) {
	std::lock_guard<std::mutex> lg(_lock);
	_nameMapping.erase(sock);
	{
		const auto it = _socketSearch.find(sock);
		if (it != _socketSearch.end()) {
			for (auto & _gameSearche : _gameSearches) {
				if (_gameSearche == it->second) {
					_gameSearche.members.erase(sock);
					break;
				}
			}
			_socketSearch.erase(it);
		}
	}
	{
		auto it = _games.find(sock);
		if (it != _games.end()) {
			it->second->members.erase(sock);
			_games.erase(it);
		}
	}
}

void MatchmakingServer::handleSearchMatch(clockUtils::sockets::TcpSocket * sock, common::SearchMatchMessage * msg) {
	// first check if given mod id is valid!
	MariaDBWrapper spineDatabase;
	if (!spineDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << std::endl;
		return;
	}

	if (!spineDatabase.query("PREPARE selectModIDStmt FROM \"SELECT ModID FROM multiplayerMods WHERE ModID = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		return;
	}
	if (!spineDatabase.query("SET @paramModID=" + std::to_string(msg->modID) + ";")) {
		std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		return;
	}
	if (!spineDatabase.query("EXECUTE selectModIDStmt USING @paramModID;")) {
		std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
		return;
	}
	const auto lastResults = spineDatabase.getResults<std::vector<std::string>>();
	if (lastResults.empty()) {
		std::cout << "Rejected MP Mod: " << msg->modID << std::endl;
		return;
	}

	std::lock_guard<std::mutex> lg(_lock);
	_nameMapping[sock] = msg->username;
	GameSearch gs;
	gs.modID = msg->modID;
	gs.identifier = msg->identifier;
	gs.numPlayers = msg->numPlayers;
	gs.friendName = msg->friendName;
	std::cout << "Looking for match " << sock->getRemoteIP() << " " << gs.modID << " " << gs.identifier << " " << gs.numPlayers << " " << gs.friendName << std::endl;
	bool found = false;
	for (auto & _gameSearche : _gameSearches) {
		if (_gameSearche == gs && matchFriends(msg->username, _gameSearche, gs)) {
			_gameSearche.members.insert(sock);
			gs = _gameSearche;
			found = true;
			break;
		}
	}
	if (!found) {
		gs.members.insert(sock);
		_gameSearches.push_back(gs);
	}
	_socketSearch[sock] = gs;
	if (static_cast<int32_t>(gs.members.size()) == gs.numPlayers) { // found a match
		std::cout << "Found match for " << sock->getRemoteIP() << " " << gs.modID << " " << gs.identifier << " " << gs.numPlayers << std::endl;
		std::vector<std::string> usernames;
		GamePtr game = std::make_shared<Game>();
		game->members = gs.members;
		for (clockUtils::sockets::TcpSocket * s : gs.members) {
			usernames.push_back(_nameMapping[s]);
			_games.insert(std::make_pair(s, game));
			std::cout << "Player #" << usernames.size() << ": " << _nameMapping[s] << std::endl;
		}
		for (clockUtils::sockets::TcpSocket * s : gs.members) {
			common::FoundMatchMessage fmm;
			fmm.users = usernames;
			const std::string serialized = fmm.SerializeBlank();
			s->writePacket(serialized);
		}
		for (clockUtils::sockets::TcpSocket * s : gs.members) {
			_socketSearch.erase(s);
			_nameMapping.erase(s);
		}
		for (auto it = _gameSearches.begin(); it != _gameSearches.end(); ++it) {
			if ((*it) == gs) {
				_gameSearches.erase(it);
				break;
			}
		}
	}
}

void MatchmakingServer::accept(clockUtils::sockets::TcpSocket * sock) {
	sock->receiveCallback(std::bind(&MatchmakingServer::receiveMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void MatchmakingServer::receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error) {
	if (error != clockUtils::ClockError::SUCCESS) {
		socketError(sock);
		std::thread([sock]() {
			sock->close();
			delete sock;
		}).detach();
	} else {
		{
			std::lock_guard<std::mutex> lg(_lock);
			auto it = _games.find(sock);
			if (it != _games.end()) {
				for (clockUtils::sockets::TcpSocket * gameSocks : it->second->members) {
					if (gameSocks != sock) {
						gameSocks->writePacket(message);
					}
				}
				return;
			}
		}
		try {
			common::Message * m = common::Message::DeserializeBlank(std::string(message.begin(), message.end()));
			if (!m) {
				sock->writePacket("Crash"); // it's a hack to stop hanging threads
				return;
			}
			if (m->type == common::MessageType::SEARCHMATCH) {
				auto * msg = dynamic_cast<common::SearchMatchMessage *>(m);
				handleSearchMatch(sock, msg);
			} else {
				std::cerr << "unexpected control message arrived: " << static_cast<int>(m->type) << std::endl;
				delete m;
				return;
			}

			delete m;
		} catch (const boost::archive::archive_exception &) {
			std::cerr << "deserialization not working" << std::endl;
			sock->writePacket("Crash"); // it's a hack to stop hanging threads
			return;
		}
	}
}

bool MatchmakingServer::matchFriends(const std::string & username, const GameSearch & oldGame, const GameSearch & newGame) const {
	bool ret = oldGame.friendName.empty() || oldGame.friendName == username;
	if (ret && !newGame.friendName.empty()) {
		ret = false;
		for (clockUtils::sockets::TcpSocket * sock : oldGame.members) {
			const auto it = _nameMapping.find(sock);
			if (it != _nameMapping.end()) {
				const std::string name = it->second;
				if (name == newGame.friendName) {
					ret = true;
					break;
				}
			}
		}
	}
	return ret;
}
