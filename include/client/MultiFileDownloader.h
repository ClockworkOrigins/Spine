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

#include <queue>

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace spine {

	class FileDownloader;
	enum class DownloadError;

	class MultiFileDownloader : public QObject {
		Q_OBJECT

	public:
		MultiFileDownloader(QObject * par);

		void addFileDownloader(FileDownloader * fileDownloader);
		void startDownloads(qint64 maxSize);

	signals:
		void downloadProgress(qint64);
		void totalBytes(qint64);
		void downloadSucceeded();
		void downloadFailed(DownloadError);
		void abort();
		void startedDownload(QString);

	private slots :
		void startDownload();
		void updateDownloadProgress(qint64 bytesReceived);
		void updateDownloadMax(qint64 bytesTotal);
		void finishedFile();

	private:
		std::map<FileDownloader *, std::pair<qint64, qint64>> _downloadStats;
		std::queue<FileDownloader *> _downloadQueue;
		std::map<FileDownloader *, std::pair<qint64, qint64>>::iterator _currentIndex;
		int _counter;
		qint64 _maxSize;
		int _downloadFilesCount;
	};

} /* namespace spine */
