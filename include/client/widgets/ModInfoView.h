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

#include "launcher/ILauncher.h"

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

namespace spine {
	class ScreenshotManager;
namespace common {
	enum class GothicVersion;
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
		ModInfoView(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QSettings * iniParser, QWidget * par);
		~ModInfoView();

		void setGothicDirectory(QString directory);
		void setGothic2Directory(QString directory);
		void selectMod(const QString & modID, const QString & iniFile);
		void start();

	signals:
		void restartAsAdmin();
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);
		void updatedG2Path();
		void errorMessage(QString);
		void installMod(int);

	public slots:
		void loginChanged();
		void setDeveloperMode(bool active);
		void setZSpyActivated(bool active);
		void setShowAchievements(bool showAchievements);
		void setHideIncompatible(bool enabled);
		void updatedMod(int modID);

	private slots:
		void restartSpineAsAdmin();
		void showErrorMessage(QString msg);

	private:
		QMainWindow * _mainWindow;

		QVBoxLayout * _layout;
		QWidget * _lastWidget;

		launcher::ILauncherPtr _currentLauncher;

		QSettings * _iniParser;

		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MODINFOVIEW_H__ */
