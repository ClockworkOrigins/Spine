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

#include "launcher/ILauncher.h"

#include "ScreenshotManager.h"
#include "SpineConfig.h"

#include "client/widgets/FeedbackDialog.h"
#include "client/widgets/UpdateLanguage.h"

#include "common/MessageStructs.h"
#include "common/ScoreOrder.h"

#include "discord/DiscordManager.h"

#include "https/Https.h"

#include "security/Hash.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/WindowsExtensions.h"

#include "widgets/RatingWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

using namespace spine::common;
using namespace spine::discord;
using namespace spine::https;
using namespace spine::launcher;
using namespace spine::security;
using namespace spine::utils;
using namespace spine::widgets;

void ILauncher::init() {
	createWidget();

	_projectID = -1;
	_showAchievements = true;

	_timer = new QElapsedTimer();

	_socket = nullptr;
	_listenSocket = nullptr;
	
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS installDates (ModID INT PRIMARY KEY, InstallDate INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "CREATE TABLE IF NOT EXISTS lastPlayed (ModID INT NOT NULL, Ini TEXT NOT NULL, PRIMARY KEY (ModID, Ini));", err);
	
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS scoreCache (ModID INT NOT NULL, Identifier INT NOT NULL, Score INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS achievementCache (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS achievementProgressCache (ModID INT NOT NULL, Identifier INT NOT NULL, Progress INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS overallSaveDataCache (ModID INT NOT NULL, Entry TEXT NOT NULL, Value TEXT NOT NULL, PRIMARY KEY (ModID, Entry));", err);

	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modScores (ModID INT NOT NULL, Username TEXT NOT NULL, Identifier INT NOT NULL, Score INT NOT NULL, PRIMARY KEY (ModID, Username, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS scoreOrders (ProjectID INT NOT NULL, Identifier INT NOT NULL, ScoreOrder INT NOT NULL, PRIMARY KEY (ProjectID, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievements (ModID INT NOT NULL, Username TEXT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Username, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievementList (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievementProgressMax (ModID INT NOT NULL, Identifier INT NOT NULL, Max INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievementProgress (ModID INT NOT NULL, Username TEXT NOT NULL, Identifier INT NOT NULL, Current INT NOT NULL, PRIMARY KEY (ModID, Username, Identifier));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS overallSaveData (Username TEXT NOT NULL, ModID INT NOT NULL, Entry TEXT NOT NULL, Value TEXT NOT NULL, PRIMARY KEY (Username, ModID, Entry));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS playTimes (ModID INT NOT NULL, Username TEXT NOT NULL, Duration INT NOT NULL, PRIMARY KEY (Username, ModID));", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS sync (Enabled INT PRIMARY KEY);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT OR IGNORE INTO sync (Enabled) VALUES (0);", err);
	
	qRegisterMetaType<ProjectStats>("common::ProjectStats");

	_screenshotManager = new ScreenshotManager(this);
}

QWidget * ILauncher::getLibraryWidget() const {
	return _widget;
}

void ILauncher::loginChanged() {
	_ratingWidget->loginChanged();

	updateView(_projectID, _iniFile);
	
	if (!Config::Username.isEmpty()) {
		tryCleanCaches();
		synchronizeOfflineData();
	}
}

void ILauncher::setDeveloperMode(bool enabled) {
	_developerMode = enabled;
}

void ILauncher::setShowAchievements(bool enabled) {
	_showAchievements = enabled;
}

void ILauncher::createWidget() {
	_widget = new QWidget();
	
	_layout = new QVBoxLayout();
	_layout->setAlignment(Qt::AlignTop);

	{
		_upperLayout = new QHBoxLayout();

		_startButton = new QPushButton(QApplication::tr("StartMod"), _widget);
		_startButton->setProperty("library", true);
		UPDATELANGUAGESETTEXT(_startButton, "StartMod");
		_startButton->setFixedWidth(150);

		_upperLayout->addWidget(_startButton, 0, Qt::AlignLeft);

		_upperLayout->addSpacing(25);

		_playTimeLabel = new QLabel(_widget);
		_playTimeLabel->setProperty("library", true);
		_playTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		_upperLayout->addWidget(_playTimeLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
		_upperLayout->addStretch(1);

		if (Config::OnlineMode) {
			_ratingWidget = new RatingWidget(RatingWidget::RatingMode::User, _widget);
			_ratingWidget->setProperty("library", true);
			_upperLayout->addWidget(_ratingWidget, 1, Qt::AlignRight);
			_ratingWidget->setEditable(true);
			_ratingWidget->setVisible(false);

			connect(this, &ILauncher::syncedNewStats, _ratingWidget, &RatingWidget::loginChanged);
			connect(_ratingWidget, &RatingWidget::editReview, this, &ILauncher::editReview);
		}

		_layout->addLayout(_upperLayout);
	}

	{
		auto * hbl = new QHBoxLayout();

		_installDate = new QLabel(_widget);
		_installDate->setProperty("library", true);
		_installDate->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		hbl->addWidget(_installDate, 0, Qt::AlignLeft | Qt::AlignVCenter);

		hbl->addSpacing(25);

		_lastPlayedDate = new QLabel(_widget);
		_lastPlayedDate->setProperty("library", true);
		hbl->addWidget(_lastPlayedDate);

		hbl->addStretch(1);

		_layout->addLayout(hbl);
	}

	{
		auto * hbl = new QHBoxLayout();

		_discussionUrlTitle = new QLabel(QApplication::tr("DiscussionUrl"), _widget);
		UPDATELANGUAGESETTEXT(_discussionUrlTitle, "DiscussionUrl");
		_discussionUrlTitle->setProperty("library", true);
		_discussionUrlTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		_discussionUrlTitle->hide();

		hbl->addWidget(_discussionUrlTitle, 0, Qt::AlignLeft | Qt::AlignVCenter);

		hbl->addSpacing(25);

		_discussionUrlText = new QLabel(_widget);
		_discussionUrlText->setProperty("library", true);
		_discussionUrlText->hide();
		connect(_discussionUrlText, &QLabel::linkActivated, this, &ILauncher::openDiscussionsUrl);
		
		hbl->addWidget(_discussionUrlText);

		hbl->addStretch(1);

		_layout->addLayout(hbl);
	}

	{
		auto * hbl = new QHBoxLayout();

		_feedbackButton = new QPushButton(QApplication::tr("Feedback"), _widget);
		UPDATELANGUAGESETTEXT(_feedbackButton, "Feedback");
		_feedbackButton->setProperty("library", true);
		_feedbackButton->hide();
		connect(_feedbackButton, &QPushButton::released, this, &ILauncher::feedbackClicked);
		
		hbl->addWidget(_feedbackButton);

		hbl->addStretch(1);

		_layout->addLayout(hbl);
	}

	_achievementLabel = new QLabel(_widget);
	_achievementLabel->setProperty("library", true);
	_achievementLabel->setAlignment(Qt::AlignCenter);
	_achievementLabel->hide();
	connect(_achievementLabel, &QLabel::linkActivated, this, &ILauncher::prepareAchievementView);

	_scoresLabel = new QLabel(_widget);
	_scoresLabel->setProperty("library", true);
	_scoresLabel->setAlignment(Qt::AlignCenter);
	_scoresLabel->hide();
	connect(_scoresLabel, &QLabel::linkActivated, this, &ILauncher::prepareScoreView);

	_layout->addWidget(_achievementLabel);
	_layout->addWidget(_scoresLabel);

	_widget->setLayout(_layout);

	connect(_startButton, &QPushButton::released, this, &ILauncher::start);

	_widget->hide();
}

void ILauncher::prepareAchievementView() {
	emit openAchievementView(_projectID, _name);
}

void ILauncher::prepareScoreView() {
	emit openScoreView(_projectID, _name);
}

void ILauncher::refresh(int projectID) {
	if (projectID != _projectID) return;

	updateView(_projectID, _iniFile);
}

bool ILauncher::isRunning() const {
	return _running;
}

void ILauncher::updateStarted(int projectID) {
	_runningUpdates.append(projectID);

	refresh(projectID);
}

void ILauncher::updateFinished(int projectID) {
	_runningUpdates.removeAll(projectID);

	refresh(projectID);
}

void ILauncher::feedbackClicked() {
	Database::DBError err;
	const auto version = Database::queryNth<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "SELECT MajorVersion, MinorVersion, PatchVersion FROM updates WHERE ModID = " + std::to_string(_projectID) + " LIMIT 1;", err);

	const uint8_t versionMajor = version.empty() ? 0 : static_cast<uint8_t>(version[0]);
	const uint8_t versionMinor = version.empty() ? 0 : static_cast<uint8_t>(version[1]);
	const uint8_t versionPatch = version.empty() ? 0 : static_cast<uint8_t>(version[2]);
	
	FeedbackDialog dlg(_projectID, FeedbackDialog::Type::Project, versionMajor, versionMinor, versionPatch);
	dlg.exec();
}

void ILauncher::openDiscussionsUrl() {
	if (_discussionUrl.isEmpty()) return;

	QDesktopServices::openUrl(QUrl(_discussionUrl));
}

void ILauncher::updateCommonView(int projectID, const QString & name) {
	_projectID = projectID;
	_name = name;
	
	_installDate->hide();
	_lastPlayedDate->hide();
	_achievementLabel->hide();
	_scoresLabel->hide();
	_playTimeLabel->setText("");
	if (Config::OnlineMode) {
		_ratingWidget->setVisible(projectID != -1);
	}
	_installDate->setText("");
	_lastPlayedDate->setText("");

	if (projectID != -1) {
		if (Config::OnlineMode) {
			_ratingWidget->setProjectID(projectID);
			_ratingWidget->setModName(name);
		}
		Database::DBError err;
		auto date = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT InstallDate FROM installDates WHERE ModID = " + std::to_string(projectID) + " LIMIT 1;", err);
		if (date.empty()) {
			const int currentDate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO installDates (ModID, InstallDate) VALUES (" + std::to_string(projectID) + ", " + std::to_string(currentDate) + ");", err);
			date.push_back(currentDate);
		}
		_installDate->setText(QApplication::tr("Installed").arg(QDate(1970, 1, 1).addDays(date[0] / 24).toString("dd.MM.yyyy")));
		_installDate->show();
		
		updateModStats();
	}
}

void ILauncher::updateModInfoView(ProjectStats ms) {
	const QString timeString = timeToString(ms.duration);
	_playTimeLabel->setText(timeString);

	_achievementLabel->setText(R"(<a href="Foobar" style="color: #181C22">)" + QApplication::tr("AchievementText").arg(ms.achievedAchievements).arg(ms.allAchievements).arg(ms.achievedAchievements * 100 / (ms.allAchievements ? ms.allAchievements : 1)) + "</a>");
	_achievementLabel->setVisible(ms.allAchievements > 0);

	_scoresLabel->setText(R"(<a href="Blafoo" style="color: #181C22">)" + ((ms.bestScoreRank > 0) ? QApplication::tr("BestScoreText").arg(s2q(ms.bestScoreName)).arg(ms.bestScoreRank).arg(ms.bestScore) : QApplication::tr("NoBestScore")) + "</a>");
	_scoresLabel->setVisible(ms.bestScoreRank != -1);

	if (ms.lastTimePlayed == -1) {
		_lastPlayedDate->hide();
	} else {
		_lastPlayedDate->setText(QApplication::tr("LastPlayed").arg(QDate(1970, 1, 1).addDays(ms.lastTimePlayed / 24).toString("dd.MM.yyyy")));
		_lastPlayedDate->show();
	}

	_feedbackButton->setVisible(ms.feedbackMailAvailable);

	_discussionUrl = s2q(ms.discussionUrl);

	_discussionUrlTitle->setVisible(!ms.discussionUrl.empty());
	_discussionUrlText->setVisible(!ms.discussionUrl.empty());

	_discussionUrlText->setText(QString("<a href=\"%1\">%1</a>").arg(_discussionUrl));
}

void ILauncher::startScreenshotManager(int projectID) {
	_screenshotManager->start(projectID);
}

void ILauncher::stopScreenshotManager() {
	_screenshotManager->stop();
}

void ILauncher::startCommon() {
	DiscordManager::instance()->updatePresence(QApplication::tr("InGame"), _name);

	_listenSocket = new clockUtils::sockets::TcpSocket();
	_listenSocket->listen(LOCAL_PORT, 1, true, std::bind(&ILauncher::acceptedConnection, this, std::placeholders::_1, std::placeholders::_2));

	_running = true;
	
	_timer->restart();

	startScreenshotManager(_projectID);
}

void ILauncher::stopCommon() {
	DiscordManager::instance()->updatePresence(QApplication::tr("Browsing"), "");

	_running = false;
	
	auto duration = _timer->elapsed();
	duration = duration / 1000; // to seconds
	duration = duration / 60; // to minutes
	delete _listenSocket;
	_listenSocket = nullptr;
	delete _socket;
	_socket = nullptr;

	{
		QJsonObject json;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["Duration"] = duration;

		QJsonArray jsonArray;

		const auto activeProjects = getActiveProjects();

		for (const auto projectID : activeProjects) {
			jsonArray << projectID;
		}
		
		json["Projects"] = jsonArray;

		Https::postAsync(DATABASESERVER_PORT, "updatePlayTime", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {
			updateModStats();

			emit syncedNewStats();
		});
	}

	stopScreenshotManager();

	if (!Config::OnlineMode) return;
	
	Database::DBError err;
	const auto version = Database::queryNth<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "SELECT MajorVersion, MinorVersion, PatchVersion FROM updates WHERE ModID = " + std::to_string(_projectID) + " LIMIT 1;", err);

	const int versionMajor = version.empty() ? 0 : static_cast<uint8_t>(version[0]);
	const int versionMinor = version.empty() ? 0 : static_cast<uint8_t>(version[1]);
	const int versionPatch = version.empty() ? 0 : static_cast<uint8_t>(version[2]);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = _projectID;
	requestData["Language"] = Config::Language;
	requestData["MajorVersion"] = versionMajor;
	requestData["MinorVersion"] = versionMinor;
	requestData["PatchVersion"] = versionPatch;
	
	Https::postAsync(MANAGEMENTSERVER_PORT, "getOwnPlayTestSurveyAnswers", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this, versionMajor, versionMinor, versionPatch](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) return;

		if (!json.contains("SurveyID")) return;

		Survey s;

		s.surveyID = json["SurveyID"].toString().toInt();

		auto it = json.find("Questions");
		if (it != json.end()) {
			const auto q = it->toArray();
			for (auto questionEntry : q) {
				const auto question = questionEntry.toObject();

				if (question.isEmpty()) continue;

				if (!question.contains("QuestionID")) continue;
				
				if (!question.contains("Question")) continue;
				
				if (!question.contains("Answer")) continue;

				SurveyQuestion sq;
				
				sq.questionID = question["QuestionID"].toString().toInt();
				sq.question = question["Question"].toString();
				sq.answer = question["Answer"].toString();
				
				s.questions.append(sq);
			}
		}

		if (s.questions.isEmpty()) return;

		emit loadedSurvey(s, versionMajor, versionMinor, versionPatch);
	});
}

void ILauncher::acceptedConnection(clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError err) {
	if (err == clockUtils::ClockError::SUCCESS) {
		delete _socket;
		_socket = sock;
		sock->receiveCallback(std::bind(&ILauncher::receivedMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
}

void ILauncher::receivedMessage(std::vector<uint8_t> packet, clockUtils::sockets::TcpSocket * socket, clockUtils::ClockError err) {
	if (err == clockUtils::ClockError::SUCCESS) {
		try {
			std::string serialized(packet.begin(), packet.end());
			Message * msg = Message::DeserializeBlank(serialized);
			if (msg) {
				if (msg->type == MessageType::REQUESTUSERNAME) {
					handleRequestUsername(socket);
				} else if (msg->type == MessageType::REQUESTSCORES) {
					auto * rsm = dynamic_cast<RequestScoresMessage *>(msg);
					handleRequestScores(socket, rsm);
				} else if (msg->type == MessageType::UPDATESCORE) {
					auto * usm = dynamic_cast<UpdateScoreMessage *>(msg);
					handleUpdateScore(usm);
				} else if (msg->type == MessageType::REQUESTACHIEVEMENTS) {
					handleRequestAchievements(socket);
				} else if (msg->type == MessageType::UNLOCKACHIEVEMENT) {
					auto * uam = dynamic_cast<UnlockAchievementMessage *>(msg);
					handleUnlockAchievement(uam);
				} else if (msg->type == MessageType::UPDATEACHIEVEMENTPROGRESS) {
					auto * uapm = dynamic_cast<UpdateAchievementProgressMessage *>(msg);
					handleUpdateAchievementProgress(uapm);
				} else if (msg->type == MessageType::REQUESTOVERALLSAVEPATH) {
					handleRequestOverallSaveDataPath(socket);
				} else if (msg->type == MessageType::REQUESTOVERALLSAVEDATA) {
					auto * rom = dynamic_cast<RequestOverallSaveDataMessage *>(msg);
					handleRequestOverallSaveData(socket, rom);
				} else if (msg->type == MessageType::UPDATEOVERALLSAVEDATA) {
					auto * uom = dynamic_cast<UpdateOverallSaveDataMessage *>(msg);
					handleUpdateOverallSaveData(uom);
				} else if (msg->type == MessageType::REQUESTALLFRIENDS) {
					auto * rafm = dynamic_cast<RequestAllFriendsMessage *>(msg);
					handleRequestAllFriends(socket, rafm);
				} else if (msg->type == MessageType::UPDATECHAPTERSTATS) {
					auto * ucsm = dynamic_cast<UpdateChapterStatsMessage *>(msg);
					handleUpdateChapterStats(ucsm);
				} else if (msg->type == MessageType::ISACHIEVEMENTUNLOCKED) {
					auto * iaum = dynamic_cast<IsAchievementUnlockedMessage *>(msg);
					handleIsAchievementUnlocked(socket, iaum);
				}
			}
			delete msg;
		} catch (boost::archive::archive_exception &) {
			socket->writePacket("empty");
			return;
		}
	}
}

void ILauncher::tryCleanCaches() {
	if (!Config::OnlineMode) return;

	Database::DBError err;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
		const auto scores = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM scoreCache;", err);
		for (const auto & t : scores) {
			QJsonObject json;
			json["ProjectID"] = t[0];
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Identifier"] = t[1];
			json["Score"] = t[2];

			Https::postAsync(DATABASESERVER_PORT, "updateScore", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) return;
				
				removeScore(t[0], t[1]);
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

			Https::postAsync(DATABASESERVER_PORT, "unlockAchievement", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) return;

				removeAchievement(t[0], t[1]);
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

			Https::postAsync(DATABASESERVER_PORT, "updateAchievementProgress", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) return;
				
				removeAchievement(t[0], t[1]);
			});
		}
		const auto overallSaveDatas = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM overallSaveDataCache;", err);
		for (const auto & t : overallSaveDatas) {
			QJsonObject json;
			json["ProjectID"] = std::stoi(t[0]);
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Key"] = s2q(t[1]);
			json["Value"] = s2q(t[2]);

			Https::postAsync(DATABASESERVER_PORT, "updateOverallSaveData", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, t](const QJsonObject &, int statusCode) {
				if (statusCode != 200) return;
				
				removeOverallSaveData(std::stoi(t[0]), t[1]);
			});
		}
	}
}

void ILauncher::cacheScore(int32_t projectID, int32_t identifier, int32_t score) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO scoreCache (ModID, Identifier, Score) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", " + std::to_string(score) + ");", err);
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE scoreCache SET Score = " + std::to_string(score) + " WHERE ModID = " + std::to_string(projectID) + " AND Identifier = " + std::to_string(identifier) + ";", err);
	}
}

void ILauncher::removeScore(int32_t projectID, int32_t identifier) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM scoreCache WHERE ModID = " + std::to_string(projectID) + " AND Identifier = " + std::to_string(identifier) + ";", err);
}

void ILauncher::cacheAchievement(int32_t projectID, int32_t identifier) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO achievementCache (ModID, Identifier) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ");", err);
}

void ILauncher::removeAchievement(int32_t projectID, int32_t identifier) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM achievementCache WHERE ModID = " + std::to_string(projectID) + " AND Identifier = " + std::to_string(identifier) + ";", err);
}

void ILauncher::cacheAchievementProgress(int32_t projectID, int32_t identifier, int32_t progress) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO achievementProgressCache (ModID, Identifier, Progress) VALUES (" + std::to_string(projectID) + ", " + std::to_string(identifier) + ", " + std::to_string(progress) + ");", err);
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE achievementProgressCache SET Progress = " + std::to_string(progress) + " WHERE ModID = " + std::to_string(projectID) + " AND Identifier = " + std::to_string(identifier) + ";", err);
	}
}

void ILauncher::removeAchievementProgress(int32_t projectID, int32_t identifier) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM achievementProgressCache WHERE ModID = " + std::to_string(projectID) + " AND Identifier = " + std::to_string(identifier) + ";", err);
}

void ILauncher::cacheOverallSaveData(int32_t projectID, const std::string & key, const std::string & value) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO overallSaveDataCache (ModID, Entry, Value) VALUES (" + std::to_string(projectID) + ", '" + key + "', '" + value + "');", err);
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE overallSaveDataCache SET Value = '" + value + "' WHERE ModID = " + std::to_string(projectID) + " AND Entry = '" + key + "';", err);
	}
}

void ILauncher::removeOverallSaveData(int32_t projectID, const std::string & key) const {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM overallSaveDataCache WHERE ModID = " + std::to_string(projectID) + " AND Entry = '" + key + "';", err);
}

void ILauncher::synchronizeOfflineData() {
	// this code is incredible slow due to around 2.500 SQL inserts
	if (!Config::OnlineMode) return;
	
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
				
				QJsonArray scoresArray;
				
				const auto scores = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier, Score FROM modScores;", err2);
				for (const auto & vec : scores) {
					QJsonObject jsonScore;
					jsonScore["ProjectID"] = std::stoi(vec[0]);
					jsonScore["Identifier"] = std::stoi(vec[1]);
					jsonScore["Score"] = std::stoi(vec[2]);

					scoresArray << jsonScore;
				}

				if (!scoresArray.isEmpty()) {
					json["Scores"] = scoresArray;
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
					jsonTime["Time"] = vec[1];

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
					if (statusCode != 200) return;
					
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

bool ILauncher::isAllowedSymlinkSuffix(QString suffix) const {
	suffix = suffix.toLower();
	const bool canSymlink = suffix == "mod" || suffix == "vdf" || suffix == "sty" || suffix == "sgt" || suffix == "dls" || suffix == "bik" || suffix == "dds" || suffix == "jpg" || suffix == "png" || suffix == "mi" || suffix == "hlsl" || suffix == "h" || suffix == "vi" || suffix == "exe" || suffix == "dll" || suffix == "bin" || suffix == "mtl" || suffix == "obj" || suffix == "txt" || suffix == "rtf" || suffix == "obj" || suffix == "ico" || suffix == "ini" || suffix == "bak" || suffix == "gsp" || suffix == "pdb" || suffix == "config" || suffix == "fx" || suffix == "3ds" || suffix == "mcache" || suffix == "fxh";
	if (!canSymlink) {
		LOGINFO("Copying extension: " << suffix.toStdString())
	}
	return canSymlink;
}

bool ILauncher::linkOrCopyFile(QString sourcePath, QString destinationPath) {
#ifdef Q_OS_WIN
	const auto suffix = QFileInfo(sourcePath).suffix();
	if (IsRunAsAdmin() && isAllowedSymlinkSuffix(suffix)) {
		const bool linked = makeSymlink(sourcePath, destinationPath);
		if (linked) return true;
	}
	
	const bool linked = CreateHardLinkW(destinationPath.toStdWString().c_str(), sourcePath.toStdWString().c_str(), nullptr);
	if (!linked) {
		const bool copied = QFile::copy(sourcePath, destinationPath);
		return copied;
	}
	return linked;
#else
	const bool copied = QFile::copy(sourcePath, destinationPath);
	return copied;
#endif
}

void ILauncher::sendUserInfos(QJsonObject json) const {
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Hash"] = Hash::calculateSystemHash();
	json["Mac"] = Hash::getMAC();
	json["Language"] = Config::Language;
	json["Count"] = 1;

	Https::postAsync(DATABASESERVER_PORT, "sendUserInfos", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});
}

void ILauncher::requestSingleProjectStats(const std::function<void(bool)> & resultCallback) {
	QJsonObject json;
	json["ProjectID"] = _projectID;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Language"] = Config::Language;

	Https::postAsync(DATABASESERVER_PORT, "requestSingleProjectStats", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, resultCallback](const QJsonObject & jsonResponse, int statusCode) {
		bool b = statusCode == 200;

		do {
			ProjectStats stats;

			if (!jsonResponse.contains("ProjectID")) {
				b = false;
				break;
			}

			if (!jsonResponse.contains("Name")) {
				b = false;
				break;
			}

			if (!jsonResponse.contains("Type")) {
				b = false;
				break;
			}

			if (!jsonResponse.contains("LastTimePlayed")) {
				b = false;
				break;
			}

			if (!jsonResponse.contains("Duration")) {
				b = false;
				break;
			}

			stats.projectID = jsonResponse["ProjectID"].toString().toInt();
			stats.name = q2s(jsonResponse["Name"].toString());
			stats.type = static_cast<ModType>(jsonResponse["Type"].toString().toInt());
			stats.lastTimePlayed = jsonResponse["LastTimePlayed"].toString().toInt();
			stats.duration = jsonResponse["Duration"].toString().toInt();

			if (jsonResponse.contains("AchievedAchievements")) {
				stats.achievedAchievements = jsonResponse["AchievedAchievements"].toString().toInt();
			}

			if (jsonResponse.contains("AllAchievements")) {
				stats.allAchievements = jsonResponse["AllAchievements"].toString().toInt();
			}

			if (jsonResponse.contains("BestScoreName")) {
				stats.bestScoreName = q2s(jsonResponse["BestScoreName"].toString());
			}

			if (jsonResponse.contains("BestScoreRank")) {
				stats.bestScoreRank = jsonResponse["BestScoreRank"].toString().toInt();
			}

			if (jsonResponse.contains("BestScore")) {
				stats.bestScore = jsonResponse["BestScore"].toString().toInt();
			}

			if (jsonResponse.contains("FeedbackMailAvailable")) {
				stats.feedbackMailAvailable = jsonResponse["FeedbackMailAvailable"].toString().toInt();
			}

			if (jsonResponse.contains("DiscussionUrl")) {
				stats.discussionUrl = q2s(jsonResponse["DiscussionUrl"].toString());
			}
			
			emit receivedModStats(stats);
		} while (false);
		
		resultCallback(b);
	});
}

QList<int32_t> ILauncher::getActiveProjects() const {
	return { _projectID };
}

void ILauncher::handleRequestUsername(clockUtils::sockets::TcpSocket * socket) const {
	SendUsernameMessage sum;
	sum.username = Config::Username.toStdString();
	sum.modID = _projectID;
	sum.userID = Config::UserID;
	const auto serialized = sum.SerializeBlank();
	socket->writePacket(serialized);
}

void ILauncher::handleRequestScores(clockUtils::sockets::TcpSocket * socket, RequestScoresMessage * msg) const {
	if (_projectID == -1) {
		const SendScoresMessage ssm;
		const auto serialized = ssm.SerializeBlank();
		socket->writePacket(serialized);
		return;
	}
	if (!msg) {
		socket->writePacket("empty");
		return;
	}
	
	if (Config::OnlineMode) {
		QJsonObject json;
		json["ProjectID"] = _projectID;
		Https::postAsync(DATABASESERVER_PORT, "requestScores", QJsonDocument(json).toJson(QJsonDocument::Compact), [socket](const QJsonObject & data, int statusCode) {
			if (statusCode != 200) {
				socket->writePacket("empty");
				return;
			}

			SendScoresMessage ssm;

			if (data.contains("Scores")) {
				QJsonArray scoresArray = data["Scores"].toArray();
				for (const auto score : scoresArray) {
					QJsonObject jsonScore = score.toObject();

					std::vector<std::pair<std::string, int32_t>> scores;

					QJsonArray scoreEntriesArray = jsonScore["Scores"].toArray();

					for (const auto scoreEntry : scoreEntriesArray) {
						QJsonObject jsonScoreEntry = scoreEntry.toObject();
						scores.emplace_back(jsonScoreEntry["Username"].toString().toStdString(), jsonScoreEntry["Score"].toString().toInt());
					}
					
					ssm.scores.emplace_back(jsonScore["ID"].toString().toInt(), scores);
				}
			}
			
			const auto serialized = ssm.SerializeBlank();
			socket->writePacket(serialized);
		});
	} else {
		Database::DBError dbErr;
		const auto scoreOrderResults = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, ScoreOrder FROM scoreOrders WHERE ProjectID = " + std::to_string(_projectID), dbErr);
		QMap<int, ScoreOrder> scoreOrders;

		for (const auto & vec : scoreOrderResults) {
			scoreOrders.insert(vec[0], static_cast<ScoreOrder>(vec[1]));
		}
		
		const auto lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Username, Score FROM modScores WHERE ModID = " + std::to_string(_projectID) + " ORDER BY Score DESC", dbErr);
		SendScoresMessage ssm;
		std::map<int, std::vector<std::pair<std::string, int32_t>>> scores;
		for (const auto & vec : lastResults) {
			auto identifier = static_cast<int32_t>(std::stoi(vec[0]));
			std::string username = vec[1];
			auto score = static_cast<int32_t>(std::stoi(vec[2]));
			if (!username.empty()) {
				scores[identifier].push_back(std::make_pair(username, score));
			}
		}

		for (auto & score : scores) {
			const auto scoreOrder = scoreOrders.value(score.first, ScoreOrder::Descending);

			if (scoreOrder == ScoreOrder::Ascending) {
				std::reverse(score.second.begin(), score.second.end());
			}
			
			ssm.scores.emplace_back(score.first, score.second);
		}
		const auto serialized = ssm.SerializeBlank();
		socket->writePacket(serialized);
	}
}

void ILauncher::handleUpdateScore(UpdateScoreMessage * msg) const {
	if (_projectID == -1) return;
	
	if (!msg) return;
	
	if (Config::OnlineMode) {
		QJsonObject json;
		json["ProjectID"] = _projectID;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["Identifier"] = msg->identifier;
		json["Score"] = msg->score;

		int32_t identifier = msg->identifier;
		int32_t score = msg->score;

		Https::postAsync(DATABASESERVER_PORT, "updateScore", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, identifier, score](const QJsonObject &, int statusCode) {
			if (statusCode != 200) {
				cacheScore(_projectID, identifier, score);
				return;
			}
			removeScore(_projectID, identifier);
		});
	} else {
		Database::DBError dbErr;
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(msg->identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(msg->score) + ");", dbErr);
		if (dbErr.error) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modScores SET Score = " + std::to_string(msg->score) + " WHERE ModID = " + std::to_string(_projectID) + " AND Identifier = " + std::to_string(msg->identifier) + " AND Username = '" + Config::Username.toStdString() + "';", dbErr);
		}
	}
}

void ILauncher::handleRequestAchievements(clockUtils::sockets::TcpSocket * socket) const {
	if (_projectID == -1) {
		SendAchievementsMessage sam;
		sam.showAchievements = _showAchievements;
		const auto serialized = sam.SerializeBlank();
		socket->writePacket(serialized);
		return;
	}
	
	if (!Config::OnlineMode || Config::Username.isEmpty()) {
		Database::DBError dbErr;
		std::vector<std::string> lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(_projectID) + ";", dbErr);

		SendAchievementsMessage sam;
		for (const std::string & s : lastResults) {
			auto identifier = static_cast<int32_t>(std::stoi(s));
			sam.achievements.push_back(identifier);
		}
		auto lastResultsVec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Max FROM modAchievementProgressMax WHERE ModID = " + std::to_string(_projectID) + ";", dbErr);
		for (const auto & vec : lastResultsVec) {
			lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE ModID = " + std::to_string(_projectID) + " AND Identifier = " + vec[0] + " LIMIT 1;", dbErr);
			if (lastResults.empty()) {
				sam.achievementProgress.emplace_back(std::stoi(vec[0]), std::make_pair(0, std::stoi(vec[1])));
			} else {
				sam.achievementProgress.emplace_back(std::stoi(vec[0]), std::make_pair(std::stoi(lastResults[0]), std::stoi(vec[1])));
			}
		}
		const auto serialized = sam.SerializeBlank();
		socket->writePacket(serialized);
		
		return;
	}

	QJsonObject json;
	json["ProjectID"] = _projectID;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	
	Https::postAsync(DATABASESERVER_PORT, "requestAchievements", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, socket](const QJsonObject & data, int statusCode) {
		if (statusCode != 200) {
			socket->writePacket("empty");
			return;
		}

		SendAchievementsMessage sam;

		if (data.contains("Achievements")) {
			QJsonArray achievementsArray = data["Achievements"].toArray();
			for (const auto achievementRef : achievementsArray) {
				sam.achievements.push_back(achievementRef.toString().toInt());
			}
		}

		if (data.contains("AchievementProgress")) {
			QJsonArray achievementProgressArray = data["AchievementProgress"].toArray();
			for (const auto achievementProgressRef : achievementProgressArray) {
				QJsonObject achievementProgress = achievementProgressRef.toObject();

				const int identifier = achievementProgress["Identifier"].toString().toInt();
				const int maxProgress = achievementProgress["Maximum"].toString().toInt();

				const int currentProgress = achievementProgress.contains("Current") ? achievementProgress["Current"].toString().toInt() : 0;
				
				sam.achievementProgress.emplace_back(identifier, std::make_pair(currentProgress, maxProgress));
			}
		}

		sam.showAchievements = _showAchievements;

		const auto serialized = sam.SerializeBlank();
		socket->writePacket(serialized);
	});
}

