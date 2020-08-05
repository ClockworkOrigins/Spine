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

#include <vector>

#include "SpineServerConfig.h"

namespace spine {
namespace server {

#define CONNECTTODATABASE(line) \
	MariaDBWrapper database;\
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {\
		std::cout << "Couldn't connect to database: " << (line) << /*" " << database.getLastError() <<*/ std::endl;\
		break;\
	}

	class ServerCommon {
	public:
		static std::string convertString(const std::string & str);
		
		/**
		 * \brief returns the id in the table for the username
		 */
		static int getUserID(const std::string & username);
		static int getUserID(const std::string & username, const std::string & password);
		static std::string getUsername(int id);
		static std::vector<std::string> getUserList();
		static void sendMail(const std::string & subject, const std::string & body, const std::string & replyTo);
		static void sendMail(const std::string & subject, const std::string & body, const std::string & replyTo, const std::string & receiver);
		static bool isValidUserID(int userID);

		static std::string getProjectName(int projectID, int preferredLanguage);

		static std::string filterUsername(const std::string & username);
	};

} /* namespace server */
} /* namespace spine */
