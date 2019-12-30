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

#ifndef __SPINE_WIDGETS_SYSTEMPACKINIPAGES_SYSTEMPAGE_H__
#define __SPINE_WIDGETS_SYSTEMPACKINIPAGES_SYSTEMPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace sp {

	class SystemPage : public QWidget {
		Q_OBJECT

	public:
		SystemPage(QSettings * iniParser, QWidget * par);
		~SystemPage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QComboBox * _FixGameUX;
		QComboBox * _Disable_D3DVBCAPS_WRITEONLY;
		QComboBox * _BorderlessWindow;
		QComboBox * _ZNORESTHREAD;
		QComboBox * _MoverBugfix;
		QComboBox * _NumLockDisable;
		QComboBox * _DisableCacheOut;
		QComboBox * _QuickSaveEnable;
		QComboBox * _USInternationalKeyboardLayout;
		QComboBox * _Polish_version;
		QComboBox * _PFXfix;
		QComboBox * _BUGFIX_already_deleted_zCObject;
		QComboBox * _Disable_HUMANS_SWIMMDS;
		QComboBox * _Game_InitEngIntl;
		QComboBox * _FixHighRes;
		QComboBox * _FixAppCompat;
		QComboBox * _FixBink;
		QComboBox * _FixMss;

		QDoubleSpinBox * _VerticalFOV;
		QComboBox * _NewFOVformula;
		QComboBox * _DisableLOD;
		QComboBox * _DisableIndoorClipping;
		QSpinBox * _SPAWN_INSERTRANGE;
		QSpinBox * _SPAWN_REMOVERANGE;
		QSpinBox * _SPAWN_INSERTTIME_MAX;
		QSpinBox * _DrawDistanceMultiplier;
		QDoubleSpinBox * _OutDoorPortalDistanceMultiplier;
		QDoubleSpinBox * _InDoorPortalDistanceMultiplier;
		QDoubleSpinBox * _WoodPortalDistanceMultiplier;
		QDoubleSpinBox * _zMouseRotationScale;
		QComboBox * _EnableShields;
		QComboBox * _No_Take_Anim;
		QComboBox * _RMB_No_Take_Anim;
		QDoubleSpinBox * _TRADE_VALUE_MULTIPLIER;
		QComboBox * _Animated_Inventory;
		QSpinBox * _keyDelayRate;
		QSpinBox * _keyDelayFirst;
		QComboBox * _HighlightMeleeFocus;
		QComboBox * _HighlightInteractFocus;
		QComboBox * _HighlightInteractNoFocus;
		QComboBox * _Fight_ANI_Interrupt;
		QDoubleSpinBox * _ReverbVolume;

		QComboBox * _bShowGothicError;
		QComboBox * _bShowMsgBox;
		QComboBox * _bUseNewHandler;
		QSpinBox * _reserveInMb;
	};

} /* namespace sp */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_SYSTEMPACKINIPAGES_SYSTEMPAGE_H__ */
