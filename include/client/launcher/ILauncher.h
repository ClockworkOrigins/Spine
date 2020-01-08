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

#include "common/GothicVersion.h"
#include "common/ModStats.h"

#include <QSharedPointer>
#include <QtPlugin>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QTimer;
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
		virtual ~ILauncher() {}

		virtual void init();

		virtual bool supportsGame(common::GothicVersion gothic) const = 0;
		virtual bool supportsModAndIni(int32_t modID, const QString & iniFile) const = 0;

		QWidget * getLibraryWidget() const;

		virtual void loginChanged();
		virtual void setDeveloperMode(bool) {}
		void setShowAchievements(bool enabled);
		virtual void setZSpyActivated(bool) {}

		virtual void restoreSettings() {}
		virtual void saveSettings() {}

		virtual void updateView(int modID, const QString & iniFile) = 0;

		virtual void start() = 0;

		void refresh(int modID);

	signals:
		void restartAsAdmin();
		void receivedModStats(common::ModStats);
		void errorMessage(QString);
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);

	protected:
		QWidget * _widget = nullptr;
		QVBoxLayout * _layout = nullptr;
		QHBoxLayout * _upperLayout = nullptr;
		QPushButton * _startButton;

		QString _name;
		int32_t _modID;

		QString _iniFile;

		QTime * _timer;

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

	private:
		QLabel * _playTimeLabel;
		widgets::RatingWidget * _ratingWidget;
		QLabel * _installDate;
		QLabel * _lastPlayedDate;
		QLabel * _achievementLabel;
		QLabel * _scoresLabel;

		ScreenshotManager * _screenshotManager;

		clockUtils::sockets::TcpSocket * _listenSocket;
		clockUtils::sockets::TcpSocket * _socket;

		bool _showAchievements;

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
