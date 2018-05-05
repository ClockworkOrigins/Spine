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

#ifndef __SPINE_WIDGETS_GOTHIC2INIPAGES_GAMEPAGE_H__
#define __SPINE_WIDGETS_GOTHIC2INIPAGES_GAMEPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g2 {

	class GamePage : public QWidget {
		Q_OBJECT

	public:
		GamePage(QSettings * iniParser, QWidget * par);
		~GamePage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QComboBox * _subtitleComboBox;
		QComboBox * _subtitleAmbientComboBox;
		QComboBox * _subtitlePlayerComboBox;
		QComboBox * _subtitleNoiseComboBox;
		QSpinBox * _gameTextAutoScrollSpinBox;

		QComboBox * _camLookaroundInverse;
		QSpinBox * _cameraLightRange;
		QComboBox * _zDontSwitchToThirdPerson;
		QComboBox * _highlightMeleeFocus;
		QComboBox * _highlightInteractFocus;

		QComboBox * _usePotionKeys;
		QComboBox * _useQuickSaveKeys;
		QComboBox * _useGothic1Controls;
		QComboBox * _pickLockScramble;

		QComboBox * _invShowArrows;
		QComboBox * _invSwitchToFirstCategory;
		QDoubleSpinBox * _zInventoryItemDistanceScale;
		QSpinBox * _invMaxColumns;
		QSpinBox * _invMaxRows;
		QComboBox * _invSplitScreen;

		QDoubleSpinBox * _SHORTKEY1FARPLANEDIST;
		QDoubleSpinBox * _SHORTKEY2FARPLANEDIST;
		QDoubleSpinBox * _SHORTKEY3FARPLANEDIST;
		QDoubleSpinBox * _SHORTKEY4FARPLANEDIST;
	};

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC2INIPAGES_GAMEPAGE_H__ */
