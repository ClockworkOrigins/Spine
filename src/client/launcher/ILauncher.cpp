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

#include "client/widgets/UpdateLanguage.h"

#include "common/MessageStructs.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"

#include "widgets/RatingWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDate>
#include <QLabel>
#include <QPushButton>
#include <QtConcurrentRun>
#include <QVBoxLayout>
#include <QWidget>

using namespace spine::common;
using namespace spine::launcher;
using namespace spine::utils;

void ILauncher::init() {
	createWidget();

	_modID = -1;
	_showAchievements = true;

	_timer = new QTime();

	_socket = nullptr;
	_listenSocket = nullptr;
	
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS installDates (ModID INT PRIMARY KEY, InstallDate INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "CREATE TABLE IF NOT EXISTS lastPlayed (ModID INT NOT NULL, Ini TEXT NOT NULL, PRIMARY KEY (ModID, Ini));", err);
	
	qRegisterMetaType<common::ModStats>("common::ModStats");

	_screenshotManager = new ScreenshotManager(this);
}

QWidget * ILauncher::getLibraryWidget() const {
	return _widget;
}

void ILauncher::loginChanged() {
	_ratingWidget->loginChanged();
	
	if (!Config::Username.isEmpty()) {
		tryCleanCaches();
		synchronizeOfflineData();
	}
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
			_ratingWidget = new widgets::RatingWidget(widgets::RatingWidget::RatingMode::User, _widget);
			_ratingWidget->setProperty("library", true);
			_upperLayout->addWidget(_ratingWidget, 1, Qt::AlignRight);
			_ratingWidget->setEditable(true);
			_ratingWidget->setVisible(false);
		}

		_layout->addLayout(_upperLayout);
	}

	{
		QHBoxLayout * hbl = new QHBoxLayout();

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
	emit openAchievementView(_modID, _name);
}

void ILauncher::prepareScoreView() {
	emit openScoreView(_modID, _name);
}

void ILauncher::refresh(int modID) {
	if (modID != _modID) return;

	updateView(_modID, _iniFile);
}

void ILauncher::updateStarted(int modID) {
	_runningUpdates.append(modID);

	refresh(modID);
}

void ILauncher::updateFinished(int modID) {
	_runningUpdates.removeAll(modID);

	refresh(modID);
}

void ILauncher::updateCommonView(int modID, const QString & name) {
	_modID = modID;
	
	_installDate->hide();
	_lastPlayedDate->hide();
	_achievementLabel->hide();
	_scoresLabel->hide();
	_playTimeLabel->setText("");
	if (Config::OnlineMode) {
		_ratingWidget->setVisible(modID != -1);
	}
	_installDate->setText("");
	_lastPlayedDate->setText("");

	if (modID != -1) {
		if (Config::OnlineMode) {
			_ratingWidget->setModID(modID);
			_ratingWidget->setModName(name);
		}
		Database::DBError err;
		auto date = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT InstallDate FROM installDates WHERE ModID = " + std::to_string(modID) + " LIMIT 1;", err);
		if (date.empty()) {
			const int currentDate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO installDates (ModID, InstallDate) VALUES (" + std::to_string(modID) + ", " + std::to_string(currentDate) +");", err);
			date.push_back(currentDate);
		}
		_installDate->setText(QApplication::tr("Installed").arg(QDate(1970, 1, 1).addDays(date[0] / 24).toString("dd.MM.yyyy")));
		_installDate->show();
		
		updateModStats();
	}
}

