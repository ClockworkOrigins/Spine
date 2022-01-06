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

#include "launcher/Gothic3Launcher.h"

#include "InstallMode.h"
#include "LibraryFilterModel.h"
#include "SpineConfig.h"
#include "SteamProcess.h"

#include "client/IconCache.h"

#include "client/widgets/LocationSettingsWidget.h"
#include "client/widgets/UpdateLanguage.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"

#include "widgets/MainWindow.h"

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
using namespace spine::utils;
using namespace spine::widgets;

Gothic3Launcher::Gothic3Launcher() {
	connect(this, &Gothic3Launcher::receivedModStats, this, &Gothic3Launcher::updateModInfoView);
}

void Gothic3Launcher::setDirectory(const QString & directory) {
	_directory = directory;

	if (!QFileInfo::exists(directory + "/Gothic3.exe")) return;

	Database::DBError err;
	const auto ids = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 400;", err);
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "400"; }) == ids.end()) {
		emit installMod(400, -1, InstallMode::Silent);
	}
}

void Gothic3Launcher::finishedGame(int, QProcess::ExitStatus) {
	stopCommon();

	MainWindow::getInstance()->setEnabled(true);
	MainWindow::getInstance()->setWindowState(_oldWindowState);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "DELETE FROM lastPlayed;", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "INSERT INTO lastPlayed (ModID, Ini) VALUES (" + std::to_string(_projectID) + ", '" + _iniFile.toStdString() + "');", err);
}

bool Gothic3Launcher::supportsGame(GameType gameType) const {
	return gameType == GameType::Gothic3;
}

bool Gothic3Launcher::supportsModAndIni(int32_t gameID, const QString &) const {
	Database::DBError err;
	const int gvInt = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(gameID) + " LIMIT 1;", err, 0);
	const auto gv = static_cast<GameType>(gvInt);

	return supportsGame(gv);
}

void Gothic3Launcher::start() {
	if (_iniFile.isEmpty()) return;

	const auto executablePath = QString("%1/Gothic3.exe").arg(_directory);

	if (!QFileInfo::exists(executablePath)) return;

	MainWindow::getInstance()->setDisabled(true);
	_oldWindowState = MainWindow::getInstance()->windowState();
	MainWindow::getInstance()->setWindowState(Qt::WindowState::WindowMinimized);

	if (!Config::Username.isEmpty() && Config::OnlineMode) {
		sendUserInfos(QJsonObject());
	}

	const QStringList args;
	
	if (canBeStartedWithSteam()) {
		startViaSteam(args);
	} else {
		auto * process = new QProcess(this);
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Gothic3Launcher::finishedGame, Qt::QueuedConnection);
		process->setWorkingDirectory(_directory);

		process->start(QString("\"%1\"").arg(executablePath), args);
	}

	startCommon();
}
		
void Gothic3Launcher::updateModStats() {
	if (!Config::OnlineMode) return;

	requestSingleProjectStats([this](bool) {});
}
		
void Gothic3Launcher::updateView(int gameID, const QString & configFile) {
	_iniFile = configFile;

	updateCommonView(gameID, QApplication::tr("Gothic3"));

	_nameLabel->setText(QApplication::tr("Gothic3"));

	_startButton->setText(QApplication::tr("StartGame"));
	UPDATELANGUAGESETTEXT(_startButton, "StartGame");
	_startButton->setEnabled(!_runningUpdates.contains(_projectID));	
}

QString Gothic3Launcher::getOverallSavePath() const {
	return _directory + "/spine.spsav";
}

void Gothic3Launcher::updateModel(QStandardItemModel * model) {
	_model = model;

	if (_developerMode) return;

	Database::DBError err;
	const auto games = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, GothicVersion FROM mods WHERE GothicVersion = " + std::to_string(static_cast<int>(GameType::Gothic3)) + ";", err);

	for (const auto & game : games) {
		parseGame(game[0], game[1]);
	}
}

void Gothic3Launcher::updatedProject(int projectID) {
	if (_developerMode) return;
	
	Database::DBError err;
	const auto games = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, GothicVersion FROM mods WHERE GothicVersion = " + std::to_string(static_cast<int>(GameType::Gothic3)) + " AND ModID = " + std::to_string(projectID) + ";", err);

	if (games.empty()) return;

	const auto game = games[0];	

	const auto idxList = _model->match(_model->index(0, 0), LibraryFilterModel::ModIDRole, QVariant::fromValue(projectID), 2, Qt::MatchRecursive);

	if (!idxList.empty()) {
		_model->removeRow(idxList[0].row(), idxList[0].parent());
	}

	parseGame(game[0], game[1]);
}

void Gothic3Launcher::finishedInstallation(int gameID, int packageID, bool success) {
	if (!success) return;

	if (packageID != -1) return;

	if (_developerMode) return;

	Database::DBError err;
	const auto gameType = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(gameID) + " LIMIT 1;", err);

	if (static_cast<GameType>(gameType) != GameType::Gothic3) return;
	
	parseGame(gameID, gameType);
	updateModStats();
}

void Gothic3Launcher::createWidget() {
	ILauncher::createWidget();

	_nameLabel = new QLabel(_widget);
	_nameLabel->setProperty("library", true);
	_nameLabel->setAlignment(Qt::AlignCenter);

	_layout->insertWidget(5, _nameLabel);
}

void Gothic3Launcher::parseGame(int gameID, int gameType) {
	const auto cfgPath = QString("%1/mods/%2/spine.cfg").arg(Config::DOWNLOADDIR).arg(gameID);
		
	if (!QFileInfo::exists(cfgPath)) return;
	
	QPixmap pixmap = IconCache::getInstance()->getIcon(gameID);

	pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
	
	auto * item = new QStandardItem(QIcon(pixmap), QApplication::tr("Gothic3"));
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

bool Gothic3Launcher::canBeStartedWithSteam() const {
	return LocationSettingsWidget::getInstance()->startGothic3WithSteam() && !_developerMode; // can't start Gothic with Steam yet
}

void Gothic3Launcher::startViaSteam(QStringList arguments) {
#ifdef Q_OS_WIN
	QFile::remove(_directory + "/System/Gothic3.exe.bak");
	QFile::rename(_directory + "/System/Gothic3.exe", _directory + "/System/Gothic3.exe.bak");

	auto * sp = new SteamProcess(39500, "Gothic3.exe", arguments);
	connect(sp, &SteamProcess::finished, this, &Gothic3Launcher::finishedGame);
	connect(sp, &SteamProcess::finished, sp, &SteamProcess::deleteLater);
	sp->start(5);
#endif
}
