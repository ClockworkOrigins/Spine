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

#pragma once

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g2 {

	class PerformancePage : public QWidget {
		Q_OBJECT

	public:
		PerformancePage(QSettings * iniParser, QWidget * par);
		~PerformancePage();

		void reject();
		void accept();

		void updateSettings(QSettings * iniParser);

	private:
		QSettings * _iniParser;
		QComboBox * _recalc;
		QDoubleSpinBox * _modelDetail;
		QComboBox * _bloodDetail;
		QComboBox * _animatedWindows;
		QComboBox * _zDetailTexturesEnabled;
		QComboBox * _zSubdivSurfacesEnabled;

		QComboBox * _skyEffects;
		QComboBox * _zAmbientPFXEnabled;
		QComboBox * _zAmbientVobsEnabled;
		QComboBox * _zVobPointLight;
		QComboBox * _zHighLightScale;
		QComboBox * _zEnvMappingEnabled;

		QComboBox * _zWindEnabled;
		QSpinBox * _zWindCycleTime;
		QSpinBox * _zWindCycleTimeVar;
		QSpinBox * _zWindStrength;
		QSpinBox * _zWindStrengthVar;
		QDoubleSpinBox * _zWindAngleVelo;
		QDoubleSpinBox * _zWindAngleVeloVar;

		QSpinBox * _sightValue;
		QComboBox * _zFarClipAlphaFade;
		QSpinBox * _zVobFarClipZScale;

		QComboBox * _zWaterAniEnabled;
		QDoubleSpinBox * _zWaterEnvMapAniFPS;
		QComboBox * _zCloudShadowScale;

		QComboBox * _zSmoothTimer;
		QComboBox * _zSmoothModelRootNode;
	};

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */
