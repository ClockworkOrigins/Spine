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
	struct DownloadSucceededMessage;
	struct ModVersionCheckMessage;
	struct PackageDownloadSucceededMessage;
	struct RequestAllModsMessage;
	struct RequestAllNewsMessage;
	struct RequestModFilesMessage;
	struct RequestOwnCompatibilitiesMessage;
	struct RequestPackageFilesMessage;
	struct SubmitCompatibilityMessage;
	struct SubmitNewsMessage;
	struct SubmitScriptFeaturesMessage;
	struct UpdateRequestMessage;
	struct UpdateSucceededMessage;
	struct UploadScreenshotsMessage;

	struct ProjectStats;
} /* namespace common */

	class GMPServer;
	class MatchmakingServer;
	class UploadServer;

namespace server {

	class DatabaseServer;
	class DownloadSizeChecker;
	class ManagementServer;

	class Server {
		friend class DatabaseServer;
	
	public:
		Server();
		~Server();

		int run();

	private:
		clockUtils::sockets::TcpSocket * _listenClient;
		clockUtils::sockets::TcpSocket * _listenMPServer;
		DownloadSizeChecker * _downloadSizeChecker;
		MatchmakingServer * _matchmakingServer;
		mutable std::mutex _newsLock;
		GMPServer * _gmpServer;
		UploadServer * _uploadServer;
		DatabaseServer * _databaseServer;
		ManagementServer * _managementServer;

		void accept(clockUtils::sockets::TcpSocket * sock);

		void receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error) const;

		void handleAutoUpdate(clockUtils::sockets::TcpSocket * sock, common::UpdateRequestMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.32.0")]]
		void handleModListRequest(clockUtils::sockets::TcpSocket * sock, common::RequestAllModsMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.32.0")]]
		void handleModFilesListRequest(clockUtils::sockets::TcpSocket * sock, common::RequestModFilesMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.32.0")]]
		void handleDownloadSucceeded(clockUtils::sockets::TcpSocket * sock, common::DownloadSucceededMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleModVersionCheck(clockUtils::sockets::TcpSocket * sock, common::ModVersionCheckMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.32.0")]]
		void handleRequestPackageFiles(clockUtils::sockets::TcpSocket * sock, common::RequestPackageFilesMessage * msg) const;

		[[deprecated("Remove in Spine 1.32.0")]]
		void handlePackageDownloadSucceeded(clockUtils::sockets::TcpSocket * sock, common::PackageDownloadSucceededMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.32.0")]]
		void handleRequestAllNews(clockUtils::sockets::TcpSocket * sock, common::RequestAllNewsMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleSubmitNews(clockUtils::sockets::TcpSocket * sock, common::SubmitNewsMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleSubmitScriptFeatures(clockUtils::sockets::TcpSocket * sock, common::SubmitScriptFeaturesMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleSubmitCompatibility(clockUtils::sockets::TcpSocket * sock, common::SubmitCompatibilityMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleRequestOwnCompatibilities(clockUtils::sockets::TcpSocket * sock, common::RequestOwnCompatibilitiesMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleUpdateSucceeded(clockUtils::sockets::TcpSocket * sock, common::UpdateSucceededMessage * msg) const;
		
		void handleUploadScreenshots(clockUtils::sockets::TcpSocket * sock, common::UploadScreenshotsMessage * msg) const;

		static bool isTeamMemberOfMod(int modID, int userID);

		static void getBestTri6Score(int userID, common::ProjectStats & projectStats);
	};

} /* namespace server */
} /* namespace spine */
