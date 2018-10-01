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
// Copyright 2018 Clockwork Origins

#ifndef __SPINE_SERVER_H__
#define __SPINE_SERVER_H__

#include <cstdint>
#include <mutex>
#include <string>
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
	struct RequestModManagementMessage;
	struct RequestModsForEditorMessage;
	struct RequestOfflineDataMessage;
	struct RequestOriginalFilesMessage;
	struct RequestOverallSaveDataMessage;
	struct RequestOwnCompatibilitiesMessage;
	struct RequestPackageFilesMessage;
	struct RequestPlayTimeMessage;
	struct RequestRandomModMessage;
	struct RequestRatingMessage;
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
	struct UpdateAchievementsMessage;
	struct UpdateChapterStatsMessage;
	struct UpdateEarlyAccessStateMessage;
	struct UpdateGeneralModConfigurationMessage;
	struct UpdateImpressionMessage;
	struct UpdateLoginTimeMessage;
	struct UpdateModVersionMessage;
	struct UpdateOfflineDataMessage;
	struct UpdateOverallSaveDataMessage;
	struct UpdatePlayingTimeMessage;
	struct UpdatePlayTimeMessage;
	struct UpdateRequestMessage;
	struct UpdateRequestEncryptedMessage;
	struct UpdateScoreMessage;
	struct UpdateScoresMessage;
	struct UpdateStartTimeMessage;
	struct UpdateSucceededMessage;
} /* namespace common */

	class DownloadSizeChecker;
	class GMPServer;
	class MatchmakingServer;
	class UploadServer;

	class Server {
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

		void accept(clockUtils::sockets::TcpSocket * sock);

		void receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error) const;

		void handleAutoUpdate(clockUtils::sockets::TcpSocket * sock, common::UpdateRequestMessage * msg) const;
		void handleModListRequest(clockUtils::sockets::TcpSocket * sock, common::RequestAllModsMessage * msg) const;
		void handleModFilesListRequest(clockUtils::sockets::TcpSocket * sock, common::RequestModFilesMessage * msg) const;
		void handleDownloadSucceeded(clockUtils::sockets::TcpSocket * sock, common::DownloadSucceededMessage * msg) const;
		void handleUpdatePlaytime(clockUtils::sockets::TcpSocket * sock, common::UpdatePlayTimeMessage * msg) const;
		void handleRequestPlaytime(clockUtils::sockets::TcpSocket * sock, common::RequestPlayTimeMessage * msg) const;
		void handleRequestScores(clockUtils::sockets::TcpSocket * sock, common::RequestScoresMessage * msg) const;
		void handleUpdateScore(clockUtils::sockets::TcpSocket * sock, common::UpdateScoreMessage * msg) const;
		void handleRequestAchievements(clockUtils::sockets::TcpSocket * sock, common::RequestAchievementsMessage * msg) const;
		void handleUnlockAchievement(clockUtils::sockets::TcpSocket * sock, common::UnlockAchievementMessage * msg) const;
		void handleModVersionCheck(clockUtils::sockets::TcpSocket * sock, common::ModVersionCheckMessage * msg) const;
		void handleFeedback(clockUtils::sockets::TcpSocket * sock, common::FeedbackMessage * msg) const;
		void handleRequestOriginalFiles(clockUtils::sockets::TcpSocket * sock, common::RequestOriginalFilesMessage * msg) const;
		void handleUpdateLoginTime(clockUtils::sockets::TcpSocket * sock, common::UpdateLoginTimeMessage * msg) const;
		void handleRequestPackageFiles(clockUtils::sockets::TcpSocket * sock, common::RequestPackageFilesMessage * msg) const;
		void handlePackageDownloadSucceeded(clockUtils::sockets::TcpSocket * sock, common::PackageDownloadSucceededMessage * msg) const;
		void handleRequestAllModStats(clockUtils::sockets::TcpSocket * sock, common::RequestAllModStatsMessage * msg) const;
		void handleRequestSingleModStat(clockUtils::sockets::TcpSocket * sock, common::RequestSingleModStatMessage * msg) const;
		void handleRequestAllAchievementStats(clockUtils::sockets::TcpSocket * sock, common::RequestAllAchievementStatsMessage * msg) const;
		void handleRequestAllScoreStats(clockUtils::sockets::TcpSocket * sock, common::RequestAllScoreStatsMessage * msg) const;
		void handleRequestAllNews(clockUtils::sockets::TcpSocket * sock, common::RequestAllNewsMessage * msg) const;
		void handleSubmitNews(clockUtils::sockets::TcpSocket * sock, common::SubmitNewsMessage * msg) const;
		void handleLinkClicked(clockUtils::sockets::TcpSocket * sock, common::LinkClickedMessage * msg) const;
		void handleSubmitScriptFeatures(clockUtils::sockets::TcpSocket * sock, common::SubmitScriptFeaturesMessage * msg) const;
		void handleRequestInfoPage(clockUtils::sockets::TcpSocket * sock, common::RequestInfoPageMessage * msg) const;
		void handleSubmitInfoPage(clockUtils::sockets::TcpSocket * sock, common::SubmitInfoPageMessage * msg) const;
		void handleSendUserInfos(clockUtils::sockets::TcpSocket * sock, common::SendUserInfosMessage * msg) const;
		void handleRequestRandomMod(clockUtils::sockets::TcpSocket * sock, common::RequestRandomModMessage * msg) const;
		void handleUpdateAchievementProgress(clockUtils::sockets::TcpSocket * sock, common::UpdateAchievementProgressMessage * msg) const;
		void handleSubmitCompatibility(clockUtils::sockets::TcpSocket * sock, common::SubmitCompatibilityMessage * msg) const;
		void handleRequestOwnCompatibilities(clockUtils::sockets::TcpSocket * sock, common::RequestOwnCompatibilitiesMessage * msg) const;
		void handleRequestCompatibilityList(clockUtils::sockets::TcpSocket * sock, common::RequestCompatibilityListMessage * msg) const;
		void handleRequestRating(clockUtils::sockets::TcpSocket * sock, common::RequestRatingMessage * msg) const;
		void handleSubmitRating(clockUtils::sockets::TcpSocket * sock, common::SubmitRatingMessage * msg) const;
		void handleAutoUpdateEncrypted(clockUtils::sockets::TcpSocket * sock, common::UpdateRequestEncryptedMessage * msg) const;
		void handleRequestOverallSaveData(clockUtils::sockets::TcpSocket * sock, common::RequestOverallSaveDataMessage * msg) const;
		void handleUpdateOverallSaveData(clockUtils::sockets::TcpSocket * sock, common::UpdateOverallSaveDataMessage * msg) const;
		void handleRequestModManagement(clockUtils::sockets::TcpSocket * sock, common::RequestModManagementMessage * msg) const;
		void handleUpdateModVersion(clockUtils::sockets::TcpSocket * sock, common::UpdateModVersionMessage * msg) const;
		void handleUpdateEarlyAccessState(clockUtils::sockets::TcpSocket * sock, common::UpdateEarlyAccessStateMessage * msg) const;
		void handleRequestModsForEditor(clockUtils::sockets::TcpSocket * sock, common::RequestModsForEditorMessage * msg) const;
		void handleUpdateOfflineData(clockUtils::sockets::TcpSocket * sock, common::UpdateOfflineDataMessage * msg) const;
		void handleRequestOfflineData(clockUtils::sockets::TcpSocket * sock, common::RequestOfflineDataMessage * msg) const;
		void handleUpdateStartTime(clockUtils::sockets::TcpSocket * sock, common::UpdateStartTimeMessage * msg) const;
		void handleUpdatePlayingTime(clockUtils::sockets::TcpSocket * sock, common::UpdatePlayingTimeMessage * msg) const;
		void handleRequestAllFriends(clockUtils::sockets::TcpSocket * sock, common::RequestAllFriendsMessage * msg) const;
		void handleSendFriendRequest(clockUtils::sockets::TcpSocket * sock, common::SendFriendRequestMessage * msg) const;
		void handleAcceptFriendRequest(clockUtils::sockets::TcpSocket * sock, common::AcceptFriendRequestMessage * msg) const;
		void handleDeclineFriendRequest(clockUtils::sockets::TcpSocket * sock, common::DeclineFriendRequestMessage * msg) const;
		void handleRequestUserLevel(clockUtils::sockets::TcpSocket * sock, common::RequestUserLevelMessage * msg) const;
		void handleUpdateGeneralModConfiguration(clockUtils::sockets::TcpSocket * sock, common::UpdateGeneralModConfigurationMessage * msg) const;
		void handleUpdateScores(clockUtils::sockets::TcpSocket * sock, common::UpdateScoresMessage * msg) const;
		void handleUpdateAchievements(clockUtils::sockets::TcpSocket * sock, common::UpdateAchievementsMessage * msg) const;
		void handleUpdateSucceeded(clockUtils::sockets::TcpSocket * sock, common::UpdateSucceededMessage * msg) const;
		void handleUpdateChapterStats(clockUtils::sockets::TcpSocket * sock, common::UpdateChapterStatsMessage * msg) const;
		void handleUpdateImpression(clockUtils::sockets::TcpSocket * sock, common::UpdateImpressionMessage * msg) const;
		void handleIsAchievementUnlocked(clockUtils::sockets::TcpSocket * sock, common::IsAchievementUnlockedMessage * msg) const;

		/**
		 * \brief returns the id in the table for the username
		 */
		int getUserID(const std::string & username) const;
		int getUserID(const std::string & username, const std::string & password) const;
		std::string getUsername(int id) const;
		static std::vector<std::string> getUserList();
		static void sendMail(const std::string & subject, const std::string & body, const std::string & replyTo);

		// MP stuff, communication with external server
		uint32_t getLevel(int userID, uint32_t & currentXP, uint32_t & nextXP) const;
	};

} /* namespace spine */

#endif /* __SPINE_SERVER_H__ */
