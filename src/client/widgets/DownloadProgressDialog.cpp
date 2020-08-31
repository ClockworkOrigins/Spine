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

#include "widgets/DownloadProgressDialog.h"

#include "utils/Conversion.h"
#include "utils/FileDownloader.h"
#include "utils/MultiFileDownloader.h"

#include <QApplication>
#include <QMainWindow>
#include <QStyle>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

using namespace spine::utils;
using namespace spine::widgets;

DownloadProgressDialog::DownloadProgressDialog(MultiFileDownloader * downloader, QString labelText, qint64 min, qint64 max, qint64 maxSize, QMainWindow * mainWindow) : QProgressDialog(QApplication::tr(labelText.toStdString().c_str()), QApplication::tr("Cancel"), min, max), _downloadSuccessful(false), _finished(false), _downloader(downloader), _taskbarProgress(nullptr), _labelText(labelText), _currentValue("0 B"), _maximumValue("0 B"), _maxSize(maxSize), _currentFileName(), _lastError(DownloadError::NoError) {
	connect(downloader, &MultiFileDownloader::totalBytes, this, &DownloadProgressDialog::setMaximum);
	connect(downloader, &MultiFileDownloader::downloadProgress, this, &DownloadProgressDialog::setValue);
	connect(downloader, &MultiFileDownloader::downloadSucceeded, this, &DownloadProgressDialog::downloadSucceeded);
	connect(downloader, &MultiFileDownloader::downloadFailed, this, &DownloadProgressDialog::downloadFailed);
	connect(downloader, &MultiFileDownloader::startedDownload, this, &DownloadProgressDialog::downloadFile);
	connect(downloader, &MultiFileDownloader::totalBytes, downloader, &MultiFileDownloader::startDownload);
	connect(this, &DownloadProgressDialog::canceled, downloader, &MultiFileDownloader::abort);

#ifdef Q_OS_WIN
	QWinTaskbarButton * button = new QWinTaskbarButton(this);
	button->setWindow(mainWindow->windowHandle());

	_taskbarProgress = button->progress();
	_taskbarProgress->setMinimum(min);
	_taskbarProgress->setMaximum(max / 1024 + 1);
	_taskbarProgress->setValue(min);
#endif
	
	if (_maxSize > 0) {
		setMaximum(_maxSize);
	}
}

bool DownloadProgressDialog::hasDownloadSucceeded() const {
	return _downloadSuccessful;
}

DownloadError DownloadProgressDialog::getLastError() const {
	return _lastError;
}

int DownloadProgressDialog::exec() {
	_finished = false;
	_downloader->querySize();
#ifdef Q_OS_WIN
	_taskbarProgress->show();
#endif
	const int result =  QProgressDialog::exec();
#ifdef Q_OS_WIN
	_taskbarProgress->hide();
#endif
	return result;
}

void DownloadProgressDialog::setValue(qint64 value) {
	QProgressDialog::setValue(static_cast<int>(value / 1024));
#ifdef Q_OS_WIN
	_taskbarProgress->setValue(static_cast<int>(value / 1024));
#endif
	_currentValue = utils::byteToString(value);
	setLabelText(QApplication::tr(_labelText.toStdString().c_str()).arg(_currentFileName) + "\n" + QApplication::tr("DownloadedBytes").arg(_currentValue, _maximumValue));
}

void DownloadProgressDialog::setMaximum(qint64 max) {
	QProgressDialog::setMaximum(static_cast<int>(max / 1024) + 1);
#ifdef Q_OS_WIN
	_taskbarProgress->setMaximum(static_cast<int>(max / 1024) + 1);
#endif
	_maximumValue = utils::byteToString(max);
	setLabelText(QApplication::tr(_labelText.toStdString().c_str()).arg(_currentFileName) + "\n" + QApplication::tr("DownloadedBytes").arg(_currentValue, _maximumValue));
}

void DownloadProgressDialog::downloadSucceeded() {
	_downloadSuccessful = true;
	_finished = true;
	accept();
}

void DownloadProgressDialog::downloadFailed(DownloadError err) {
	_downloadSuccessful = false;
	_finished = true;
	_lastError = err;
	reject();
}

void DownloadProgressDialog::downloadFile(QString fileName) {
	_currentFileName = fileName;
	setLabelText(QApplication::tr(_labelText.toStdString().c_str()).arg(_currentFileName) + "\n" + QApplication::tr("DownloadedBytes").arg(_currentValue).arg(_maximumValue));
}

void DownloadProgressDialog::closeEvent(QCloseEvent * evt) {
	emit _downloader->abort();
	QProgressDialog::closeEvent(evt);
}
