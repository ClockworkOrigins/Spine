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

#include "launcher/GameLauncher.h"

#include "LibraryFilterModel.h"
#include "SpineConfig.h"

#include "client/IconCache.h"

#include "common/MessageStructs.h"

#include "security/Hash.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"

#include "widgets/MainWindow.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QFileInfo>
#include <QLabel>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QTime>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::common;
using namespace spine::launcher;
using namespace spine::security;
using namespace spine::utils;
using namespace spine::widgets;

GameLauncher::GameLauncher() {
	connect(this, &GameLauncher::receivedModStats, this, &GameLauncher::updateModInfoView);
}

void GameLauncher::finishedGame(int, QProcess::ExitStatus) {
	stopCommon();

	widgets::MainWindow::getInstance()->setEnabled(true);
	widgets::MainWindow::getInstance()->setWindowState(_oldWindowState);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "DELETE FROM lastPlayed;", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "INSERT INTO lastPlayed (ModID, Ini) VALUES (" + std::to_string(_modID) + ", '" + _iniFile.toStdString() + "');", err);

	QFile(_directory + "/SpineAPI.dll").remove();
	QFile(_directory + "/SpineAPI64.dll").remove();
}

bool GameLauncher::supportsGame(GameType gameType) const {
	return gameType == GameType::Game;
}

bool GameLauncher::supportsModAndIni(int32_t gameID, const QString &) const {
	Database::DBError err;
	const int gvInt = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(gameID) + " LIMIT 1;", err, 0);
	const auto gv = static_cast<common::GameType>(gvInt);

	return supportsGame(gv);
}

void GameLauncher::start() {
	if (_iniFile.isEmpty()) return;

	QSettings cfgFile(_iniFile, QSettings::IniFormat);
	cfgFile.beginGroup("INFO");
		const QString executable = cfgFile.value("Executable", "").toString();
	cfgFile.endGroup();

	const auto executablePath = QString("%1/%2").arg(_directory).arg(executable);

	if (!QFileInfo::exists(executablePath)) return;

	widgets::MainWindow::getInstance()->setDisabled(true);
	_oldWindowState = widgets::MainWindow::getInstance()->windowState();
	widgets::MainWindow::getInstance()->setWindowState(Qt::WindowState::WindowMinimized);
	
	QProcess * process = new QProcess(this);
	connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &GameLauncher::finishedGame);
	process->setWorkingDirectory(_directory);
	
	QStringList args;
	if (!Config::Username.isEmpty() && Config::OnlineMode) {
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
			common::SendUserInfosMessage suim;
			suim.username = Config::Username.toStdString();
			suim.password = Config::Password.toStdString();
			suim.hash = security::Hash::calculateSystemHash().toStdString();
			suim.mac = security::Hash::getMAC().toStdString();

			const std::string serialized = suim.SerializePublic();
			sock.writePacket(serialized);
		}
	}

	linkOrCopyFile(qApp->applicationDirPath() + "/SpineAPI.dll", _directory + "/SpineAPI.dll");
	linkOrCopyFile(qApp->applicationDirPath() + "/SpineAPI64.dll", _directory + "/SpineAPI64.dll");

	process->start(QString("\"%1\"").arg(executablePath), args);

	startCommon();
}
		
void GameLauncher::updateModStats() {
	
	if (!Config::OnlineMode) return;

	int gameID = _modID;
	QtConcurrent::run([this, gameID]() {
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			{
				common::RequestSingleModStatMessage rsmsm;
				rsmsm.modID = gameID;
				rsmsm.username = Config::Username.toStdString();
				rsmsm.password = Config::Password.toStdString();
				rsmsm.language = Config::Language.toStdString();
				std::string serialized = rsmsm.SerializePublic();
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendSingleModStatMessage * ssmsm = dynamic_cast<common::SendSingleModStatMessage *>(m);
							emit receivedModStats(ssmsm->mod);
						}
						delete m;
					} catch (...) {}
				}
			}
		}
	});
}
		
void GameLauncher::updateView(int gameID, const QString & configFile) {
	_iniFile = configFile;

	_directory = QString("%1/mods/%2").arg(Config::DOWNLOADDIR).arg(gameID);

	QSettings cfgFile(_iniFile, QSettings::IniFormat);
	cfgFile.beginGroup("INFO");
		QString title = cfgFile.value("Title", "").toString();
		title = cfgFile.value("INFO/Title_" + Config::Language, title).toString(); // load language specific title, fall back to generic title if not present
	cfgFile.endGroup();

	updateCommonView(gameID, title);

	if (!title.isEmpty()) {
		_nameLabel->setText(title);
	}

	_startButton->setEnabled(!_runningUpdates.contains(_modID));	
}

QString GameLauncher::getOverallSavePath() const {
	return _directory + "/spine.spsav";
}

void GameLauncher::updateModel(QStandardItemModel * model) {
	_model = model;

	Database::DBError err;
	const auto games = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, GothicVersion FROM mods WHERE GothicVersion = " + std::to_string(static_cast<int>(GameType::Game)) + ";", err);

	for (const auto & game : games) {
		parseGame(game[0], game[1]);
	}
}

void GameLauncher::finishedInstallation(int gameID, int packageID, bool success) {
	if (!success) return;

	if (packageID != -1) return;

	Database::DBError err;
	const auto gameType = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(gameID) + " LIMIT 1;", err);
	
	parseGame(gameID, gameType);
	updateModStats();
}

void GameLauncher::createWidget() {
	ILauncher::createWidget();

	_nameLabel = new QLabel(_widget);
	_nameLabel->setProperty("library", true);
	_nameLabel->setAlignment(Qt::AlignCenter);

	_layout->insertWidget(3, _nameLabel);
}

void GameLauncher::parseGame(int gameID, int gameType) {
	const auto cfgPath = QString("%1/mods/%2/spine.cfg").arg(Config::DOWNLOADDIR).arg(gameID);
		
	if (!QFileInfo::exists(cfgPath)) return;

	const QSettings cfgFile(cfgPath, QSettings::IniFormat);

	auto title = cfgFile.value("INFO/Title").toString();
	title = cfgFile.value("INFO/Title_" + Config::Language, title).toString(); // load language specific title, fall back to generic title if not present
	
	const auto icon = cfgFile.value("INFO/Icon").toString();

	const auto iconPath = QString("%1/mods/%2/%3").arg(Config::DOWNLOADDIR).arg(gameID).arg(icon);

	if (!IconCache::getInstance()->hasIcon(gameID)) {
		IconCache::getInstance()->cacheIcon(gameID, iconPath);
	}
	
	QPixmap pixmap = IconCache::getInstance()->getIcon(gameID);

	pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
	while (title.startsWith(' ') || title.startsWith('\t')) {
		title = title.remove(0, 1);
	}
	QStandardItem * item = new QStandardItem(QIcon(pixmap), title);
	item->setData(cfgPath, LibraryFilterModel::IniFileRole);
	item->setData(true, LibraryFilterModel::InstalledRole);
	item->setData(gameID, LibraryFilterModel::ModIDRole);
	item->setData(gameType, LibraryFilterModel::GameRole);

	Database::DBError err;
	if (!Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM hiddenMods WHERE ModID = " + std::to_string(gameID) + " LIMIT 1;", err).empty()) {
		item->setData(true, LibraryFilterModel::HiddenRole);
	} else {
		item->setData(false, LibraryFilterModel::HiddenRole);
	}
	item->setEditable(false);
	_model->appendRow(item);
}
