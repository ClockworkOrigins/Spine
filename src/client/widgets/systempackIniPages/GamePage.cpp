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

#include "widgets/systempackIniPages/GamePage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::widgets::sp;

GamePage::GamePage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
	QVBoxLayout * l = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * groupBox = new QGroupBox(QApplication::tr("Barrier"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("AlwaysON", groupBox);
			lbl->setToolTip(QApplication::tr("AlwaysONTooltip"));
			_AlwaysON = new QComboBox(groupBox);
			_AlwaysON->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_AlwaysON, 0, 1);

			lbl = new QLabel("AlwaysOFF", groupBox);
			lbl->setToolTip(QApplication::tr("AlwaysOFFTooltip"));
			_AlwaysOFF = new QComboBox(groupBox);
			_AlwaysOFF->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_AlwaysOFF, 1, 1);

			lbl = new QLabel("DisableSound", groupBox);
			lbl->setToolTip(QApplication::tr("DisableSoundTooltip"));
			_DisableSound = new QComboBox(groupBox);
			_DisableSound->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_DisableSound, 2, 1);

			lbl = new QLabel("DisableDamage", groupBox);
			lbl->setToolTip(QApplication::tr("DisableDamageTooltip"));
			_DisableDamage = new QComboBox(groupBox);
			_DisableDamage->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_DisableDamage, 3, 1);

			groupBox->setLayout(gl);

			hl->addWidget(groupBox);
		}

		{
			QGroupBox * groupBox = new QGroupBox(QApplication::tr("Subtitles"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("Control", groupBox);
			lbl->setToolTip(QApplication::tr("ControlTooltip"));
			_Control = new QComboBox(groupBox);
			_Control->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_Control, 0, 1);

			lbl = new QLabel("TimeMultiplier", groupBox);
			lbl->setToolTip(QApplication::tr("TimeMultiplierTooltip"));
			_TimeMultiplier = new QDoubleSpinBox(groupBox);
			_TimeMultiplier->setMinimum(0.1);
			_TimeMultiplier->setMaximum(100.0);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_TimeMultiplier, 1, 1);

			lbl = new QLabel("MaxTimePerPhrase", groupBox);
			lbl->setToolTip(QApplication::tr("MaxTimePerPhraseTooltip"));
			_MaxTimePerPhrase = new QDoubleSpinBox(groupBox);
			_MaxTimePerPhrase->setMinimum(0);
			_MaxTimePerPhrase->setMaximum(1000.0);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_MaxTimePerPhrase, 2, 1);

			lbl = new QLabel("TimePerChar", groupBox);
			lbl->setToolTip(QApplication::tr("TimePerCharTooltip"));
			_TimePerChar = new QDoubleSpinBox(groupBox);
			_TimePerChar->setMinimum(1.00);
			_TimePerChar->setMaximum(1000.0);
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_TimePerChar, 3, 1);

			groupBox->setLayout(gl);

			hl->addWidget(groupBox);
		}

		{
			QGroupBox * groupBox = new QGroupBox(QApplication::tr("Returning2"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("Enable", groupBox);
			lbl->setToolTip(QApplication::tr("Returning2EnableTooltip"));
			_Enable = new QComboBox(groupBox);
			_Enable->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_Enable, 0, 1);

			groupBox->setLayout(gl);

			hl->addWidget(groupBox);
		}

		l->addLayout(hl);
	}
	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * groupBox = new QGroupBox(QApplication::tr("Interface"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("Scale", groupBox);
			lbl->setToolTip(QApplication::tr("ScaleTooltip"));
			_Scale = new QComboBox(groupBox);
			_Scale->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_Scale, 0, 1);

			lbl = new QLabel("ForceMenuScale", groupBox);
			lbl->setToolTip(QApplication::tr("ForceMenuScaleTooltip"));
			_ForceMenuScale = new QComboBox(groupBox);
			_ForceMenuScale->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_ForceMenuScale, 1, 1);

			lbl = new QLabel("ScaleMenusX", groupBox);
			lbl->setToolTip(QApplication::tr("ScaleMenusXTooltip"));
			_ScaleMenusX = new QSpinBox(groupBox);
			_ScaleMenusX->setMinimum(0);
			_ScaleMenusX->setMaximum(10000);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_ScaleMenusX, 2, 1);

			lbl = new QLabel("ScaleMenusY", groupBox);
			lbl->setToolTip(QApplication::tr("ScaleMenusYTooltip"));
			_ScaleMenusY = new QSpinBox(groupBox);
			_ScaleMenusY->setMinimum(0);
			_ScaleMenusY->setMaximum(10000);
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_ScaleMenusY, 3, 1);

			lbl = new QLabel("DialogBoxX", groupBox);
			lbl->setToolTip(QApplication::tr("DialogBoxXTooltip"));
			_DialogBoxX = new QSpinBox(groupBox);
			_DialogBoxX->setMinimum(0);
			_DialogBoxX->setMaximum(10000);
			gl->addWidget(lbl, 4, 0);
			gl->addWidget(_DialogBoxX, 4, 1);

			lbl = new QLabel("DialogBoxY", groupBox);
			lbl->setToolTip(QApplication::tr("DialogBoxYTooltip"));
			_DialogBoxY = new QSpinBox(groupBox);
			_DialogBoxY->setMinimum(0);
			_DialogBoxY->setMaximum(10000);
			gl->addWidget(lbl, 5, 0);
			gl->addWidget(_DialogBoxY, 5, 1);

			lbl = new QLabel("SubtitlesBoxX", groupBox);
			lbl->setToolTip(QApplication::tr("SubtitlesBoxXTooltip"));
			_SubtitlesBoxX = new QSpinBox(groupBox);
			_SubtitlesBoxX->setMinimum(0);
			_SubtitlesBoxX->setMaximum(10000);
			gl->addWidget(lbl, 0, 2);
			gl->addWidget(_SubtitlesBoxX, 0, 3);

			lbl = new QLabel("ShowManaBar", groupBox);
			lbl->setToolTip(QApplication::tr("ShowManaBarTooltip"));
			_ShowManaBar = new QComboBox(groupBox);
			_ShowManaBar->addItems(QStringList() << QApplication::tr("AlwaysInvisible") << QApplication::tr("VisibleWithSpell") << QApplication::tr("AlwaysVisible"));
			gl->addWidget(lbl, 1, 2);
			gl->addWidget(_ShowManaBar, 1, 3);

			lbl = new QLabel("ShowSwimBar", groupBox);
			lbl->setToolTip(QApplication::tr("ShowSwimBarTooltip"));
			_ShowSwimBar = new QComboBox(groupBox);
			_ShowSwimBar->addItems(QStringList() << QApplication::tr("AlwaysInvisible") << QApplication::tr("VisibleUnderwater") << QApplication::tr("AlwaysVisible"));
			gl->addWidget(lbl, 2, 2);
			gl->addWidget(_ShowSwimBar, 2, 3);

			lbl = new QLabel("HideHealthBar", groupBox);
			lbl->setToolTip(QApplication::tr("HideHealthBarTooltip"));
			_HideHealthBar = new QComboBox(groupBox);
			_HideHealthBar->addItems(QStringList() << QApplication::tr("AlwaysVisible") << QApplication::tr("AlwaysInvisible"));
			gl->addWidget(lbl, 3, 2);
			gl->addWidget(_HideHealthBar, 3, 3);

			lbl = new QLabel("NewChapterSizeX", groupBox);
			lbl->setToolTip(QApplication::tr("NewChapterSizeXTooltip"));
			_NewChapterSizeX = new QSpinBox(groupBox);
			_NewChapterSizeX->setMinimum(0);
			_NewChapterSizeX->setMaximum(10000);
			gl->addWidget(lbl, 4, 2);
			gl->addWidget(_NewChapterSizeX, 4, 3);

			lbl = new QLabel("NewChapterSizeY", groupBox);
			lbl->setToolTip(QApplication::tr("NewChapterSizeYTooltip"));
			_NewChapterSizeY = new QSpinBox(groupBox);
			_NewChapterSizeY->setMinimum(0);
			_NewChapterSizeY->setMaximum(10000);
			gl->addWidget(lbl, 5, 2);
			gl->addWidget(_NewChapterSizeY, 5, 3);

			lbl = new QLabel("SaveGameImageSizeX", groupBox);
			lbl->setToolTip(QApplication::tr("SaveGameImageSizeXTooltip"));
			_SaveGameImageSizeX = new QSpinBox(groupBox);
			_SaveGameImageSizeX->setMinimum(0);
			_SaveGameImageSizeX->setMaximum(10000);
			gl->addWidget(lbl, 0, 4);
			gl->addWidget(_SaveGameImageSizeX, 0, 5);

			lbl = new QLabel("SaveGameImageSizeY", groupBox);
			lbl->setToolTip(QApplication::tr("SaveGameImageSizeYTooltip"));
			_SaveGameImageSizeY = new QSpinBox(groupBox);
			_SaveGameImageSizeY->setMinimum(0);
			_SaveGameImageSizeY->setMaximum(10000);
			gl->addWidget(lbl, 1, 4);
			gl->addWidget(_SaveGameImageSizeY, 1, 5);

			lbl = new QLabel("InventoryItemNoteSizeX", groupBox);
			lbl->setToolTip(QApplication::tr("InventoryItemNoteSizeXTooltip"));
			_InventoryItemNoteSizeX = new QSpinBox(groupBox);
			_InventoryItemNoteSizeX->setMinimum(0);
			_InventoryItemNoteSizeX->setMaximum(10000);
			gl->addWidget(lbl, 2, 4);
			gl->addWidget(_InventoryItemNoteSizeX, 2, 5);

			lbl = new QLabel("InventoryCellSize", groupBox);
			lbl->setToolTip(QApplication::tr("InventoryCellSizeTooltip"));
			_InventoryCellSize = new QSpinBox(groupBox);
			_InventoryCellSize->setMinimum(0);
			_InventoryCellSize->setMaximum(10000);
			gl->addWidget(lbl, 3, 4);
			gl->addWidget(_InventoryCellSize, 3, 5);

			groupBox->setLayout(gl);

			hl->addWidget(groupBox);
		}

		l->addLayout(hl);
	}

	setLayout(l);

	reject();
}

GamePage::~GamePage() {
}

void GamePage::reject() {
	// Barrier
	int i;
	double d;
	_iniParser->beginGroup("BARRIER");
	i = _iniParser->value("AlwaysON", 0).toInt();
	_AlwaysON->setCurrentIndex(i);
	i = _iniParser->value("AlwaysOFF", 0).toInt();
	_AlwaysOFF->setCurrentIndex(i);
	i = _iniParser->value("DisableSound", 0).toInt();
	_DisableSound->setCurrentIndex(i);
	i = _iniParser->value("DisableDamage", 0).toInt();
	_DisableDamage->setCurrentIndex(i);
	_iniParser->endGroup();

	// Subtitles
	_iniParser->beginGroup("SUBTITLES");
	i = _iniParser->value("Control", 0).toInt();
	_Control->setCurrentIndex(i);
	d = _iniParser->value("TimeMultiplier", 2.0).toDouble();
	_TimeMultiplier->setValue(d);
	d = _iniParser->value("MaxTimePerPhrase", 30.0).toDouble();
	_MaxTimePerPhrase->setValue(d);
	d = _iniParser->value("TimePerChar", 100.0).toDouble();
	_TimePerChar->setValue(d);
	_iniParser->endGroup();

	// Returning 2
	_iniParser->beginGroup("RETURNING2");
	i = _iniParser->value("Enable", 0).toInt();
	_Enable->setCurrentIndex(i);
	_iniParser->endGroup();

	// Interface
	_iniParser->beginGroup("INTERFACE");
	i = _iniParser->value("Scale", 1).toInt();
	_Scale->setCurrentIndex(i);
	i = _iniParser->value("ForceMenuScale", 0).toInt();
	_ForceMenuScale->setCurrentIndex(i);
	i = _iniParser->value("ScaleMenusX", 640).toInt();
	_ScaleMenusX->setValue(i);
	i = _iniParser->value("ScaleMenusY", 480).toInt();
	_ScaleMenusY->setValue(i);
	i = _iniParser->value("DialogBoxX", 800).toInt();
	_DialogBoxX->setValue(i);
	i = _iniParser->value("DialogBoxY", 600).toInt();
	_DialogBoxY->setValue(i);
	i = _iniParser->value("SubtitlesBoxX", 600).toInt();
	_SubtitlesBoxX->setValue(i);
	i = _iniParser->value("ShowManaBar", 1).toInt();
	_ShowManaBar->setCurrentIndex(i);
	i = _iniParser->value("ShowSwimBar", 1).toInt();
	_ShowSwimBar->setCurrentIndex(i);
	i = _iniParser->value("HideHealthBar", 0).toInt();
	_HideHealthBar->setCurrentIndex(i);
	i = _iniParser->value("NewChapterSizeX", 640).toInt();
	_NewChapterSizeX->setValue(i);
	i = _iniParser->value("NewChapterSizeY", 480).toInt();
	_NewChapterSizeY->setValue(i);
	i = _iniParser->value("SaveGameImageSizeX", 320).toInt();
	_SaveGameImageSizeX->setValue(i);
	i = _iniParser->value("SaveGameImageSizeY", 200).toInt();
	_SaveGameImageSizeY->setValue(i);
	i = _iniParser->value("InventoryItemNoteSizeX", 450).toInt();
	_InventoryItemNoteSizeX->setValue(i);
	i = _iniParser->value("InventoryCellSize", 70).toInt();
	_InventoryCellSize->setValue(i);
	_iniParser->endGroup();
}

