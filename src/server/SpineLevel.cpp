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
// Copyright 2020 Clockwork Origins

#include "SpineLevel.h"

#include <thread>

#include "MariaDBWrapper.h"
#include "ServerCommon.h"

using namespace boost::property_tree;

using namespace spine::common;
using namespace spine::server;

std::mutex SpineLevel::_lock;
std::map<int, SendUserLevelMessage> SpineLevel::_levels;
std::vector<SpineLevel::RankingEntry> SpineLevel::_rankings;
std::mutex SpineLevel::_rankingLock;
std::list<int> SpineLevel::_updateQueue;
std::mutex SpineLevel::_updateQueueLock;

void SpineLevel::init() {
	std::thread([]() {
		std::map<int, std::tuple<int, int>> cachedEntries;
		
		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE selectStmt FROM \"SELECT UserID, Level, XP, NextXP FROM levels\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectStmt;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();

			std::lock_guard<std::mutex> lg(_lock);
			
			for (const auto & vec : results) {
				const int userID = std::stoi(vec[0]);
				const int level = std::stoi(vec[1]);
				const int xp = std::stoi(vec[2]);
				const int nextXP = std::stoi(vec[3]);
				
				cachedEntries.insert(std::make_pair(userID, std::make_tuple(level, xp)));

				SendUserLevelMessage sulm;
				sulm.level = level;
				sulm.currentXP = xp;
				sulm.nextXP = nextXP;
				
				_levels[userID] = sulm;
			}
		} while (false);
		
		do {
			MariaDBWrapper accountDatabase;
			if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
				break;
			}

			if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT ID, Username FROM accounts\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << accountDatabase.getLastError() << std::endl;
				break;
			}
			if (!accountDatabase.query("EXECUTE selectStmt;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << accountDatabase.getLastError() << std::endl;
				break;
			}
			const auto results = accountDatabase.getResults<std::vector<std::string>>();

			std::lock_guard<std::mutex> lg(_rankingLock);
			std::lock_guard<std::mutex> lg2(_updateQueueLock);

			for (const auto & vec : results) {
				const int id = std::stoi(vec[0]);
				const std::string username = vec[1];

				RankingEntry re;
				re.userID = id;
				re.username = username;
				re.level = 0;
				re.xp = 0;

				const auto it = cachedEntries.find(id);

				if (it != cachedEntries.end()) {
					re.level = std::get<0>(it->second);
					re.xp = std::get<1>(it->second);
				}
				
				_updateQueue.push_back(id);
				_rankings.push_back(re);
			}
		} while (false);
		
		while (true) {
			_updateQueueLock.lock();
			
			if (!_updateQueue.empty()) {
				const int id = _updateQueue.front();
				_updateQueue.pop_front();

				_updateQueueLock.unlock();

				cacheLevel(id);
			} else {
				_updateQueueLock.unlock();
				std::this_thread::sleep_for(std::chrono::seconds(30));
			}
		}
	}).detach();
}

SendUserLevelMessage SpineLevel::getLevel(int userID) {
	{
		std::lock_guard<std::mutex> lg(_lock);
		const auto it = _levels.find(userID);
		
		if (it != _levels.end()) return it->second;
	}

	cacheLevel(userID);
	
	std::lock_guard<std::mutex> lg(_lock);
	
	const auto it = _levels.find(userID);
	
	return it->second;
}

void SpineLevel::updateLevel(int userID) {
	if (userID == -1) return;
	
	std::lock_guard<std::mutex> lg(_updateQueueLock);

	const auto it = std::find_if(_updateQueue.begin(), _updateQueue.end(), [userID](int id) {
		return userID == id;
	});

	if (it != _updateQueue.end()) return; // already queued
	
	_updateQueue.push_back(userID);
}

void SpineLevel::clear(const std::vector<int> & userList) {
	std::lock_guard<std::mutex> lg(_updateQueueLock);
	for (int userID : userList) {
		const auto it = std::find_if(_updateQueue.begin(), _updateQueue.end(), [userID](int id) {
			return userID == id;
		});

		if (it != _updateQueue.end()) continue; // already queued
		
		_updateQueue.push_back(userID);
	}
}

