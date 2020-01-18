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

#include "widgets/gothic2IniPages/PerformancePage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::widgets::g2;

PerformancePage::PerformancePage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
	QVBoxLayout * l = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * detailingBox = new QGroupBox(QApplication::tr("Detailing"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("recalc", detailingBox);
			lbl->setToolTip(QApplication::tr("recalcTooltip"));
			_recalc = new QComboBox(detailingBox);
			_recalc->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_recalc, 0, 1);

			lbl = new QLabel("modelDetail", detailingBox);
			lbl->setToolTip(QApplication::tr("modelDetailTooltip"));
			_modelDetail = new QDoubleSpinBox(detailingBox);
			_modelDetail->setMinimum(0.0);
			_modelDetail->setMaximum(1.0);
			_modelDetail->setSingleStep(0.01);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_modelDetail, 1, 1);

			lbl = new QLabel("bloodDetail", detailingBox);
			lbl->setToolTip(QApplication::tr("bloodDetailTooltip"));
			_bloodDetail = new QComboBox(detailingBox);
			_bloodDetail->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("Low") << QApplication::tr("Medium") << QApplication::tr("High"));
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_bloodDetail, 2, 1);

			lbl = new QLabel("animatedWindows", detailingBox);
			lbl->setToolTip(QApplication::tr("animatedWindowsTooltip"));
			_animatedWindows = new QComboBox(detailingBox);
			_animatedWindows->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_animatedWindows, 3, 1);

			lbl = new QLabel("zDetailTexturesEnabled", detailingBox);
			lbl->setToolTip(QApplication::tr("zDetailTexturesEnabledTooltip"));
			_zDetailTexturesEnabled = new QComboBox(detailingBox);
			_zDetailTexturesEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 4, 0);
			gl->addWidget(_zDetailTexturesEnabled, 4, 1);

			lbl = new QLabel("zSubdivSurfacesEnabled", detailingBox);
			lbl->setToolTip(QApplication::tr("zSubdivSurfacesEnabledTooltip"));
			_zSubdivSurfacesEnabled = new QComboBox(detailingBox);
			_zSubdivSurfacesEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 5, 0);
			gl->addWidget(_zSubdivSurfacesEnabled, 5, 1);

			detailingBox->setLayout(gl);

			hl->addWidget(detailingBox);
		}

		{
			QGroupBox * effectsBox = new QGroupBox(QApplication::tr("Effects"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("skyEffects", effectsBox);
			lbl->setToolTip(QApplication::tr("skyEffectsTooltip"));
			_skyEffects = new QComboBox(effectsBox);
			_skyEffects->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_skyEffects, 0, 1);

			lbl = new QLabel("zAmbientPFXEnabled", effectsBox);
			lbl->setToolTip(QApplication::tr("zAmbientPFXEnabledTooltip"));
			_zAmbientPFXEnabled = new QComboBox(effectsBox);
			_zAmbientPFXEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zAmbientPFXEnabled, 1, 1);

			lbl = new QLabel("zAmbientVobsEnabled", effectsBox);
			lbl->setToolTip(QApplication::tr("zAmbientVobsEnabledTooltip"));
			_zAmbientVobsEnabled = new QComboBox(effectsBox);
			_zAmbientVobsEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zAmbientVobsEnabled, 2, 1);

			lbl = new QLabel("zVobPointLight", effectsBox);
			lbl->setToolTip(QApplication::tr("zVobPointLightTooltip"));
			_zVobPointLight = new QComboBox(effectsBox);
			_zVobPointLight->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_zVobPointLight, 3, 1);

			lbl = new QLabel("zHighLightScale", effectsBox);
			lbl->setToolTip(QApplication::tr("zHighLightScaleTooltip"));
			_zHighLightScale = new QComboBox(effectsBox);
			_zHighLightScale->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 4, 0);
			gl->addWidget(_zHighLightScale, 4, 1);

			lbl = new QLabel("zEnvMappingEnabled", effectsBox);
			lbl->setToolTip(QApplication::tr("zEnvMappingEnabledTooltip"));
			_zEnvMappingEnabled = new QComboBox(effectsBox);
			_zEnvMappingEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 5, 0);
			gl->addWidget(_zEnvMappingEnabled, 5, 1);

			effectsBox->setLayout(gl);

			hl->addWidget(effectsBox);
		}

		{
			QGroupBox * windEffectsBox = new QGroupBox(QApplication::tr("WindEffects"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zWindEnabled", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindEnabledTooltip"));
			_zWindEnabled = new QComboBox(windEffectsBox);
			_zWindEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zWindEnabled, 0, 1);

			lbl = new QLabel("zWindCycleTime", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindCycleTimeTooltip"));
			_zWindCycleTime = new QSpinBox(windEffectsBox);
			_zWindCycleTime->setMinimum(0);
			_zWindCycleTime->setMaximum(100);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zWindCycleTime, 1, 1);

			lbl = new QLabel("zWindCycleTimeVar", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindCycleTimeVarTooltip"));
			_zWindCycleTimeVar = new QSpinBox(windEffectsBox);
			_zWindCycleTimeVar->setMinimum(0);
			_zWindCycleTimeVar->setMaximum(100);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zWindCycleTimeVar, 2, 1);

			lbl = new QLabel("zWindStrength", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindStrengthTooltip"));
			_zWindStrength = new QSpinBox(windEffectsBox);
			_zWindStrength->setMinimum(0);
			_zWindStrength->setMaximum(1000);
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_zWindStrength, 3, 1);

			lbl = new QLabel("zWindStrengthVar", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindStrengthVarTooltip"));
			_zWindStrengthVar = new QSpinBox(windEffectsBox);
			_zWindStrengthVar->setMinimum(0);
			_zWindStrengthVar->setMaximum(1000);
			gl->addWidget(lbl, 4, 0);
			gl->addWidget(_zWindStrengthVar, 4, 1);

			lbl = new QLabel("zWindAngleVelo", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindAngleVeloTooltip"));
			_zWindAngleVelo = new QDoubleSpinBox(windEffectsBox);
			_zWindAngleVelo->setMinimum(0.0);
			_zWindAngleVelo->setMaximum(1.0);
			_zWindAngleVelo->setSingleStep(0.01);
			gl->addWidget(lbl, 5, 0);
			gl->addWidget(_zWindAngleVelo, 5, 1);

			lbl = new QLabel("zWindAngleVeloVar", windEffectsBox);
			lbl->setToolTip(QApplication::tr("zWindAngleVeloVarTooltip"));
			_zWindAngleVeloVar = new QDoubleSpinBox(windEffectsBox);
			_zWindAngleVeloVar->setMinimum(0.0);
			_zWindAngleVeloVar->setMaximum(1.0);
			_zWindAngleVeloVar->setSingleStep(0.01);
			gl->addWidget(lbl, 6, 0);
			gl->addWidget(_zWindAngleVeloVar, 6, 1);

			windEffectsBox->setLayout(gl);

			hl->addWidget(windEffectsBox);
		}

		l->addLayout(hl);
	}

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * visibilityBox = new QGroupBox(QApplication::tr("Visibility"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("sightValue", visibilityBox);
			lbl->setToolTip(QApplication::tr("sightValueTooltip"));
			_sightValue = new QSpinBox(visibilityBox);
			_sightValue->setMinimum(0);
			_sightValue->setMaximum(14);
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_sightValue, 0, 1);

			lbl = new QLabel("zFarClipAlphaFade", visibilityBox);
			lbl->setToolTip(QApplication::tr("zFarClipAlphaFadeTooltip"));
			_zFarClipAlphaFade = new QComboBox(visibilityBox);
			_zFarClipAlphaFade->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zFarClipAlphaFade, 1, 1);

			lbl = new QLabel("zVobFarClipZScale", visibilityBox);
			lbl->setToolTip(QApplication::tr("zVobFarClipZScaleTooltip"));
			_zVobFarClipZScale = new QSpinBox(visibilityBox);
			_zVobFarClipZScale->setMinimum(1);
			_zVobFarClipZScale->setMaximum(14);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zVobFarClipZScale, 2, 1);

			visibilityBox->setLayout(gl);

			hl->addWidget(visibilityBox);
		}

		{
			QGroupBox * waterEffectsBox = new QGroupBox(QApplication::tr("WaterEffects"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zWaterAniEnabled", waterEffectsBox);
			lbl->setToolTip(QApplication::tr("zWaterAniEnabledTooltip"));
			_zWaterAniEnabled = new QComboBox(waterEffectsBox);
			_zWaterAniEnabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zWaterAniEnabled, 0, 1);

			lbl = new QLabel("zWaterEnvMapAniFPS", waterEffectsBox);
			lbl->setToolTip(QApplication::tr("zWaterEnvMapAniFPSTooltip"));
			_zWaterEnvMapAniFPS = new QDoubleSpinBox(waterEffectsBox);
			_zWaterEnvMapAniFPS->setMinimum(0.0);
			_zWaterEnvMapAniFPS->setMaximum(100.0);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zWaterEnvMapAniFPS, 1, 1);

			lbl = new QLabel("zCloudShadowScale", waterEffectsBox);
			lbl->setToolTip(QApplication::tr("zCloudShadowScaleTooltip"));
			_zCloudShadowScale = new QComboBox(waterEffectsBox);
			_zCloudShadowScale->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zCloudShadowScale, 2, 1);

			waterEffectsBox->setLayout(gl);

			hl->addWidget(waterEffectsBox);
		}

		{
			QGroupBox * smoothingBox = new QGroupBox(QApplication::tr("Smoothing"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zSmoothTimer", smoothingBox);
			lbl->setToolTip(QApplication::tr("zSmoothTimerTooltip"));
			_zSmoothTimer = new QComboBox(smoothingBox);
			_zSmoothTimer->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zSmoothTimer, 0, 1);

			lbl = new QLabel("zSmoothModelRootNode", smoothingBox);
			lbl->setToolTip(QApplication::tr("zSmoothModelRootNodeTooltip"));
			_zSmoothModelRootNode = new QComboBox(smoothingBox);
			_zSmoothModelRootNode->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zSmoothModelRootNode, 1, 1);

			smoothingBox->setLayout(gl);

			hl->addWidget(smoothingBox);
		}

		l->addLayout(hl);
	}

	setLayout(l);

	reject();
}

PerformancePage::~PerformancePage() {
}

void PerformancePage::reject() {
	// Detailing
	int idx = _iniParser->value("PERFORMANCE/recalc", 0).toInt();
	_recalc->setCurrentIndex(idx);
	double d;
	d = _iniParser->value("PERFORMANCE/modelDetail", 0.5).toDouble();
	_modelDetail->setValue(d);
	idx = _iniParser->value("GAME/bloodDetail", 0).toInt();
	_bloodDetail->setCurrentIndex(idx);
	idx = _iniParser->value("GAME/animatedWindows", 0).toInt();
	_animatedWindows->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zDetailTexturesEnabled", 1).toInt();
	_zDetailTexturesEnabled->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zSubdivSurfacesEnabled", 1).toInt();
	_zSubdivSurfacesEnabled->setCurrentIndex(idx);

	// Effects
	idx = _iniParser->value("GAME/skyEffects", 0).toInt();
	_skyEffects->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zAmbientPFXEnabled", 1).toInt();
	_zAmbientPFXEnabled->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zAmbientVobsEnabled", 0).toInt();
	_zAmbientVobsEnabled->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zVobPointLight", 0).toInt();
	_zVobPointLight->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zHighLightScale", 0).toInt();
	_zHighLightScale->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zEnvMappingEnabled", 0).toInt();
	_zEnvMappingEnabled->setCurrentIndex(idx);

	// Wind Effects
	idx = _iniParser->value("ENGINE/zWindEnabled", 0).toInt();
	_zWindEnabled->setCurrentIndex(idx);
	int value;
	value = _iniParser->value("ENGINE/zWindCycleTime", 4).toInt();
	_zWindCycleTime->setValue(value);
	value = _iniParser->value("ENGINE/zWindCycleTimeVar", 2).toInt();
	_zWindCycleTimeVar->setValue(value);
	value = _iniParser->value("ENGINE/zWindStrength", 70).toInt();
	_zWindStrength->setValue(value);
	value = _iniParser->value("ENGINE/zWindStrengthVar", 40).toInt();
	_zWindStrengthVar->setValue(value);
	d = _iniParser->value("ENGINE/zWindAngleVelo", 0.9).toDouble();
	_zWindAngleVelo->setValue(d);
	d = _iniParser->value("ENGINE/zWindAngleVeloVar", 0.8).toDouble();
	_zWindAngleVeloVar->setValue(d);

	// Visibility
	value = _iniParser->value("PERFORMANCE/sightValue", 4).toInt();
	_sightValue->setValue(value);
	idx = _iniParser->value("ENGINE/zFarClipAlphaFade", 0).toInt();
	_zFarClipAlphaFade->setCurrentIndex(idx);
	value = _iniParser->value("ENGINE/zVobFarClipZScale", 1).toInt();
	_zVobFarClipZScale->setValue(value);

	// Water Effects
	idx = _iniParser->value("ENGINE/zWaterAniEnabled", 1).toInt();
	_zWaterAniEnabled->setCurrentIndex(idx);
	d = _iniParser->value("ENGINE/zWaterEnvMapAniFPS", 0.0).toDouble();
	_zWaterEnvMapAniFPS->setValue(d);
	idx = _iniParser->value("ENGINE/zCloudShadowScale", 0).toInt();
	_zCloudShadowScale->setCurrentIndex(idx);

	// Smoothing
	idx = _iniParser->value("ENGINE/zSmoothTimer", 1).toInt();
	_zSmoothTimer->setCurrentIndex(idx);
	idx = _iniParser->value("ENGINE/zSmoothModelRootNode", 1).toInt();
	_zSmoothModelRootNode->setCurrentIndex(idx);
}

