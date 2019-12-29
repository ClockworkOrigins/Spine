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

#pragma once

#include "common/MessageStructs.h"

#include <QDialog>

class QMainWindow;
class QTabWidget;

namespace spine {
namespace models {
	class SpineEditorModel;
} /* namespace models */
namespace widgets {

	class AchievementSpineSettingsWidget;
	class GamepadSpineSettingsWidget;
	class GeneralSpineSettingsWidget;
	class LeGoSpineSettingsWidget;
	class ScoreSpineSettingsWidget;

	class SpineEditor : public QDialog {
		Q_OBJECT

	public:
		SpineEditor(QMainWindow * mainWindow);
		~SpineEditor();

		models::SpineEditorModel * getModel() const;

	public slots:
		int exec() override;

	private slots:
		void achievementStateChanged(int checkState);
		void scoreStateChanged(int checkState);
		void gamepadStateChanged(int checkState);
		void updateFromModel();
		void accept() override;
		void reject() override;
		void submit();
		void installSpineScripts();
		void updateSpineScripts();
		void installIkarusScripts();
		void updateIkarusScripts();
		void installLeGoScripts();
		void updateLeGoScripts();

	private:
		models::SpineEditorModel * _model;
		QTabWidget * _tabWidget;
		GeneralSpineSettingsWidget * _generalSpineSettingsWidget;
		AchievementSpineSettingsWidget * _achievementSpineSettingsWidget;
		ScoreSpineSettingsWidget * _scoreSpineSettingsWidget;
		GamepadSpineSettingsWidget * _gamepadSpineSettingsWidget;
		LeGoSpineSettingsWidget * _legoSpineSettingsWidget;
		QPushButton * _installSpineButton;
		QPushButton * _updateSpineButton;
		QPushButton * _installIkarusButton;
		QPushButton * _updateIkarusButton;
		QPushButton * _installLeGoButton;
		QPushButton * _updateLeGoButton;
		QMainWindow * _mainWindow;
		std::vector<common::SendModsForEditorMessage::ModForEditor> _modList;

		void closeEvent(QCloseEvent * evt) override;
		void checkSpineVersion();
		void checkIkarusVersion();
		void checkIkarusInitialized();
		void checkLeGoVersion();
		void checkLeGoInitialized();
		void restoreSettings();
		void saveSettings();
		void loadMods();
	};

} /* namespace widgets */
} /* namespace spine */
