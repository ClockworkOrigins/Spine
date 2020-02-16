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

#include <QMap>
#include <QQueue>
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace spine {
namespace utils {

	class FileDownloader;
	enum class DownloadError;

	class MultiFileDownloader : public QObject {
		Q_OBJECT

	public:
		MultiFileDownloader(QObject * par);

		void addFileDownloader(FileDownloader * fileDownloader);
		void startDownloads(qint64 maxSize); // deprecated
		void startDownload();
		void querySize();
		void setSize(qint64 size);

	signals:
		void downloadProgress(qint64);
		void totalBytes(qint64);
		void downloadSucceeded();
		void downloadFailed(DownloadError);
		void abort();
		void startedDownload(QString);

	private slots :
		void startDownloadInternal();
		void updateDownloadProgress(qint64 bytesReceived);
		void updateDownloadMax(qint64 bytesTotal);
		void finishedFile();

	private:
		QMap<FileDownloader *, QPair<qint64, qint64>> _downloadStats;
		QQueue<FileDownloader *> _downloadQueue;
		QMap<FileDownloader *, QPair<qint64, qint64>>::iterator _currentIndex;
		int _counter;
		qint64 _maxSize;
		int _downloadFilesCount;
	};

} /* namespace utils */
} /* namespace spine */
