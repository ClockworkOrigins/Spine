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

#include "widgets/gothicIniPages/ControlsPage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::widgets::g1;

ControlsPage::ControlsPage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
	QVBoxLayout * l = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * mouseBox = new QGroupBox(QApplication::tr("Mouse"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("enableMouse", mouseBox);
			lbl->setToolTip(QApplication::tr("enableMouseTooltip"));
			_enableMouse = new QComboBox(mouseBox);
			_enableMouse->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_enableMouse, 0, 1);

			lbl = new QLabel("mouseSensitivity", mouseBox);
			lbl->setToolTip(QApplication::tr("mouseSensitivityTooltip"));
			_mouseSensitivity = new QDoubleSpinBox(mouseBox);
			_mouseSensitivity->setMinimum(0.0);
			_mouseSensitivity->setMaximum(1.0);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_mouseSensitivity, 1, 1);

			mouseBox->setLayout(gl);

			hl->addWidget(mouseBox);
		}

		l->addLayout(hl);
	}

	setLayout(l);

	reject();
}

ControlsPage::~ControlsPage() {
}

void ControlsPage::reject() {
	int idx;
	double d;

	// Mouse
	_iniParser->beginGroup("GAME");
	idx = _iniParser->value("enableMouse", 0).toInt();
	_enableMouse->setCurrentIndex(idx);
	d = _iniParser->value("mouseSensitivity", 0.5).toDouble();
	_mouseSensitivity->setValue(d);
	_iniParser->endGroup();
}

void ControlsPage::accept() {
	// Mouse
	_iniParser->beginGroup("GAME");
	_iniParser->setValue("enableMouse", _enableMouse->currentIndex());
	_iniParser->setValue("mouseSensitivity", _mouseSensitivity->value());
	_iniParser->endGroup();
}

void ControlsPage::updateSettings(QSettings * iniParser) {
	_iniParser = iniParser;

	reject();
}
