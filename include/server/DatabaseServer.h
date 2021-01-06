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

#include "simple-web-server/server_https.hpp"

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

namespace spine {
namespace server {

	class DatabaseServer {
	public:
		DatabaseServer();
		~DatabaseServer();

		int run();
		void stop();

	private:
		HttpsServer * _server;
		std::thread * _runner;

		void getModnameForIDs(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getUserID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getOwnRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getWeightedRating(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getRatings(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void unlockAchievementServer(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getUserIDForDiscordID(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
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
	};

} /* namespace server */
} /* namespace spine */
