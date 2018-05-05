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

#include "widgets/systempackIniPages/SystemPage.h"

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace sp {

	SystemPage::SystemPage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();

		QScrollArea * scrollArea = new QScrollArea(this);
		scrollArea->setProperty("default", true);

		QWidget * w = new QWidget(scrollArea);

		scrollArea->setWidget(w);
		scrollArea->setWidgetResizable(true);
		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * debugBox = new QGroupBox(QApplication::tr("Debug"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("FixGameUX", debugBox);
				lbl->setToolTip(QApplication::tr("FixGameUXTooltip"));
				_FixGameUX = new QComboBox(debugBox);
				_FixGameUX->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_FixGameUX, 0, 1);

				lbl = new QLabel("Disable_D3DVBCAPS_WRITEONLY", debugBox);
				lbl->setToolTip(QApplication::tr("Disable_D3DVBCAPS_WRITEONLYTooltip"));
				_Disable_D3DVBCAPS_WRITEONLY = new QComboBox(debugBox);
				_Disable_D3DVBCAPS_WRITEONLY->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_Disable_D3DVBCAPS_WRITEONLY, 1, 1);

				lbl = new QLabel("BorderlessWindow", debugBox);
				lbl->setToolTip(QApplication::tr("BorderlessWindowTooltip"));
				_BorderlessWindow = new QComboBox(debugBox);
				_BorderlessWindow->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_BorderlessWindow, 2, 1);

				lbl = new QLabel("ZNORESTHREAD", debugBox);
				lbl->setToolTip(QApplication::tr("ZNORESTHREADTooltip"));
				_ZNORESTHREAD = new QComboBox(debugBox);
				_ZNORESTHREAD->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_ZNORESTHREAD, 3, 1);

				lbl = new QLabel("MoverBugfix", debugBox);
				lbl->setToolTip(QApplication::tr("MoverBugfixTooltip"));
				_MoverBugfix = new QComboBox(debugBox);
				_MoverBugfix->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_MoverBugfix, 4, 1);

				lbl = new QLabel("NumLockDisable", debugBox);
				lbl->setToolTip(QApplication::tr("NumLockDisableTooltip"));
				_NumLockDisable = new QComboBox(debugBox);
				_NumLockDisable->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 5, 0);
				gl->addWidget(_NumLockDisable, 5, 1);

				lbl = new QLabel("DisableCacheOut", debugBox);
				lbl->setToolTip(QApplication::tr("DisableCacheOutTooltip"));
				_DisableCacheOut = new QComboBox(debugBox);
				_DisableCacheOut->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 6, 0);
				gl->addWidget(_DisableCacheOut, 6, 1);

				lbl = new QLabel("QuickSaveEnable", debugBox);
				lbl->setToolTip(QApplication::tr("QuickSaveEnableTooltip"));
				_QuickSaveEnable = new QComboBox(debugBox);
				_QuickSaveEnable->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 7, 0);
				gl->addWidget(_QuickSaveEnable, 7, 1);

				lbl = new QLabel("USInternationalKeyboardLayout", debugBox);
				lbl->setToolTip(QApplication::tr("USInternationalKeyboardLayoutTooltip"));
				_USInternationalKeyboardLayout = new QComboBox(debugBox);
				_USInternationalKeyboardLayout->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 8, 0);
				gl->addWidget(_USInternationalKeyboardLayout, 8, 1);

				lbl = new QLabel("Polish_version", debugBox);
				lbl->setToolTip(QApplication::tr("Polish_versionTooltip"));
				_Polish_version = new QComboBox(debugBox);
				_Polish_version->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 9, 0);
				gl->addWidget(_Polish_version, 9, 1);

				lbl = new QLabel("PFXfix", debugBox);
				lbl->setToolTip(QApplication::tr("PFXfixTooltip"));
				_PFXfix = new QComboBox(debugBox);
				_PFXfix->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 10, 0);
				gl->addWidget(_PFXfix, 10, 1);

				lbl = new QLabel("BUGFIX_already_deleted_zCObject", debugBox);
				lbl->setToolTip(QApplication::tr("BUGFIX_already_deleted_zCObjectTooltip"));
				_BUGFIX_already_deleted_zCObject = new QComboBox(debugBox);
				_BUGFIX_already_deleted_zCObject->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 11, 0);
				gl->addWidget(_BUGFIX_already_deleted_zCObject, 11, 1);

				lbl = new QLabel("Disable_HUMANS_SWIM.MDS", debugBox);
				lbl->setToolTip(QApplication::tr("Disable_HUMANS_SWIM.MDSTooltip"));
				_Disable_HUMANS_SWIMMDS = new QComboBox(debugBox);
				_Disable_HUMANS_SWIMMDS->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 12, 0);
				gl->addWidget(_Disable_HUMANS_SWIMMDS, 12, 1);

				lbl = new QLabel("Game_InitEngIntl", debugBox);
				lbl->setToolTip(QApplication::tr("Game_InitEngIntlTooltip"));
				_Game_InitEngIntl = new QComboBox(debugBox);
				_Game_InitEngIntl->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 13, 0);
				gl->addWidget(_Game_InitEngIntl, 13, 1);

				lbl = new QLabel("FixHighRes", debugBox);
				lbl->setToolTip(QApplication::tr("FixHighResTooltip"));
				_FixHighRes = new QComboBox(debugBox);
				_FixHighRes->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 14, 0);
				gl->addWidget(_FixHighRes, 14, 1);

				lbl = new QLabel("FixAppCompat", debugBox);
				lbl->setToolTip(QApplication::tr("FixAppCompatTooltip"));
				_FixAppCompat = new QComboBox(debugBox);
				_FixAppCompat->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 15, 0);
				gl->addWidget(_FixAppCompat, 15, 1);

				lbl = new QLabel("FixBink", debugBox);
				lbl->setToolTip(QApplication::tr("FixBinkTooltip"));
				_FixBink = new QComboBox(debugBox);
				_FixBink->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 16, 0);
				gl->addWidget(_FixBink, 16, 1);

				lbl = new QLabel("FixMss", debugBox);
				lbl->setToolTip(QApplication::tr("FixMssTooltip"));
				_FixMss = new QComboBox(debugBox);
				_FixMss->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 17, 0);
				gl->addWidget(_FixMss, 17, 1);

				debugBox->setLayout(gl);

				hl->addWidget(debugBox);
			}

			{
				QGroupBox * debugBox = new QGroupBox(QApplication::tr("Parameters"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("VerticalFOV", debugBox);
				lbl->setToolTip(QApplication::tr("VerticalFOVTooltip"));
				_VerticalFOV = new QDoubleSpinBox(debugBox);
				_VerticalFOV->setMinimum(0.0);
				_VerticalFOV->setMaximum(1000.0);
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_VerticalFOV, 0, 1);

				lbl = new QLabel("NewFOVformula", debugBox);
				lbl->setToolTip(QApplication::tr("NewFOVformulaTooltip"));
				_NewFOVformula = new QComboBox(debugBox);
				_NewFOVformula->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_NewFOVformula, 1, 1);

				lbl = new QLabel("DisableLOD", debugBox);
				lbl->setToolTip(QApplication::tr("DisableLODTooltip"));
				_DisableLOD = new QComboBox(debugBox);
				_DisableLOD->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_DisableLOD, 2, 1);

				lbl = new QLabel("DisableIndoorClipping", debugBox);
				lbl->setToolTip(QApplication::tr("DisableIndoorClippingTooltip"));
				_DisableIndoorClipping = new QComboBox(debugBox);
				_DisableIndoorClipping->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_DisableIndoorClipping, 3, 1);

				lbl = new QLabel("SPAWN_INSERTRANGE", debugBox);
				lbl->setToolTip(QApplication::tr("SPAWN_INSERTRANGETooltip"));
				_SPAWN_INSERTRANGE = new QSpinBox(debugBox);
				_SPAWN_INSERTRANGE->setMinimum(0);
				_SPAWN_INSERTRANGE->setMaximum(100000);
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_SPAWN_INSERTRANGE, 4, 1);

				lbl = new QLabel("SPAWN_REMOVERANGE", debugBox);
				lbl->setToolTip(QApplication::tr("SPAWN_REMOVERANGETooltip"));
				_SPAWN_REMOVERANGE = new QSpinBox(debugBox);
				_SPAWN_REMOVERANGE->setMinimum(0);
				_SPAWN_REMOVERANGE->setMaximum(100000);
				gl->addWidget(lbl, 5, 0);
				gl->addWidget(_SPAWN_REMOVERANGE, 5, 1);

				lbl = new QLabel("SPAWN_INSERTTIME_MAX", debugBox);
				lbl->setToolTip(QApplication::tr("SPAWN_INSERTTIME_MAXTooltip"));
				_SPAWN_INSERTTIME_MAX = new QSpinBox(debugBox);
				_SPAWN_INSERTTIME_MAX->setMinimum(0);
				_SPAWN_INSERTTIME_MAX->setMaximum(100000);
				gl->addWidget(lbl, 6, 0);
				gl->addWidget(_SPAWN_INSERTTIME_MAX, 6, 1);

				lbl = new QLabel("DrawDistanceMultiplier", debugBox);
				lbl->setToolTip(QApplication::tr("DrawDistanceMultiplierTooltip"));
				_DrawDistanceMultiplier = new QSpinBox(debugBox);
				_DrawDistanceMultiplier->setMinimum(0);
				_DrawDistanceMultiplier->setMaximum(26);
				gl->addWidget(lbl, 7, 0);
				gl->addWidget(_DrawDistanceMultiplier, 7, 1);

				lbl = new QLabel("OutDoorPortalDistanceMultiplier", debugBox);
				lbl->setToolTip(QApplication::tr("OutDoorPortalDistanceMultiplierTooltip"));
				_OutDoorPortalDistanceMultiplier = new QDoubleSpinBox(debugBox);
				_OutDoorPortalDistanceMultiplier->setMinimum(0.0);
				_OutDoorPortalDistanceMultiplier->setMaximum(10.0);
				gl->addWidget(lbl, 8, 0);
				gl->addWidget(_OutDoorPortalDistanceMultiplier, 8, 1);

				lbl = new QLabel("InDoorPortalDistanceMultiplier", debugBox);
				lbl->setToolTip(QApplication::tr("InDoorPortalDistanceMultiplierTooltip"));
				_InDoorPortalDistanceMultiplier = new QDoubleSpinBox(debugBox);
				_InDoorPortalDistanceMultiplier->setMinimum(0.0);
				_InDoorPortalDistanceMultiplier->setMaximum(10.0);
				gl->addWidget(lbl, 9, 0);
				gl->addWidget(_InDoorPortalDistanceMultiplier, 9, 1);

				lbl = new QLabel("WoodPortalDistanceMultiplier", debugBox);
				lbl->setToolTip(QApplication::tr("WoodPortalDistanceMultiplierTooltip"));
				_WoodPortalDistanceMultiplier = new QDoubleSpinBox(debugBox);
				_WoodPortalDistanceMultiplier->setMinimum(0.0);
				_WoodPortalDistanceMultiplier->setMaximum(10.0);
				gl->addWidget(lbl, 10, 0);
				gl->addWidget(_WoodPortalDistanceMultiplier, 10, 1);

				lbl = new QLabel("zMouseRotationScale", debugBox);
				lbl->setToolTip(QApplication::tr("zMouseRotationScaleTooltip"));
				_zMouseRotationScale = new QDoubleSpinBox(debugBox);
				_zMouseRotationScale->setMinimum(0.1);
				_zMouseRotationScale->setMaximum(0.3);
				gl->addWidget(lbl, 11, 0);
				gl->addWidget(_zMouseRotationScale, 11, 1);

				lbl = new QLabel("EnableShields", debugBox);
				lbl->setToolTip(QApplication::tr("EnableShieldsTooltip"));
				_EnableShields = new QComboBox(debugBox);
				_EnableShields->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 12, 0);
				gl->addWidget(_EnableShields, 12, 1);

				lbl = new QLabel("No_Take_Anim", debugBox);
				lbl->setToolTip(QApplication::tr("No_Take_AnimTooltip"));
				_No_Take_Anim = new QComboBox(debugBox);
				_No_Take_Anim->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 13, 0);
				gl->addWidget(_No_Take_Anim, 13, 1);

				lbl = new QLabel("RMB_No_Take_Anim", debugBox);
				lbl->setToolTip(QApplication::tr("RMB_No_Take_AnimTooltip"));
				_RMB_No_Take_Anim = new QComboBox(debugBox);
				_RMB_No_Take_Anim->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 14, 0);
				gl->addWidget(_RMB_No_Take_Anim, 14, 1);

				lbl = new QLabel("TRADE_VALUE_MULTIPLIER", debugBox);
				lbl->setToolTip(QApplication::tr("TRADE_VALUE_MULTIPLIERTooltip"));
				_TRADE_VALUE_MULTIPLIER = new QDoubleSpinBox(debugBox);
				_TRADE_VALUE_MULTIPLIER->setMinimum(0.01);
				_TRADE_VALUE_MULTIPLIER->setMaximum(100.0);
				gl->addWidget(lbl, 15, 0);
				gl->addWidget(_TRADE_VALUE_MULTIPLIER, 15, 1);

				lbl = new QLabel("Animated_Inventory", debugBox);
				lbl->setToolTip(QApplication::tr("Animated_InventoryTooltip"));
				_Animated_Inventory = new QComboBox(debugBox);
				_Animated_Inventory->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 16, 0);
				gl->addWidget(_Animated_Inventory, 16, 1);

				lbl = new QLabel("keyDelayRate", debugBox);
				lbl->setToolTip(QApplication::tr("keyDelayRateTooltip"));
				_keyDelayRate = new QSpinBox(debugBox);
				_keyDelayRate->setMinimum(10);
				_keyDelayRate->setMaximum(300);
				gl->addWidget(lbl, 17, 0);
				gl->addWidget(_keyDelayRate, 17, 1);

				lbl = new QLabel("keyDelayFirst", debugBox);
				lbl->setToolTip(QApplication::tr("keyDelayFirstTooltip"));
				_keyDelayFirst = new QSpinBox(debugBox);
				_keyDelayFirst->setMinimum(100);
				_keyDelayFirst->setMaximum(1000);
				gl->addWidget(lbl, 18, 0);
				gl->addWidget(_keyDelayFirst, 18, 1);

				lbl = new QLabel("HighlightMeleeFocus", debugBox);
				lbl->setToolTip(QApplication::tr("HighlightMeleeFocusTooltip"));
				_HighlightMeleeFocus = new QComboBox(debugBox);
				_HighlightMeleeFocus->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 19, 0);
				gl->addWidget(_HighlightMeleeFocus, 19, 1);

				lbl = new QLabel("HighlightInteractFocus", debugBox);
				lbl->setToolTip(QApplication::tr("HighlightInteractFocusTooltip"));
				_HighlightInteractFocus = new QComboBox(debugBox);
				_HighlightInteractFocus->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 20, 0);
				gl->addWidget(_HighlightInteractFocus, 20, 1);

				lbl = new QLabel("HighlightInteractNoFocus", debugBox);
				lbl->setToolTip(QApplication::tr("HighlightInteractNoFocusTooltip"));
				_HighlightInteractNoFocus = new QComboBox(debugBox);
				_HighlightInteractNoFocus->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 21, 0);
				gl->addWidget(_HighlightInteractNoFocus, 21, 1);

				lbl = new QLabel("Fight_ANI_Interrupt", debugBox);
				lbl->setToolTip(QApplication::tr("Fight_ANI_InterruptTooltip"));
				_Fight_ANI_Interrupt = new QComboBox(debugBox);
				_Fight_ANI_Interrupt->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 22, 0);
				gl->addWidget(_Fight_ANI_Interrupt, 22, 1);

				lbl = new QLabel("ReverbVolume", debugBox);
				lbl->setToolTip(QApplication::tr("ReverbVolumeTooltip"));
				_ReverbVolume = new QDoubleSpinBox(debugBox);
				_ReverbVolume->setMinimum(0.1);
				_ReverbVolume->setMaximum(10.0);
				gl->addWidget(lbl, 23, 0);
				gl->addWidget(_ReverbVolume, 23, 1);

				debugBox->setLayout(gl);

				hl->addWidget(debugBox);
			}

			{
				QGroupBox * debugBox = new QGroupBox(QApplication::tr("Shw32"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("bShowGothicError", debugBox);
				lbl->setToolTip(QApplication::tr("bShowGothicErrorTooltip"));
				_bShowGothicError = new QComboBox(debugBox);
				_bShowGothicError->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_bShowGothicError, 0, 1);

				lbl = new QLabel("bShowMsgBox", debugBox);
				lbl->setToolTip(QApplication::tr("bShowMsgBoxTooltip"));
				_bShowMsgBox = new QComboBox(debugBox);
				_bShowMsgBox->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_bShowMsgBox, 1, 1);

				lbl = new QLabel("bUseNewHandler", debugBox);
				lbl->setToolTip(QApplication::tr("bUseNewHandlerTooltip"));
				_bUseNewHandler = new QComboBox(debugBox);
				_bUseNewHandler->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_bUseNewHandler, 2, 1);

				lbl = new QLabel("reserveInMb", debugBox);
				lbl->setToolTip(QApplication::tr("reserveInMbTooltip"));
				_reserveInMb = new QSpinBox(debugBox);
				_reserveInMb->setMinimum(5);
				_reserveInMb->setMaximum(300);
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_reserveInMb, 3, 1);

				debugBox->setLayout(gl);

				hl->addWidget(debugBox);
			}

			w->setLayout(hl);
		}
		l->addWidget(scrollArea);

		setLayout(l);

		reject();
	}

	SystemPage::~SystemPage() {
	}

	void SystemPage::reject() {
		// Debug
		int i;
		double d;
		_iniParser->beginGroup("DEBUG");
		i = _iniParser->value("FixGameUX", 1).toInt();
		_FixGameUX->setCurrentIndex(i);
		i = _iniParser->value("Disable_D3DVBCAPS_WRITEONLY", 1).toInt();
		_Disable_D3DVBCAPS_WRITEONLY->setCurrentIndex(i);
		i = _iniParser->value("BorderlessWindow", 0).toInt();
		_BorderlessWindow->setCurrentIndex(i);
		i = _iniParser->value("ZNORESTHREAD", 1).toInt();
		_ZNORESTHREAD->setCurrentIndex(i);
		i = _iniParser->value("MoverBugfix", 1).toInt();
		_MoverBugfix->setCurrentIndex(i);
		i = _iniParser->value("NumLockDisable", 1).toInt();
		_NumLockDisable->setCurrentIndex(i);
		i = _iniParser->value("DisableCacheOut", 1).toInt();
		_DisableCacheOut->setCurrentIndex(i);
		i = _iniParser->value("QuickSaveEnable", 1).toInt();
		_QuickSaveEnable->setCurrentIndex(i);
		i = _iniParser->value("USInternationalKeyboardLayout", 1).toInt();
		_USInternationalKeyboardLayout->setCurrentIndex(i);
		i = _iniParser->value("Polish_version", 0).toInt();
		_Polish_version->setCurrentIndex(i);
		i = _iniParser->value("PFXfix", 1).toInt();
		_PFXfix->setCurrentIndex(i);
		i = _iniParser->value("BUGFIX_already_deleted_zCObject", 1).toInt();
		_BUGFIX_already_deleted_zCObject->setCurrentIndex(i);
		i = _iniParser->value("Disable_HUMANS_SWIM", 0).toInt();
		_Disable_HUMANS_SWIMMDS->setCurrentIndex(i);
		i = _iniParser->value("Game_InitEngIntl", 1).toInt();
		_Game_InitEngIntl->setCurrentIndex(i);
		i = _iniParser->value("FixHighRes", 1).toInt();
		_FixHighRes->setCurrentIndex(i);
		i = _iniParser->value("FixAppCompat", 1).toInt();
		_FixAppCompat->setCurrentIndex(i);
		i = _iniParser->value("FixBink", 1).toInt();
		_FixBink->setCurrentIndex(i);
		i = _iniParser->value("FixMss", 1).toInt();
		_FixMss->setCurrentIndex(i);
		_iniParser->endGroup();

		// Parameters
		_iniParser->beginGroup("PARAMETERS");
		d = _iniParser->value("VerticalFOV", 67.5).toDouble();
		_VerticalFOV->setValue(d);
		i = _iniParser->value("NewFOVformula", 0).toInt();
		_NewFOVformula->setCurrentIndex(i);
		i = _iniParser->value("DisableLOD", 0).toInt();
		_DisableLOD->setCurrentIndex(i);
		i = _iniParser->value("DisableIndoorClipping", 1).toInt();
		_DisableIndoorClipping->setCurrentIndex(i);
		i = _iniParser->value("SPAWN_INSERTRANGE", 4500).toInt();
		_SPAWN_INSERTRANGE->setValue(i);
		i = _iniParser->value("SPAWN_REMOVERANGE", 5000).toInt();
		_SPAWN_REMOVERANGE->setValue(i);
		i = _iniParser->value("SPAWN_INSERTTIME_MAX", 1000).toInt();
		_SPAWN_INSERTTIME_MAX->setValue(i);
		i = _iniParser->value("DrawDistanceMultiplier", 1).toInt();
		_DrawDistanceMultiplier->setValue(i);
		d = _iniParser->value("OutDoorPortalDistanceMultiplier", 1).toDouble();
		_OutDoorPortalDistanceMultiplier->setValue(d);
		d = _iniParser->value("InDoorPortalDistanceMultiplier", 1).toDouble();
		_InDoorPortalDistanceMultiplier->setValue(d);
		d = _iniParser->value("WoodPortalDistanceMultiplier", 1).toDouble();
		_WoodPortalDistanceMultiplier->setValue(d);
		d = _iniParser->value("zMouseRotationScale", 0.1).toDouble();
		_zMouseRotationScale->setValue(d);
		i = _iniParser->value("EnableShields", 1).toInt();
		_EnableShields->setCurrentIndex(i);
		i = _iniParser->value("No_Take_Anim", 0).toInt();
		_No_Take_Anim->setCurrentIndex(i);
		i = _iniParser->value("RMB_No_Take_Anim", 0).toInt();
		_RMB_No_Take_Anim->setCurrentIndex(i);
		d = _iniParser->value("TRADE_VALUE_MULTIPLIER", 0.5).toDouble();
		_TRADE_VALUE_MULTIPLIER->setValue(d);
		i = _iniParser->value("Animated_Inventory", 1).toInt();
		_Animated_Inventory->setCurrentIndex(i);
		i = _iniParser->value("keyDelayRate", 150).toInt();
		_keyDelayRate->setValue(i);
		i = _iniParser->value("keyDelayFirst", 150).toInt();
		_keyDelayFirst->setValue(i);
		i = _iniParser->value("HighlightMeleeFocus", 1).toInt();
		_HighlightMeleeFocus->setCurrentIndex(i);
		i = _iniParser->value("HighlightInteractFocus", 1).toInt();
		_HighlightInteractFocus->setCurrentIndex(i);
		i = _iniParser->value("HighlightInteractNoFocus", 1).toInt();
		_HighlightInteractNoFocus->setCurrentIndex(i);
		i = _iniParser->value("Fight_ANI_Interrupt", 0).toInt();
		_Fight_ANI_Interrupt->setCurrentIndex(i);
		d = _iniParser->value("ReverbVolume", 3.0).toDouble();
		_ReverbVolume->setValue(d);
		_iniParser->endGroup();

		// SHW32
		_iniParser->beginGroup("SHW32");
		i = _iniParser->value("bShowGothicError", 1).toInt();
		_bShowGothicError->setCurrentIndex(i);
		i = _iniParser->value("bShowMsgBox", 0).toInt();
		_bShowMsgBox->setCurrentIndex(i);
		i = _iniParser->value("bUseNewHandler", 1).toInt();
		_bUseNewHandler->setCurrentIndex(i);
		i = _iniParser->value("reserveInMb", 50).toInt();
		_reserveInMb->setValue(i);
		_iniParser->endGroup();
	}

	void SystemPage::accept() {
		// Debug
		_iniParser->beginGroup("DEBUG");
		_iniParser->setValue("FixGameUX", _FixGameUX->currentIndex());
		_iniParser->setValue("Disable_D3DVBCAPS_WRITEONLY", _Disable_D3DVBCAPS_WRITEONLY->currentIndex());
		_iniParser->setValue("BorderlessWindow", _BorderlessWindow->currentIndex());
		_iniParser->setValue("ZNORESTHREAD", _ZNORESTHREAD->currentIndex());
		_iniParser->setValue("MoverBugfix", _MoverBugfix->currentIndex());
		_iniParser->setValue("NumLockDisable", _NumLockDisable->currentIndex());
		_iniParser->setValue("DisableCacheOut", _DisableCacheOut->currentIndex());
		_iniParser->setValue("QuickSaveEnable", _QuickSaveEnable->currentIndex());
		_iniParser->setValue("USInternationalKeyboardLayout", _USInternationalKeyboardLayout->currentIndex());
		_iniParser->setValue("Polish_version", _Polish_version->currentIndex());
		_iniParser->setValue("PFXfix", _PFXfix->currentIndex());
		_iniParser->setValue("BUGFIX_already_deleted_zCObject", _BUGFIX_already_deleted_zCObject->currentIndex());
		_iniParser->setValue("Disable_HUMANS_SWIM.MDS", _Disable_HUMANS_SWIMMDS->currentIndex());
		_iniParser->setValue("Game_InitEngIntl", _Game_InitEngIntl->currentIndex());
		_iniParser->setValue("FixHighRes", _FixHighRes->currentIndex());
		_iniParser->setValue("FixAppCompat", _FixAppCompat->currentIndex());
		_iniParser->setValue("FixBink", _FixBink->currentIndex());
		_iniParser->setValue("FixMss", _FixMss->currentIndex());
		_iniParser->endGroup();

		// Parameters
		_iniParser->beginGroup("PARAMETERS");
		_iniParser->setValue("VerticalFOV", _VerticalFOV->value());
		_iniParser->setValue("NewFOVformula", _NewFOVformula->currentIndex());
		_iniParser->setValue("DisableLOD", _DisableLOD->currentIndex());
		_iniParser->setValue("DisableIndoorClipping", _DisableIndoorClipping->currentIndex());
		_iniParser->setValue("SPAWN_INSERTRANGE", _SPAWN_INSERTRANGE->value());
		_iniParser->setValue("SPAWN_REMOVERANGE", _SPAWN_REMOVERANGE->value());
		_iniParser->setValue("SPAWN_INSERTTIME_MAX", _SPAWN_INSERTTIME_MAX->value());
		_iniParser->setValue("DrawDistanceMultiplier", _DrawDistanceMultiplier->value());
		_iniParser->setValue("OutDoorPortalDistanceMultiplier", _OutDoorPortalDistanceMultiplier->value());
		_iniParser->setValue("InDoorPortalDistanceMultiplier", _InDoorPortalDistanceMultiplier->value());
		_iniParser->setValue("WoodPortalDistanceMultiplier", _WoodPortalDistanceMultiplier->value());
		_iniParser->setValue("zMouseRotationScale", _zMouseRotationScale->value());
		_iniParser->setValue("EnableShields", _EnableShields->currentIndex());
		_iniParser->setValue("No_Take_Anim", _No_Take_Anim->currentIndex());
		_iniParser->setValue("RMB_No_Take_Anim", _RMB_No_Take_Anim->currentIndex());
		_iniParser->setValue("TRADE_VALUE_MULTIPLIER", _TRADE_VALUE_MULTIPLIER->value());
		_iniParser->setValue("Animated_Inventory", _Animated_Inventory->currentIndex());
		_iniParser->setValue("keyDelayRate", _keyDelayRate->value());
		_iniParser->setValue("keyDelayFirst", _keyDelayFirst->value());
		_iniParser->setValue("HighlightMeleeFocus", _HighlightMeleeFocus->currentIndex());
		_iniParser->setValue("HighlightInteractFocus", _HighlightInteractFocus->currentIndex());
		_iniParser->setValue("HighlightInteractNoFocus", _HighlightInteractNoFocus->currentIndex());
		_iniParser->setValue("Fight_ANI_Interrupt", _Fight_ANI_Interrupt->currentIndex());
		_iniParser->setValue("ReverbVolume", _ReverbVolume->value());
		_iniParser->endGroup();

		// Shw32
		_iniParser->beginGroup("SHW32");
		_iniParser->setValue("bShowGothicError", _bShowGothicError->currentIndex());
		_iniParser->setValue("bShowMsgBox", _bShowMsgBox->currentIndex());
		_iniParser->setValue("bUseNewHandler", _bUseNewHandler->currentIndex());
		_iniParser->setValue("reserveInMb", _reserveInMb->value());
		_iniParser->endGroup();
	}

} /* namespace sp */
} /* namespace widgets */
} /* namespace spine */
