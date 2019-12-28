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

#include "widgets/gothic2IniPages/GamePage.h"

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

	GamePage::GamePage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();

		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * subtitleBox = new QGroupBox(QApplication::tr("Subtitles"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("subTitles", subtitleBox);
				lbl->setToolTip(QApplication::tr("subTitlesTooltip"));
				_subtitleComboBox = new QComboBox(subtitleBox);
				_subtitleComboBox->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_subtitleComboBox, 0, 1);

				lbl = new QLabel("subTitlesAmbient", subtitleBox);
				lbl->setToolTip(QApplication::tr("subTitlesAmbientTooltip"));
				_subtitleAmbientComboBox = new QComboBox(subtitleBox);
				_subtitleAmbientComboBox->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_subtitleAmbientComboBox, 1, 1);

				lbl = new QLabel("subTitlesNoise", subtitleBox);
				lbl->setToolTip(QApplication::tr("subTitlesNoiseTooltip"));
				_subtitleNoiseComboBox = new QComboBox(subtitleBox);
				_subtitleNoiseComboBox->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_subtitleNoiseComboBox, 2, 1);

				lbl = new QLabel("subTitlesPlayer", subtitleBox);
				lbl->setToolTip(QApplication::tr("subTitlesPlayerTooltip"));
				_subtitlePlayerComboBox = new QComboBox(subtitleBox);
				_subtitlePlayerComboBox->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_subtitlePlayerComboBox, 3, 1);

				{
					QLabel * gameTextAutoScrollLabel = new QLabel("gametextAutoScroll", subtitleBox);
					gameTextAutoScrollLabel->setToolTip(QApplication::tr("gametextAutoScrollTooltip"));
					_gameTextAutoScrollSpinBox = new QSpinBox(subtitleBox);
					_gameTextAutoScrollSpinBox->setMinimum(0);
					_gameTextAutoScrollSpinBox->setMaximum(100000);
					_gameTextAutoScrollSpinBox->setSuffix(" " + QApplication::tr("Milliseconds"));
					gl->addWidget(gameTextAutoScrollLabel, 4, 0);
					gl->addWidget(_gameTextAutoScrollSpinBox, 4, 1);
				}

				subtitleBox->setLayout(gl);

				hl->addWidget(subtitleBox);
			}

			{
				QGroupBox * cameraAndFocusBox = new QGroupBox(QApplication::tr("CameraAndFocus"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("camLookaroundInverse", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("camLookaroundInverseTooltip"));
				_camLookaroundInverse = new QComboBox(cameraAndFocusBox);
				_camLookaroundInverse->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_camLookaroundInverse, 0, 1);

				lbl = new QLabel("cameraLightRange", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("cameraLightRangeTooltip"));
				_cameraLightRange = new QSpinBox(cameraAndFocusBox);
				_cameraLightRange->setMinimum(0);
				_cameraLightRange->setMaximum(100000);
				_cameraLightRange->setSuffix(" " + QApplication::tr("cm"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_cameraLightRange, 1, 1);

				lbl = new QLabel("zDontSwitchToThirdPerson", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("zDontSwitchToThirdPersonTooltip"));
				_zDontSwitchToThirdPerson = new QComboBox(cameraAndFocusBox);
				_zDontSwitchToThirdPerson->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_zDontSwitchToThirdPerson, 2, 1);

				lbl = new QLabel("highlightMeleeFocus", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("highlightMeleeFocusTooltip"));
				_highlightMeleeFocus = new QComboBox(cameraAndFocusBox);
				_highlightMeleeFocus->addItems(QStringList() << QApplication::tr("None") << QApplication::tr("Quadrat") << QApplication::tr("Brighten") << QApplication::tr("Both"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_highlightMeleeFocus, 3, 1);

				lbl = new QLabel("highlightInteractFocus", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("highlightInteractFocusTooltip"));
				_highlightInteractFocus = new QComboBox(cameraAndFocusBox);
				_highlightInteractFocus->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_highlightInteractFocus, 4, 1);

				cameraAndFocusBox->setLayout(gl);

				hl->addWidget(cameraAndFocusBox);
			}

			{
				QGroupBox * controlsAndHotkeysBox = new QGroupBox(QApplication::tr("ControlsAndHotkeys"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("usePotionKeys", controlsAndHotkeysBox);
				lbl->setToolTip(QApplication::tr("usePotionKeysTooltip"));
				_usePotionKeys = new QComboBox(controlsAndHotkeysBox);
				_usePotionKeys->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_usePotionKeys, 0, 1);

				lbl = new QLabel("useQuickSaveKeys", controlsAndHotkeysBox);
				lbl->setToolTip(QApplication::tr("useQuickSaveKeysTooltip"));
				_useQuickSaveKeys = new QComboBox(controlsAndHotkeysBox);
				_useQuickSaveKeys->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_useQuickSaveKeys, 1, 1);

				lbl = new QLabel("useGothic1Controls", controlsAndHotkeysBox);
				lbl->setToolTip(QApplication::tr("useGothic1ControlsTooltip"));
				_useGothic1Controls = new QComboBox(controlsAndHotkeysBox);
				_useGothic1Controls->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_useGothic1Controls, 2, 1);

				lbl = new QLabel("pickLockScramble", controlsAndHotkeysBox);
				lbl->setToolTip(QApplication::tr("pickLockScrambleTooltip"));
				_pickLockScramble = new QComboBox(controlsAndHotkeysBox);
				_pickLockScramble->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_pickLockScramble, 3, 1);

				controlsAndHotkeysBox->setLayout(gl);

				hl->addWidget(controlsAndHotkeysBox);
			}

			l->addLayout(hl);
		}

		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * inventoryBox = new QGroupBox(QApplication::tr("Inventory"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("invShowArrows", inventoryBox);
				lbl->setToolTip(QApplication::tr("invShowArrowsTooltip"));
				_invShowArrows = new QComboBox(inventoryBox);
				_invShowArrows->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_invShowArrows, 0, 1);

				lbl = new QLabel("invSwitchToFirstCategory", inventoryBox);
				lbl->setToolTip(QApplication::tr("invSwitchToFirstCategoryTooltip"));
				_invSwitchToFirstCategory = new QComboBox(inventoryBox);
				_invSwitchToFirstCategory->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_invSwitchToFirstCategory, 1, 1);

				lbl = new QLabel("zInventoryItemDistanceScale", inventoryBox);
				lbl->setToolTip(QApplication::tr("zInventoryItemDistanceScaleTooltip"));
				_zInventoryItemDistanceScale = new QDoubleSpinBox(inventoryBox);
				_zInventoryItemDistanceScale->setMinimum(0.0);
				_zInventoryItemDistanceScale->setMaximum(10.0);
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_zInventoryItemDistanceScale, 2, 1);

				lbl = new QLabel("invMaxColumns", inventoryBox);
				lbl->setToolTip(QApplication::tr("invMaxColumnsTooltip"));
				_invMaxColumns = new QSpinBox(inventoryBox);
				_invMaxColumns->setMinimum(0);
				_invMaxColumns->setMaximum(100);
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_invMaxColumns, 3, 1);

				lbl = new QLabel("invMaxRows", inventoryBox);
				lbl->setToolTip(QApplication::tr("invMaxRowsTooltip"));
				_invMaxRows = new QSpinBox(inventoryBox);
				_invMaxRows->setMinimum(0);
				_invMaxRows->setMaximum(100);
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_invMaxRows, 4, 1);

				lbl = new QLabel("invSplitScreen", inventoryBox);
				lbl->setToolTip(QApplication::tr("invSplitScreenTooltip"));
				_invSplitScreen = new QComboBox(inventoryBox);
				_invSplitScreen->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 5, 0);
				gl->addWidget(_invSplitScreen, 5, 1);

				inventoryBox->setLayout(gl);

				hl->addWidget(inventoryBox);
			}

			{
				QGroupBox * shortKeysBox = new QGroupBox(QApplication::tr("ShortKeys"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("SHORTKEY1FARPLANEDIST", shortKeysBox);
				lbl->setToolTip(QApplication::tr("SHORTKEY1FARPLANEDISTTooltip"));
				_SHORTKEY1FARPLANEDIST = new QDoubleSpinBox(shortKeysBox);
				_SHORTKEY1FARPLANEDIST->setMinimum(0.0);
				_SHORTKEY1FARPLANEDIST->setMaximum(10.0);
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_SHORTKEY1FARPLANEDIST, 0, 1);

				lbl = new QLabel("SHORTKEY2FARPLANEDIST", shortKeysBox);
				lbl->setToolTip(QApplication::tr("SHORTKEY2FARPLANEDISTTooltip"));
				_SHORTKEY2FARPLANEDIST = new QDoubleSpinBox(shortKeysBox);
				_SHORTKEY2FARPLANEDIST->setMinimum(0.0);
				_SHORTKEY2FARPLANEDIST->setMaximum(10.0);
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_SHORTKEY2FARPLANEDIST, 1, 1);

				lbl = new QLabel("SHORTKEY3FARPLANEDIST", shortKeysBox);
				lbl->setToolTip(QApplication::tr("SHORTKEY3FARPLANEDISTTooltip"));
				_SHORTKEY3FARPLANEDIST = new QDoubleSpinBox(shortKeysBox);
				_SHORTKEY3FARPLANEDIST->setMinimum(0.0);
				_SHORTKEY3FARPLANEDIST->setMaximum(10.0);
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_SHORTKEY3FARPLANEDIST, 2, 1);

				lbl = new QLabel("SHORTKEY4FARPLANEDIST", shortKeysBox);
				lbl->setToolTip(QApplication::tr("SHORTKEY4FARPLANEDISTTooltip"));
				_SHORTKEY4FARPLANEDIST = new QDoubleSpinBox(shortKeysBox);
				_SHORTKEY4FARPLANEDIST->setMinimum(0.0);
				_SHORTKEY4FARPLANEDIST->setMaximum(10.0);
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_SHORTKEY4FARPLANEDIST, 3, 1);

				shortKeysBox->setLayout(gl);

				hl->addWidget(shortKeysBox);
			}

			l->addLayout(hl);
		}

		setLayout(l);

		reject();
	}

	GamePage::~GamePage() {
	}

	void GamePage::reject() {
		// Subtitles
		_iniParser->beginGroup("GAME");
		int idx;
		idx = _iniParser->value("subTitles", 0).toInt();
		_subtitleComboBox->setCurrentIndex(idx);
		idx = _iniParser->value("subTitlesAmbient", 0).toInt();
		_subtitleAmbientComboBox->setCurrentIndex(idx);
		idx = _iniParser->value("subTitlesPlayer", 0).toInt();
		_subtitlePlayerComboBox->setCurrentIndex(idx);
		idx = _iniParser->value("subTitlesNoise", 0).toInt();
		_subtitleNoiseComboBox->setCurrentIndex(idx);
		int value;
		value = _iniParser->value("gametextAutoScroll", 1000).toInt();
		_gameTextAutoScrollSpinBox->setValue(value);

		// Camera and Focus
		idx = _iniParser->value("camLookaroundInverse", 0).toInt();
		_camLookaroundInverse->setCurrentIndex(idx);
		value = _iniParser->value("cameraLightRange", 0).toInt();
		_cameraLightRange->setValue(value);
		idx = _iniParser->value("zDontSwitchToThirdPerson", 0).toInt();
		_zDontSwitchToThirdPerson->setCurrentIndex(idx);
		idx = _iniParser->value("highlightMeleeFocus", 0).toInt();
		_highlightMeleeFocus->setCurrentIndex(idx);
		idx = _iniParser->value("highlightInteractFocus", 0).toInt();
		_highlightInteractFocus->setCurrentIndex(idx);

		// Controls and Hotkeys
		idx = _iniParser->value("usePotionKeys", 0).toInt();
		_usePotionKeys->setCurrentIndex(idx);
		idx = _iniParser->value("useQuickSaveKeys", 0).toInt();
		_useQuickSaveKeys->setCurrentIndex(idx);
		idx = _iniParser->value("useGothic1Controls", 0).toInt();
		_useGothic1Controls->setCurrentIndex(idx);
		idx = _iniParser->value("pickLockScramble", 0).toInt();
		_pickLockScramble->setCurrentIndex(idx);

		// Inventory
		idx = _iniParser->value("invShowArrows", 0).toInt();
		_invShowArrows->setCurrentIndex(idx);
		idx = _iniParser->value("invSwitchToFirstCategory", 0).toInt();
		_invSwitchToFirstCategory->setCurrentIndex(idx);
		double d;
		d = _iniParser->value("zInventoryItemDistanceScale", 1.3).toDouble();
		_zInventoryItemDistanceScale->setValue(d);
		value = _iniParser->value("invMaxColumns", 5).toInt();
		_invMaxColumns->setValue(value);
		value = _iniParser->value("invMaxRows", 0).toInt();
		_invMaxRows->setValue(value);
		idx = _iniParser->value("invSplitScreen", 0).toInt();
		_invSplitScreen->setCurrentIndex(idx);

		// Short Keys
		d = _iniParser->value("SHORTKEY1FARPLANEDIST", 0.8).toDouble();
		_SHORTKEY1FARPLANEDIST->setValue(d);
		d = _iniParser->value("SHORTKEY2FARPLANEDIST", 1.2).toDouble();
		_SHORTKEY2FARPLANEDIST->setValue(d);
		d = _iniParser->value("SHORTKEY3FARPLANEDIST", 2.0).toDouble();
		_SHORTKEY3FARPLANEDIST->setValue(d);
		d = _iniParser->value("SHORTKEY4FARPLANEDIST", 3.00).toDouble();
		_SHORTKEY4FARPLANEDIST->setValue(d);
		_iniParser->endGroup();
	}

	void GamePage::accept() {
		_iniParser->beginGroup("GAME");
		// Subtitles
		_iniParser->setValue("subTitles", _subtitleComboBox->currentIndex());
		_iniParser->setValue("subTitlesAmbient", _subtitleAmbientComboBox->currentIndex());
		_iniParser->setValue("subTitlesPlayer", _subtitlePlayerComboBox->currentIndex());
		_iniParser->setValue("subTitlesNoise", _subtitleNoiseComboBox->currentIndex());
		_iniParser->setValue("gametextAutoScroll", _gameTextAutoScrollSpinBox->value());

		// Camera and Focus
		_iniParser->setValue("camLookaroundInverse", _camLookaroundInverse->currentIndex());
		_iniParser->setValue("cameraLightRange", _cameraLightRange->value());
		_iniParser->setValue("zDontSwitchToThirdPerson", _zDontSwitchToThirdPerson->currentIndex());
		_iniParser->setValue("highlightMeleeFocus", _highlightMeleeFocus->currentIndex());
		_iniParser->setValue("highlightInteractFocus", _highlightInteractFocus->currentIndex());

		// Controls and Hotkeys
		_iniParser->setValue("usePotionKeys", _usePotionKeys->currentIndex());
		_iniParser->setValue("useQuickSaveKeys", _useQuickSaveKeys->currentIndex());
		_iniParser->setValue("useGothic1Controls", _useGothic1Controls->currentIndex());
		_iniParser->setValue("pickLockScramble", _pickLockScramble->currentIndex());

		// Inventory
		_iniParser->setValue("invShowArrows", _invShowArrows->currentIndex());
		_iniParser->setValue("invSwitchToFirstCategory", _invSwitchToFirstCategory->currentIndex());
		_iniParser->setValue("zInventoryItemDistanceScale", _zInventoryItemDistanceScale->value());
		_iniParser->setValue("invMaxColumns", _invMaxColumns->value());
		_iniParser->setValue("invMaxRows", _invMaxRows->value());
		_iniParser->setValue("invSplitScreen", _invSplitScreen->currentIndex());

		// Short Keys
		_iniParser->setValue("SHORTKEY1FARPLANEDIST", _SHORTKEY1FARPLANEDIST->value());
		_iniParser->setValue("SHORTKEY2FARPLANEDIST", _SHORTKEY2FARPLANEDIST->value());
		_iniParser->setValue("SHORTKEY3FARPLANEDIST", _SHORTKEY3FARPLANEDIST->value());
		_iniParser->setValue("SHORTKEY4FARPLANEDIST", _SHORTKEY4FARPLANEDIST->value());
		_iniParser->endGroup();
	}

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */
