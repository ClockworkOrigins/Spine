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

#ifndef __SPINE_WIDGETS_GOTHIC1INIPAGES_VISUALIZATIONPAGE_H__
#define __SPINE_WIDGETS_GOTHIC1INIPAGES_VISUALIZATIONPAGE_H__

#include <QWidget>

class QLineEdit;
class QSettings;

namespace spine {
namespace widgets {
namespace g1 {

	class VisualizationPage : public QWidget {
		Q_OBJECT

	public:
		VisualizationPage(QSettings * iniParser, QWidget * par);
		~VisualizationPage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QLineEdit * _zDayColor0;
		QLineEdit * _zDayColor1;
		QLineEdit * _zDayColor2;
		QLineEdit * _zDayColor3;
	};

} /* namespace g1 */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC1INIPAGES_VISUALIZATIONPAGE_H__ */
