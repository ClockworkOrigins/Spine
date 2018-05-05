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

#include "widgets/gothic2IniPages/ControlsPage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace g2 {

	ControlsPage::ControlsPage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();

		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * keyboardBox = new QGroupBox(QApplication::tr("Keyboard"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("keyDelayRate", keyboardBox);
				lbl->setToolTip(QApplication::tr("keyDelayRateTooltip"));
				_keyDelayRate = new QSpinBox(keyboardBox);
				_keyDelayRate->setMinimum(0);
				_keyDelayRate->setMaximum(100000);
				_keyDelayRate->setSuffix(" " + QApplication::tr("Milliseconds"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_keyDelayRate, 0, 1);

				lbl = new QLabel("keyDelayFirst", keyboardBox);
				lbl->setToolTip(QApplication::tr("keyDelayFirstTooltip"));
				_keyDelayFirst = new QSpinBox(keyboardBox);
				_keyDelayFirst->setMinimum(0);
				_keyDelayFirst->setMaximum(100000);
				_keyDelayFirst->setSuffix(" " + QApplication::tr("Milliseconds"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_keyDelayFirst, 1, 1);

				lbl = new QLabel("extendedVideoKeys", keyboardBox);
				lbl->setToolTip(QApplication::tr("extendedVideoKeysTooltip"));
				_extendedVideoKeys = new QComboBox(keyboardBox);
				_extendedVideoKeys->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_extendedVideoKeys, 2, 1);

				lbl = new QLabel("disallowVideoInput", keyboardBox);
				lbl->setToolTip(QApplication::tr("disallowVideoInputTooltip"));
				_disallowVideoInput = new QComboBox(keyboardBox);
				_disallowVideoInput->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_disallowVideoInput, 3, 1);

				lbl = new QLabel("zKillSysKeys", keyboardBox);
				lbl->setToolTip(QApplication::tr("zKillSysKeysTooltip"));
				_zKillSysKeys = new QComboBox(keyboardBox);
				_zKillSysKeys->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_zKillSysKeys, 4, 1);

				lbl = new QLabel("keyboardLayout", keyboardBox);
				lbl->setToolTip(QApplication::tr("keyboardLayoutTooltip"));
				_keyboardLayout = new QComboBox(keyboardBox);
				_keyboardLayout->addItems(QStringList() << QApplication::tr("German") << QApplication::tr("USInternational"));
				gl->addWidget(lbl, 5, 0);
				gl->addWidget(_keyboardLayout, 5, 1);

				keyboardBox->setLayout(gl);

				hl->addWidget(keyboardBox);
			}

			{
				QGroupBox * mouseBox = new QGroupBox(QApplication::tr("Mouse"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("enableMouse", mouseBox);
				lbl->setToolTip(QApplication::tr("enableMouseTooltip"));
				_enableMouse = new QComboBox(mouseBox);
				_enableMouse->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_enableMouse, 0, 1);

				lbl = new QLabel("mouseSensitivity", mouseBox);
				lbl->setToolTip(QApplication::tr("mouseSensitivityTooltip"));
				_mouseSensitivity = new QDoubleSpinBox(mouseBox);
				_mouseSensitivity->setMinimum(0.0);
				_mouseSensitivity->setMaximum(1.0);
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_mouseSensitivity, 1, 1);

				lbl = new QLabel("zMouseRotationScale", mouseBox);
				lbl->setToolTip(QApplication::tr("zMouseRotationScaleTooltip"));
				_zMouseRotationScale = new QDoubleSpinBox(mouseBox);
				_zMouseRotationScale->setMinimum(0.0);
				_zMouseRotationScale->setMaximum(100.0);
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_zMouseRotationScale, 2, 1);

				lbl = new QLabel("zSmoothMouse", mouseBox);
				lbl->setToolTip(QApplication::tr("zSmoothMouseTooltip"));
				_zSmoothMouse = new QSpinBox(mouseBox);
				_zSmoothMouse->setMinimum(0);
				_zSmoothMouse->setMaximum(100);
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_zSmoothMouse, 3, 1);

				mouseBox->setLayout(gl);

				hl->addWidget(mouseBox);
			}

			{
				QGroupBox * joystickBox = new QGroupBox(QApplication::tr("Joystick"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("enableJoystick", joystickBox);
				lbl->setToolTip(QApplication::tr("enableJoystickTooltip"));
				_enableJoystick = new QComboBox(joystickBox);
				_enableJoystick->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_enableJoystick, 0, 1);

				joystickBox->setLayout(gl);

				hl->addWidget(joystickBox);
			}

			l->addLayout(hl);
		}

		setLayout(l);

		reject();
	}

	ControlsPage::~ControlsPage() {
	}

	void ControlsPage::reject() {
		// Keyboard
		int idx;
		int value;
		double d;
		QString text;
		value = _iniParser->value("GAME/keyDelayRate", 150).toInt();
		_keyDelayRate->setValue(value);
		value = _iniParser->value("GAME/keyDelayFirst", 0).toInt();
		_keyDelayFirst->setValue(value);
		idx = _iniParser->value("GAME/extendedVideoKeys", 0).toInt();
		_extendedVideoKeys->setCurrentIndex(idx);
		idx = _iniParser->value("GAME/disallowVideoInput", 0).toInt();
		_disallowVideoInput->setCurrentIndex(idx);
		idx = _iniParser->value("ENGINE/zKillSysKeys", 0).toInt();
		_zKillSysKeys->setCurrentIndex(idx);
		text = _iniParser->value("GAME/keyboardLayout", "00000407").toString();
		_keyboardLayout->setCurrentIndex((text == "00000407") ? 0 : 1);

		// Mouse
		idx = _iniParser->value("GAME/enableMouse", 0).toInt();
		_enableMouse->setCurrentIndex(idx);
		d = _iniParser->value("GAME/mouseSensitivity", 0.5).toDouble();
		_mouseSensitivity->setValue(d);
		d = _iniParser->value("ENGINE/zMouseRotationScale", 2.0).toDouble();
		_zMouseRotationScale->setValue(d);
		value = _iniParser->value("ENGINE/zSmoothMouse", 3).toInt();
		_zSmoothMouse->setValue(value);

		// Joystick
		idx = _iniParser->value("GAME/enableJoystick", 0).toInt();
		_enableJoystick->setCurrentIndex(idx);
	}

	void ControlsPage::accept() {
		// Keyboard
		_iniParser->setValue("GAME/keyDelayRate", _keyDelayRate->value());
		_iniParser->setValue("GAME/keyDelayFirst", _keyDelayFirst->value());
		_iniParser->setValue("GAME/extendedVideoKeys", _extendedVideoKeys->currentIndex());
		_iniParser->setValue("GAME/disallowVideoInput", _disallowVideoInput->currentIndex());
		_iniParser->setValue("ENGINE/zKillSysKeys", _zKillSysKeys->currentIndex());
		_iniParser->setValue("GAME/keyboardLayout", (_keyboardLayout->currentIndex() == 0) ? "00000407" : "00020409");

		// Mouse
		_iniParser->setValue("GAME/enableMouse", _enableMouse->currentIndex());
		_iniParser->setValue("GAME/mouseSensitivity", _mouseSensitivity->value());
		_iniParser->setValue("ENGINE/zMouseRotationScale", _zMouseRotationScale->value());
		_iniParser->setValue("ENGINE/zSmoothMouse", _zSmoothMouse->value());

		// Joystick
		_iniParser->setValue("GAME/enableJoystick", _enableJoystick->currentIndex());
	}

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */
