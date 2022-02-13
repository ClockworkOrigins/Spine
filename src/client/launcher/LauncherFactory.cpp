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

#include "launcher/LauncherFactory.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "launcher/GameLauncher.h"
#include "launcher/Gothic1Launcher.h"
#include "launcher/Gothic2Launcher.h"
#include "launcher/Gothic3Launcher.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/ErrorReporting.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>

using namespace spine::common;
using namespace spine::https;
using namespace spine::launcher;
using namespace spine::utils;

LauncherFactory * LauncherFactory::getInstance() {
	static LauncherFactory factory;
	return &factory;
}

LauncherFactory::LauncherFactory() {
	_launchers.append(ILauncherPtr(new Gothic1Launcher));
	_launchers.append(ILauncherPtr(new Gothic2Launcher));
	_launchers.append(ILauncherPtr(new Gothic3Launcher));
	_launchers.append(ILauncherPtr(new GameLauncher));

	for (auto & launcher : _launchers) {
		initLauncher(launcher);
	}
}

ILauncherPtr LauncherFactory::getLauncher(GameType gameType) const {
	for (const auto & l : _launchers) {
		if (!l->supportsGame(gameType)) continue;

		return l;
	}

	return ILauncherPtr();
}

ILauncherPtr LauncherFactory::getLauncher(int32_t modID, const QString & iniFile) const {
	for (const auto & l : _launchers) {
		if (!l->supportsModAndIni(modID, iniFile)) continue;

		return l;
	}

	return ILauncherPtr();
}

void LauncherFactory::loginChanged() {
	for (const auto & l : _launchers) {
		l->loginChanged();
	}

	if (Config::Username.isEmpty())
		return;

	tryCleanCaches();
	synchronizeOfflineData();
}

void LauncherFactory::setDeveloperMode(bool enabled) {		
	for (const auto & l : _launchers) {
		l->setDeveloperMode(enabled);
	}
}

void LauncherFactory::setShowAchievements(bool enabled) {		
	for (const auto & l : _launchers) {
		l->setShowAchievements(enabled);
	}
}

void LauncherFactory::setZSpyActivated(bool enabled) {		
	for (const auto & l : _launchers) {
		l->setZSpyActivated(enabled);
	}
}

void LauncherFactory::restoreSettings() {
	for (const auto & l : _launchers) {
		l->restoreSettings();
	}
}

void LauncherFactory::saveSettings() {
	for (const auto & l : _launchers) {
		l->saveSettings();
	}
}

void LauncherFactory::updateModel(QStandardItemModel * model) {
	for (const auto & l : _launchers) {
		l->updateModel(model);
	}
}

void LauncherFactory::updatedProject(int projectID) {
	for (const auto & l : _launchers) {
		l->updatedProject(projectID);
	}
}

bool LauncherFactory::isRunning() const {
	bool running = false;
	for (const auto & l : _launchers) {
		running |= l->isRunning();
	}
	return running;
}

void LauncherFactory::initLauncher(const ILauncherPtr & launcher) const {
	if (!launcher) return;
	
	launcher->init();
	
	connect(launcher.data(), &ILauncher::restartAsAdmin, this, &LauncherFactory::restartAsAdmin);
	connect(launcher.data(), &ILauncher::errorMessage, this, &LauncherFactory::errorMessage);
	connect(launcher.data(), &ILauncher::openAchievementView, this, &LauncherFactory::openAchievementView);
	connect(launcher.data(), &ILauncher::openScoreView, this, &LauncherFactory::openScoreView);
	connect(launcher.data(), &ILauncher::loadedSurvey, this, &LauncherFactory::showSurvey);
	connect(launcher.data(), &ILauncher::editReview, this, &LauncherFactory::editReview);
	connect(this, &LauncherFactory::finishedInstallation, launcher.data(), &ILauncher::finishedInstallation);
	connect(this, &LauncherFactory::updateStarted, launcher.data(), &ILauncher::updateStarted);
	connect(this, &LauncherFactory::updateFinished, launcher.data(), &ILauncher::updateFinished);
}

