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

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g1 {

	class SoundPage : public QWidget {
		Q_OBJECT

	public:
		SoundPage(QSettings * iniParser, QWidget * par);
		~SoundPage();

		void reject();
		void accept();

		void updateSettings(QSettings * iniParser);

	private:
		QSettings * _iniParser;
		QComboBox * _soundEnabled;
		QComboBox * _musicEnabled;

		QDoubleSpinBox * _soundVolume;
		QDoubleSpinBox * _musicVolume;

		QComboBox * _soundUseReverb;
	};

} /* namespace g1 */
} /* namespace widgets */
} /* namespace spine */