void ILauncher::updateModInfoView(ModStats ms) {
	const QString timeString = utils::timeToString(ms.duration);
	_playTimeLabel->setText(timeString);

	_achievementLabel->setText(R"(<a href="Foobar" style="color: #181C22">)" + QApplication::tr("AchievementText").arg(ms.achievedAchievements).arg(ms.allAchievements).arg(ms.achievedAchievements * 100 / ((ms.allAchievements) ? ms.allAchievements : 1)) + "</a>");
	_achievementLabel->setVisible(ms.allAchievements > 0);

	_scoresLabel->setText(R"(<a href="Blafoo" style="color: #181C22">)" + ((ms.bestScoreRank > 0) ? QApplication::tr("BestScoreText").arg(s2q(ms.bestScoreName)).arg(ms.bestScoreRank).arg(ms.bestScore) : QApplication::tr("NoBestScore")) + "</a>");
	_scoresLabel->setVisible(ms.bestScoreRank != -1);

	if (ms.lastTimePlayed == -1) {
		_lastPlayedDate->hide();
	} else {
		_lastPlayedDate->setText(QApplication::tr("LastPlayed").arg(QDate(1970, 1, 1).addDays(ms.lastTimePlayed / 24).toString("dd.MM.yyyy")));
		_lastPlayedDate->show();
	}
}

void ILauncher::startScreenshotManager(int modID) {
	_screenshotManager->start(modID);
}

void ILauncher::stopScreenshotManager() {
	_screenshotManager->stop();
}

void ILauncher::startCommon() {
	_listenSocket = new clockUtils::sockets::TcpSocket();
	_listenSocket->listen(LOCAL_PORT, 1, true, std::bind(&ILauncher::acceptedConnection, this, std::placeholders::_1, std::placeholders::_2));
	
	_timer->start();
	
	if (Config::OnlineMode) {
		QtConcurrent::run([]() {
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
				UpdatePlayingTimeMessage uptm;
				uptm.dayOfTheWeek = QDate::currentDate().dayOfWeek();
				uptm.hour = QTime::currentTime().hour();
				const std::string serialized = uptm.SerializePublic();
				sock.writePacket(serialized);
			}
		});
	}

	startScreenshotManager(_modID);
}

void ILauncher::stopCommon() {
	int duration = _timer->elapsed();
	duration = duration / 1000; // to seconds
	duration = duration / 60; // to minutes
	delete _listenSocket;
	_listenSocket = nullptr;
	delete _socket;
	_socket = nullptr;

	QtConcurrent::run([this, duration]() {
		UpdatePlayTimeMessage uptm;
		uptm.username = Config::Username.toStdString();
		uptm.password = Config::Password.toStdString();
		uptm.modID = _modID;
		uptm.duration = duration;
		const std::string serialized = uptm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
			sock.writePacket(serialized);

			syncAdditionalTimes(duration);
			
			updateModStats();
		}
	});

	stopScreenshotManager();
}

