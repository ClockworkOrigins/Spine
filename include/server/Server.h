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

#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {
namespace common {
	struct UpdateRequestMessage;
	struct UploadScreenshotsMessage;

	struct ProjectStats;
} /* namespace common */


namespace server {

	class DatabaseServer;
	class GMPServer;
	class ManagementServer;
	class MatchmakingServer;
	class UploadServer;

	class Server {
		friend class DatabaseServer;
	
	public:
		Server();
		~Server();

		int run();

	private:
		clockUtils::sockets::TcpSocket * _listenClient;
		clockUtils::sockets::TcpSocket * _listenMPServer;
		MatchmakingServer * _matchmakingServer;
		mutable std::mutex _newsLock;
		GMPServer * _gmpServer;
		UploadServer * _uploadServer;
		DatabaseServer * _databaseServer;
		ManagementServer * _managementServer;

		void accept(clockUtils::sockets::TcpSocket * sock);

		void receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error) const;

		void handleAutoUpdate(clockUtils::sockets::TcpSocket * sock, common::UpdateRequestMessage * msg) const;
		
		void handleUploadScreenshots(clockUtils::sockets::TcpSocket * sock, common::UploadScreenshotsMessage * msg) const;

		static bool isTeamMemberOfMod(int modID, int userID);

		static void getBestTri6Score(int userID, common::ProjectStats & projectStats);
	};

} /* namespace server */
} /* namespace spine */