void ILauncher::handleUnlockAchievement(UnlockAchievementMessage * msg) const {
	if (_projectID == -1) return;

	if (!msg) return;

	if (Config::OnlineMode) {
		auto duration = _timer->elapsed();
		duration = duration / 1000; // to seconds
		duration = duration / 60;
		
		QJsonObject json;
		json["ProjectID"] = _projectID;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["Identifier"] = msg->identifier;
		json["Duration"] = duration;

		int32_t identifier = msg->identifier;

		Https::postAsync(DATABASESERVER_PORT, "unlockAchievement", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, identifier](const QJsonObject &, int statusCode) {
			if (statusCode != 200) {
				cacheAchievement(_projectID, identifier);
				return;
			}
			removeAchievement(_projectID, identifier);
		});
	} else {
		Database::DBError dbErr;
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(msg->identifier) + ", '" + Config::Username.toStdString() + ");", dbErr);
	}
}

void ILauncher::handleUpdateAchievementProgress(UpdateAchievementProgressMessage * msg) const {
	if (_projectID == -1) return;

	if (!msg) return;

	if (Config::OnlineMode) {
		QJsonObject json;
		json["ProjectID"] = _projectID;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["Identifier"] = msg->identifier;
		json["Progress"] = msg->progress;

		int32_t identifier = msg->identifier;
		int32_t progress = msg->progress;

		Https::postAsync(DATABASESERVER_PORT, "updateAchievementProgress", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, identifier, progress](const QJsonObject &, int statusCode) {
			if (statusCode != 200) {
				cacheAchievementProgress(_projectID, identifier, progress);
				return;
			}
			removeAchievement(_projectID, identifier);
		});
	} else {
		Database::DBError dbErr;
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(msg->identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(msg->progress) + ");", dbErr);
		if (dbErr.error) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modAchievementProgress SET Current = " + std::to_string(msg->progress) + " WHERE ModID = " + std::to_string(_projectID) + " AND Identifier = " + std::to_string(msg->identifier) + " AND Username = '" + Config::Username.toStdString() + "';", dbErr);
		}
	}
}

