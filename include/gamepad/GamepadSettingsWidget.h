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

#include <QMap>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;

namespace spine {
namespace gamepad {
	
	enum GamePadIndex;
	enum GamePadButton;

	class GamepadSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		GamepadSettingsWidget(QWidget * par);
		~GamepadSettingsWidget();

		void saveSettings();
		void rejectSettings();

		bool isEnabled() const;
		GamePadIndex getIndex() const;
		int getKeyDelay() const;

		QMap<QString, GamePadButton> getKeyMapping() const;

	private slots:
		void changedGamepadState(int checkState);
		void newButton();

	private:
		QCheckBox * _gamepadEnabled;
		QComboBox * _controllerList;
		QSpinBox * _keyDelayBox;

		QMap<GamePadButton, QPushButton *> _gamepadButtonToButtonMap;
		QMap<QString, QPushButton *> _actionToButtonMap;
	};

} /* namespace gamepad */
} /* namespace spine */
