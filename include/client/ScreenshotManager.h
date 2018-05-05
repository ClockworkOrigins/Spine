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

#ifndef __SPINE_SCREENSHOTMANAGER_H__
#define __SPINE_SCREENSHOTMANAGER_H__

#include <cstdint>

#include <QObject>
#include <QString>

namespace std {
	class thread;
} /* namespace std */

namespace spine {
namespace widgets {
	class LocationSettingsWidget;
} /* namespace widgets */

	class ScreenshotManager : public QObject {
		Q_OBJECT

	public:
		ScreenshotManager(widgets::LocationSettingsWidget * locationSettingsWidget, QObject * par);
		~ScreenshotManager();

		void start(int32_t modID);
		void stop();

	private slots:
		void setScreenshotDirectory(QString screenshotDirectory);

	private:
		bool _running;
		std::thread * _workerThread;
		QString _screenshotDirectory;
		int32_t _modID;

		void execute();
		void takeScreenshot();
	};

} /* namespace spine */

#endif /* __SPINE_SCREENSHOTMANAGER_H__ */
