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

#ifndef __SPINE_WIDGETS_GOTHIC2INIPAGES_VISUALIZATIONPAGE_H__
#define __SPINE_WIDGETS_GOTHIC2INIPAGES_VISUALIZATIONPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g2 {

	class VisualizationPage : public QWidget {
		Q_OBJECT

	public:
		VisualizationPage(QSettings * iniParser, QWidget * par);
		~VisualizationPage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QComboBox * _zSkyDome;
		QComboBox * _zColorizeSky;
		QLineEdit * _zDayColor0;
		QLineEdit * _zDayColor1;
		QLineEdit * _zDayColor2;
		QLineEdit * _zDayColor3;
		QLineEdit * _zDayColor0_OW;
		QLineEdit * _zDayColor1_OW;
		QLineEdit * _zDayColor2_OW;
		QLineEdit * _zDayColor3_OW;
		QLineEdit * _zSunName;
		QSpinBox * _zSunSize;
		QSpinBox * _zSunAlpha;
		QDoubleSpinBox * _zSunMaxScreenBlendScale;
		QLineEdit * _zMoonName;
		QSpinBox * _zMoonSize;
		QSpinBox * _zMoonAlpha;
		QDoubleSpinBox * _zRainWindScale;
		QDoubleSpinBox * _zNearFogScale;
		QDoubleSpinBox * _zFarFogScale;
	};

} /* namespace g2 */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC2INIPAGES_VISUALIZATIONPAGE_H__ */
