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

#include "widgets/gothic2IniPages/VisualizationPage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::widgets::g2;

VisualizationPage::VisualizationPage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
	QVBoxLayout * l = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * skyOutdoorBox = new QGroupBox(QApplication::tr("SkyOutdoor"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zSkyDome", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zSkyDomeTooltip"));
			_zSkyDome = new QComboBox(skyOutdoorBox);
			_zSkyDome->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zSkyDome, 0, 1);

			lbl = new QLabel("zColorizeSky", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zColorizeSkyTooltip"));
			_zColorizeSky = new QComboBox(skyOutdoorBox);
			_zColorizeSky->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zColorizeSky, 1, 1);

			lbl = new QLabel("zDayColor0", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor0Tooltip"));
			_zDayColor0 = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zDayColor0, 2, 1);

			lbl = new QLabel("zDayColor1", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor1Tooltip"));
			_zDayColor1 = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_zDayColor1, 3, 1);

			lbl = new QLabel("zDayColor2", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor2Tooltip"));
			_zDayColor2 = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 4, 0);
			gl->addWidget(_zDayColor2, 4, 1);

			lbl = new QLabel("zDayColor3", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor3Tooltip"));
			_zDayColor3 = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 5, 0);
			gl->addWidget(_zDayColor3, 5, 1);

			lbl = new QLabel("zDayColor0_OW", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor0_OWTooltip"));
			_zDayColor0_OW = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 6, 0);
			gl->addWidget(_zDayColor0_OW, 6, 1);

			lbl = new QLabel("zDayColor1_OW", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor1_OWTooltip"));
			_zDayColor1_OW = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 7, 0);
			gl->addWidget(_zDayColor1_OW, 7, 1);

			lbl = new QLabel("zDayColor2_OW", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor2_OWTooltip"));
			_zDayColor2_OW = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 8, 0);
			gl->addWidget(_zDayColor2_OW, 8, 1);

			lbl = new QLabel("zDayColor3_OW", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zDayColor3_OWTooltip"));
			_zDayColor3_OW = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 9, 0);
			gl->addWidget(_zDayColor3_OW, 9, 1);

			lbl = new QLabel("zSunName", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zSunNameTooltip"));
			_zSunName = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 0, 2);
			gl->addWidget(_zSunName, 0, 3);

			lbl = new QLabel("zSunSize", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zSunSizeTooltip"));
			_zSunSize = new QSpinBox(skyOutdoorBox);
			_zSunSize->setMinimum(0);
			_zSunSize->setMaximum(1000);
			gl->addWidget(lbl, 1, 2);
			gl->addWidget(_zSunSize, 1, 3);

			lbl = new QLabel("zSunAlpha", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zSunAlphaTooltip"));
			_zSunAlpha = new QSpinBox(skyOutdoorBox);
			_zSunAlpha->setMinimum(0);
			_zSunAlpha->setMaximum(255);
			gl->addWidget(lbl, 2, 2);
			gl->addWidget(_zSunAlpha, 2, 3);

			lbl = new QLabel("zSunMaxScreenBlendScale", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zSunMaxScreenBlendScaleTooltip"));
			_zSunMaxScreenBlendScale = new QDoubleSpinBox(skyOutdoorBox);
			_zSunMaxScreenBlendScale->setMinimum(0.0);
			_zSunMaxScreenBlendScale->setMaximum(1.0);
			_zSunMaxScreenBlendScale->setSingleStep(0.01);
			gl->addWidget(lbl, 3, 2);
			gl->addWidget(_zSunMaxScreenBlendScale, 3, 3);

			lbl = new QLabel("zMoonName", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zMoonNameTooltip"));
			_zMoonName = new QLineEdit(skyOutdoorBox);
			gl->addWidget(lbl, 4, 2);
			gl->addWidget(_zMoonName, 4, 3);

			lbl = new QLabel("zMoonSize", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zMoonSizeTooltip"));
			_zMoonSize = new QSpinBox(skyOutdoorBox);
			_zMoonSize->setMinimum(0);
			_zMoonSize->setMaximum(1000);
			gl->addWidget(lbl, 5, 2);
			gl->addWidget(_zMoonSize, 5, 3);

			lbl = new QLabel("zMoonAlpha", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zMoonAlphaTooltip"));
			_zMoonAlpha = new QSpinBox(skyOutdoorBox);
			_zMoonAlpha->setMinimum(0);
			_zMoonAlpha->setMaximum(255);
			gl->addWidget(lbl, 6, 2);
			gl->addWidget(_zMoonAlpha, 6, 3);

			lbl = new QLabel("zRainWindScale", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zRainWindScaleTooltip"));
			_zRainWindScale = new QDoubleSpinBox(skyOutdoorBox);
			_zRainWindScale->setMinimum(0.0);
			_zRainWindScale->setMaximum(1.0);
			_zRainWindScale->setSingleStep(0.001);
			gl->addWidget(lbl, 7, 2);
			gl->addWidget(_zRainWindScale, 7, 3);

			lbl = new QLabel("zNearFogScale", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zNearFogScaleTooltip"));
			_zNearFogScale = new QDoubleSpinBox(skyOutdoorBox);
			_zNearFogScale->setMinimum(0.0);
			_zNearFogScale->setMaximum(1.0);
			_zNearFogScale->setSingleStep(0.01);
			gl->addWidget(lbl, 8, 2);
			gl->addWidget(_zNearFogScale, 8, 3);

			lbl = new QLabel("zFarFogScale", skyOutdoorBox);
			lbl->setToolTip(QApplication::tr("zFarFogScaleTooltip"));
			_zFarFogScale = new QDoubleSpinBox(skyOutdoorBox);
			_zFarFogScale->setMinimum(0.0);
			_zFarFogScale->setMaximum(1.0);
			_zFarFogScale->setSingleStep(0.01);
			gl->addWidget(lbl, 9, 2);
			gl->addWidget(_zFarFogScale, 9, 3);

			skyOutdoorBox->setLayout(gl);

			hl->addWidget(skyOutdoorBox);
		}

		l->addLayout(hl);
	}

	setLayout(l);

	reject();
}

VisualizationPage::~VisualizationPage() {
}

void VisualizationPage::reject() {
	// Sky Outdoor
	int idx;
	int value;
	double d;
	QString text;
	idx = _iniParser->value("SKY_OUTDOOR/zSkyDome", 1).toInt();
	_zSkyDome->setCurrentIndex(idx);
	idx = _iniParser->value("SKY_OUTDOOR/zColorizeSky", 0).toInt();
	_zColorizeSky->setCurrentIndex(idx);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor0", "230 115 0").toString();
	_zDayColor0->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor1", "255 255 0").toString();
	_zDayColor1->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor2", "64 0 0").toString();
	_zDayColor2->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor3", "230 115 0").toString();
	_zDayColor3->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor0_OW", "90 80 80").toString();
	_zDayColor0_OW->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor1_OW", "90 80 80").toString();
	_zDayColor1_OW->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor2_OW", "90 80 80").toString();
	_zDayColor2_OW->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zDayColor3_OW", "90 80 80").toString();
	_zDayColor3_OW->setText(text);
	text = _iniParser->value("SKY_OUTDOOR/zSunName", "unsun5.tga").toString();
	_zSunName->setText(text);
	value = _iniParser->value("SKY_OUTDOOR/zSunSize", 200).toInt();
	_zSunSize->setValue(value);
	value = _iniParser->value("SKY_OUTDOOR/zSunAlpha", 230).toInt();
	_zSunAlpha->setValue(value);
	d = _iniParser->value("ENGINE/zSunMaxScreenBlendScale", 0.8).toDouble();
	_zSunMaxScreenBlendScale->setValue(d);
	text = _iniParser->value("SKY_OUTDOOR/zMoonName", "moon.tga").toString();
	_zMoonName->setText(text);
	value = _iniParser->value("SKY_OUTDOOR/zMoonSize", 400).toInt();
	_zMoonSize->setValue(value);
	value = _iniParser->value("SKY_OUTDOOR/zMoonAlpha", 255).toInt();
	_zMoonAlpha->setValue(value);
	d = _iniParser->value("SKY_OUTDOOR/zRainWindScale", 0.003).toDouble();
	_zRainWindScale->setValue(d);
	d = _iniParser->value("SKY_OUTDOOR/zNearFogScale", 1.0).toDouble();
	_zNearFogScale->setValue(d);
	d = _iniParser->value("SKY_OUTDOOR/zFarFogScale", 1.0).toDouble();
	_zFarFogScale->setValue(d);
}

void VisualizationPage::accept() {
	// Sky Outdoor
	_iniParser->setValue("SKY_OUTDOOR/zSkyDome", _zSkyDome->currentIndex());
	_iniParser->setValue("SKY_OUTDOOR/zColorizeSky", _zColorizeSky->currentIndex());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor0", _zDayColor0->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor1", _zDayColor1->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor2", _zDayColor2->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor3", _zDayColor3->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor0_OW", _zDayColor0_OW->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor1_OW", _zDayColor1_OW->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor2_OW", _zDayColor2_OW->text());
	_iniParser->setValue("SKY_OUTDOOR/zDayColor3_OW", _zDayColor3_OW->text());
	_iniParser->setValue("SKY_OUTDOOR/zSunName", _zSunName->text());
	_iniParser->setValue("SKY_OUTDOOR/zSunSize", _zSunSize->value());
	_iniParser->setValue("SKY_OUTDOOR/zSunAlpha", _zSunAlpha->value());
	_iniParser->setValue("ENGINE/zSunMaxScreenBlendScale", _zSunMaxScreenBlendScale->value());
	_iniParser->setValue("SKY_OUTDOOR/zMoonName", _zMoonName->text());
	_iniParser->setValue("SKY_OUTDOOR/zMoonSize", _zMoonSize->value());
	_iniParser->setValue("SKY_OUTDOOR/zMoonAlpha", _zMoonAlpha->value());
	_iniParser->setValue("SKY_OUTDOOR/zRainWindScale", _zRainWindScale->value());
	_iniParser->setValue("SKY_OUTDOOR/zNearFogScale", _zNearFogScale->value());
	_iniParser->setValue("SKY_OUTDOOR/zFarFogScale", _zFarFogScale->value());
}

void VisualizationPage::updateSettings(QSettings * iniParser) {
	_iniParser = iniParser;

	reject();
}
