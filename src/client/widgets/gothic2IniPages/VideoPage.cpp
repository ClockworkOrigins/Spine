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

#include "widgets/gothic2IniPages/VideoPage.h"

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

				lbl = new QLabel("zFogRadial", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zFogRadialTooltip"));
				_zFogRadial = new QComboBox(rendererD3DBox);
				_zFogRadial->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_zFogRadial, 1, 1);

				lbl = new QLabel("zForceZBuffer", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zForceZBufferTooltip"));
				_zForceZBuffer = new QComboBox(rendererD3DBox);
				_zForceZBuffer->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_zForceZBuffer, 2, 1);

				lbl = new QLabel("zEnableTaskSwitch", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zEnableTaskSwitchTooltip"));
				_zEnableTaskSwitch = new QComboBox(rendererD3DBox);
				_zEnableTaskSwitch->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_zEnableTaskSwitch, 3, 1);

				lbl = new QLabel("zSyncAmbientCol", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zSyncAmbientColTooltip"));
				_zSyncAmbientCol = new QComboBox(rendererD3DBox);
				_zSyncAmbientCol->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 4, 0);
				gl->addWidget(_zSyncAmbientCol, 4, 1);

				lbl = new QLabel("zVidEnableAntiAliasing", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zVidEnableAntiAliasingTooltip"));
				_zVidEnableAntiAliasing = new QComboBox(rendererD3DBox);
				_zVidEnableAntiAliasing->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 5, 0);
				gl->addWidget(_zVidEnableAntiAliasing, 5, 1);

				lbl = new QLabel("zTexAnisotropicFiltering", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zTexAnisotropicFilteringTooltip"));
				_zTexAnisotropicFiltering = new QComboBox(rendererD3DBox);
				_zTexAnisotropicFiltering->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 6, 0);
				gl->addWidget(_zTexAnisotropicFiltering, 6, 1);

				lbl = new QLabel("zSkyRenderFirst", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("zSkyRenderFirstTooltip"));
				_zSkyRenderFirst = new QComboBox(rendererD3DBox);
				_zSkyRenderFirst->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 7, 0);
				gl->addWidget(_zSkyRenderFirst, 7, 1);

				lbl = new QLabel("radeonHackAmbientColBug", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("radeonHackAmbientColBugTooltip"));
				_radeonHackAmbientColBug = new QComboBox(rendererD3DBox);
				_radeonHackAmbientColBug->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 8, 0);
				gl->addWidget(_radeonHackAmbientColBug, 8, 1);

				lbl = new QLabel("geforce3HackWBufferBug", rendererD3DBox);
				lbl->setToolTip(QApplication::tr("geforce3HackWBufferBugTooltip"));
				_geforce3HackWBufferBug = new QComboBox(rendererD3DBox);
				_geforce3HackWBufferBug->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 9, 0);
				gl->addWidget(_geforce3HackWBufferBug, 9, 1);

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

				lbl = new QLabel("zVidRefreshRate", resolutionBox);
				lbl->setToolTip(QApplication::tr("zVidRefreshRateTooltip"));
				_zVidRefreshRate = new QSpinBox(resolutionBox);
				_zVidRefreshRate->setMinimum(0);
				_zVidRefreshRate->setMaximum(1000);
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_zVidRefreshRate, 3, 1);

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
		idx = _iniParser->value("RENDERER_D3D/zFogDisabled", 0).toInt();
		_zFogDisabled->setCurrentIndex(idx);
		idx = _iniParser->value("RENDERER_D3D/zFogRadial", 1).toInt();
		_zFogRadial->setCurrentIndex(idx);
		idx = _iniParser->value("RENDERER_D3D/zForceZBuffer", 0).toInt();
		_zForceZBuffer->setCurrentIndex(idx);
		idx = _iniParser->value("RENDERER_D3D/zEnableTaskSwitch", 1).toInt();
		_zEnableTaskSwitch->setCurrentIndex(idx);
		idx = _iniParser->value("RENDERER_D3D/zSyncAmbientCol", 0).toInt();
		_zSyncAmbientCol->setCurrentIndex(idx);
		idx = _iniParser->value("ENGINE/zVidEnableAntiAliasing", 0).toInt();
		_zVidEnableAntiAliasing->setCurrentIndex(idx);
		idx = _iniParser->value("ENGINE/zTexAnisotropicFiltering", 0).toInt();
		_zTexAnisotropicFiltering->setCurrentIndex(idx);
		idx = _iniParser->value("ENGINE/zSkyRenderFirst", 1).toInt();
		_zSkyRenderFirst->setCurrentIndex(idx);
		idx = _iniParser->value("RENDERER_D3D/radeonHackAmbientColBug", 0).toInt();
		_radeonHackAmbientColBug->setCurrentIndex(idx);
		idx = _iniParser->value("RENDERER_D3D/geforce3HackWBufferBug", 1).toInt();
		_geforce3HackWBufferBug->setCurrentIndex(idx);

		// Video
		value = _iniParser->value("VIDEO/zVidDevice", 0).toInt();
		_zVidDevice->setValue(value);
		d = _iniParser->value("VIDEO/zVidBrightness", 0.5).toDouble();
		_zVidBrightness->setValue(d);
		d = _iniParser->value("VIDEO/zVidContrast", 0.5).toDouble();
		_zVidContrast->setValue(d);
		d = _iniParser->value("VIDEO/zVidGamma", 0.5).toDouble();
		_zVidGamma->setValue(d);
		value = _iniParser->value("VIDEO/zTexMaxSize", 16384).toInt();
		_zTexMaxSize->setValue(value);
		idx = _iniParser->value("VIDEO/zStartupWindowed", 0).toInt();
		_zStartupWindowed->setCurrentIndex(idx);
		idx = _iniParser->value("GAME/scaleVideos", 1).toInt();
		_scaleVideos->setCurrentIndex(idx);
		idx = _iniParser->value("GAME/playLogoVideos", 0).toInt();
		_playLogoVideos->setCurrentIndex(idx);

		// Resolution
		value = _iniParser->value("VIDEO/zVidResFullscreenX", 0).toInt();
		_zVidResFullscreenX->setValue(value);
		value = _iniParser->value("VIDEO/zVidResFullscreenY", 0).toInt();
		_zVidResFullscreenY->setValue(value);
		value = _iniParser->value("VIDEO/zVidResFullscreenBPP", 32).toInt();
		_zVidResFullscreenBPP->setValue(value);
		value = _iniParser->value("RENDERER_D3D/zVidRefreshRate", 0).toInt();
		_zVidRefreshRate->setValue(value);
	}

	void VideoPage::accept() {
		// Renderer D3D
		_iniParser->setValue("RENDERER_D3D/zFogDisabled", _zFogDisabled->currentIndex());
		_iniParser->setValue("RENDERER_D3D/zFogRadial", _zFogRadial->currentIndex());
		_iniParser->setValue("RENDERER_D3D/zForceZBuffer", _zForceZBuffer->currentIndex());
		_iniParser->setValue("RENDERER_D3D/zEnableTaskSwitch", _zEnableTaskSwitch->currentIndex());
		_iniParser->setValue("RENDERER_D3D/zSyncAmbientCol", _zSyncAmbientCol->currentIndex());
		_iniParser->setValue("ENGINE/zVidEnableAntiAliasing", _zVidEnableAntiAliasing->currentIndex());
		_iniParser->setValue("ENGINE/zTexAnisotropicFiltering", _zTexAnisotropicFiltering->currentIndex());
		_iniParser->setValue("ENGINE/zSkyRenderFirst", _zSkyRenderFirst->currentIndex());
		_iniParser->setValue("RENDERER_D3D/radeonHackAmbientColBug", _radeonHackAmbientColBug->currentIndex());
		_iniParser->setValue("RENDERER_D3D/geforce3HackWBufferBug", _geforce3HackWBufferBug->currentIndex());

		// Video
		_iniParser->setValue("VIDEO/zVidDevice", _zVidDevice->value());
		_iniParser->setValue("VIDEO/zVidBrightness", _zVidBrightness->value());
		_iniParser->setValue("VIDEO/zVidContrast", _zVidContrast->value());
		_iniParser->setValue("VIDEO/zVidGamma", _zVidGamma->value());
		_iniParser->setValue("VIDEO/zTexMaxSize", _zTexMaxSize->value());
		_iniParser->setValue("VIDEO/zStartupWindowed", _zStartupWindowed->currentIndex());
		_iniParser->setValue("GAME/scaleVideos", _scaleVideos->currentIndex());
		_iniParser->setValue("GAME/playLogoVideos", _playLogoVideos->currentIndex());

		// Resolution
		_iniParser->setValue("VIDEO/zVidResFullscreenX", _zVidResFullscreenX->value());
		_iniParser->setValue("VIDEO/zVidResFullscreenY", _zVidResFullscreenY->value());
		_iniParser->setValue("VIDEO/zVidResFullscreenBPP", _zVidResFullscreenBPP->value());
		_iniParser->setValue("RENDERER_D3D/zVidRefreshRate", _zVidRefreshRate->value());
	}

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */
