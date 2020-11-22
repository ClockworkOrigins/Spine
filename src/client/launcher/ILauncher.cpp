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
	
	qRegisterMetaType<common::ProjectStats>("common::ProjectStats");

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
			_ratingWidget->setModID(projectID);
			_ratingWidget->setModName(name);
		}
		Database::DBError err;
		auto date = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT InstallDate FROM installDates WHERE ModID = " + std::to_string(projectID) + " LIMIT 1;", err);
		if (date.empty()) {
			const int currentDate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO installDates (ModID, InstallDate) VALUES (" + std::to_string(projectID) + ", " + std::to_string(currentDate) +");", err);
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
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "getOwnPlayTestSurveyAnswers", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this, versionMajor, versionMinor, versionPatch](const QJsonObject & json, int statusCode) {
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
					SendUsernameMessage sum;
					sum.username = Config::Username.toStdString();
					sum.modID = _projectID;
					sum.userID = Config::UserID;
					serialized = sum.SerializeBlank();
					socket->writePacket(serialized);
				} else if (msg->type == MessageType::REQUESTSCORES) {
					auto * rsm = dynamic_cast<RequestScoresMessage *>(msg);
					if (_projectID != -1) {
						if (rsm) {
							rsm->modID = _projectID;
							if (Config::OnlineMode) {
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = rsm->SerializePublic();
									if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
										if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
											Message * newMsg = Message::DeserializePublic(serialized);
											serialized = newMsg->SerializeBlank();
											delete newMsg;
											socket->writePacket(serialized);
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								Database::DBError dbErr;
								std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Username, Score FROM modScores WHERE ModID = " + std::to_string(_projectID) + " ORDER BY Score DESC", dbErr);
								SendScoresMessage ssm;
								std::map<int, std::vector<std::pair<std::string, int32_t>>> scores;
								for (auto vec : lastResults) {
									auto identifier = static_cast<int32_t>(std::stoi(vec[0]));
									std::string username = vec[1];
									auto score = static_cast<int32_t>(std::stoi(vec[2]));
									if (!username.empty()) {
										scores[identifier].push_back(std::make_pair(username, score));
									}
								}
								for (auto & score : scores) {
									ssm.scores.emplace_back(score.first, score.second);
								}
								serialized = ssm.SerializeBlank();
								socket->writePacket(serialized);
							}
						} else {
							socket->writePacket("empty");
						}
					} else {
						SendScoresMessage ssm;
						serialized = ssm.SerializeBlank();
						socket->writePacket(serialized);
					}
				} else if (msg->type == MessageType::UPDATESCORE) {
					auto * usm = dynamic_cast<UpdateScoreMessage *>(msg);
					if (_projectID != -1) {
						if (usm) {
							if (Config::OnlineMode) {
								usm->modID = _projectID;
								usm->username = Config::Username.toStdString();
								usm->password = Config::Password.toStdString();
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = usm->SerializePublic();
									if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
										cacheScore(usm);
									} else {
										removeScore(usm);
									}
								} else {
									cacheScore(usm);
								}
							} else {
								Database::DBError dbErr;
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(usm->identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(usm->score) + ");", dbErr);
								if (dbErr.error) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modScores SET Score = " + std::to_string(usm->score) + " WHERE ModID = " + std::to_string(_projectID) + " AND Identifier = " + std::to_string(usm->identifier) + " AND Username = '" + Config::Username.toStdString() + "';", dbErr);
								}
							}
						}
					}
				} else if (msg->type == MessageType::REQUESTACHIEVEMENTS) {
					auto * ram = dynamic_cast<RequestAchievementsMessage *>(msg);
					if (_projectID != -1) {
						if (ram && Config::OnlineMode && !Config::Username.isEmpty()) {
							ram->modID = _projectID;
							ram->username = Config::Username.toStdString();
							ram->password = Config::Password.toStdString();
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								serialized = ram->SerializePublic();
								if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
									if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
										auto * sam = dynamic_cast<SendAchievementsMessage *>(Message::DeserializePublic(serialized));
										sam->showAchievements = _showAchievements;
										serialized = sam->SerializeBlank();
										socket->writePacket(serialized);
										delete sam;
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							Database::DBError dbErr;
							std::vector<std::string> lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(_projectID) + ";", dbErr);

							SendAchievementsMessage sam;
							for (const std::string & s : lastResults) {
								auto identifier = static_cast<int32_t>(std::stoi(s));
								sam.achievements.push_back(identifier);
							}
							auto lastResultsVec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Max FROM modAchievementProgressMax WHERE ModID = " + std::to_string(_projectID) + ";", dbErr);
							for (auto vec : lastResultsVec) {
								lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE ModID = " + std::to_string(_projectID) + " AND Identifier = " + vec[0] + " LIMIT 1;", dbErr);
								if (lastResults.empty()) {
									sam.achievementProgress.emplace_back(std::stoi(vec[0]), std::make_pair(0, std::stoi(vec[1])));
								} else {
									sam.achievementProgress.emplace_back(std::stoi(vec[0]), std::make_pair(std::stoi(lastResults[0]), std::stoi(vec[1])));
								}
							}
							serialized = sam.SerializeBlank();
							socket->writePacket(serialized);
						}
					} else {
						SendAchievementsMessage sam;
						sam.showAchievements = _showAchievements;
						serialized = sam.SerializeBlank();
						socket->writePacket(serialized);
					}
				} else if (msg->type == MessageType::UNLOCKACHIEVEMENT) {
					auto * uam = dynamic_cast<UnlockAchievementMessage *>(msg);
					if (_projectID != -1) {
						if (uam) {
							if (Config::OnlineMode) {
								auto duration = _timer->elapsed();
								duration = duration / 1000; // to seconds
								duration = duration / 60;

								uam->modID = _projectID;
								uam->username = Config::Username.toStdString();
								uam->password = Config::Password.toStdString();
								uam->duration = duration;
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = uam->SerializePublic();
									if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
										cacheAchievement(uam);
									} else {
										removeAchievement(uam);
									}
								} else {
									cacheAchievement(uam);
								}
							} else {
								uam->modID = _projectID;
								
								cacheAchievement(uam);
								
								Database::DBError dbErr;
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(uam->identifier) + ", '" + Config::Username.toStdString() + ");", dbErr);
							}
						}
					}
				} else if (msg->type == MessageType::UPDATEACHIEVEMENTPROGRESS) {
					auto * uapm = dynamic_cast<UpdateAchievementProgressMessage *>(msg);
					if (_projectID != -1) {
						if (uapm) {
							if (Config::OnlineMode) {
								uapm->modID = _projectID;
								uapm->username = Config::Username.toStdString();
								uapm->password = Config::Password.toStdString();
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = uapm->SerializePublic();
									if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
										cacheAchievementProgress(uapm);
									} else {
										removeAchievementProgress(uapm);
									}
								} else {
									cacheAchievementProgress(uapm);
								}
							} else {
								Database::DBError dbErr;
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(uapm->identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(uapm->progress) + ");", dbErr);
								if (dbErr.error) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modAchievementProgress SET Current = " + std::to_string(uapm->progress) + " WHERE ModID = " + std::to_string(_projectID) + " AND Identifier = " + std::to_string(uapm->identifier) + " AND Username = '" + Config::Username.toStdString() + "';", dbErr);
								}
							}
						}
					}
				} else if (msg->type == MessageType::SEARCHMATCH) {
					if (_projectID != -1 && Config::OnlineMode) {
						auto * smm = dynamic_cast<SearchMatchMessage *>(msg);
						if (smm) {
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								smm->modID = _projectID;
								smm->username = Config::Username.toStdString();
								smm->password = Config::Password.toStdString();
								serialized = smm->SerializePublic();
								if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
									if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
										Message * newMsg = Message::DeserializePublic(serialized);
										serialized = newMsg->SerializeBlank();
										delete newMsg;
										socket->writePacket(serialized);
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							socket->writePacket("empty");
						}
					} else {
						FoundMatchMessage fmm;
						serialized = fmm.SerializeBlank();
						socket->writePacket(serialized);
					}
				} else if (msg->type == MessageType::REQUESTOVERALLSAVEPATH) {
					auto * rospm = dynamic_cast<RequestOverallSavePathMessage *>(msg);
					if (rospm) {
						QString overallSavePath = getOverallSavePath();
						SendOverallSavePathMessage sospm;
#ifdef Q_OS_WIN
						sospm.path = q2ws(overallSavePath);
#else
						sospm.path = q2s(overallSavePath);
#endif
						socket->writePacket(sospm.SerializeBlank());
					} else {
						socket->writePacket("empty");
					}
				} else if (msg->type == MessageType::REQUESTOVERALLSAVEDATA) {
					auto * rom = dynamic_cast<RequestOverallSaveDataMessage *>(msg);
					if (_projectID != -1) {
						if (rom && !Config::Username.isEmpty()) {
							if (Config::OnlineMode) {
								rom->modID = _projectID;
								rom->username = Config::Username.toStdString();
								rom->password = Config::Password.toStdString();
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = rom->SerializePublic();
									if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
										if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
											serialized = Message::DeserializePublic(serialized)->SerializeBlank();
											socket->writePacket(serialized);
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								SendOverallSaveDataMessage som;
								Database::DBError dbErr;
								std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Entry, Value FROM overallSaveData WHERE ModID = " + std::to_string(_projectID) + " AND Username = '" = Config::Username.toStdString() + "';", dbErr);
								for (auto vec : lastResults) {
									if (vec.size() == 2) {
										som.data.emplace_back(vec[0], vec[1]);
									}
								}
								serialized = som.SerializeBlank();
								socket->writePacket(serialized);
							}
						} else {
							socket->writePacket("empty");
						}
					} else {
						SendOverallSaveDataMessage som;
						serialized = som.SerializeBlank();
						socket->writePacket(serialized);
					}
				} else if (msg->type == MessageType::UPDATEOVERALLSAVEDATA) {
					auto * uom = dynamic_cast<UpdateOverallSaveDataMessage *>(msg);
					if (_projectID != -1) {
						if (uom) {
							if (Config::OnlineMode) {
								uom->modID = _projectID;
								uom->username = Config::Username.toStdString();
								uom->password = Config::Password.toStdString();
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = uom->SerializePublic();
									if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
										cacheOverallSaveData(uom);
									} else {
										removeOverallSaveData(uom);
									}
								} else {
									cacheOverallSaveData(uom);
								}
							} else {
								Database::DBError dbErr;
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(_projectID) + ", '" + Config::Username.toStdString() + "', '" + uom->entry + "', '" + uom->value + "');", dbErr);
								if (dbErr.error) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE overallSaveData SET Value = '" + uom->value + "' WHERE ModID = " + std::to_string(_projectID) + " AND Entry = '" + uom->entry + "' AND Username = '" + Config::Username.toStdString() + "';", dbErr);
								}
							}
						}
					}
				} else if (msg->type == MessageType::REQUESTALLFRIENDS) {
					auto * rafm = dynamic_cast<RequestAllFriendsMessage *>(msg);
					if (Config::OnlineMode) {
						if (rafm && !Config::Username.isEmpty()) {
							rafm->username = Config::Username.toStdString();
							rafm->password = Config::Password.toStdString();
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								serialized = rafm->SerializePublic();
								if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
									if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
										auto * safm = dynamic_cast<SendAllFriendsMessage *>(Message::DeserializePublic(serialized));
										serialized = safm->SerializeBlank();
										socket->writePacket(serialized);
										delete safm;
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							socket->writePacket("empty");
						}
					} else {
						SendAllFriendsMessage safm;
						serialized = safm.SerializeBlank();
						socket->writePacket(serialized);
					}
				} else if (msg->type == MessageType::UPDATECHAPTERSTATS) {
					auto * ucsm = dynamic_cast<UpdateChapterStatsMessage *>(msg);
					if (_projectID != -1) {
						if (ucsm) {
							if (Config::OnlineMode) {
								ucsm->modID = _projectID;
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = ucsm->SerializePublic();
									sock.writePacket(serialized);
								}
							}
						}
					}
				} else if (msg->type == MessageType::ISACHIEVEMENTUNLOCKED) {
					auto * iaum = dynamic_cast<IsAchievementUnlockedMessage *>(msg);
					if (_projectID != -1) {
						if (iaum) {
							iaum->username = q2s(Config::Username);
							iaum->password = q2s(Config::Password);
							if (Config::OnlineMode) {
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = iaum->SerializePublic();
									if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
										if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
											Message * newMsg = Message::DeserializePublic(serialized);
											serialized = newMsg->SerializeBlank();
											delete newMsg;
											socket->writePacket(serialized);
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								Database::DBError dbErr;
								std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(iaum->modID) + " AND Identifier = " + std::to_string(iaum->achievementID) + "", dbErr);
								SendAchievementUnlockedMessage saum;
								saum.unlocked = !lastResults.empty();
								serialized = saum.SerializeBlank();
								socket->writePacket(serialized);
							}
						} else {
							socket->writePacket("empty");
						}
					} else {
						SendScoresMessage ssm;
						serialized = ssm.SerializeBlank();
						socket->writePacket(serialized);
					}
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
	if (!Config::OnlineMode) {
		return;
	}
	Database::DBError err;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
		std::vector<std::vector<int>> scores = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM scoreCache;", err);
		for (auto t : scores) {
			UpdateScoreMessage usm;
			usm.modID = t[0];
			usm.identifier = t[1];
			usm.score = t[2];
			usm.username = Config::Username.toStdString();
			usm.password = Config::Password.toStdString();
			const std::string serialized = usm.SerializePublic();
			if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
				removeScore(&usm);
			}
		}
		std::vector<std::vector<int>> achievements = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM achievementCache;", err);
		for (auto t : achievements) {
			UnlockAchievementMessage uam;
			uam.modID = t[0];
			uam.identifier = t[1];
			uam.username = Config::Username.toStdString();
			uam.password = Config::Password.toStdString();
			const std::string serialized = uam.SerializePublic();
			if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
				removeAchievement(&uam);
			}
		}
		std::vector<std::vector<int>> achievementProgresses = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM achievementProgressCache;", err);
		for (auto t : achievementProgresses) {
			UpdateAchievementProgressMessage uapm;
			uapm.modID = t[0];
			uapm.identifier = t[1];
			uapm.progress = t[2];
			uapm.username = Config::Username.toStdString();
			uapm.password = Config::Password.toStdString();
			const std::string serialized = uapm.SerializePublic();
			if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
				removeAchievementProgress(&uapm);
			}
		}
		std::vector<std::vector<std::string>> overallSaveDatas = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM overallSaveDataCache;", err);
		for (auto t : overallSaveDatas) {
			UpdateOverallSaveDataMessage uom;
			uom.modID = std::stoi(t[0]);
			uom.entry = t[1];
			uom.value = t[2];
			uom.username = Config::Username.toStdString();
			uom.password = Config::Password.toStdString();
			const std::string serialized = uom.SerializePublic();
			if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
				removeOverallSaveData(&uom);
			}
		}
	}
}

void ILauncher::cacheScore(UpdateScoreMessage * usm) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO scoreCache (ModID, Identifier, Score) VALUES (" + std::to_string(usm->modID) + ", " + std::to_string(usm->identifier) + ", " + std::to_string(usm->score) + ");", err);
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE scoreCache SET Score = " + std::to_string(usm->score) + " WHERE ModID = " + std::to_string(usm->modID) + " AND Identifier = " + std::to_string(usm->identifier) + ";", err);
	}
}

void ILauncher::removeScore(UpdateScoreMessage * usm) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM scoreCache WHERE ModID = " + std::to_string(usm->modID) + " AND Identifier = " + std::to_string(usm->identifier) + ";", err);
}

void ILauncher::cacheAchievement(UnlockAchievementMessage * uam) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO achievementCache (ModID, Identifier) VALUES (" + std::to_string(uam->modID) + ", " + std::to_string(uam->identifier) + ");", err);
}

void ILauncher::removeAchievement(UnlockAchievementMessage * uam) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM achievementCache WHERE ModID = " + std::to_string(uam->modID) + " AND Identifier = " + std::to_string(uam->identifier) + ";", err);
}

void ILauncher::cacheAchievementProgress(UpdateAchievementProgressMessage * uapm) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO achievementProgressCache (ModID, Identifier, Progress) VALUES (" + std::to_string(uapm->modID) + ", " + std::to_string(uapm->identifier) + ", " + std::to_string(uapm->progress) + ");", err);
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE achievementProgressCache SET Progress = " + std::to_string(uapm->progress) + " WHERE ModID = " + std::to_string(uapm->modID) + " AND Identifier = " + std::to_string(uapm->identifier) + ";", err);
	}
}

void ILauncher::removeAchievementProgress(UpdateAchievementProgressMessage * uapm) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM achievementProgressCache WHERE ModID = " + std::to_string(uapm->modID) + " AND Identifier = " + std::to_string(uapm->identifier) + ";", err);
}

void ILauncher::cacheOverallSaveData(UpdateOverallSaveDataMessage * uom) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO overallSaveDataCache (ModID, Entry, Value) VALUES (" + std::to_string(uom->modID) + ", '" + uom->entry + "', '" + uom->value + "');", err);
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE overallSaveDataCache SET Value = '" + uom->value + "' WHERE ModID = " + std::to_string(uom->modID) + " AND Entry = '" + uom->entry + "';", err);
	}
}

void ILauncher::removeOverallSaveData(UpdateOverallSaveDataMessage * uom) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM overallSaveDataCache WHERE ModID = " + std::to_string(uom->modID) + " AND Entry = '" + uom->entry + "';", err);
}

void ILauncher::synchronizeOfflineData() {
	// this code is incredible slow due to around 2.500 SQL inserts
	QtConcurrent::run([]() {
		try {
			if (Config::OnlineMode) {
				// update server from local data in case Sync flag is set
				Database::DBError err;
				const int sync = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Enabled FROM sync LIMIT 1;", err);
				if (sync) {
					{
						clockUtils::sockets::TcpSocket sock;
						if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM sync;", err);
							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO sync (Enabled) VALUES (0);", err);
							UpdateOfflineDataMessage uodm;
							const auto achievements = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier FROM modAchievements;", err);
							for (const auto & vec : achievements) {
								UpdateOfflineDataMessage::AchievementData ad {};
								ad.modID = std::stoi(vec[0]);
								ad.identifier = std::stoi(vec[1]);
								const auto progress = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE Username = '" + Config::Username.toStdString() + "' AND ModID = " + vec[0] + " AND Identifier = " + vec[1] + " LIMIT 1;", err);
								if (!progress.empty()) {
									ad.current = progress[0];
								}
								uodm.achievements.push_back(ad);
							}
							const auto scores = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier, Score FROM modScores;", err);
							for (const auto & vec : scores) {
								UpdateOfflineDataMessage::ScoreData sd {};
								sd.modID = std::stoi(vec[0]);
								sd.identifier = std::stoi(vec[1]);
								sd.score = std::stoi(vec[2]);
								uodm.scores.push_back(sd);
							}
							const auto overallSaveData = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Entry, Value FROM overallSaveData;", err);
							for (const auto & vec : overallSaveData) {
								UpdateOfflineDataMessage::OverallSaveData od;
								od.modID = std::stoi(vec[0]);
								od.entry = vec[1];
								od.value = vec[2];
								uodm.overallSaves.push_back(od);
							}
							const auto playTimes = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Duration FROM playTimes;", err);
							for (const auto & vec : playTimes) {
								uodm.playTimes.emplace_back(vec[0], vec[1]);
							}
							Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes;", err);
							const std::string serialized = uodm.SerializePublic();
							sock.writePacket(serialized);
						}
					}
					{
						// Load data from server
						clockUtils::sockets::TcpSocket sock;
						if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
							RequestOfflineDataMessage rodm;
							rodm.username = Config::Username.toStdString();
							rodm.password = Config::Password.toStdString();
							std::string serialized = rodm.SerializePublic();
							sock.writePacket(serialized);
							if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
								Message * msg = Message::DeserializePublic(serialized);
								if (msg) {
									auto * sodm = dynamic_cast<SendOfflineDataMessage *>(msg);
									if (sodm) {
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes WHERE Username = '" + Config::Username.toStdString() + "';", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementList;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementProgress;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementProgressMax;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievements;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modScores;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM overallSaveData;", err);

										Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA synchronous = OFF;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA journal_mode = MEMORY;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA cache_size=10000;", err);
										Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
										for (const auto & ad : sodm->achievements) {
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementList (ModID, Identifier) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ");", err);

											if (ad.current > 0) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ", '" + ad.username + "', " + std::to_string(ad.current) + ");", err);
											}
											if (ad.max > 0) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgressMax (ModID, Identifier, Max) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ", " + std::to_string(ad.max) + ");", err);
											}
											if (ad.unlocked) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ", '" + ad.username + "');", err);
											}
										}
										for (const auto & sd : sodm->scores) {
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(sd.modID) + ", " + std::to_string(sd.identifier) + ", '" + sd.username + "', " + std::to_string(sd.score) + ");", err);
										}
										for (const auto & od : sodm->overallSaves) {
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(od.modID) + ", '" + od.username + "', '" + od.entry + "', '" + od.value + "');", err);
										}
										Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
										Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
									}
								}
								delete msg;
							}
						}
					}
				}
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
			stats.type = static_cast<ModType>(jsonResponse["ProjectID"].toString().toInt());
			stats.lastTimePlayed = jsonResponse["ProjectID"].toString().toInt();
			stats.duration = jsonResponse["ProjectID"].toString().toInt();

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
