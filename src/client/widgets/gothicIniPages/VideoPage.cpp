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

#include "widgets/gothicIniPages/VideoPage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::widgets::g1;

VideoPage::VideoPage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
	QVBoxLayout * l = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		{
			QGroupBox * rendererD3DBox = new QGroupBox(QApplication::tr("RendererD3D"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zFogDisabled", rendererD3DBox);
			lbl->setToolTip(QApplication::tr("zFogDisabledTooltip"));
			_zFogDisabled = new QComboBox(rendererD3DBox);
			_zFogDisabled->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zFogDisabled, 0, 1);

			rendererD3DBox->setLayout(gl);

			hl->addWidget(rendererD3DBox);
		}

		{
			QGroupBox * videoBox = new QGroupBox(QApplication::tr("Video"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zVidDevice", videoBox);
			lbl->setToolTip(QApplication::tr("zVidDeviceTooltip"));
			_zVidDevice = new QSpinBox(videoBox);
			_zVidDevice->setMinimum(0);
			_zVidDevice->setMaximum(10);
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zVidDevice, 0, 1);

			lbl = new QLabel("zVidBrightness", videoBox);
			lbl->setToolTip(QApplication::tr("zVidBrightnessTooltip"));
			_zVidBrightness = new QDoubleSpinBox(videoBox);
			_zVidBrightness->setMinimum(0.0);
			_zVidBrightness->setMaximum(1.0);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zVidBrightness, 1, 1);

			lbl = new QLabel("zVidContrast", videoBox);
			lbl->setToolTip(QApplication::tr("zVidContrastTooltip"));
			_zVidContrast = new QDoubleSpinBox(videoBox);
			_zVidContrast->setMinimum(0.0);
			_zVidContrast->setMaximum(1.0);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zVidContrast, 2, 1);

			lbl = new QLabel("zVidGamma", videoBox);
			lbl->setToolTip(QApplication::tr("zVidGammaTooltip"));
			_zVidGamma = new QDoubleSpinBox(videoBox);
			_zVidGamma->setMinimum(0.0);
			_zVidGamma->setMaximum(1.0);
			gl->addWidget(lbl, 3, 0);
			gl->addWidget(_zVidGamma, 3, 1);

			lbl = new QLabel("zTexMaxSize", videoBox);
			lbl->setToolTip(QApplication::tr("zTexMaxSizeTooltip"));
			_zTexMaxSize = new QSpinBox(videoBox);
			_zTexMaxSize->setMinimum(0);
			_zTexMaxSize->setMaximum(1000000);
			gl->addWidget(lbl, 4, 0);
			gl->addWidget(_zTexMaxSize, 4, 1);

			lbl = new QLabel("zStartupWindowed", videoBox);
			lbl->setToolTip(QApplication::tr("zStartupWindowedTooltip"));
			_zStartupWindowed = new QComboBox(videoBox);
			_zStartupWindowed->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 5, 0);
			gl->addWidget(_zStartupWindowed, 5, 1);

			lbl = new QLabel("scaleVideos", videoBox);
			lbl->setToolTip(QApplication::tr("scaleVideosTooltip"));
			_scaleVideos = new QComboBox(videoBox);
			_scaleVideos->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 6, 0);
			gl->addWidget(_scaleVideos, 6, 1);

			lbl = new QLabel("playLogoVideos", videoBox);
			lbl->setToolTip(QApplication::tr("playLogoVideosTooltip"));
			_playLogoVideos = new QComboBox(videoBox);
			_playLogoVideos->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
			gl->addWidget(lbl, 7, 0);
			gl->addWidget(_playLogoVideos, 7, 1);

			videoBox->setLayout(gl);

			hl->addWidget(videoBox);
		}

		{
			QGroupBox * resolutionBox = new QGroupBox(QApplication::tr("Resolution"), this);

			QGridLayout * gl = new QGridLayout();
			gl->setAlignment(Qt::AlignTop);

			QLabel * lbl = new QLabel("zVidResFullscreenX", resolutionBox);
			lbl->setToolTip(QApplication::tr("zVidResFullscreenXTooltip"));
			_zVidResFullscreenX = new QSpinBox(resolutionBox);
			_zVidResFullscreenX->setMinimum(0);
			_zVidResFullscreenX->setMaximum(100000);
			gl->addWidget(lbl, 0, 0);
			gl->addWidget(_zVidResFullscreenX, 0, 1);

			lbl = new QLabel("zVidResFullscreenY", resolutionBox);
			lbl->setToolTip(QApplication::tr("zVidResFullscreenYTooltip"));
			_zVidResFullscreenY = new QSpinBox(resolutionBox);
			_zVidResFullscreenY->setMinimum(0);
			_zVidResFullscreenY->setMaximum(1000000);
			gl->addWidget(lbl, 1, 0);
			gl->addWidget(_zVidResFullscreenY, 1, 1);

			lbl = new QLabel("zVidResFullscreenBPP", resolutionBox);
			lbl->setToolTip(QApplication::tr("zVidResFullscreenBPPTooltip"));
			_zVidResFullscreenBPP = new QSpinBox(resolutionBox);
			_zVidResFullscreenBPP->setMinimum(0);
			_zVidResFullscreenBPP->setMaximum(32);
			gl->addWidget(lbl, 2, 0);
			gl->addWidget(_zVidResFullscreenBPP, 2, 1);

			resolutionBox->setLayout(gl);

			hl->addWidget(resolutionBox);
		}

		l->addLayout(hl);
	}

	setLayout(l);

	reject();
}

VideoPage::~VideoPage() {
}

void VideoPage::reject() {
	// Renderer D3D
	int idx;
	int value;
	double d;
	_iniParser->beginGroup("RENDERER_D3D");
	idx = _iniParser->value("zFogDisabled", 0).toInt();
	_zFogDisabled->setCurrentIndex(idx);
	_iniParser->endGroup();

	// Video
	_iniParser->beginGroup("VIDEO");
	value = _iniParser->value("zVidDevice", 0).toInt();
	_zVidDevice->setValue(value);
	d = _iniParser->value("zVidBrightness", 0.5).toDouble();
	_zVidBrightness->setValue(d);
	d = _iniParser->value("zVidContrast", 0.5).toDouble();
	_zVidContrast->setValue(d);
	d = _iniParser->value("zVidGamma", 0.5).toDouble();
	_zVidGamma->setValue(d);
	value = _iniParser->value("zTexMaxSize", 16384).toInt();
	_zTexMaxSize->setValue(value);
	idx = _iniParser->value("zStartupWindowed", 0).toInt();
	_zStartupWindowed->setCurrentIndex(idx);

	// Resolution
	value = _iniParser->value("zVidResFullscreenX", 0).toInt();
	_zVidResFullscreenX->setValue(value);
	value = _iniParser->value("zVidResFullscreenY", 0).toInt();
	_zVidResFullscreenY->setValue(value);
	value = _iniParser->value("zVidResFullscreenBPP", 32).toInt();
	_zVidResFullscreenBPP->setValue(value);
	_iniParser->endGroup();

	_iniParser->beginGroup("GAME");
	idx = _iniParser->value("scaleVideos", 1).toInt();
	_scaleVideos->setCurrentIndex(idx);
	idx = _iniParser->value("playLogoVideos", 0).toInt();
	_playLogoVideos->setCurrentIndex(idx);
	_iniParser->endGroup();
}

void VideoPage::accept() {
	// Renderer D3D
	_iniParser->beginGroup("RENDERER_D3D");
	_iniParser->setValue("zFogDisabled", _zFogDisabled->currentIndex());
	_iniParser->endGroup();

	// Video
	_iniParser->beginGroup("VIDEO");
	_iniParser->setValue("zVidDevice", _zVidDevice->value());
	_iniParser->setValue("zVidBrightness", _zVidBrightness->value());
	_iniParser->setValue("zVidContrast", _zVidContrast->value());
	_iniParser->setValue("zVidGamma", _zVidGamma->value());
	_iniParser->setValue("zTexMaxSize", _zTexMaxSize->value());
	_iniParser->setValue("zStartupWindowed", _zStartupWindowed->currentIndex());

	// Resolution
	_iniParser->setValue("zVidResFullscreenX", _zVidResFullscreenX->value());
	_iniParser->setValue("zVidResFullscreenY", _zVidResFullscreenY->value());
	_iniParser->setValue("zVidResFullscreenBPP", _zVidResFullscreenBPP->value());
	_iniParser->endGroup();

	_iniParser->beginGroup("GAME");
	_iniParser->setValue("scaleVideos", _scaleVideos->currentIndex());
	_iniParser->setValue("playLogoVideos", _playLogoVideos->currentIndex());
	_iniParser->endGroup();
}

void VideoPage::updateSettings(QSettings * iniParser) {
	_iniParser = iniParser;

	reject();
}
