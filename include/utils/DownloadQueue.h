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
// Copyright 2020 Clockwork Origins

#pragma once

#include <QMap>
#include <QObject>
#include <QQueue>

class QMainWindow;
class QWinTaskbarButton;
class QWinTaskbarProgress;

namespace spine {
namespace utils {

	class MultiFileDownloader;
	
	class DownloadQueue : public QObject {
		Q_OBJECT
		
	public:
		DownloadQueue();

		static DownloadQueue * getInstance();

		void add(MultiFileDownloader * downloader);

		void setWindow(QMainWindow * mainWindow);

	private slots:
		void updateTotalBytes(qint64 bytes);
		void downloadedBytes(qint64 bytes);
		void finishedDownload();

	private:
		static DownloadQueue * instance;
		
		QQueue<MultiFileDownloader *> _queue;
		QWinTaskbarButton * _taskbarButton;
		QWinTaskbarProgress * _taskbarProgress;
		bool _running;
		QMap<const MultiFileDownloader *, qint64> _totalBytesMap;
		QMap<const MultiFileDownloader *, qint64> _downloadedBytesMap;

		void checkQueue();
	};

} /* namespace utils */
} /* namespace spine */