void ILauncher::acceptedConnection(clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError err) {
	if (err == clockUtils::ClockError::SUCCESS) {
		delete _socket;
		_socket = sock;
		sock->receiveCallback(std::bind(&ILauncher::receivedMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
}

void ILauncher::receivedMessage(std::vector<uint8_t> message, clockUtils::sockets::TcpSocket * socket, clockUtils::ClockError err) {
	if (err == clockUtils::ClockError::SUCCESS) {
		try {
			std::string serialized(message.begin(), message.end());
			Message * msg = Message::DeserializeBlank(serialized);
			if (msg) {
				if (msg->type == MessageType::REQUESTUSERNAME) {
					SendUsernameMessage sum;
					sum.username = Config::Username.toStdString();
					sum.password = Config::Password.toStdString();
					sum.modID = _modID;
					serialized = sum.SerializeBlank();
					socket->writePacket(serialized);
				} else if (msg->type == MessageType::REQUESTSCORES) {
					RequestScoresMessage * rsm = dynamic_cast<RequestScoresMessage *>(msg);
					if (_modID != -1) {
						if (rsm) {
							rsm->modID = _modID;
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
								std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Username, Score FROM modScores WHERE ModID = " + std::to_string(_modID) + " ORDER BY Score DESC", dbErr);
								SendScoresMessage ssm;
								std::map<int, std::vector<std::pair<std::string, int32_t>>> scores;
								for (auto vec : lastResults) {
									int32_t identifier = int32_t(std::stoi(vec[0]));
									std::string username = vec[1];
									int32_t score = int32_t(std::stoi(vec[2]));
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
					UpdateScoreMessage * usm = dynamic_cast<UpdateScoreMessage *>(msg);
					if (_modID != -1) {
						if (usm) {
							if (Config::OnlineMode) {
								usm->modID = _modID;
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
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(_modID) + ", " + std::to_string(usm->identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(usm->score) + ");", dbErr);
								if (dbErr.error) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modScores SET Score = " + std::to_string(usm->score) + " WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + std::to_string(usm->identifier) + " AND Username = '" + Config::Username.toStdString() + "';", dbErr);
								}
							}
						}
					}
				} else if (msg->type == MessageType::REQUESTACHIEVEMENTS) {
					RequestAchievementsMessage * ram = dynamic_cast<RequestAchievementsMessage *>(msg);
					if (_modID != -1) {
						if (ram && Config::OnlineMode && !Config::Username.isEmpty()) {
							ram->modID = _modID;
							ram->username = Config::Username.toStdString();
							ram->password = Config::Password.toStdString();
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								serialized = ram->SerializePublic();
								if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
									if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
										SendAchievementsMessage * sam = dynamic_cast<SendAchievementsMessage *>(Message::DeserializePublic(serialized));
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
							std::vector<std::string> lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(_modID) + ";", dbErr);

							SendAchievementsMessage sam;
							for (const std::string & s : lastResults) {
								int32_t identifier = int32_t(std::stoi(s));
								sam.achievements.push_back(identifier);
							}
							auto lastResultsVec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Max FROM modAchievementProgressMax WHERE ModID = " + std::to_string(_modID) + ";", dbErr);
							for (auto vec : lastResultsVec) {
								lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + vec[0] + " LIMIT 1;", dbErr);
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
					UnlockAchievementMessage * uam = dynamic_cast<UnlockAchievementMessage *>(msg);
					if (_modID != -1) {
						if (uam) {
							if (Config::OnlineMode) {
								int duration = _timer->elapsed();
								duration = duration / 1000; // to seconds
								duration = duration / 60;

								uam->modID = _modID;
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
								uam->modID = _modID;
								
								cacheAchievement(uam);
								
								Database::DBError dbErr;
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(_modID) + ", " + std::to_string(uam->identifier) + ", '" + Config::Username.toStdString() + ");", dbErr);
							}
						}
					}
				} else if (msg->type == MessageType::UPDATEACHIEVEMENTPROGRESS) {
					UpdateAchievementProgressMessage * uapm = dynamic_cast<UpdateAchievementProgressMessage *>(msg);
					if (_modID != -1) {
						if (uapm) {
							if (Config::OnlineMode) {
								uapm->modID = _modID;
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
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(_modID) + ", " + std::to_string(uapm->identifier) + ", '" + Config::Username.toStdString() + "', " + std::to_string(uapm->progress) + ");", dbErr);
								if (dbErr.error) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modAchievementProgress SET Current = " + std::to_string(uapm->progress) + " WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + std::to_string(uapm->identifier) + " AND Username = '" + Config::Username.toStdString() + "';", dbErr);
								}
							}
						}
					}
				} else if (msg->type == MessageType::SEARCHMATCH) {
					if (_modID != -1 && Config::OnlineMode) {
						SearchMatchMessage * smm = dynamic_cast<SearchMatchMessage *>(msg);
						if (smm) {
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								smm->modID = _modID;
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
					RequestOverallSavePathMessage * rospm = dynamic_cast<RequestOverallSavePathMessage *>(msg);
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
					RequestOverallSaveDataMessage * rom = dynamic_cast<RequestOverallSaveDataMessage *>(msg);
					if (_modID != -1) {
						if (rom && !Config::Username.isEmpty()) {
							if (Config::OnlineMode) {
								rom->modID = _modID;
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
								std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Entry, Value FROM overallSaveData WHERE ModID = " + std::to_string(_modID) + " AND Username = '" = Config::Username.toStdString() + "';", dbErr);
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
					UpdateOverallSaveDataMessage * uom = dynamic_cast<UpdateOverallSaveDataMessage *>(msg);
					if (_modID != -1) {
						if (uom) {
							if (Config::OnlineMode) {
								uom->modID = _modID;
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
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(_modID) + ", '" + Config::Username.toStdString() + "', '" + uom->entry + "', '" + uom->value + "');", dbErr);
								if (dbErr.error) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE overallSaveData SET Value = '" + uom->value + "' WHERE ModID = " + std::to_string(_modID) + " AND Entry = '" + uom->entry + "' AND Username = '" + Config::Username.toStdString() + "';", dbErr);
								}
							}
						}
					}
				} else if (msg->type == MessageType::REQUESTALLFRIENDS) {
					RequestAllFriendsMessage * rafm = dynamic_cast<RequestAllFriendsMessage *>(msg);
					if (Config::OnlineMode) {
						if (rafm && !Config::Username.isEmpty()) {
							rafm->username = Config::Username.toStdString();
							rafm->password = Config::Password.toStdString();
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								serialized = rafm->SerializePublic();
								if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
									if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
										SendAllFriendsMessage * safm = dynamic_cast<SendAllFriendsMessage *>(Message::DeserializePublic(serialized));
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
					UpdateChapterStatsMessage * ucsm = dynamic_cast<UpdateChapterStatsMessage *>(msg);
					if (_modID != -1) {
						if (ucsm) {
							if (Config::OnlineMode) {
								ucsm->modID = _modID;
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = ucsm->SerializePublic();
									sock.writePacket(serialized);
								}
							}
						}
					}
				} else if (msg->type == MessageType::ISACHIEVEMENTUNLOCKED) {
					IsAchievementUnlockedMessage * iaum = dynamic_cast<IsAchievementUnlockedMessage *>(msg);
					if (_modID != -1) {
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
							for (auto vec : achievements) {
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
							for (auto vec : scores) {
								UpdateOfflineDataMessage::ScoreData sd {};
								sd.modID = std::stoi(vec[0]);
								sd.identifier = std::stoi(vec[1]);
								sd.score = std::stoi(vec[2]);
								uodm.scores.push_back(sd);
							}
							const auto overallSaveData = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Entry, Value FROM overallSaveData;", err);
							for (auto vec : overallSaveData) {
								UpdateOfflineDataMessage::OverallSaveData od;
								od.modID = std::stoi(vec[0]);
								od.entry = vec[1];
								od.value = vec[2];
								uodm.overallSaves.push_back(od);
							}
							const auto playTimes = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Duration FROM playTimes;", err);
							for (auto vec : playTimes) {
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
									SendOfflineDataMessage * sodm = dynamic_cast<SendOfflineDataMessage *>(msg);
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
