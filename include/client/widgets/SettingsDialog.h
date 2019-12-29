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

#include <QDialog>

class QLabel;

namespace spine {
namespace gamepad {
	class GamepadSettingsWidget;
}
namespace widgets {

	class DeveloperSettingsWidget;
	class GameSettingsWidget;
	class GeneralSettingsWidget;
	class LocationSettingsWidget;

	class SettingsDialog : public QDialog {
		Q_OBJECT

	public:
		SettingsDialog(QWidget * par);
		~SettingsDialog();

		DeveloperSettingsWidget * getDeveloperSettingsWidget() const {
			return _developerSettingsWidget;
		}

		GameSettingsWidget * getGameSettingsWidget() const {
			return _gameSettingsWidget;
		}

		gamepad::GamepadSettingsWidget * getGamepadSettingsWidget() const {
			return _gamepadSettingsWidget;
		}

		GeneralSettingsWidget * getGeneralSettingsWidget() const {
			return _generalSettingsWidget;
		}

		LocationSettingsWidget * getLocationSettingsWidget() const {
			return _locationSettingsWidget;
		}

	private slots:
		void accept() override;
		void reject() override;

	private:
		DeveloperSettingsWidget * _developerSettingsWidget;
		GameSettingsWidget * _gameSettingsWidget;
		gamepad::GamepadSettingsWidget * _gamepadSettingsWidget;
		GeneralSettingsWidget * _generalSettingsWidget;
		LocationSettingsWidget * _locationSettingsWidget;

		void closeEvent(QCloseEvent * evt) override;
		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */
