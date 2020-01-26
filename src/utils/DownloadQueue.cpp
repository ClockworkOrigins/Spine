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

#include "utils/DownloadQueue.h"

#include "utils/MultiFileDownloader.h"

#include <QMainWindow>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>

using namespace spine;
using namespace spine::utils;

DownloadQueue * DownloadQueue::instance = nullptr;

DownloadQueue::DownloadQueue(QMainWindow * mainWindow) : _running(false), _totalBytes(0), _downloadedBytes(0) {
	instance = this;
	
#ifdef Q_OS_WIN
	QWinTaskbarButton * button = new QWinTaskbarButton(this);
	button->setWindow(mainWindow->windowHandle());

	_taskbarProgress = button->progress();
	_taskbarProgress->setMinimum(0);
	_taskbarProgress->setMaximum(0);
	_taskbarProgress->setValue(0);

	_taskbarProgress->hide();
#endif
}

DownloadQueue * DownloadQueue::getInstance() {
	return instance;
}

void DownloadQueue::add(MultiFileDownloader * downloader) {
	_totalBytesMap.insert(downloader, 0);
	_downloadedBytesMap.insert(downloader, 0);

	connect(downloader, &MultiFileDownloader::totalBytes, this, &DownloadQueue::updateTotalBytes);
	connect(downloader, &MultiFileDownloader::downloadProgress, this, &DownloadQueue::downloadedBytes);
	connect(downloader, &MultiFileDownloader::downloadSucceeded, this, &DownloadQueue::finishedDownload);
	connect(downloader, &MultiFileDownloader::downloadFailed, this, &DownloadQueue::finishedDownload);

	downloader->querySize();
}

void DownloadQueue::checkQueue() {
	if (_queue.isEmpty()) return;

	if (_running) return;

	auto downloader = _queue.dequeue();

	_running = true;
	downloader->startDownload();
}

void DownloadQueue::updateTotalBytes(qint64 bytes) {
	MultiFileDownloader * downloader = dynamic_cast<MultiFileDownloader *>(sender());

	_totalBytes -= _totalBytesMap[downloader];
	_totalBytes += bytes;
	
	_totalBytesMap[downloader] = bytes;

	_taskbarProgress->setMaximum(static_cast<int>(_totalBytes / 1024 + 1));
	
	_queue.enqueue(downloader);

	checkQueue();
}

void DownloadQueue::downloadedBytes(qint64 bytes) {
	const MultiFileDownloader * downloader = dynamic_cast<const MultiFileDownloader *>(sender());

	_downloadedBytes -= _downloadedBytesMap[downloader];
	_downloadedBytes += bytes;
	
	_downloadedBytesMap[downloader] = bytes;

	_taskbarProgress->setValue(static_cast<int>(_downloadedBytes / 1024 + 1));
}

void DownloadQueue::finishedDownload() {
	_totalBytesMap.clear();
	_downloadedBytesMap.clear();

	_running = false;

	sender()->deleteLater();

	checkQueue();
}
