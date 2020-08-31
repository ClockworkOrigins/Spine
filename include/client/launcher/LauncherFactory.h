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

#include "launcher/ILauncher.h"

#include <QList>

namespace spine {
namespace launcher {

	class LauncherFactory : public QObject {
		Q_OBJECT
		
	public:
		static LauncherFactory * getInstance();
		
		ILauncherPtr getLauncher(common::GameType gameType) const;
		ILauncherPtr getLauncher(int32_t modID, const QString & iniFile) const;

		void loginChanged();
		void setDeveloperMode(bool enabled);
		void setShowAchievements(bool enabled);
		void setZSpyActivated(bool enabled);

		void restoreSettings();
		void saveSettings();

		void updateModel(QStandardItemModel * model);

		void updatedProject(int projectID);

	signals:
		void restartAsAdmin();
		void errorMessage(QString);
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);
		void finishedInstallation(int, int, bool);
		void updateStarted(int);
		void updateFinished(int);
		void showSurvey(widgets::Survey survey, int versionMajor, int versionMinor, int versionPatch);

	private:
		QList<ILauncherPtr> _launchers;
		
		LauncherFactory();
		
		void initLauncher(ILauncherPtr launcher) const;
	};

} /* namespace launcher */
} /* namespace spine */
