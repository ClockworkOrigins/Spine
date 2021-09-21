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

#include "StatsCollector.h"

#include <iostream>
#include <thread>

#include "ServerCommon.h"
#include "MariaDBWrapper.h"

using namespace spine::server;

void StatsCollector::init() {
	std::thread(&StatsCollector::exec).detach();
}

void StatsCollector::exec() {
	while (true) {
		const auto timestamp = time(nullptr);

		auto days = timestamp;
		days /= 60; // seconds to minutes
		days /= 60; // minutes to hours
		days /= 24; // hours to days
		
		do {
			std::vector<int> userList;
			
			{
				CONNECTTODATABASE(__LINE__)

				if (!database.query("PREPARE selectStmt FROM \"SELECT IFNULL(MAX(DaysSinceEpoch), 0) AS DaysSinceEpoch FROM playersPerDay\";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					break;
				}

				if (!database.query("PREPARE selectPlayersStmt FROM \"SELECT IFNULL(COUNT(UserID), 0) AS Amount FROM lastLoginTimes WHERE Timestamp >= ? - 24\";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					break;
				}

				if (!database.query("PREPARE insertStmt FROM \"INSERT INTO playersPerDay (DaysSinceEpoch, PlayerCount) VALUES (?, ?)\";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					break;
				}

				if (!database.query("EXECUTE selectStmt;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
					break;
				}

				auto results = database.getResults<std::vector<std::string>>();

				if (results.empty() || std::stoi(results[0][0]) < days) {
					if (!database.query("SET @paramTimestamp=" + std::to_string(timestamp / 60 / 60) + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
						break;
					}
					if (!database.query("SET @paramDays=" + std::to_string(days) + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
						break;
					}

					if (!database.query("EXECUTE selectPlayersStmt USING @paramTimestamp;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
						break;
					}
					results = database.getResults<std::vector<std::string>>();
					
					if (!database.query("SET @paramCount=" + results[0][0] + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
						break;
					}

					if (!database.query("EXECUTE insertStmt USING @paramDays, @paramCount;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
						break;
					}
				}
			}
		} while (false);

		std::this_thread::sleep_for(std::chrono::hours(1));
	}
}
