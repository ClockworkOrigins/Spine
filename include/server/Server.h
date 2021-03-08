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
	struct AcceptFriendRequestMessage;
	struct DeclineFriendRequestMessage;
	struct DownloadSucceededMessage;
	struct FeedbackMessage;
	struct IsAchievementUnlockedMessage;
	struct LinkClickedMessage;
	struct ModVersionCheckMessage;
	struct PackageDownloadSucceededMessage;
	struct RequestAchievementsMessage;
	struct RequestAllAchievementStatsMessage;
	struct RequestAllFriendsMessage;
	struct RequestAllModsMessage;
	struct RequestAllModStatsMessage;
	struct RequestAllNewsMessage;
	struct RequestAllScoreStatsMessage;
	struct RequestCompatibilityListMessage;
	struct RequestModFilesMessage;
	struct RequestInfoPageMessage;
	struct RequestModsForEditorMessage;
	struct RequestOfflineDataMessage;
	struct RequestOriginalFilesMessage;
	struct RequestOverallSaveDataMessage;
	struct RequestOwnCompatibilitiesMessage;
	struct RequestPackageFilesMessage;
	struct RequestPlayTimeMessage;
	struct RequestRandomModMessage;
	struct RequestScoresMessage;
	struct RequestSingleModStatMessage;
	struct RequestUserLevelMessage;
	struct SendAllFriendsMessage;
	struct SendFriendRequestMessage;
	struct SendInfoPageMessage;
	struct SendUserInfosMessage;
	struct SubmitCompatibilityMessage;
	struct SubmitInfoPageMessage;
	struct SubmitNewsMessage;
	struct SubmitRatingMessage;
	struct SubmitScriptFeaturesMessage;
	struct UnlockAchievementMessage;
	struct UpdateAchievementProgressMessage;
	struct UpdateChapterStatsMessage;
	struct UpdateLoginTimeMessage;
	struct UpdateOfflineDataMessage;
	struct UpdateOverallSaveDataMessage;
	struct UpdatePlayingTimeMessage;
	struct UpdatePlayTimeMessage;
	struct UpdateRequestMessage;
	struct UpdateScoreMessage;
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
		void handleModListRequest(clockUtils::sockets::TcpSocket * sock, common::RequestAllModsMessage * msg) const;
		void handleModFilesListRequest(clockUtils::sockets::TcpSocket * sock, common::RequestModFilesMessage * msg) const;
		void handleDownloadSucceeded(clockUtils::sockets::TcpSocket * sock, common::DownloadSucceededMessage * msg) const;
		
		void handleRequestPlaytime(clockUtils::sockets::TcpSocket * sock, common::RequestPlayTimeMessage * msg) const;

		[[deprecated("Remove in Spine 1.31.0")]]
		void handleModVersionCheck(clockUtils::sockets::TcpSocket * sock, common::ModVersionCheckMessage * msg) const;
		void handleRequestPackageFiles(clockUtils::sockets::TcpSocket * sock, common::RequestPackageFilesMessage * msg) const;
		void handlePackageDownloadSucceeded(clockUtils::sockets::TcpSocket * sock, common::PackageDownloadSucceededMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.30.0")]]
		void handleRequestAllModStats(clockUtils::sockets::TcpSocket * sock, common::RequestAllModStatsMessage * msg) const;

		[[deprecated("Remove in Spine 1.30.0")]]
		void handleRequestAllAchievementStats(clockUtils::sockets::TcpSocket * sock, common::RequestAllAchievementStatsMessage * msg) const;

		[[deprecated("Remove in Spine 1.30.0")]]
		void handleRequestAllScoreStats(clockUtils::sockets::TcpSocket * sock, common::RequestAllScoreStatsMessage * msg) const;
		void handleRequestAllNews(clockUtils::sockets::TcpSocket * sock, common::RequestAllNewsMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleSubmitNews(clockUtils::sockets::TcpSocket * sock, common::SubmitNewsMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleSubmitScriptFeatures(clockUtils::sockets::TcpSocket * sock, common::SubmitScriptFeaturesMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleSubmitCompatibility(clockUtils::sockets::TcpSocket * sock, common::SubmitCompatibilityMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleRequestOwnCompatibilities(clockUtils::sockets::TcpSocket * sock, common::RequestOwnCompatibilitiesMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.30.0")]]
		void handleSubmitRating(clockUtils::sockets::TcpSocket * sock, common::SubmitRatingMessage * msg) const;
		
		void handleRequestModsForEditor(clockUtils::sockets::TcpSocket * sock, common::RequestModsForEditorMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.30.0")]]
		void handleRequestUserLevel(clockUtils::sockets::TcpSocket * sock, common::RequestUserLevelMessage * msg) const;
		
		[[deprecated("Remove in Spine 1.31.0")]]
		void handleUpdateSucceeded(clockUtils::sockets::TcpSocket * sock, common::UpdateSucceededMessage * msg) const;
		
		void handleUploadScreenshots(clockUtils::sockets::TcpSocket * sock, common::UploadScreenshotsMessage * msg) const;

		static bool isTeamMemberOfMod(int modID, int userID);

		[[deprecated("Remove in Spine 1.30.0")]]
		void handleRequestAllTri6ScoreStats(clockUtils::sockets::TcpSocket * sock) const;
		static void getBestTri6Score(int userID, common::ProjectStats & projectStats);
	};

} /* namespace server */
} /* namespace spine */
