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

#include "widgets/GamepadSpineSettingsWidget.h"

#include "models/SpineEditorModel.h"

#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::widgets;

GamepadSpineSettingsWidget::GamepadSpineSettingsWidget(models::SpineEditorModel * model, QWidget * par) : QWidget(par), _model(model), _earthquakeVibrationLabel(nullptr), _earthquakeVibrationCheckBox(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QHBoxLayout * hl = new QHBoxLayout();
		_earthquakeVibrationLabel = new QLabel(QApplication::tr("EarthquakeVibration"), this);
		UPDATELANGUAGESETTEXT(_earthquakeVibrationLabel, "EarthquakeVibration");
		_earthquakeVibrationCheckBox = new QCheckBox(this);

		hl->addWidget(_earthquakeVibrationLabel);
		hl->addWidget(_earthquakeVibrationCheckBox);

		l->addLayout(hl);
	}

	setLayout(l);
}

GamepadSpineSettingsWidget::~GamepadSpineSettingsWidget() {
}

void GamepadSpineSettingsWidget::save() {
	_model->setEarthquakeVibration(_earthquakeVibrationCheckBox->isChecked());
}

void GamepadSpineSettingsWidget::updateFromModel() {
	_earthquakeVibrationCheckBox->setChecked(_model->getEarthquakeVibration());
}
