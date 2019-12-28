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

#include "MultiFileDownloader.h"

#include <cassert>

#include "FileDownloader.h"

namespace spine {

	MultiFileDownloader::MultiFileDownloader(QObject * par) : QObject(par), _downloadStats(), _downloadQueue(), _counter(0), _maxSize(0), _downloadFilesCount(0) {
	}

	void MultiFileDownloader::addFileDownloader(FileDownloader * fileDownloader) {
		_downloadStats.insert(std::make_pair(fileDownloader, std::make_pair(0, 100)));
		_downloadQueue.push(fileDownloader);
	}

	void MultiFileDownloader::startDownloads(qint64 maxSize) {
		if (maxSize != 0) {
			_maxSize = maxSize;
			for (auto & downloadStat : _downloadStats) {
				connect(downloadStat.first, &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
				connect(downloadStat.first, &FileDownloader::downloadFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
				connect(downloadStat.first, &FileDownloader::downloadSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
				connect(downloadStat.first, &FileDownloader::downloadSucceeded, this, &MultiFileDownloader::startDownload, Qt::UniqueConnection);
				connect(this, SIGNAL(abort()), downloadStat.first, SIGNAL(abort()), Qt::UniqueConnection);
				connect(downloadStat.first, &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
			}
			startDownload();
		} else {
			_currentIndex = _downloadStats.begin();
			if (_currentIndex != _downloadStats.end()) {
				emit startedDownload(_currentIndex->first->getFileName());
				connect(_currentIndex->first, &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
				connect(_currentIndex->first, &FileDownloader::totalBytes, this, &MultiFileDownloader::updateDownloadMax, Qt::UniqueConnection);
				connect(_currentIndex->first, &FileDownloader::downloadFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
				connect(_currentIndex->first, &FileDownloader::downloadSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
				connect(_currentIndex->first, &FileDownloader::downloadSucceeded, this, &MultiFileDownloader::startDownload, Qt::UniqueConnection);
				connect(_currentIndex->first, &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
				_currentIndex->first->requestFileSize();
				connect(this, SIGNAL(abort()), _currentIndex->first, SIGNAL(abort()), Qt::UniqueConnection);
			}
		}
	}

	void MultiFileDownloader::startDownload() {
		if (!_downloadQueue.empty()) {
			FileDownloader * fd = _downloadQueue.front();
			_downloadQueue.pop();
			fd->startDownload();
		}
	}

	void MultiFileDownloader::updateDownloadProgress(qint64 bytesReceived) {
		FileDownloader * fd = dynamic_cast<FileDownloader *>(sender());
		assert(fd);
		_downloadStats[fd].first = bytesReceived;
		qint64 sum = 0;
		for (auto & downloadStat : _downloadStats) {
			sum += downloadStat.second.first;
		}
		emit downloadProgress(sum);
	}

	void MultiFileDownloader::updateDownloadMax(qint64 bytesTotal) {
		FileDownloader * fd = dynamic_cast<FileDownloader *>(sender());
		assert(fd);
		_maxSize += bytesTotal;
		_downloadStats[fd].second = bytesTotal;
		emit totalBytes(_maxSize);
		if (++_currentIndex != _downloadStats.end()) {
			connect(_currentIndex->first, &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
			connect(_currentIndex->first, &FileDownloader::totalBytes, this, &MultiFileDownloader::updateDownloadMax, Qt::UniqueConnection);
			connect(_currentIndex->first, &FileDownloader::downloadFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
			connect(_currentIndex->first, &FileDownloader::downloadSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
			connect(_currentIndex->first, &FileDownloader::downloadSucceeded, this, &MultiFileDownloader::startDownload, Qt::UniqueConnection);
			connect(_currentIndex->first, &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
			_currentIndex->first->requestFileSize();
			connect(this, SIGNAL(abort()), _currentIndex->first, SIGNAL(abort()), Qt::UniqueConnection);
		} else {
			if (!_downloadQueue.empty()) {
				startDownload();
			}
		}
	}

	void MultiFileDownloader::finishedFile() {
		_downloadFilesCount++;
		if (_downloadFilesCount == int(_downloadStats.size())) {
			emit downloadSucceeded();
		}
	}

} /* namespace spine */
