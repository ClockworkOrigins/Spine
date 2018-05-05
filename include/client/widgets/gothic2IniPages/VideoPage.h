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

#ifndef __SPINE_WIDGETS_GOTHIC2INIPAGES_VIDEOPAGE_H__
#define __SPINE_WIDGETS_GOTHIC2INIPAGES_VIDEOPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g2 {

	class VideoPage : public QWidget {
		Q_OBJECT

	public:
		VideoPage(QSettings * iniParser, QWidget * par);
		~VideoPage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QComboBox * _zFogDisabled;
		QComboBox * _zFogRadial;
		QComboBox * _zForceZBuffer;
		QComboBox * _zEnableTaskSwitch;
		QComboBox * _zSyncAmbientCol;
		QComboBox * _zVidEnableAntiAliasing;
		QComboBox * _zTexAnisotropicFiltering;
		QComboBox * _zSkyRenderFirst;
		QComboBox * _radeonHackAmbientColBug;
		QComboBox * _geforce3HackWBufferBug;

		QSpinBox * _zVidDevice;
		QDoubleSpinBox * _zVidBrightness;
		QDoubleSpinBox * _zVidContrast;
		QDoubleSpinBox * _zVidGamma;
		QSpinBox * _zTexMaxSize;
		QComboBox * _zStartupWindowed;
		QComboBox * _scaleVideos;
		QComboBox * _playLogoVideos;

		QSpinBox * _zVidResFullscreenX;
		QSpinBox * _zVidResFullscreenY;
		QSpinBox * _zVidResFullscreenBPP;
		QSpinBox * _zVidRefreshRate;
	};

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC2INIPAGES_VIDEOPAGE_H__ */