void ILauncher::handleRequestOverallSaveDataPath(clockUtils::sockets::TcpSocket * socket) const {
	const QString overallSavePath = getOverallSavePath();
	SendOverallSavePathMessage sospm;
#ifdef Q_OS_WIN
	sospm.path = q2ws(overallSavePath);
#else
	sospm.path = q2s(overallSavePath);
#endif
	socket->writePacket(sospm.SerializeBlank());
}

void ILauncher::handleRequestOverallSaveData(clockUtils::sockets::TcpSocket * socket, RequestOverallSaveDataMessage * msg) const {
	if (_projectID == -1) {
		SendOverallSaveDataMessage som;
		const auto serialized = som.SerializeBlank();
		socket->writePacket(serialized);
		return;
	}

	if (!msg) return;

	if (Config::OnlineMode) {
		QJsonObject json;
		json["ProjectID"] = _projectID;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;

		Https::postAsync(DATABASESERVER_PORT, "requestOverallSaveData", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, socket](const QJsonObject & data, int statusCode) {
			if (statusCode != 200) {
				socket->writePacket("empty");
				return;
			}
			SendOverallSaveDataMessage som;

			if (data.contains("Data")) {
				const auto arr = data["Data"].toArray();
				for (const auto jsonRef : arr) {
					const auto json = jsonRef.toObject();
					som.data.emplace_back(q2s(json["Key"].toString()), q2s(json["Value"].toString()));
				}
			}

			const auto serialized = som.SerializeBlank();
			
			socket->writePacket(serialized);
		});
	} else {
		SendOverallSaveDataMessage som;
		Database::DBError dbErr;
		const auto lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Entry, Value FROM overallSaveData WHERE ModID = " + std::to_string(_projectID) + " AND Username = '" = Config::Username.toStdString() + "';", dbErr);
		for (const auto & vec : lastResults) {
			if (vec.size() == 2) {
				som.data.emplace_back(vec[0], vec[1]);
			}
		}
		const auto serialized = som.SerializeBlank();
		socket->writePacket(serialized);
	}
}

