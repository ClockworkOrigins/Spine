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
// Copyright 2019 Clockwork Origins

#pragma once

#include "boost/property_tree/ptree_fwd.hpp"

#include "simple-web-server/server_https.hpp"

using namespace boost::property_tree;
using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

namespace spine {
namespace server {

	class DownloadSizeChecker;
	class MariaDBWrapper;

	class DatabaseServer {
	public:
		DatabaseServer(DownloadSizeChecker * downloadSizeChecker);
		~DatabaseServer();

		int run();
		void stop();

	private:
		HttpsServer * _server;
		std::thread * _runner;
		mutable std::mutex _newsLock;

		DownloadSizeChecker * _downloadSizeChecker;

		void getModnameForIDs(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getUserID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getOwnRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getWeightedRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getRatings(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void unlockAchievementServer(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getUserIDForDiscordID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getDiscordIDForUserID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getSpineLevelRanking(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void sendUserInfos(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestSingleProjectStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestCompatibilityList(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updatePlayTime(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestScores(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateScore(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getReviews(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateReview(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void unlockAchievement(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateAchievementProgress(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestOverallSaveData(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAllFriends(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateChapterStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void isAchievementUnlocked(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateOfflineData(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestOfflineData(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void friendRequest(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void feedback(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void acceptFriend(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void declineFriend(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateLoginTime(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestRandomPage(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestInfoPage(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void submitInfoPage(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void removeFriend(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestOriginalFiles(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void linkClicked(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void submitRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestUserLevel(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAllProjectStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAllAchievementStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAllScoreStats(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateSucceeded(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void projectVersionCheck(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void submitNews(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAllProjects(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void submitCompatibility(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestOwnCompatibilities(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestAllNews(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void downloadSucceeded(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void packageDownloadSucceeded(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestProjectFiles(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void requestPackageFiles(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void gmpLogin(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		
		void requestAllTri6ScoreStats(ptree & responseTree) const;

		std::string getFileServer(int userID, int projectID, int majorVersion, int minorVersion, int patchVersion, int spineVersion) const;
		void attachNewsData(MariaDBWrapper & database, const std::string & newsID, int language, ptree & newsNode) const;
	};

} /* namespace server */
} /* namespace spine */