void GamePage::accept() {
	// Barrier
	_iniParser->beginGroup("BARRIER");
	_iniParser->setValue("AlwaysON", _AlwaysON->currentIndex());
	_iniParser->setValue("AlwaysOFF", _AlwaysOFF->currentIndex());
	_iniParser->setValue("DisableSound", _DisableSound->currentIndex());
	_iniParser->setValue("DisableDamage", _DisableDamage->currentIndex());
	_iniParser->endGroup();

	// Subtitles
	_iniParser->beginGroup("SUBTITLES");
	_iniParser->setValue("Control", _Control->currentIndex());
	_iniParser->setValue("TimeMultiplier", _TimeMultiplier->value());
	_iniParser->setValue("MaxTimePerPhrase", _MaxTimePerPhrase->value());
	_iniParser->setValue("TimePerChar", _TimePerChar->value());
	_iniParser->endGroup();

	// Returning 2
	_iniParser->beginGroup("RETURNING2");
	_iniParser->setValue("Enable", _Enable->currentIndex());
	_iniParser->endGroup();

	// Interface
	_iniParser->beginGroup("INTERFACE");
	_iniParser->setValue("Scale", _Scale->currentIndex());
	_iniParser->setValue("ForceMenuScale", _ForceMenuScale->currentIndex());
	_iniParser->setValue("ScaleMenusX", _ScaleMenusX->value());
	_iniParser->setValue("ScaleMenusY", _ScaleMenusY->value());
	_iniParser->setValue("DialogBoxX", _DialogBoxX->value());
	_iniParser->setValue("DialogBoxY", _DialogBoxY->value());
	_iniParser->setValue("SubtitlesBoxX", _SubtitlesBoxX->value());
	_iniParser->setValue("ShowManaBar", _ShowManaBar->currentIndex());
	_iniParser->setValue("ShowSwimBar", _ShowSwimBar->currentIndex());
	_iniParser->setValue("HideHealthBar", _HideHealthBar->currentIndex());
	_iniParser->setValue("NewChapterSizeX", _NewChapterSizeX->value());
	_iniParser->setValue("NewChapterSizeY", _NewChapterSizeY->value());
	_iniParser->setValue("SaveGameImageSizeX", _SaveGameImageSizeX->value());
	_iniParser->setValue("SaveGameImageSizeY", _SaveGameImageSizeY->value());
	_iniParser->setValue("InventoryItemNoteSizeX", _InventoryItemNoteSizeX->value());
	_iniParser->setValue("InventoryCellSize", _InventoryCellSize->value());
	_iniParser->endGroup();
}

void GamePage::updateSettings(QSettings * iniParser) {
	_iniParser = iniParser;

	reject();
}