void PerformancePage::accept() {
	// Detailing
	_iniParser->setValue("PERFORMANCE/recalc", _recalc->currentIndex());
	_iniParser->setValue("PERFORMANCE/modelDetail", _modelDetail->value());
	_iniParser->setValue("GAME/bloodDetail", _bloodDetail->currentIndex());
	_iniParser->setValue("GAME/animatedWindows", _animatedWindows->currentIndex());
	_iniParser->setValue("ENGINE/zDetailTexturesEnabled", _zDetailTexturesEnabled->currentIndex());
	_iniParser->setValue("ENGINE/zSubdivSurfacesEnabled", _zSubdivSurfacesEnabled->currentIndex());

	// Effects
	_iniParser->setValue("GAME/skyEffects", _skyEffects->currentIndex());
	_iniParser->setValue("ENGINE/zAmbientPFXEnabled", _zAmbientPFXEnabled->currentIndex());
	_iniParser->setValue("ENGINE/zAmbientVobsEnabled", _zAmbientVobsEnabled->currentIndex());
	_iniParser->setValue("ENGINE/zVobPointLight", _zVobPointLight->currentIndex());
	_iniParser->setValue("ENGINE/zHighLightScale", _zHighLightScale->currentIndex());
	_iniParser->setValue("ENGINE/zEnvMappingEnabled", _zEnvMappingEnabled->currentIndex());

	// Wind Effects
	_iniParser->setValue("ENGINE/zWindEnabled", _zWindEnabled->currentIndex());
	_iniParser->setValue("ENGINE/zWindCycleTime", _zWindCycleTime->value());
	_iniParser->setValue("ENGINE/zWindCycleTimeVar", _zWindCycleTimeVar->value());
	_iniParser->setValue("ENGINE/zWindStrength", _zWindStrength->value());
	_iniParser->setValue("ENGINE/zWindStrengthVar", _zWindStrengthVar->value());
	_iniParser->setValue("ENGINE/zWindAngleVelo", _zWindAngleVelo->value());
	_iniParser->setValue("ENGINE/zWindAngleVeloVar", _zWindAngleVeloVar->value());

	// Visibility
	_iniParser->setValue("PERFORMANCE/sightValue", _sightValue->value());
	_iniParser->setValue("ENGINE/zFarClipAlphaFade", _zFarClipAlphaFade->currentIndex());
	_iniParser->setValue("ENGINE/zVobFarClipZScale", _zVobFarClipZScale->value());

	// Water Effects
	_iniParser->setValue("ENGINE/zWaterAniEnabled", _zWaterAniEnabled->currentIndex());
	_iniParser->setValue("ENGINE/zWaterEnvMapAniFPS", _zWaterEnvMapAniFPS->value());
	_iniParser->setValue("ENGINE/zCloudShadowScale", _zCloudShadowScale->currentIndex());

	// Smoothing
	_iniParser->setValue("ENGINE/zSmoothTimer", _zSmoothTimer->currentIndex());
	_iniParser->setValue("ENGINE/zSmoothModelRootNode", _zSmoothModelRootNode->currentIndex());
}

void PerformancePage::updateSettings(QSettings * iniParser) {
	_iniParser = iniParser;

	reject();
}
