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
#include "common/ProjectStats.h"

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
	struct RequestAllFriendsMessage;
	struct RequestOverallSaveDataMessage;
	struct RequestScoresMessage;
	struct RequestUsernameMessage;
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

		virtual void updateView(int projectID, const QString & iniFile) = 0;

		virtual void start() = 0;

		void refresh(int projectID);

		virtual void updateModel(QStandardItemModel * model) = 0;

		virtual void updatedProject(int projectID) = 0;

		bool isRunning() const;

	signals:
		void restartAsAdmin();
		void receivedModStats(common::ProjectStats);
		void errorMessage(QString);
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);
		void loadedSurvey(widgets::Survey survey, int versionMajor, int versionMinor, int versionPatch);
		void syncedNewStats();
		void editReview(int32_t projectID, const QString & review);

	public slots:
		virtual void finishedInstallation(int projectID, int packageID, bool success) = 0;
		void updateStarted(int projectID);
		void updateFinished(int projectID);

	private slots:
		void feedbackClicked();
		void openDiscussionsUrl();

	protected:
		QWidget * _widget = nullptr;
		QVBoxLayout * _layout = nullptr;
		QHBoxLayout * _upperLayout = nullptr;
		QPushButton * _startButton = nullptr;

		QString _name;
		int32_t _projectID = -1;

		QString _iniFile;

		QElapsedTimer * _timer = nullptr;

		QStandardItemModel * _model = nullptr;

		QList<int> _runningUpdates;

		bool _developerMode = false;

		virtual void createWidget();

		void updateCommonView(int projectID, const QString & name);

		void updateModInfoView(common::ProjectStats ms);

		void startScreenshotManager(int projectID);
		void stopScreenshotManager();

		void startCommon();
		void stopCommon();

		virtual QString getOverallSavePath() const = 0;

		virtual void updateModStats() {}

		bool isAllowedSymlinkSuffix(QString suffix) const;
		bool linkOrCopyFile(QString sourcePath, QString destinationPath);

		void sendUserInfos(QJsonObject json) const;

		/**
		 * \brief requests stats for this project to display in the library (e.g. play time)
		 * resultCallback is called when response is received, true for success, false for error
		 */
		void requestSingleProjectStats(const std::function<void(bool)> & resultCallback);

		/**
		 * \brief returns a list of all projects active for the current selection
		 * default implementation will just return _projectID as in most cases that's true
		 */
		virtual QList<int32_t> getActiveProjects() const;

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

		void cacheScore(int32_t projectID, int32_t identifier, int32_t score) const;
		void removeScore(int32_t projectID, int32_t identifier) const;

		void cacheAchievement(int32_t projectID, int32_t identifier) const;
		void removeAchievement(int32_t projectID, int32_t identifier) const;

		void cacheAchievementProgress(int32_t projectID, int32_t identifier, int32_t progress) const;
		void removeAchievementProgress(int32_t projectID, int32_t identifier) const;

		void cacheOverallSaveData(int32_t projectID, const std::string & key, const std::string & value) const;
		void removeOverallSaveData(int32_t projectID, const std::string & key) const;

		void synchronizeOfflineData();

		void handleRequestUsername(clockUtils::sockets::TcpSocket * socket) const;
		void handleRequestScores(clockUtils::sockets::TcpSocket * socket, common::RequestScoresMessage * msg) const;
		void handleUpdateScore(common::UpdateScoreMessage * msg) const;
		void handleRequestAchievements(clockUtils::sockets::TcpSocket * socket) const;
		void handleUnlockAchievement(common::UnlockAchievementMessage * msg) const;
		void handleUpdateAchievementProgress(common::UpdateAchievementProgressMessage * msg) const;
		void handleRequestOverallSaveDataPath(clockUtils::sockets::TcpSocket * socket) const;
		void handleRequestOverallSaveData(clockUtils::sockets::TcpSocket * socket, common::RequestOverallSaveDataMessage * msg) const;
		void handleUpdateOverallSaveData(common::UpdateOverallSaveDataMessage * msg) const;
		void handleRequestAllFriends(clockUtils::sockets::TcpSocket * socket, common::RequestAllFriendsMessage * msg) const;
	};
	typedef QSharedPointer<ILauncher> ILauncherPtr;

} /* namespace launcher */
} /* namespace spine */

Q_DECLARE_INTERFACE(spine::launcher::ILauncher, "spine::launcher::ILauncher")
