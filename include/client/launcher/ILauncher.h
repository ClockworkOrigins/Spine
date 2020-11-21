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

#pragma once

#include "client/widgets/SurveyDialog.h"

#include "common/GameType.h"
#include "common/ModStats.h"

#include <QtPlugin>

class QElapsedTimer;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QStandardItemModel;
class QVBoxLayout;

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
}
}

namespace spine {
	class ScreenshotManager;
namespace common {
	struct UpdateAchievementProgressMessage;
	struct UpdateScoreMessage;
	struct UnlockAchievementMessage;
	struct UpdateOverallSaveDataMessage;
} /* namespace common */
namespace widgets {
	class RatingWidget;
}
namespace launcher {

	class ILauncher : public QObject {
		Q_OBJECT
		
	public:
		virtual ~ILauncher() = default;

		virtual void init();

		virtual bool supportsGame(common::GameType gothic) const = 0;
		virtual bool supportsModAndIni(int32_t modID, const QString & iniFile) const = 0;

		QWidget * getLibraryWidget() const;

		virtual void loginChanged();
		virtual void setDeveloperMode(bool enabled);
		void setShowAchievements(bool enabled);
		virtual void setZSpyActivated(bool) {}

		virtual void restoreSettings() {}
		virtual void saveSettings() {}

		virtual void updateView(int modID, const QString & iniFile) = 0;

		virtual void start() = 0;

		void refresh(int modID);

		virtual void updateModel(QStandardItemModel * model) = 0;

		virtual void updatedProject(int projectID) = 0;

		bool isRunning() const;

	signals:
		void restartAsAdmin();
		void receivedModStats(common::ModStats);
		void errorMessage(QString);
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);
		void loadedSurvey(widgets::Survey survey, int versionMajor, int versionMinor, int versionPatch);
		void syncedNewStats();

	public slots:
		virtual void finishedInstallation(int modID, int packageID, bool success) = 0;
		void updateStarted(int modID);
		void updateFinished(int modID);

	private slots:
		void feedbackClicked();
		void openDiscussionsUrl();

	protected:
		QWidget * _widget = nullptr;
		QVBoxLayout * _layout = nullptr;
		QHBoxLayout * _upperLayout = nullptr;
		QPushButton * _startButton = nullptr;

		QString _name;
		int32_t _modID = -1;

		QString _iniFile;

		QElapsedTimer * _timer = nullptr;

		QStandardItemModel * _model = nullptr;

		QList<int> _runningUpdates;

		bool _developerMode = false;

		virtual void createWidget();

		void updateCommonView(int modID, const QString & name);

		void updateModInfoView(common::ModStats ms);

		void startScreenshotManager(int modID);
		void stopScreenshotManager();

		void startCommon();
		void stopCommon();

		virtual QString getOverallSavePath() const = 0;

		virtual void syncAdditionalTimes(int) {}

		virtual void updateModStats() {}

		bool isAllowedSymlinkSuffix(QString suffix) const;
		bool linkOrCopyFile(QString sourcePath, QString destinationPath);

	private:
		QLabel * _playTimeLabel = nullptr;
		widgets::RatingWidget * _ratingWidget = nullptr;
		QLabel * _installDate = nullptr;
		QLabel * _lastPlayedDate = nullptr;
		QLabel * _achievementLabel = nullptr;
		QLabel * _scoresLabel = nullptr;
		QPushButton * _feedbackButton = nullptr;
		QLabel * _discussionUrlTitle = nullptr;
		QLabel * _discussionUrlText = nullptr;

		ScreenshotManager * _screenshotManager = nullptr;

		clockUtils::sockets::TcpSocket * _listenSocket = nullptr;
		clockUtils::sockets::TcpSocket * _socket = nullptr;

		QString _discussionUrl;

		bool _showAchievements = true;

		bool _running = false;

		void prepareAchievementView();
		void prepareScoreView();

		void acceptedConnection(clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError err);
		void receivedMessage(std::vector<uint8_t> packet, clockUtils::sockets::TcpSocket * socket, clockUtils::ClockError err);

		void tryCleanCaches();

		void cacheScore(common::UpdateScoreMessage * usm);
		void removeScore(common::UpdateScoreMessage * usm);

		void cacheAchievement(common::UnlockAchievementMessage * uam);
		void removeAchievement(common::UnlockAchievementMessage * uam);

		void cacheAchievementProgress(common::UpdateAchievementProgressMessage * uapm);
		void removeAchievementProgress(common::UpdateAchievementProgressMessage * uapm);

		void cacheOverallSaveData(common::UpdateOverallSaveDataMessage * uom);
		void removeOverallSaveData(common::UpdateOverallSaveDataMessage * uom);

		void synchronizeOfflineData();
	};
	typedef QSharedPointer<ILauncher> ILauncherPtr;

} /* namespace launcher */
} /* namespace spine */

Q_DECLARE_INTERFACE(spine::launcher::ILauncher, "spine::launcher::ILauncher")
