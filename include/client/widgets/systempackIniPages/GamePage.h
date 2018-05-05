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

#ifndef __SPINE_WIDGETS_SYSTEMPACKINIPAGES_GAMEPAGE_H__
#define __SPINE_WIDGETS_SYSTEMPACKINIPAGES_GAMEPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace sp {

	class GamePage : public QWidget {
		Q_OBJECT

	public:
		GamePage(QSettings * iniParser, QWidget * par);
		~GamePage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QComboBox * _AlwaysON;
		QComboBox * _AlwaysOFF;
		QComboBox * _DisableSound;
		QComboBox * _DisableDamage;

		QComboBox * _Control;
		QDoubleSpinBox * _TimeMultiplier;
		QDoubleSpinBox * _MaxTimePerPhrase;
		QDoubleSpinBox * _TimePerChar;

		QComboBox * _Enable;

		QComboBox * _Scale;
		QComboBox * _ForceMenuScale;
		QSpinBox * _ScaleMenusX;
		QSpinBox * _ScaleMenusY;
		QSpinBox * _DialogBoxX;
		QSpinBox * _DialogBoxY;
		QSpinBox * _SubtitlesBoxX;
		QComboBox * _ShowManaBar;
		QComboBox * _ShowSwimBar;
		QComboBox * _HideHealthBar;
		QSpinBox * _NewChapterSizeX;
		QSpinBox * _NewChapterSizeY;
		QSpinBox * _SaveGameImageSizeX;
		QSpinBox * _SaveGameImageSizeY;
		QSpinBox * _InventoryItemNoteSizeX;
		QSpinBox * _InventoryCellSize;
	};

} /* namespace sp */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_SYSTEMPACKINIPAGES_GAMEPAGE_H__ */
