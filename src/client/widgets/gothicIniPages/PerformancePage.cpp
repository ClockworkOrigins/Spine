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

#include "widgets/gothicIniPages/PerformancePage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace g1 {

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

				effectsBox->setLayout(gl);

				hl->addWidget(effectsBox);
			}

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

				lbl = new QLabel("zVobFarClipZScale", visibilityBox);
				lbl->setToolTip(QApplication::tr("zVobFarClipZScaleTooltip"));
				_zVobFarClipZScale = new QSpinBox(visibilityBox);
				_zVobFarClipZScale->setMinimum(1);
				_zVobFarClipZScale->setMaximum(3);
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_zVobFarClipZScale, 1, 1);

				visibilityBox->setLayout(gl);

				hl->addWidget(visibilityBox);
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
		int idx;
		int value;
		double d;
		idx = _iniParser->value("PERFORMANCE/recalc", 0).toInt();
		_recalc->setCurrentIndex(idx);
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
		idx = _iniParser->value("VIDEO/skyEffects", 0).toInt();
		_skyEffects->setCurrentIndex(idx);

		// Visibility
		value = _iniParser->value("PERFORMANCE/sightValue", 4).toInt();
		_sightValue->setValue(value);
		value = _iniParser->value("ENGINE/zVobFarClipZScale", 1).toInt();
		_zVobFarClipZScale->setValue(value);
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
		_iniParser->setValue("VIDE0/skyEffects", _skyEffects->currentIndex());

		// Visibility
		_iniParser->setValue("PERFORMANCE/sightValue", _sightValue->value());
		_iniParser->setValue("ENGINE/zVobFarClipZScale", _zVobFarClipZScale->value());
	}

} /* namespace g1 */
} /* namespace widgets */
} /* namespace spine */
