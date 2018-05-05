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

#ifndef __SPINE_WIDGETS_GOTHIC1INIWIDGET_H__
#define __SPINE_WIDGETS_GOTHIC1INIWIDGET_H__

#include <QWidget>

class QSettings;
class QTabWidget;

namespace spine {
namespace widgets {
namespace g1 {
	class ControlsPage;
	class EnginePage;
	class GamePage;
	class PerformancePage;
	class SoundPage;
	class VideoPage;
	class VisualizationPage;
} /* namespace g1 */

	class Gothic1IniWidget : public QWidget {
		Q_OBJECT

	public:
		Gothic1IniWidget(QString directory, QWidget * par);
		~Gothic1IniWidget();

		void accept();
		void reject();

	private slots:
		void backup();
		void restore();

	private:
		QSettings * _iniParser;
		QString _directory;
		QTabWidget * _tabWidget;
		g1::GamePage * _gamePage;
		g1::PerformancePage * _performancePage;
		g1::EnginePage * _enginePage;
		g1::VisualizationPage * _visualizationPage;
		g1::VideoPage * _videoPage;
		g1::SoundPage * _soundPage;
		g1::ControlsPage * _controlsPage;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC1INIWIDGET_H__ */
