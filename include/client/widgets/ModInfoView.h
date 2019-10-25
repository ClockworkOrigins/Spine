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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#ifndef __SPINE_WIDGETS_MODINFOVIEW_H__
#define __SPINE_WIDGETS_MODINFOVIEW_H__

#include <cstdint>
#include <set>

#include "common/ModStats.h"

#include <QProcess>
#include <QWidget>

class QCheckBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QMainWindow;
class QNetworkAccessManager;
class QPushButton;
class QSettings;
class QSlider;
class QVBoxLayout;

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {
	class ScreenshotManager;
namespace common {
	enum class GothicVersion;
	struct UpdateAchievementProgressMessage;
	struct UpdateScoreMessage;
	struct UnlockAchievementMessage;
	struct UpdateOverallSaveDataMessage;
} /* namespace common */
namespace gamepad {
	class GamePadXbox;
} /* namespace gamepad */
namespace widgets {

	class GamepadSettingsWidget;
	class GeneralSettingsWidget;
	class LocationSettingsWidget;
	class RatingWidget;

	class ModInfoView : public QWidget {
		Q_OBJECT

	public:
		ModInfoView(bool onlineMode, QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, LocationSettingsWidget * locationSettingsWidget, GamepadSettingsWidget * gamepadSettingsWidget, QSettings * iniParser, QWidget * par);
		~ModInfoView();

		void setIniFile(QString file);
		void setIsInstalled(bool isInstalled);
		void setModID(QString modID);
		void setGothicDirectory(QString directory);
		void setGothic2Directory(QString directory);

	signals:
		void descriptionChanged(QString);
		void receivedModStats(common::ModStats);
		void restartAsAdmin();
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);
		void updatedG1Path();
		void updatedG2Path();
		void receivedCompatibilityList(int, std::vector<int32_t>, std::vector<int32_t>);
		void changeSplashMessage(QString, int, QColor);
		void errorMessage(QString);
		void installMod(int);

	public slots:
		void loggedIn(QString name, QString password);
		void setDeveloperMode(bool active);
		void setZSpyActivated(bool active);
		void setLanguage(QString language);
		void setShowAchievements(bool showAchievements);
		void setHideIncompatible(bool enabled);
		void startMod();
		void updatedMod(int modID);

	private slots:
		void errorOccurred(QProcess::ProcessError error);
		void finishedMod(int exitCode, QProcess::ExitStatus exitStatus);
		void changedPatchState();
		void updateModInfoView(common::ModStats ms);
		void restartSpineAsAdmin();
		void prepareAchievementView();
		void prepareScoreView();
		void patchCheckG1();
		void patchCheckG2();
		void updateCompatibilityList(int modID, std::vector<int32_t> incompatiblePatches, std::vector<int32_t> forbiddenPatches);
		void startSpacer();
		void finishedSpacer();
		void showErrorMessage(QString msg);

	private:
		QMainWindow * _mainWindow;
		QPushButton * _startButton;
		QLabel * _adminInfoLabel;
		QLabel * _nameLabel;
		QLabel * _versionLabel;
		QLabel * _teamLabel;
		QLabel * _contactLabel;
		QLabel * _homepageLabel;
		QGroupBox * _patchGroup;
		QGridLayout * _patchLayout;
		QGroupBox * _pdfGroup;
		QVBoxLayout * _pdfLayout;
		QLabel * _playTimeLabel;
		QLabel * _achievementLabel;
		QLabel * _scoresLabel;
		QLabel * _installDate;
		QLabel * _lastPlayedDate;

		QString _iniFile;
		bool _isInstalled;
		int _modID;
		QString _gothicDirectory;
		QString _gothic2Directory;
		Qt::WindowStates _oldWindowState;

		QString _username;
		QString _password;
		QTime * _timer;

		QList<QCheckBox *> _patchList;
		QList<QLabel *> _pdfList;

		std::map<QCheckBox *, int32_t> _checkboxPatchIDMapping;

		bool _developerModeActive;

		clockUtils::sockets::TcpSocket * _listenSocket;
		clockUtils::sockets::TcpSocket * _socket;

		std::vector<std::tuple<QString, QString, QString>> _gothicIniBackup;
		std::vector<std::tuple<QString, QString, QString>> _systempackIniBackup;

		bool _zSpyActivated;

		QString _language;

		std::set<QString> _copiedFiles;
		QString _lastBaseDir;

		bool _showAchievements;

		bool _hideIncompatible;

		RatingWidget * _ratingWidget;

		ScreenshotManager * _screenshotManager;

		QColor _splashTextColor;

		gamepad::GamePadXbox * _gamepad;
		GamepadSettingsWidget * _gamepadSettingsWigdet;

		QCheckBox * _compileScripts;
		QCheckBox * _startupWindowed;
		QCheckBox * _convertTextures;
		QCheckBox *_noSound;
		QCheckBox * _noMusic;
		QLabel * _zSpyLabel;
		QSlider * _zSpyLevel;

		QSettings * _iniParser;

		QPushButton * _startSpacerButton;

		bool _onlineMode;

		QNetworkAccessManager * _networkAccessManager;

		int _patchCounter;

		int _gmpCounterBackup;

		QStringList getGothicFiles() const;
		void removeGothicFiles();
		void removeModFiles();
		QString getUsedBaseDir() const;
		common::GothicVersion getGothicVersion() const;
		common::GothicVersion getGothicVersion(QString path) const;

		void acceptedConnection(clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError err);
		void receivedMessage(std::vector<uint8_t> packet, clockUtils::sockets::TcpSocket * socket, clockUtils::ClockError err);
		void updateModStats();
		bool isAllowedSymlinkSuffix(QString suffix) const;

		bool prepareModStart(QString * usedBaseDir, QString * usedExecutable, QStringList * backgroundExecutables, bool * newGMP, QSet<QString> * dependencies);
		void checkToolCfg(QString path, QString * usedBaseDir, QStringList * backgroundExecutables, bool * newGMP);

		void tryCleanCaches();

		void cacheScore(common::UpdateScoreMessage * usm);
		void removeScore(common::UpdateScoreMessage * usm);

		void cacheAchievement(common::UnlockAchievementMessage * uam);
		void removeAchievement(common::UnlockAchievementMessage * uam);

		void cacheAchievementProgress(common::UpdateAchievementProgressMessage * uapm);
		void removeAchievementProgress(common::UpdateAchievementProgressMessage * uapm);

		void cacheOverallSaveData(common::UpdateOverallSaveDataMessage * uom);
		void removeOverallSaveData(common::UpdateOverallSaveDataMessage * uom);

		void emitSplashMessage(QString message);

		void restoreSettings();
		void saveSettings();

		void removeEmptyDirs(QString baseDir);

		void synchronizeOfflineData();

		void collectDependencies(int modID, QSet<QString> * dependencies, QSet<QString> * forbidden);

		void prepareForNinja(QString * usedBaseDir);
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MODINFOVIEW_H__ */
