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

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

namespace clockUtils {
namespace sockets {
	class TcpSocket;
}
}

namespace spine {
namespace common {
	struct UploadAchievementIconsMessage;
}
namespace server {

	class MariaDBWrapper;

	class ManagementServer {
		friend class Server;
		
	public:
		ManagementServer();
		~ManagementServer();

		int run();
		void stop();

	private:
		HttpsServer * _server;
		std::thread * _runner;

		void getMods(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateAchievements(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getGeneralConfiguration(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateGeneralConfiguration(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getCustomStatistics(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getModFiles(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateModVersion(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getStatistics(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getScores(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updateScores(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getUsers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void changeUserAccess(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void createPlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void enablePlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void updatePlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getPlayTestSurveys(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getPlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void submitPlayTestSurveyAnswers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getOwnPlayTestSurveyAnswers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void getAllPlayTestSurveyAnswers(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void deletePlayTestSurvey(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;
		void clearAchievementProgress(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) const;

		void uploadAchievementIcons(common::UploadAchievementIconsMessage * msg) const;

		bool hasAdminAccessToMod(int userID, int modID) const;

		static void pushToDiscord(const std::string & message);

		bool addAccessibleMod(int projectID, const std::string & language, MariaDBWrapper & database, boost::property_tree::ptree & modNode) const;
	};

} /* namespace server */
} /* namespace spine */
