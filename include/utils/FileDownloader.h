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

#include <QNetworkReply>
#include <QObject>
#include <QUrl>

class QFile;
class QNetworkAccessManager;

namespace spine {
namespace utils {

	enum class DownloadError {
		NoError,
		UnknownError,
		DiskSpaceError,
		HashError,
		NetworkError,
		CanceledError
	};

	class FileDownloader : public QObject {
		Q_OBJECT

	public:
		FileDownloader(QUrl url, QString targetDirectory, QString fileName, QString hash, QObject * par);
		~FileDownloader();

		void startDownload();
		void requestFileSize();
		QString getFileName() const;

	signals:
		void downloadProgress(qint64);
		void totalBytes(qint64);
		void downloadSucceeded();
		void downloadFailed(DownloadError);
		void abort();
		void startedDownload(QString);

	private slots:
		void updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
		void fileDownloaded();
		void determineFileSize();
		void writeToFile();
		void networkError(QNetworkReply::NetworkError err);

	private:
		QNetworkAccessManager * _webAccessManager;
		QUrl _url;
		QString _targetDirectory;
		QString _fileName;
		QString _hash;
		qint64 _filesize;
		QFile * _outputFile;
	};

} /* namespace utils */
} /* namespace spine */