void LauncherFactory::tryCleanCaches() {
	if (!Config::OnlineMode)
		return;

	QtConcurrent::run([] {
		Database::DBError err;
		const auto scores = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM scoreCache;", err);
		for (const auto & t : scores) {
			QJsonObject json;
			json["ProjectID"] = t[0];
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Identifier"] = t[1];
			json["Score"] = t[2];

			Https::postAsync(DATABASESERVER_PORT, "updateScore", QJsonDocument(json).toJson(QJsonDocument::Compact), [t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) {
					ErrorReporting::report(QStringLiteral("Failed to synchronize score: %1 - %2 - %3 - %4 - %5").arg(Config::Username).arg(t[0]).arg(t[1]).arg(t[2]).arg(statusCode));
					return;
				}

				ILauncher::removeScore(t[0], t[1]);
			});
		}
		const auto achievements = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM achievementCache;", err);
		for (const auto & t : achievements) {
			QJsonObject json;
			json["ProjectID"] = t[0];
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Identifier"] = t[1];
			json["Duration"] = 0;

			Https::postAsync(DATABASESERVER_PORT, "unlockAchievement", QJsonDocument(json).toJson(QJsonDocument::Compact), [t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) {
					ErrorReporting::report(QStringLiteral("Failed to synchronize achievement: %1 - %2 - %3 - %4").arg(Config::Username).arg(t[0]).arg(t[1]).arg(statusCode));
					return;
				}

				ILauncher::removeAchievement(t[0], t[1]);
			});
		}
		const auto achievementProgresses = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM achievementProgressCache;", err);
		for (auto t : achievementProgresses) {
			QJsonObject json;
			json["ProjectID"] = t[0];
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Identifier"] = t[1];
			json["Progress"] = t[2];

			Https::postAsync(DATABASESERVER_PORT, "updateAchievementProgress", QJsonDocument(json).toJson(QJsonDocument::Compact), [t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) {
					ErrorReporting::report(QStringLiteral("Failed to synchronize achievement progress: %1 - %2 - %3 - %4 - %5").arg(Config::Username).arg(t[0]).arg(t[1]).arg(t[2]).arg(statusCode));
					return;
				}

				ILauncher::removeAchievementProgress(t[0], t[1]);
			});
		}
		const auto overallSaveDatas = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM overallSaveDataCache;", err);
		for (const auto & t : overallSaveDatas) {
			QJsonObject json;
			json["ProjectID"] = std::stoi(t[0]);
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Key"] = encodeString(s2q(t[1]));
			json["Value"] = encodeString(s2q(t[2]));

			Https::postAsync(DATABASESERVER_PORT, "updateOverallSaveData", QJsonDocument(json).toJson(QJsonDocument::Compact), [t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) {
					ErrorReporting::report(QStringLiteral("Failed to synchronize overall save data: %1 - %2 - %3 - %4 - %5").arg(Config::Username).arg(s2q(t[0])).arg(s2q(t[1])).arg(s2q(t[2])).arg(statusCode));
					return;
				}

				ILauncher::removeOverallSaveData(std::stoi(t[0]), t[1]);
			});
		}
	});
}

