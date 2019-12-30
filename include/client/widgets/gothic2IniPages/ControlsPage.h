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

#ifndef __SPINE_WIDGETS_GOTHIC2INIPAGES_CONTROLSPAGE_H__
#define __SPINE_WIDGETS_GOTHIC2INIPAGES_CONTROLSPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g2 {

	class ControlsPage : public QWidget {
		Q_OBJECT

	public:
		ControlsPage(QSettings * iniParser, QWidget * par);
		~ControlsPage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QSpinBox * _keyDelayRate;
		QSpinBox * _keyDelayFirst;
		QComboBox * _extendedVideoKeys;
		QComboBox * _disallowVideoInput;
		QComboBox * _zKillSysKeys;
		QComboBox * _keyboardLayout;

		QComboBox * _enableMouse;
		QDoubleSpinBox * _mouseSensitivity;
		QDoubleSpinBox * _zMouseRotationScale;
		QSpinBox * _zSmoothMouse;

		QComboBox * _enableJoystick;
	};

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC2INIPAGES_CONTROLSPAGE_H__ */
