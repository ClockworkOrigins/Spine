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

#include "utils/MultiFileDownloader.h"

#include <cassert>

#include "utils/FileDownloader.h"

using namespace spine::utils;

MultiFileDownloader::MultiFileDownloader(QObject * par) : QObject(par), _downloadStats(), _downloadQueue(), _counter(0), _maxSize(0), _downloadFilesCount(0) {
}

void MultiFileDownloader::addFileDownloader(FileDownloader * fileDownloader) {
	_downloadStats.insert(fileDownloader, qMakePair(0, 100));
	_downloadQueue.enqueue(fileDownloader);
}

void MultiFileDownloader::startDownloads(qint64 maxSize) {
	if (maxSize != 0) {
		_maxSize = maxSize;
		for (auto it = _downloadStats.begin(); it != _downloadStats.end(); ++it) {
			connect(it.key(), &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
			connect(it.key(), &FileDownloader::fileFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
			connect(it.key(), &FileDownloader::fileSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
			connect(it.key(), &FileDownloader::downloadFinished, this, &MultiFileDownloader::startDownloadInternal, Qt::UniqueConnection);
			connect(this, &MultiFileDownloader::abort, it.key(), &FileDownloader::abort, Qt::UniqueConnection);
			connect(it.key(), &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
		}
		startDownload();
	} else {
		_currentIndex = _downloadStats.begin();
		if (_currentIndex != _downloadStats.end()) {
			emit startedDownload(_currentIndex.key()->getFileName());
			connect(_currentIndex.key(), &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
			connect(_currentIndex.key(), &FileDownloader::totalBytes, this, &MultiFileDownloader::updateDownloadMax, Qt::UniqueConnection);
			connect(_currentIndex.key(), &FileDownloader::fileFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
			connect(_currentIndex.key(), &FileDownloader::fileSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
			connect(_currentIndex.key(), &FileDownloader::downloadFinished, this, &MultiFileDownloader::startDownloadInternal, Qt::UniqueConnection);
			connect(_currentIndex.key(), &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
			_currentIndex.key()->requestFileSize();
			connect(this, &MultiFileDownloader::abort, _currentIndex.key(), &FileDownloader::abort, Qt::UniqueConnection);
		}
	}
}

void MultiFileDownloader::querySize() {
	_currentIndex = _downloadStats.begin();
	if (_currentIndex != _downloadStats.end()) {
		connect(_currentIndex.key(), &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::totalBytes, this, &MultiFileDownloader::updateDownloadMax, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::fileFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::fileSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::downloadFinished, this, &MultiFileDownloader::startDownloadInternal, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
		_currentIndex.key()->requestFileSize();
		connect(this, &MultiFileDownloader::abort, _currentIndex.key(), &FileDownloader::abort, Qt::UniqueConnection);
	} else {
		emit totalBytes(0);
	}
}

void MultiFileDownloader::startDownload() {
	if (_downloadQueue.empty()) {
		emit downloadSucceeded();
		return;
	}
	
	FileDownloader * fd = _downloadQueue.dequeue();
	fd->startDownload();
}

void MultiFileDownloader::startDownloadInternal() {
	if (_downloadQueue.empty()) return;
	
	FileDownloader * fd = _downloadQueue.dequeue();
	fd->startDownload();
}

void MultiFileDownloader::updateDownloadProgress(qint64 bytesReceived) {
	FileDownloader * fd = dynamic_cast<FileDownloader *>(sender());
	assert(fd);
	_downloadStats[fd].first = bytesReceived;
	qint64 sum = 0;
	for (auto & downloadStat : _downloadStats) {
		sum += downloadStat.first;
	}
	emit downloadProgress(sum);
}

void MultiFileDownloader::updateDownloadMax(qint64 bytesTotal) {
	FileDownloader * fd = dynamic_cast<FileDownloader *>(sender());
	assert(fd);
	_maxSize += bytesTotal;
	_downloadStats[fd].second = bytesTotal;
	if (++_currentIndex != _downloadStats.end()) {
		connect(_currentIndex.key(), &FileDownloader::downloadProgress, this, &MultiFileDownloader::updateDownloadProgress, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::totalBytes, this, &MultiFileDownloader::updateDownloadMax, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::fileFailed, this, &MultiFileDownloader::downloadFailed, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::fileSucceeded, this, &MultiFileDownloader::finishedFile, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::downloadFinished, this, &MultiFileDownloader::startDownloadInternal, Qt::UniqueConnection);
		connect(_currentIndex.key(), &FileDownloader::startedDownload, this, &MultiFileDownloader::startedDownload, Qt::UniqueConnection);
		_currentIndex.key()->requestFileSize();
		connect(this, &MultiFileDownloader::abort, _currentIndex.key(), &FileDownloader::abort, Qt::UniqueConnection);
	} else {
		emit totalBytes(_maxSize);
	}
}

void MultiFileDownloader::finishedFile() {
	_downloadFilesCount++;
	if (_downloadFilesCount == int(_downloadStats.size())) {
		emit downloadSucceeded();
	}
}
