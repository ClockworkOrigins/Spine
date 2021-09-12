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
 // Copyright 2021 Clockwork Origins

#include "Cleanup.h"

#include <algorithm>
#include <iostream>
#include <thread>

#include "ServerCommon.h"
#include "MariaDBWrapper.h"

using namespace spine::server;

void Cleanup::init() {
	std::thread(&Cleanup::cleanup).detach();
}

void Cleanup::cleanup() {
	while (true) {
		std::cout << "Performing database cleanup" << std::endl;
		
		do {
			std::vector<int> userList;
			
			{
				MariaDBWrapper accountDatabase;
				if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}

				if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT ID FROM accounts\";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}

				if (!accountDatabase.query("EXECUTE selectStmt;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}

				const auto results = accountDatabase.getResults<std::vector<std::string>>();

				for (const auto & vec : results) {
					userList.push_back(std::stoi(vec[0]));
				}
			}

			cleanupTable("ratings", userList);
			cleanupTable("reviews", userList);
			cleanupTable("playtimes", userList);
			cleanupTable("earlyUnlocks", userList);
			cleanupTable("modScores", userList);
			cleanupTable("modAchievements", userList);
			cleanupTable("modAchievementProgress", userList);
			cleanupTable("lastLoginTimes", userList);
			cleanupTable("lastPlayTimes", userList);
			cleanupTable("teammembers", userList);
			cleanupTable("editrights", userList);
			cleanupTable("userHashes", userList);
			cleanupTable("userSessionInfos", userList);
			cleanupTable("userSettings", userList);
			cleanupTable("bannedUsers", userList);
			cleanupTable("provisoricalBans", userList);
			cleanupTable("compatibilityList", userList);
			cleanupTable("overallSaveData", userList);
			cleanupTable("userLanguages", userList);
			cleanupTable("newsWriter", userList);
			cleanupTable("friends", userList);
			cleanupTable("cheaters", userList);
			cleanupTable("playTestSurveyAnswers", userList);
			cleanupTable("levels", userList);
			cleanupTable("projectPrivileges", userList);
			cleanupTable("userPrivileges", userList);
			cleanupTable("teamPrivileges", userList);
			cleanupTable("donations", userList);
		} while (false);

		std::this_thread::sleep_for(std::chrono::hours(8));
	}
}

void Cleanup::cleanupTable(const std::string & tableName, const std::vector<int> & userList) {
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectStmt FROM \"SELECT DISTINCT UserID FROM " + tableName + "\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM " + tableName + " WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		const auto results = database.getResults<std::vector<std::string>>();

		for (const auto & vec : results) {
			const auto userID = std::stoi(vec[0]);

			if (userID == -1) continue; // special case
			if (userID == 0) continue; // special case, only for newsTicker actually

			if (std::find_if(userList.begin(), userList.end(), [userID](const int uid) { return userID == uid; }) != userList.end()) continue;

			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				break;
			}

			if (!database.query("EXECUTE deleteStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				break;
			}
		}
	} while (false);
}