void SpineLevel::addRanking(ptree & json) {
	std::lock_guard<std::mutex> lg(_rankingLock);

	std::sort(_rankings.begin(), _rankings.end(), [](const RankingEntry & a, const RankingEntry & b) {
		return a.level > b.level || (a.level == b.level && a.xp > b.xp);
	});

	uint32_t rank = 0;
	uint32_t lastXP = 0;
	uint32_t realRank = 0;

	ptree rankingList;
	
	for (const auto & re : _rankings) {
		if (re.xp == 0) break;

		realRank++;

		if (re.xp != lastXP) {
			rank = realRank;
			lastXP = re.xp;
		}

		ptree rankingEntry;

		rankingEntry.put("Name", re.username);
		rankingEntry.put("Level", re.level);
		rankingEntry.put("XP", re.xp);
		rankingEntry.put("Rank", rank);

		rankingList.push_back(std::make_pair("", rankingEntry));
	}

	json.add_child("Names", rankingList);
}

void SpineLevel::cacheLevel(int userID) {
	SendUserLevelMessage sulm;

	int level = 0;
	uint32_t currentXP = 0;

	do {
		CONNECTTODATABASE(__LINE__)
		
		if (!database.query("PREPARE selectAchievementsStmt FROM \"SELECT COUNT(*) FROM modAchievements WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAchievementListStmt FROM \"SELECT COUNT(*) FROM modAchievementList WHERE ModID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAchievementsPerModStmt FROM \"SELECT COUNT(*) FROM modAchievements WHERE UserID = ? AND ModID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAchievementWithModsStmt FROM \"SELECT DISTINCT ModID FROM modAchievements WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectScoresStmt FROM \"SELECT Count(*) FROM modScores WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectRatingsStmt FROM \"SELECT Count(*) FROM ratings WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectReviewsStmt FROM \"SELECT Count(*) FROM reviews WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectCompatibilitiesStmt FROM \"SELECT Count(*) FROM compatibilityList WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectPlayedModsWithTimeStmt FROM \"SELECT ModID, Duration FROM playtimes WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectPlayTimesStmt FROM \"SELECT Duration FROM playtimes WHERE ModID = ? ORDER BY Duration ASC\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDonationStmt FROM \"SELECT Amount FROM donations WHERE UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectCheaterStmt FROM \"SELECT UserID FROM cheaters WHERE UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("PREPARE selectUserVotesStmt FROM \"SELECT Count(*) FROM userVotes WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectCheaterStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		auto lastResults = database.getResults<std::vector<std::string>>();
		const bool cheater = !lastResults.empty();
		
		if (!database.query("EXECUTE selectAchievementsStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) {
			const int count = std::stoi(lastResults[0][0]);
			currentXP += count * 50; // 50 EP per achievement
		}

		// perfect games (all achievements)
		if (!database.query("EXECUTE selectAchievementWithModsStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		for (const auto & vec : lastResults) {
			if (!database.query("SET @paramModID=" + vec[0] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectAchievementListStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			auto r = database.getResults<std::vector<std::string>>();
			const uint32_t achievementCount = std::stoi(r[0][0]);
			
			if (!database.query("EXECUTE selectAchievementsPerModStmt USING @paramUserID, @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			r = database.getResults<std::vector<std::string>>();
			const uint32_t unlockedAchievementCount = std::stoi(r[0][0]);

			if (unlockedAchievementCount == achievementCount) {
				currentXP += 1000; // 1000 EP for perfect games
			}
		}
		
		if (!database.query("EXECUTE selectScoresStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty() && !cheater) {
			const int count = std::stoi(lastResults[0][0]);
			
			currentXP += count * 100; // 100 EP per score
		}
		
		if (!database.query("EXECUTE selectRatingsStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) {
			const int count = std::stoi(lastResults[0][0]);
			
			currentXP += count * 250; // 250 EP per rating
		}

		if (!database.query("EXECUTE selectReviewsStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) {
			const int count = std::stoi(lastResults[0][0]);

			currentXP += count * 250; // 250 EP per review for now, maybe more later, but can easily be abused by one letter reviews
		}
		
		if (!database.query("EXECUTE selectCompatibilitiesStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) {
			const int count = std::stoi(lastResults[0][0]);
			
			currentXP += count * 10; // 10 EP per compatibility list entry
		}

		// play time over median or even third quartile gives some bonus XP
		
		if (!database.query("EXECUTE selectPlayedModsWithTimeStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		for (const auto & vec : lastResults) {
			const int ownDuration = std::stoi(vec[1]);
			if (!database.query("SET @paramModID=" + vec[0] + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectPlayTimesStmt USING @paramModID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			const auto r = database.getResults<std::vector<std::string>>();
			const int firstQuartile = std::stoi(r[r.size() / 4][0]);
			const int median = std::stoi(r[r.size() / 2][0]);
			const int thirdQuartile = std::stoi(r[(r.size() * 3) / 4][0]);

			if (ownDuration > thirdQuartile) {
				currentXP += 100;
			} else if (ownDuration > median) {
				currentXP += 50;
			} else if (ownDuration > firstQuartile) {
				currentXP += 25;
			} else {
				currentXP += 10;
			}
		}
		if (!database.query("EXECUTE selectDonationStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) {
			currentXP += std::stoi(lastResults[0][0]);
		}
		
		if (!database.query("EXECUTE selectUserVotesStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
			break;
		}
		lastResults = database.getResults<std::vector<std::string>>();
		if (!lastResults.empty()) {
			currentXP += std::stoi(lastResults[0][0]) * 250;
		}

		// Patreon also counts
		{
			MariaDBWrapper accountDatabase;
			if (!accountDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, ACCOUNTSDATABASE, 0)) {
				break;
			}

			if (!accountDatabase.query("PREPARE selectStmt FROM \"SELECT LifetimeAmount FROM patronLevels WHERE ID = ? AND ProjectID = 0 LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!accountDatabase.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!accountDatabase.query("EXECUTE selectStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			const auto results = accountDatabase.getResults<std::vector<std::string>>();

			if (!results.empty()) {
				currentXP += std::stoi(results[0][0]);
			}
		}

		// bonus XP for people that support us by playing our games
		{
			MariaDBWrapper ewDatabase;
			if (!ewDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, EWDATABASE, 0)) {
				std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			if (!ewDatabase.query("PREPARE selectPlayedTimeStmt FROM \"SELECT Time FROM playTimes WHERE UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
		
			if (!ewDatabase.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!ewDatabase.query("EXECUTE selectPlayedTimeStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			const auto r = ewDatabase.getResults<std::vector<std::string>>();
			if (!r.empty()) {
				currentXP += 1000;
			}
		}
		{
			MariaDBWrapper tri6Database;
			if (!tri6Database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, TRI6DATABASE, 0)) {
				std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			if (!tri6Database.query("PREPARE selectPlayedTimeStmt FROM \"SELECT Time FROM playTimes WHERE UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!tri6Database.query("PREPARE selectPlayedTimeDemoStmt FROM \"SELECT Time FROM playTimesDemo WHERE UserID = ? LIMIT 1\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
		
			if (!tri6Database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!tri6Database.query("EXECUTE selectPlayedTimeStmt USING @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
			auto r = tri6Database.getResults<std::vector<std::string>>();
			if (!r.empty()) {
				currentXP += 1000;
			} else {
				if (!tri6Database.query("EXECUTE selectPlayedTimeDemoStmt USING @paramUserID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
					break;
				}
				r = tri6Database.getResults<std::vector<std::string>>();
				if (!r.empty()) {
					currentXP += 250;
				}
			}
		}
	} while (false);

	uint32_t nextXP = 500;
	while (currentXP >= nextXP) {
		level++;
		nextXP += (level + 1) * 500;
	}
	
	sulm.level = level;
	sulm.currentXP = currentXP;
	sulm.nextXP = nextXP;

	const auto username = ServerCommon::getUsername(userID);

	if (!username.empty()) {
		do {
			CONNECTTODATABASE(__LINE__)
			
			if (!database.query("PREPARE insertStmt FROM \"INSERT INTO levels (UserID, Level, XP, NextXP) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE Level = ?, XP = ?, NextXP = ?\";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			
			if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("SET @paramLevel=" + std::to_string(level) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("SET @paramCurrentXP=" + std::to_string(currentXP) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("SET @paramNextXP=" + std::to_string(nextXP) + ";")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertStmt USING @paramUserID, @paramLevel, @paramCurrentXP, @paramNextXP, @paramLevel, @paramCurrentXP, @paramNextXP;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << " " << database.getLastError() << std::endl;
				break;
			}
		} while (false);
	}

	{
		std::lock_guard<std::mutex> lg(_lock);
		_levels[userID] = sulm;
	}

	if (!username.empty()) {
		std::lock_guard<std::mutex> lg(_rankingLock);
		
		const auto it = std::find_if(_rankings.begin(), _rankings.end(), [userID](const RankingEntry & re) {
			return re.userID == userID;
		});
		
		if (it == _rankings.end()) {
			RankingEntry re;
			re.userID = userID;
			re.xp = currentXP;
			re.level = level;
			re.username = username;
			
			_rankings.push_back(re);
		} else {
			it->xp = currentXP;
			it->level = level;
		}
	}
}
