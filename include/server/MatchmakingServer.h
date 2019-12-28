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

#ifndef __SPINE_MATCHMAKINGSERVER_H__
#define __SPINE_MATCHMAKINGSERVER_H__

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {
namespace common {
	struct SearchMatchMessage;
} /* namespace common */

	struct GameSearch {
		int32_t modID {};
		int32_t identifier {};
		int32_t numPlayers {};
		std::set<clockUtils::sockets::TcpSocket *> members;
		std::string friendName;

		bool operator==(const GameSearch & other) const {
			return modID == other.modID && identifier == other.identifier && numPlayers == other.numPlayers;
		}
	};

	struct Game {
		std::set<clockUtils::sockets::TcpSocket *> members;
	};
	typedef std::shared_ptr<Game> GamePtr;

	class MatchmakingServer {
	public:
		MatchmakingServer();
		~MatchmakingServer();

		void socketError(clockUtils::sockets::TcpSocket * sock);
		void handleSearchMatch(clockUtils::sockets::TcpSocket * sock, common::SearchMatchMessage * msg);

	private:
		std::mutex _lock;
		std::map<clockUtils::sockets::TcpSocket *, std::string> _nameMapping;
		std::map<clockUtils::sockets::TcpSocket *, GameSearch> _socketSearch;
		std::vector<GameSearch> _gameSearches;
		std::map<clockUtils::sockets::TcpSocket *, GamePtr> _games;
		clockUtils::sockets::TcpSocket * _listenClient;

		void accept(clockUtils::sockets::TcpSocket * sock);
		void receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error);
		bool matchFriends(const std::string & username, const GameSearch & oldGame, const GameSearch & newGame) const;
	};

} /* namespace spine */

#endif /* __SPINE_MATCHMAKINGSERVER_H__ */