void LauncherFactory::synchronizeOfflineData() {
	// this code is incredible slow due to around 2.500 SQL inserts
	if (!Config::OnlineMode)
		return;

	Database::DBError err;
	const int sync = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Enabled FROM sync LIMIT 1;", err);

	if (!sync) return;

	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM sync;", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO sync (Enabled) VALUES (0);", err);

	QtConcurrent::run([]() {
		try {
			// update server from local data in case Sync flag is set
			{
				Database::DBError err2;

				QJsonObject json;
				json["Username"] = Config::Username;
				json["Password"] = Config::Password;

				QJsonArray achievementsArray;

				const auto achievements = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier FROM modAchievements;", err2);
				for (const auto & vec : achievements) {
					QJsonObject jsonAchievement;
					jsonAchievement["ProjectID"] = std::stoi(vec[0]);
					jsonAchievement["Identifier"] = std::stoi(vec[1]);

					const auto progress = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE Username = '" + Config::Username.toStdString() + "' AND ModID = " + vec[0] + " AND Identifier = " + vec[1] + " LIMIT 1;", err2);

					if (!progress.empty()) {
						jsonAchievement["Progress"] = progress[0];
					}

					achievementsArray << jsonAchievement;
				}

				if (!achievementsArray.isEmpty()) {
					json["Achievements"] = achievementsArray;
				}

				QJsonArray overallSaveDataArray;

				const auto overallSaveData = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Entry, Value FROM overallSaveData;", err2);
				for (const auto & vec : overallSaveData) {
					QJsonObject jsonData;
					jsonData["ProjectID"] = std::stoi(vec[0]);
					jsonData["Key"] = s2q(vec[1]);
					jsonData["Value"] = s2q(vec[2]);

					overallSaveDataArray << jsonData;
				}

				if (!overallSaveDataArray.isEmpty()) {
					json["OverallSaveData"] = overallSaveDataArray;
				}

				QJsonArray playTimesArray;

				const auto playTimes = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Duration FROM playTimes;", err2);
				for (const auto & vec : playTimes) {
					QJsonObject jsonTime;
					jsonTime["ProjectID"] = vec[0];

					auto duration = vec[1];
					duration = duration / 1000; // to seconds
					duration = duration / 60;

					jsonTime["Time"] = duration;

					playTimesArray << jsonTime;
				}

				if (!playTimesArray.isEmpty()) {
					json["PlayTimes"] = playTimesArray;
				}

				Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes;", err2);

				Https::post(DATABASESERVER_PORT, "updateOfflineData", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});
			}
			{
				// Load data from server
				QJsonObject json;
				json["Username"] = Config::Username;
				json["Password"] = Config::Password;

				Https::postAsync(DATABASESERVER_PORT, "requestOfflineData", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject & data, int statusCode) {
					if (statusCode != 200)
						return;

					Database::DBError err2;

					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes WHERE Username = '" + Config::Username.toStdString() + "';", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementList;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementProgress;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementProgressMax;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievements;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modScores;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM scoreOrders;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM overallSaveData;", err2);

					Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA synchronous = OFF;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA journal_mode = MEMORY;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA cache_size=10000;", err2);
					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err2);

					if (data.contains("Achievements")) {
						const auto achievementArray = data["Achievements"].toArray();

						for (const auto jsonRef : achievementArray) {
							const auto jsonAchievement = jsonRef.toObject();

							const auto projectID = jsonAchievement["ProjectID"].toString().toInt();
							const auto identifier = jsonAchievement["Identifier"].toString().toInt();

							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementList (ModID, Identifier) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ");", err2);

							if (jsonAchievement.contains("Progress")) {
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(jsonAchievement["Progress"].toString().toInt()) + ");", err2);
							}
							if (jsonAchievement.contains("Max")) {
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgressMax (ModID, Identifier, Max) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", " + std::to_string(jsonAchievement["Max"].toString().toInt()) + ");", err2);
							}
							if (jsonAchievement.contains("Unlocked")) {
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", '" + Config::Username.toStdString() + "');", err2);
							}
						}
					}
					if (data.contains("Scores")) {
						const auto scoresArray = data["Scores"].toArray();

						for (const auto jsonRef : scoresArray) {
							const auto jsonScore = jsonRef.toObject();

							const auto projectID = jsonScore["ProjectID"].toString().toInt();
							const auto identifier = jsonScore["Identifier"].toString().toInt();
							const auto score = jsonScore["Score"].toString().toInt();
							const auto username = jsonScore["Username"].toString();
							const auto scoreOrder = jsonScore["ScoreOrder"].toString().toInt();

							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", '" + username.toStdString() + "', " + std::to_string(score) + ");", err2);
							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT OR IGNORE INTO scoreOrders (ProjectID, Identifier, ScoreOrder) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", " + std::to_string(scoreOrder) + ");", err2);
						}
					}
					if (data.contains("OverallSaveData")) {
						const auto overallSaveDataArray = data["OverallSaveData"].toArray();

						for (const auto jsonRef : overallSaveDataArray) {
							const auto jsonOverallSaveData = jsonRef.toObject();

							const auto projectID = jsonOverallSaveData["ProjectID"].toString().toInt();
							const auto key = jsonOverallSaveData["Key"].toString();
							const auto value = jsonOverallSaveData["Value"].toString();

							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(projectID) + ", '" + Config::Username.toStdString() + "', '" + q2s(key) + "', '" + q2s(value) + "');", err2);
						}
					}

					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err2);
					Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err2);
				});
			}
		} catch (...) {
		}
	});
}
