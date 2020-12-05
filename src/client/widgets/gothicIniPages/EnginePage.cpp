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

#include "widgets/gothicIniPages/EnginePage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::widgets::g1;

EnginePage::EnginePage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
	QVBoxLayout * l = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * engineBox = new QGroupBox(QApplication::tr("Engine"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zTexCacheOutTimeMSec", engineBox);
			lbl->setToolTip(QApplication::tr("zTexCacheOutTimeMSecTooltip"));
			_zTexCacheOutTimeMSec = new QSpinBox(engineBox);
			_zTexCacheOutTimeMSec->setMinimum(0);
			_zTexCacheOutTimeMSec->setMaximum(1000000);
			_zTexCacheOutTimeMSec->setSuffix(" " + QApplication::tr("Milliseconds"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zTexCacheOutTimeMSec, 0, 1);

			lbl = new QLabel("zTexCacheSizeMaxBytes", engineBox);
			lbl->setToolTip(QApplication::tr("zTexCacheSizeMaxByteTooltip"));
			_zTexCacheSizeMaxByte = new QSpinBox(engineBox);
			_zTexCacheSizeMaxByte->setMinimum(0);
			_zTexCacheSizeMaxByte->setMaximum(1000000000);
			_zTexCacheSizeMaxByte->setSuffix(" " + QApplication::tr("Bytes"));
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zTexCacheSizeMaxByte, 1, 1);

			engineBox->setLayout(gl);

			hl->addWidget(engineBox);
		}

		l->addLayout(hl);
	}

	setLayout(l);

	reject();
}

EnginePage::~EnginePage() {
}

void EnginePage::reject() {
	// Engine
	_iniParser->beginGroup("ENGINE");
	int value = _iniParser->value("zTexCacheOutTimeMSec", 24000).toInt();
	_zTexCacheOutTimeMSec->setValue(value);
	value = _iniParser->value("zTexCacheSizeMaxBytes", 100000000).toInt();
	_zTexCacheSizeMaxByte->setValue(value);
	_iniParser->endGroup();
}

void EnginePage::accept() {
	// Engine
	_iniParser->beginGroup("ENGINE");
	_iniParser->setValue("zTexCacheOutTimeMSec", _zTexCacheOutTimeMSec->value());
	_iniParser->setValue("zTexCacheSizeMaxBytes", _zTexCacheSizeMaxByte->value());
	_iniParser->endGroup();
}

void EnginePage::updateSettings(QSettings * iniParser) {
	_iniParser = iniParser;

	reject();
}
