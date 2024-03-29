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
// Copyright 2018 Clockwork Origins

#pragma once

#include <cstdint>

#include "launcher/ILauncher.h"

#include <QWidget>

class QVBoxLayout;

namespace spine {
namespace client {
	enum class InstallMode;
}
namespace widgets {

	class GeneralSettingsWidget;

	class ModInfoView : public QWidget {
		Q_OBJECT

	public:
		ModInfoView(GeneralSettingsWidget * generalSettingsWidget, QWidget * par);
		~ModInfoView();

		void setGothicDirectory(QString directory);
		void setGothic2Directory(QString directory);
		void setGothic3Directory(QString directory);
		void selectMod(const QString & modID, const QString & iniFile);
		void start();

	signals:
		void restartAsAdmin();
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);
		void updatedG2Path();
		void errorMessage(QString, bool canFix, const std::function<void()> & fixCallback);
		void installMod(int, int, client::InstallMode);

	public slots:
		void loginChanged();
		void setDeveloperMode(bool active);
		void setZSpyActivated(bool active);
		void setShowAchievements(bool showAchievements);
		void setHideIncompatible(bool enabled);
		void updatedMod(int modID);

	private slots:
		void restartSpineAsAdmin();
		void showErrorMessage(QString msg, bool canFix, const std::function<void()> & fixCallback);
		void showSurvey(Survey survey, int versionMajor, int versionMinor, int versionPatch);

	private:
		QVBoxLayout * _layout;
		QWidget * _lastWidget;

		launcher::ILauncherPtr _currentLauncher;

		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */
