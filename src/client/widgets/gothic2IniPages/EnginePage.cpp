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

#include "widgets/gothic2IniPages/EnginePage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace g2 {

	EnginePage::EnginePage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();

		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * engineBox = new QGroupBox(QApplication::tr("Engine"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("ZMAXFPS", engineBox);
				lbl->setToolTip(QApplication::tr("ZMAXFPSTooltip"));
				_ZMAXFPS = new QSpinBox(engineBox);
				_ZMAXFPS->setMinimum(0);
				_ZMAXFPS->setMaximum(1000);
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_ZMAXFPS, 0, 1);

				lbl = new QLabel("ZNEARVALUE", engineBox);
				lbl->setToolTip(QApplication::tr("ZNEARVALUETooltip"));
				_ZNEARVALUE = new QDoubleSpinBox(engineBox);
				_ZNEARVALUE->setMinimum(-1.0);
				_ZNEARVALUE->setMaximum(1.0);
				_ZNEARVALUE->setSingleStep(0.01);
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_ZNEARVALUE, 1, 1);

				lbl = new QLabel("NOAMBIENTPFX", engineBox);
				lbl->setToolTip(QApplication::tr("NOAMBIENTPFXTooltip"));
				_NOAMBIENTPFX = new QComboBox(engineBox);
				_NOAMBIENTPFX->addItems(QStringList() << QApplication::tr("On") << QApplication::tr("Off"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_NOAMBIENTPFX, 2, 1);

				lbl = new QLabel("zCacheInAllNSCAtNewGame", engineBox);
				lbl->setToolTip(QApplication::tr("zCacheInAllNSCAtNewGameTooltip"));
				_zCacheInAllNSCAtNewGame = new QComboBox(engineBox);
				_zCacheInAllNSCAtNewGame->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_zCacheInAllNSCAtNewGame, 3, 1);

				lbl = new QLabel("zEnvMapTextureName", engineBox);
				lbl->setToolTip(QApplication::tr("zEnvMapTextureNameTooltip"));
				_zEnvMapTextureName = new QLineEdit(engineBox);
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_zEnvMapTextureName, 4, 1);

				lbl = new QLabel("zTexCacheOutTimeMSec", engineBox);
				lbl->setToolTip(QApplication::tr("zTexCacheOutTimeMSecTooltip"));
				_zTexCacheOutTimeMSec = new QSpinBox(engineBox);
				_zTexCacheOutTimeMSec->setMinimum(0);
				_zTexCacheOutTimeMSec->setMaximum(1000000);
				_zTexCacheOutTimeMSec->setSuffix(" " + QApplication::tr("Milliseconds"));
				gl->addWidget(lbl, 0, 2);
				gl->addWidget(_zTexCacheOutTimeMSec, 0, 3);

				lbl = new QLabel("zTexCacheSizeMaxByte", engineBox);
				lbl->setToolTip(QApplication::tr("zTexCacheSizeMaxByteTooltip"));
				_zTexCacheSizeMaxByte = new QSpinBox(engineBox);
				_zTexCacheSizeMaxByte->setMinimum(0);
				_zTexCacheSizeMaxByte->setMaximum(1000000000);
				_zTexCacheSizeMaxByte->setSuffix(" " + QApplication::tr("Bytes"));
				gl->addWidget(lbl, 1, 2);
				gl->addWidget(_zTexCacheSizeMaxByte, 1, 3);

				lbl = new QLabel("zSndCacheOutTimeMSec", engineBox);
				lbl->setToolTip(QApplication::tr("zSndCacheOutTimeMSecTooltip"));
				_zSndCacheOutTimeMSec = new QSpinBox(engineBox);
				_zSndCacheOutTimeMSec->setMinimum(0);
				_zSndCacheOutTimeMSec->setMaximum(1000000);
				_zSndCacheOutTimeMSec->setSuffix(" " + QApplication::tr("Milliseconds"));
				gl->addWidget(lbl, 2, 2);
				gl->addWidget(_zSndCacheOutTimeMSec, 2, 3);

				lbl = new QLabel("zSndCacheSizeMaxBytes", engineBox);
				lbl->setToolTip(QApplication::tr("zSndCacheSizeMaxBytesTooltip"));
				_zSndCacheSizeMaxBytes = new QSpinBox(engineBox);
				_zSndCacheSizeMaxBytes->setMinimum(0);
				_zSndCacheSizeMaxBytes->setMaximum(1000000000);
				_zSndCacheSizeMaxBytes->setSuffix(" " + QApplication::tr("Bytes"));
				gl->addWidget(lbl, 3, 2);
				gl->addWidget(_zSndCacheSizeMaxBytes, 3, 3);

				lbl = new QLabel("zWaterEnvMapTextureName", engineBox);
				lbl->setToolTip(QApplication::tr("zWaterEnvMapTextureNameTooltip"));
				_zWaterEnvMapTextureName = new QLineEdit(engineBox);
				gl->addWidget(lbl, 4, 2);
				gl->addWidget(_zWaterEnvMapTextureName, 4, 3);

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
		int idx;
		int value;
		double d;
		QString text;
		_iniParser->beginGroup("ENGINE");
		value = _iniParser->value("ZMAXFPS", 0).toInt();
		_ZMAXFPS->setValue(value);
		d = _iniParser->value("ZNEARVALUE", -1.0).toDouble();
		_ZNEARVALUE->setValue(d);
		idx = _iniParser->value("NOAMBIENTPFX", 0).toInt();
		_NOAMBIENTPFX->setCurrentIndex(idx);
		idx = _iniParser->value("zCacheInAllNSCAtNewGame", 0).toInt();
		_zCacheInAllNSCAtNewGame->setCurrentIndex(idx);
		text = _iniParser->value("zEnvMapTextureName", "zflare1.tga").toString();
		_zEnvMapTextureName->setText(text);
		value = _iniParser->value("zTexCacheOutTimeMSec", 24000).toInt();
		_zTexCacheOutTimeMSec->setValue(value);
		value = _iniParser->value("zTexCacheSizeMaxByte", 100000000).toInt();
		_zTexCacheSizeMaxByte->setValue(value);
		value = _iniParser->value("zSndCacheOutTimeMSec", 10000).toInt();
		_zSndCacheOutTimeMSec->setValue(value);
		value = _iniParser->value("zSndCacheSizeMaxBytes", 10000000).toInt();
		_zSndCacheSizeMaxBytes->setValue(value);
		text = _iniParser->value("zWaterEnvMapTextureName", "cloudenv_bright.tga").toString();
		_zWaterEnvMapTextureName->setText(text);
		_iniParser->endGroup();
	}

	void EnginePage::accept() {
		// Engine
		_iniParser->beginGroup("ENGINE");
		_iniParser->setValue("ZMAXFPS", _ZMAXFPS->value());
		_iniParser->setValue("ZNEARVALUE", _ZNEARVALUE->value());
		_iniParser->setValue("NOAMBIENTPFX", _NOAMBIENTPFX->currentIndex());
		_iniParser->setValue("zCacheInAllNSCAtNewGame", _zCacheInAllNSCAtNewGame->currentIndex());
		_iniParser->setValue("zEnvMapTextureName", _zEnvMapTextureName->text());
		_iniParser->setValue("zTexCacheOutTimeMSec", _zTexCacheOutTimeMSec->value());
		_iniParser->setValue("zTexCacheSizeMaxByte", _zTexCacheSizeMaxByte->value());
		_iniParser->setValue("zSndCacheOutTimeMSec", _zSndCacheOutTimeMSec->value());
		_iniParser->setValue("zSndCacheSizeMaxBytes", _zSndCacheSizeMaxBytes->value());
		_iniParser->setValue("zWaterEnvMapTextureName", _zWaterEnvMapTextureName->text());
		_iniParser->endGroup();
	}

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */
