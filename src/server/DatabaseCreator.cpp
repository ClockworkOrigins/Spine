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

#include "DatabaseCreator.h"

#include <iostream>

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

using namespace spine;

void DatabaseCreator::createTables() {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}

	// ModID | ID of the team | Enabled in GUI or only internal | 1 or 2 either if the mod is for Gothic 1 or 2 | Release date encoded as integer in days | one of Mod or Patch encoded as integer
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS mods (ModID INT AUTO_INCREMENT PRIMARY KEY, TeamID INT NOT NULL, Enabled INT NOT NULL, Gothic INT NOT NULL, ReleaseDate INT NOT NULL, Type INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS optionalpackages (PackageID INT AUTO_INCREMENT PRIMARY KEY, ModID INT NOT NULL, Enabled INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modnames (ModID INT NOT NULL, Name TEXT NOT NULL, Language TEXT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS optionalpackagenames (PackageID INT NOT NULL, Name TEXT NOT NULL, Language TEXT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modfiles (FileID INT AUTO_INCREMENT PRIMARY KEY, ModID INT NOT NULL, Path TEXT NOT NULL, Language TEXT NOT NULL, Hash TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS optionalpackagefiles (FileID INT AUTO_INCREMENT PRIMARY KEY, PackageID INT NOT NULL, Path TEXT NOT NULL, Language TEXT NOT NULL, Hash TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS news (NewsID INT AUTO_INCREMENT PRIMARY KEY, Title TEXT NOT NULL, Body TEXT NOT NULL, Timestamp INT NOT NULL, Language TEXT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	// used for direct download links from news
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS newsModReferences (NewsID INT NOT NULL, ModID INT NOT NULL, PRIMARY KEY (NewsID, ModID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS newsImageReferences (NewsID INT NOT NULL, File TEXT NOT NULL, Hash TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS ratings (ModID INT NOT NULL, UserID INT NOT NULL, Rating INT NOT NULL, PRIMARY KEY (ModID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS reviews (ProjectID INT NOT NULL, UserID INT NOT NULL, Review TEXT NOT NULL, ReviewDate INT NOT NULL, PlayTime INT NOT NULL, PRIMARY KEY (ProjectID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS teams (TeamID INT AUTO_INCREMENT PRIMARY KEY, Homepage TEXT);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS teamnames (TeamID INT NOT NULL, Name TEXT NOT NULL, Language TEXT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS descriptions (ModID INT NOT NULL, Description TEXT NOT NULL, Language TEXT NOT NULL, PRIMARY KEY (ModID, Language(100))) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS features (ModID INT NOT NULL, Feature TEXT NOT NULL, Language TEXT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS screens (ModID INT NOT NULL, Image TEXT NOT NULL, Hash TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS spinefeatures (ModID INT PRIMARY KEY, Features INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS playtimes (ModID INT NOT NULL, UserID INT NOT NULL, Duration INT NOT NULL, PRIMARY KEY (ModID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS devtimes (ModID INT PRIMARY KEY, Duration INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS downloads (ModID INT PRIMARY KEY, Counter INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS downloadsPerVersion (ModID INT NOT NULL, Version TEXT NOT NULL, Counter INT NOT NULL, PRIMARY KEY (ModID, Version(20)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS sessionTimes (ModID INT NOT NULL, Duration INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS achievementTimes (ModID INT NOT NULL, Identifier INT NOT NULL, Duration INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS packagedownloads (PackageID INT PRIMARY KEY, Counter INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS earlyUnlocks (ModID INT NOT NULL, UserID INT NOT NULL, PRIMARY KEY (ModID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modScores (ModID INT NOT NULL, UserID INT NOT NULL, Identifier INT NOT NULL, Score INT NOT NULL, PRIMARY KEY (ModID, UserID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modScoreList (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modScoreNames (ModID INT NOT NULL, Identifier INT NOT NULL, Name TEXT NOT NULL, Language TEXT NOT NULL, PRIMARY KEY (ModID, Identifier, Language(100))) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievements (ModID INT NOT NULL, UserID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, UserID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementList (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementNames (ModID INT NOT NULL, Identifier INT NOT NULL, Name TEXT NOT NULL, Language TEXT NOT NULL, PRIMARY KEY (ModID, Identifier, Language(100))) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementProgressMax (ModID INT NOT NULL, Identifier INT NOT NULL, Max INT NOT NULL, PRIMARY KEY (ModID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementProgress (ModID INT NOT NULL, UserID INT NOT NULL, Identifier INT NOT NULL, Current INT NOT NULL, PRIMARY KEY (ModID, UserID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementDescriptions (ModID INT NOT NULL, Identifier INT NOT NULL, Description TEXT NOT NULL, Language TEXT NOT NULL, PRIMARY KEY (ModID, Identifier, Language(100))) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementIcons (ModID INT NOT NULL, Identifier INT NOT NULL, LockedIcon TEXT NOT NULL, LockedHash TEXT NOT NULL, UnlockedIcon TEXT NOT NULL, UnlockedHash TEXT NOT NULL, PRIMARY KEY (ModID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS modAchievementHidden (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS lastLoginTimes (UserID INT NOT NULL, Timestamp INT NOT NULL, PRIMARY KEY (UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS lastPlayTimes (ModID INT NOT NULL, UserID INT NOT NULL, Timestamp INT NOT NULL, PRIMARY KEY (ModID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS teammembers (TeamID INT NOT NULL, UserID INT NOT NULL, PRIMARY KEY (TeamID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS linksClicked (NewsID INT NOT NULL, Url TEXT NOT NULL, Counter INT NOT NULL, PRIMARY KEY (NewsID, Url(256)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS editrights (ModID INT NOT NULL, UserID INT NOT NULL, PRIMARY KEY (ModID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS userHashes (UserID INT NOT NULL, Hash TEXT NOT NULL, PRIMARY KEY (UserID, Hash(100)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS userSessionInfos (UserID INT PRIMARY KEY, Mac TEXT NOT NULL, IP TEXT NOT NULL, Hash TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS userSettings (UserID INT NOT NULL, Entry TEXT NOT NULL, Value TEXT NOT NULL, PRIMARY KEY (UserID, Entry(100)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS bannedUsers (UserID INT PRIMARY KEY);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS bannedHashes (Hash TEXT NOT NULL, PRIMARY KEY (Hash(100)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS provisoricalBans (UserID INT PRIMARY KEY);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS compatibilityList (UserID INT NOT NULL, ModID INT NOT NULL, PatchID INT NOT NULL, Compatible INT NOT NULL, PRIMARY KEY (UserID, ModID, PatchID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS overallSaveData (UserID INT NOT NULL, ModID INT NOT NULL, Entry TEXT NOT NULL, Value TEXT NOT NULL, PRIMARY KEY (UserID, ModID, Entry(100)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS userLanguages (UserID INT PRIMARY KEY, Language TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS newsWriter (UserID INT NOT NULL, Language TEXT NOT NULL, PRIMARY KEY (UserID, Language(100)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS gmpWhitelist (IP TEXT NOT NULL, ModID INT NOT NULL, PRIMARY KEY (ModID, IP(15)));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS multiplayerMods (ModID INT PRIMARY KEY);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS forbiddenPatches (ModID INT NOT NULL, PatchID INT NOT NULL, PRIMARY KEY (ModID, PatchID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS startTimes (DayOfWeek INT NOT NULL, Hour INT NOT NULL, Counter INT NOT NULL, PRIMARY KEY (DayOfWeek, Hour));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS playingTimes (DayOfWeek INT NOT NULL, Hour INT NOT NULL, Counter INT NOT NULL, PRIMARY KEY (DayOfWeek, Hour));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS friends (UserID INT NOT NULL, FriendID INT NOT NULL, PRIMARY KEY (UserID, FriendID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS chapterStats (ModID INT NOT NULL, Identifier INT NOT NULL, Guild INT NOT NULL, StatName TEXT NOT NULL, StatValue INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS cheaters (UserID INT PRIMARY KEY);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS lastUpdated (ProjectID INT PRIMARY KEY, Date INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS feedbackMails (ProjectID INT PRIMARY KEY, Mail TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS discussionUrls (ProjectID INT PRIMARY KEY, Url TEXT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS playTestSurveys (SurveyID INT AUTO_INCREMENT PRIMARY KEY, ProjectID INT NOT NULL, Language TEXT NOT NULL, Enabled INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS playTestSurveyQuestions (QuestionID INT AUTO_INCREMENT PRIMARY KEY, SurveyID INT NOT NULL, Question TEXT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS playTestSurveyAnswers (SurveyID INT NOT NULL, UserID INT NOT NULL, QuestionID INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, Answer TEXT NOT NULL, PRIMARY KEY(SurveyID, UserID, QuestionID)) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS newsticker (NewsID INT AUTO_INCREMENT PRIMARY KEY, ProjectID INT NOT NULL, Type INT NOT NULL, Date INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS updateNews (NewsID INT PRIMARY KEY, ProjectID INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SavegameCompatible INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS changelogs (NewsID INT NOT NULL, Changelog TEXT NOT NULL, Language TEXT NOT NULL, PRIMARY KEY(NewsID, Language(20))) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS levels (UserID INT PRIMARY KEY, Level INT NOT NULL, XP INT NOT NULL, NextXP INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS teamNames (TeamID INT NOT NULL, Name TEXT NOT NULL, Languages INT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS projectNames (ProjectID INT NOT NULL, Name TEXT NOT NULL, Languages INT NOT NULL) CHARACTER SET utf8;"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS projectPrivileges (ProjectID INT NOT NULL, UserID INT NOT NULL, Privileges BIGINT NOT NULL, PRIMARY KEY (ProjectID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS userPrivileges (UserID INT PRIMARY KEY, Privileges BIGINT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS teamPrivileges (TeamID INT NOT NULL, UserID INT NOT NULL, Privileges BIGINT NOT NULL, PRIMARY KEY (TeamID, UserID));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS donations (UserID INT PRIMARY KEY, Amount INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS scoreOrders (ProjectID INT NOT NULL, Identifier INT NOT NULL, ScoreOrder INT NOT NULL, PRIMARY KEY (ProjectID, Identifier));"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
	if (!database.query(std::string("CREATE TABLE IF NOT EXISTS playersPerDay (DaysSinceEpoch INT PRIMARY KEY, PlayerCount INT NOT NULL);"))) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}

	createFileserverTables();
}

void DatabaseCreator::createFileserverTables() {
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}

	if (!database.query("CREATE TABLE IF NOT EXISTS fileserverList (ServerID INT AUTO_INCREMENT PRIMARY KEY, Enabled INT NOT NULL, PatronOnly INT NOT NULL, Username TEXT NOT NULL, Password TEXT NOT NULL, FtpHost TEXT NOT NULL, RootFolder TEXT NOT NULL, Url TEXT NOT NULL);")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}

	if (!database.query("CREATE TABLE IF NOT EXISTS projectsOnFileservers (ServerID INT NOT NULL, ProjectID INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL, PRIMARY KEY (ServerID, ProjectID));")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}

	if (!database.query("CREATE TABLE IF NOT EXISTS fileserverSynchronizationQueue (JobID INT AUTO_INCREMENT PRIMARY KEY, ServerID INT NOT NULL, ProjectID INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL, Path TEXT NOT NULL, Operation INT NOT NULL);")) {
		std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
		return;
	}
}
