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

#ifndef __SPINE_WIDGETS_GOTHIC2INIWIDGET_H__
#define __SPINE_WIDGETS_GOTHIC2INIWIDGET_H__

#include <QWidget>

class QSettings;
class QTabWidget;

namespace spine {
namespace widgets {
namespace g2 {
	class ControlsPage;
	class EnginePage;
	class GamePage;
	class PerformancePage;
	class SoundPage;
	class VideoPage;
	class VisualizationPage;
} /* namespace g1 */

	class Gothic2IniWidget : public QWidget {
		Q_OBJECT

	public:
		Gothic2IniWidget(QString directory, QWidget * par);
		~Gothic2IniWidget();

		void accept();
		void reject();

	private slots:
		void backup();
		void restore();

	private:
		QSettings * _iniParser;
		QString _directory;
		QTabWidget * _tabWidget;
		g2::GamePage * _gamePage;
		g2::PerformancePage * _performancePage;
		g2::EnginePage * _enginePage;
		g2::VisualizationPage * _visualizationPage;
		g2::VideoPage * _videoPage;
		g2::SoundPage * _soundPage;
		g2::ControlsPage * _controlsPage;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC2INIWIDGET_H__ */
