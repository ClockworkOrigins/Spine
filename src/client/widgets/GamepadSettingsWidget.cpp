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

#include "widgets/GamepadSettingsWidget.h"

#include "UpdateLanguage.h"

#include "gamepad/XBoxController.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace {
	QString buttonToString(gamepad::GamePadButton button) {
		switch (button) {
		case gamepad::GamePadButton::GamePad_Button_DPAD_UP: {
			return "DPAD UP";
		}
		case gamepad::GamePadButton::GamePad_Button_DPAD_DOWN: {
			return "DPAD DOWN";
		}
		case gamepad::GamePadButton::GamePad_Button_DPAD_LEFT: {
			return "DPAD LEFT";
		}
		case gamepad::GamePadButton::GamePad_Button_DPAD_RIGHT: {
			return "DPAD RIGHT";
		}
		case gamepad::GamePadButton::GamePad_Button_START: {
			return "START";
		}
		case gamepad::GamePadButton::GamePad_Button_BACK: {
			return "BACK";
		}
		case gamepad::GamePadButton::GamePad_Button_LEFT_THUMB: {
			return "LEFT THUMB";
		}
		case gamepad::GamePadButton::GamePad_Button_RIGHT_THUMB: {
			return "RIGHT THUMB";
		}
		case gamepad::GamePadButton::GamePad_Button_LEFT_SHOULDER: {
			return "LB";
		}
		case gamepad::GamePadButton::GamePad_Button_RIGHT_SHOULDER: {
			return "RB";
		}
		case gamepad::GamePadButton::GamePad_Button_A: {
			return "A";
		}
		case gamepad::GamePadButton::GamePad_Button_B: {
			return "B";
		}
		case gamepad::GamePadButton::GamePad_Button_X: {
			return "X";
		}
		case gamepad::GamePadButton::GamePad_Button_Y: {
			return "Y";
		}
		case gamepad::GamePadButton::GamePad_LTrigger: {
			return "LT";
		}
		case gamepad::GamePadButton::GamePad_RTrigger: {
			return "RT";
		}
		case gamepad::GamePadButton::GamePad_LStick_X_Pos: {
			return "LSTICK RIGHT";
		}
		case gamepad::GamePadButton::GamePad_LStick_X_Neg: {
			return "LSTICK LEFT";
		}
		case gamepad::GamePadButton::GamePad_LStick_Y_Pos: {
			return "LSTICK UP";
		}
		case gamepad::GamePadButton::GamePad_LStick_Y_Neg: {
			return "LSTICK DOWN";
		}
		case gamepad::GamePadButton::GamePad_RStick_X_Pos: {
			return "RSTICK RIGHT";
		}
		case gamepad::GamePadButton::GamePad_RStick_X_Neg: {
			return "RSTICK LEFT";
		}
		case gamepad::GamePadButton::GamePad_RStick_Y_Pos: {
			return "RSTICK UP";
		}
		case gamepad::GamePadButton::GamePad_RStick_Y_Neg: {
			return "RSTICK DOWN";
		}
		default: {
			return QApplication::tr("Unassigned");
		}
		}
	}
}

	GamepadSettingsWidget::GamepadSettingsWidget(QSettings * iniParser, GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _iniParser(iniParser), _gamepadButtonToButtonMap(), _actionToButtonMap() {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		_gamepadEnabled = new QCheckBox(QApplication::tr("GamepadActive"), this);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, _gamepadEnabled, "GamepadActive");
		l->addWidget(_gamepadEnabled);

		{
			QHBoxLayout * hl = new QHBoxLayout();

			QLabel * lbl = new QLabel(QApplication::tr("UsedGamepad"), this);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "UsedGamepad");
			hl->addWidget(lbl);

			_controllerList = new QComboBox(this);
			for (int idx = gamepad::GamePadIndex::GamePadIndex_One; idx < gamepad::GamePadIndex::COUNT; idx++) {
				if (gamepad::GamePadXbox::isConnected(gamepad::GamePadIndex(idx))) {
					_controllerList->addItem(QApplication::tr("Controller").arg(idx + 1));
					_controllerList->setItemData(_controllerList->count() - 1, idx, Qt::UserRole);
				}
			}
			hl->addWidget(_controllerList);

			l->addLayout(hl);
		}
		{
			QHBoxLayout * hl = new QHBoxLayout();

			QLabel * lbl = new QLabel(QApplication::tr("KeyDelay"), this);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "KeyDelay");
			hl->addWidget(lbl);

			_keyDelayBox = new QSpinBox(this);
			_keyDelayBox->setMinimum(0);
			_keyDelayBox->setMaximum(1000);
			hl->addWidget(_keyDelayBox);

			l->addLayout(hl);
		}
		{
			QGroupBox * gb = new QGroupBox(QApplication::tr("AssignmentOfKeys"));
			UPDATELANGUAGESETTITLE(generalSettingsWidget, gb, "AssignmentOfKeys");
			QGridLayout * gl = new QGridLayout();

			int row = 0;
			{
				QLabel * lbl = new QLabel(QApplication::tr("MoveForward"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "MoveForward");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyUp");

				_actionToButtonMap.insert("keyUp", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("MoveBackward"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "MoveBackward");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyDown");

				_actionToButtonMap.insert("keyDown", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("TurnLeft"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "TurnLeft");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyLeft");

				_actionToButtonMap.insert("keyLeft", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("TurnRight"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "TurnRight");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyRight");

				_actionToButtonMap.insert("keyRight", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("ActionLeft"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "ActionLeft");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyActionLeft");

				_actionToButtonMap.insert("keyActionLeft", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("ActionRight"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "ActionRight");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyActionRight");

				_actionToButtonMap.insert("keyActionRight", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Action"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Action");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyAction");

				_actionToButtonMap.insert("keyAction", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("StrafeLeft"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "StrafeLeft");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyStrafeLeft");

				_actionToButtonMap.insert("keyStrafeLeft", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("StrafeRight"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "StrafeRight");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyStrafeRight");

				_actionToButtonMap.insert("keyStrafeRight", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Inventory"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Inventory");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyInventory");

				_actionToButtonMap.insert("keyInventory", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Map"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Map");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyShowMap");

				_actionToButtonMap.insert("keyShowMap", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Sneak"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Sneak");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keySneak");

				_actionToButtonMap.insert("keySneak", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Status"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Status");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyShowStatus");

				_actionToButtonMap.insert("keyShowStatus", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Log"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Log");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyShowLog");

				_actionToButtonMap.insert("keyShowLog", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Heal"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Heal");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyHeal");

				_actionToButtonMap.insert("keyHeal", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Potion"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Potion");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyPotion");

				_actionToButtonMap.insert("keyPotion", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("LockTarget"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "LockTarget");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyLockTarget");

				_actionToButtonMap.insert("keyLockTarget", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Parade"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Parade");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyParade");

				_actionToButtonMap.insert("keyParade", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Slow"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Slow");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keySlow");

				_actionToButtonMap.insert("keySlow", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Jump"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Jump");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keySMove");

				_actionToButtonMap.insert("keySMove", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("DrawWeapon"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "DrawWeapon");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyWeapon");

				_actionToButtonMap.insert("keyWeapon", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("DrawMeleeWeapon"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "DrawMeleeWeapon");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyDrawMeleeWeapon");

				_actionToButtonMap.insert("keyDrawMeleeWeapon", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("DrawRangedWeapon"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "DrawRangedWeapon");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyDrawRangedWeapon");

				_actionToButtonMap.insert("keyDrawRangedWeapon", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Look"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Look");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyLook");

				_actionToButtonMap.insert("keyLook", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("LookFP"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "LookFP");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyLookFP");

				_actionToButtonMap.insert("keyLookFP", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("PreviousSpell"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "PreviousSpell");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyPreviousSpell");

				_actionToButtonMap.insert("keyPreviousSpell", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("NextSpell"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "NextSpell");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyNextSpell");

				_actionToButtonMap.insert("keyNextSpell", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("DrawSpell"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "DrawSpell");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyDrawSpell");

				_actionToButtonMap.insert("keyDrawSpell", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));

				row++;
			}
			{
				QLabel * lbl = new QLabel(QApplication::tr("Enter"), this);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, lbl, "Enter");
				gl->addWidget(lbl, row % 14, 2 * (row / 14));

				QPushButton * pb = new QPushButton(QApplication::tr("Unassigned"), this);
				gl->addWidget(pb, row % 14, 2 * (row / 14) + 1);
				pb->setProperty("action", "keyEnter");

				_actionToButtonMap.insert("keyEnter", pb);

				connect(pb, SIGNAL(released()), this, SLOT(newButton()));
			}

			gb->setLayout(gl);
			l->addWidget(gb);
		}

		setLayout(l);

		connect(_gamepadEnabled, SIGNAL(stateChanged(int)), this, SLOT(changedGamepadState(int)));

		rejectSettings();

		changedGamepadState(_gamepadEnabled->checkState());
	}

	GamepadSettingsWidget::~GamepadSettingsWidget() {
	}

	void GamepadSettingsWidget::saveSettings() {
		_iniParser->beginGroup("GAMEPAD");
		_iniParser->setValue("enabled", _gamepadEnabled->isChecked());
		_iniParser->setValue("index", _controllerList->count() ? _controllerList->currentData(Qt::UserRole).toInt() : 0);
		_iniParser->setValue("keyDelay", _keyDelayBox->value());

		for (auto it = _gamepadButtonToButtonMap.begin(); it != _gamepadButtonToButtonMap.end(); ++it) {
			_iniParser->setValue(it.value()->property("action").toString(), it.key());
		}
		_iniParser->endGroup();
	}

	void GamepadSettingsWidget::rejectSettings() {
		_iniParser->beginGroup("GAMEPAD");
		const bool firstStartup = !_iniParser->contains("enabled");
		const bool enabled = _iniParser->value("enabled", false).toBool();
		_gamepadEnabled->setChecked(enabled);

		int i = _iniParser->value("index", 0).toInt();
		for (int j = 0; j < _controllerList->count(); j++) {
			if (_controllerList->itemData(j, Qt::UserRole).toInt() == i) {
				_controllerList->setCurrentIndex(j);
				break;
			}
		}

		i = _iniParser->value("keyDelay", 150).toInt();
		_keyDelayBox->setValue(i);

		if (firstStartup) {
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_LStick_Y_Pos, _actionToButtonMap["keyUp"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_LStick_Y_Neg, _actionToButtonMap["keyDown"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_LStick_X_Pos, _actionToButtonMap["keyRight"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_LStick_X_Neg, _actionToButtonMap["keyLeft"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_LEFT_THUMB, _actionToButtonMap["keySneak"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_RIGHT_THUMB, _actionToButtonMap["keySlow"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_START, _actionToButtonMap["keyShowStatus"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_A, _actionToButtonMap["keyEnter"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_X, _actionToButtonMap["keyDrawMeleeWeapon"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_B, _actionToButtonMap["keyDrawRangedWeapon"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_Y, _actionToButtonMap["keyShowLog"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_LEFT_SHOULDER, _actionToButtonMap["keyInventory"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_RIGHT_SHOULDER, _actionToButtonMap["keyWeapon"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_RTrigger, _actionToButtonMap["keyAction"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_LTrigger, _actionToButtonMap["keySMove"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_DPAD_LEFT, _actionToButtonMap["keyPreviousSpell"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_DPAD_RIGHT, _actionToButtonMap["keyNextSpell"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_DPAD_UP, _actionToButtonMap["keyDrawSpell"]);
			_gamepadButtonToButtonMap.insert(gamepad::GamePadButton::GamePad_Button_DPAD_DOWN, _actionToButtonMap["keyShowMap"]);
			for (auto it = _gamepadButtonToButtonMap.begin(); it != _gamepadButtonToButtonMap.end(); ++it) {
				const QString buttonText = buttonToString(it.key());
				it.value()->setText(buttonText);
			}
		} else {
			for (auto it = _actionToButtonMap.begin(); it != _actionToButtonMap.end(); ++it) {
				i = _iniParser->value(it.key(), gamepad::GamePadButton_Max).toInt();
				it.value()->setText(buttonToString(gamepad::GamePadButton(i)));
				if (i != gamepad::GamePadButton_Max) {
					_gamepadButtonToButtonMap.insert(gamepad::GamePadButton(i), it.value());
				}
			}
		}
		_iniParser->endGroup();
	}

	bool GamepadSettingsWidget::isEnabled() const {
		return _gamepadEnabled->isChecked() && _controllerList->count();
	}

	gamepad::GamePadIndex GamepadSettingsWidget::getIndex() const {
		return _controllerList->count() ? gamepad::GamePadIndex(_controllerList->currentData(Qt::UserRole).toInt()) : gamepad::GamePadIndex::GamePadIndex_One;
	}

	int GamepadSettingsWidget::getKeyDelay() const {
		return _keyDelayBox->value();
	}

	QMap<QString, gamepad::GamePadButton> GamepadSettingsWidget::getKeyMapping() const {
		QMap<QString, gamepad::GamePadButton> map;
		for (auto it = _gamepadButtonToButtonMap.begin(); it != _gamepadButtonToButtonMap.end(); ++it) {
			for (auto it2 = _actionToButtonMap.begin(); it2 != _actionToButtonMap.end(); ++it2) {
				if (it.value() == it2.value()) {
					map.insert(it2.key(), it.key());
					break;
				}
			}
		}
		return map;
	}

	void GamepadSettingsWidget::changedGamepadState(int checkState) {
		_controllerList->setEnabled(checkState == Qt::CheckState::Checked);
		_keyDelayBox->setEnabled(checkState == Qt::CheckState::Checked);
	}

	void GamepadSettingsWidget::newButton() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		if (pb) {
			const gamepad::GamePadButton button = gamepad::GamePadXbox::getButtonPressed(gamepad::GamePadIndex(_controllerList->count() ? _controllerList->currentData(Qt::UserRole).toInt() : 0));
			if (button == gamepad::GamePadButton_Max) {
				return;
			}
			auto it = _gamepadButtonToButtonMap.find(button);
			if (it != _gamepadButtonToButtonMap.end()) { // button already used => remove assignment
				it.value()->setText(QApplication::tr("Unassigned"));
				_gamepadButtonToButtonMap.erase(it);
			}
			for (it = _gamepadButtonToButtonMap.begin(); it != _gamepadButtonToButtonMap.end(); ++it) {
				if (it.value() == pb) {
					_gamepadButtonToButtonMap.erase(it);
					break;
				}
			}
			pb->setText(buttonToString(button));
			_gamepadButtonToButtonMap.insert(button, pb);
		}
	}

} /* namespace widgets */
} /* namespace spine */
