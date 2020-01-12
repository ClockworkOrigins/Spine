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
		
		ILauncherPtr getLauncher(common::GameType gothic) const;
		ILauncherPtr getLauncher(int32_t modID, const QString & iniFile) const;

		void loginChanged();
		void setDeveloperMode(bool enabled);
		void setShowAchievements(bool enabled);
		void setZSpyActivated(bool enabled);

		void restoreSettings();
		void saveSettings();

	signals:
		void restartAsAdmin();
		void errorMessage(QString);
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);

	private:
		QList<ILauncherPtr> _launchers;
		
		LauncherFactory();
		
		ILauncherPtr createLauncher(common::GameType gothic) const;
	};

} /* namespace launcher */
} /* namespace spine */