void ILauncher::handleUpdateOverallSaveData(UpdateOverallSaveDataMessage * msg) const {
	if (_projectID == -1) return;

	if (!msg) return;

	if (Config::OnlineMode) {
		QJsonObject json;
		json["ProjectID"] = _projectID;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["Key"] = s2q(msg->entry);
		json["Value"] = s2q(msg->value);

		const auto key = msg->entry;
		const auto value = msg->value;

		Https::postAsync(DATABASESERVER_PORT, "updateOverallSaveData", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, key, value](const QJsonObject &, int statusCode) {
			if (statusCode != 200) {
				cacheOverallSaveData(_projectID, key, value);
				return;
			}
			removeOverallSaveData(_projectID, key);
		});
	} else {
		Database::DBError dbErr;
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(_projectID) + ", '" + Config::Username.toStdString() + "', '" + msg->entry + "', '" + msg->value + "');", dbErr);
		if (dbErr.error) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE overallSaveData SET Value = '" + msg->value + "' WHERE ModID = " + std::to_string(_projectID) + " AND Entry = '" + msg->entry + "' AND Username = '" + Config::Username.toStdString() + "';", dbErr);
		}
	}
}

void ILauncher::handleRequestAllFriends(clockUtils::sockets::TcpSocket * socket, RequestAllFriendsMessage * msg) const {
	if (_projectID == -1) {
		const SendAllFriendsMessage safm;
		const auto serialized = safm.SerializeBlank();
		socket->writePacket(serialized);
		return;
	}

	if (!msg) return;

	if (Config::OnlineMode) {
		QJsonObject json;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["FriendsOnly"] = 1;

		Https::postAsync(DATABASESERVER_PORT, "requestAllFriends", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, socket](const QJsonObject & data, int statusCode) {
			if (statusCode != 200) {
				socket->writePacket("empty");
				return;
			}
			SendAllFriendsMessage safm;

			if (data.contains("Friends")) {
				const auto arr = data["Friends"].toArray();
				for (const auto jsonRef : arr) {
					const auto json2 = jsonRef.toObject();

					const Friend f(q2s(json2["Name"].toString()), json2["Level"].toString().toInt());
					
					safm.friends.push_back(f);
				}
			}

			const auto serialized = safm.SerializeBlank();

			socket->writePacket(serialized);
		});
	} else {
		const SendAllFriendsMessage safm;
		const auto serialized = safm.SerializeBlank();
		socket->writePacket(serialized);
	}
}

void ILauncher::handleUpdateChapterStats(UpdateChapterStatsMessage * msg) const {
	if (_projectID == -1) return;

	if (!msg) return;

	if (!Config::OnlineMode) return;
	
	QJsonObject json;
	json["ProjectID"] = _projectID;
	json["Identifier"] = msg->identifier;
	json["Guild"] = msg->guild;
	json["Key"] = s2q(msg->statName);
	json["Value"] = msg->statValue;

	Https::postAsync(DATABASESERVER_PORT, "updateChapterStats", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});
}

void ILauncher::handleIsAchievementUnlocked(clockUtils::sockets::TcpSocket * socket, IsAchievementUnlockedMessage * msg) const {
	if (_projectID == -1) {
		const SendAchievementUnlockedMessage saum;
		const auto serialized = saum.SerializeBlank();
		socket->writePacket(serialized);
		return;
	}

	if (!msg) return;

	if (Config::OnlineMode) {
		QJsonObject json;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["ProjectID"] = msg->modID;
		json["AchievementID"] = msg->achievementID;

		Https::postAsync(DATABASESERVER_PORT, "isAchievementUnlocked", QJsonDocument(json).toJson(QJsonDocument::Compact), [socket](const QJsonObject & data, int statusCode) {
			if (statusCode != 200) {
				socket->writePacket("empty");
				return;
			}
			
			SendAchievementUnlockedMessage saum;
			saum.unlocked = data.contains("Unlocked");

			const auto serialized = saum.SerializeBlank();

			socket->writePacket(serialized);
		});
	} else {
		Database::DBError dbErr;
		const auto lastResults = Database::queryAll<std::vector<std::string>, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(msg->modID) + " AND Identifier = " + std::to_string(msg->achievementID) + "", dbErr);
		SendAchievementUnlockedMessage saum;
		saum.unlocked = !lastResults.empty();
		const auto serialized = saum.SerializeBlank();
		socket->writePacket(serialized);
	}
}
