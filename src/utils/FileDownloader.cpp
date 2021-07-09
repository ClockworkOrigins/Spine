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

#include "FileDownloader.h"

#include "utils/Compression.h"
#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Hashing.h"

#include "utils/ErrorReporting.h"

#include "boost/iostreams/filter/zlib.hpp"

#include "clockUtils/log/Log.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProcess>
#include <QSettings>
#include <QtConcurrentRun>

#include "zipper/unzipper.h"

#ifdef Q_OS_WIN
	#include <Windows.h>
	#include <shellapi.h>
#endif

using namespace spine::utils;

FileDownloader::FileDownloader(QUrl url, QString targetDirectory, QString fileName, QString hash, QObject * par) : FileDownloader(url, url, targetDirectory, fileName, hash, par) {
	connect(this, &FileDownloader::retry, this, &FileDownloader::startDownload, Qt::QueuedConnection); // queue to process potential errors earlier so they get blocked
}

FileDownloader::FileDownloader(QUrl url, QUrl fallbackUrl, QString targetDirectory, QString fileName, QString hash, QObject * par) : QObject(par), _url(url), _fallbackUrl(fallbackUrl), _targetDirectory(targetDirectory), _fileName(fileName), _hash(hash), _filesize(-1), _outputFile(nullptr), _finished(false), _retried(false), _blockErrors(false) {
	connect(this, &FileDownloader::retry, this, &FileDownloader::startDownload, Qt::QueuedConnection); // queue to process potential errors earlier so they get blocked
}

FileDownloader::~FileDownloader() {
	delete _outputFile;
}

void FileDownloader::requestFileSize() {
	const QNetworkRequest request(_url);
	auto * networkAccessManager = new QNetworkAccessManager(this);
	QNetworkReply * reply = networkAccessManager->head(request);
	reply->setReadBufferSize(Config::downloadRate * 8);
	connect(reply, &QNetworkReply::sslErrors, this, &FileDownloader::sslErrors);
	connect(reply, &QNetworkReply::finished, this, &FileDownloader::determineFileSize);
	connect(reply, &QNetworkReply::finished, networkAccessManager, &QObject::deleteLater);
	connect(this, &FileDownloader::abort, reply, &QNetworkReply::abort);
}

QString FileDownloader::getFileName() const {
	return _fileName;
}

void FileDownloader::cancel() {
	emit abort();
}

void FileDownloader::startDownload() {
	_blockErrors = false;
	
	if (Config::extendedLogging) {
		LOGINFO("Starting Download of file " << _fileName.toStdString() << " from " << _url.toString().toStdString())
	}
	QDir dir(_targetDirectory);
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		if (!b) {
			emit downloadFinished();
			emit fileFailed(DownloadError::UnknownError);
			return;
		}
	}
	QString realName = _fileName;
	if (QFileInfo(realName).suffix() == "z") {
		realName.chop(2);
	}
	if (QFileInfo::exists(_targetDirectory + "/" + realName)) {
		QEventLoop hashLoop;
		QFutureWatcher<bool> watcher;
		connect(&watcher, &QFutureWatcher<bool>::finished, &hashLoop, &QEventLoop::quit);
		QFuture<bool> f = QtConcurrent::run([&]() {
			return Hashing::checkHash(_targetDirectory + "/" + realName, _hash);
		});
		watcher.setFuture(f);
		hashLoop.exec();
		
		if (f.result()) {
			if (Config::extendedLogging) {
				LOGINFO("Skipping file as it already exists")
			}
			if (_filesize == -1) {
				_finished = true;
				
				QEventLoop loop;
				connect(this, &FileDownloader::fileSizeDetermined, &loop, &QEventLoop::quit);
				requestFileSize();
				loop.exec();
			}
			emit downloadProgress(_filesize);

			emit downloadFinished();
			emit fileSucceeded();
			return;
		}
	}

	if (_fileName.contains("directx_Jun2010_redist.exe", Qt::CaseInsensitive) && Config::IniParser->value("INSTALLATION/DirectX", true).toBool()) {
		if (_filesize == -1) {
			_finished = true;
			
			QEventLoop loop;
			connect(this, &FileDownloader::fileSizeDetermined, &loop, &QEventLoop::quit);
			requestFileSize();
			loop.exec();
		}
		emit downloadProgress(_filesize);

		emit downloadFinished();
		emit fileSucceeded();
		return;
	}
	_outputFile = new QFile(_targetDirectory + "/" + _fileName);
	if (!_outputFile->open(QIODevice::WriteOnly)) {
		if (Config::extendedLogging) {
			LOGINFO("Can't open file for output")
		}

		emit downloadFinished();
		emit fileFailed(DownloadError::UnknownError);
		return;
	}
	const QNetworkRequest request(_url);
	auto * networkAccessManager = new QNetworkAccessManager(this);
	QNetworkReply * reply = networkAccessManager->get(request);
	reply->setReadBufferSize(Config::downloadRate * 8);
	connect(reply, &QNetworkReply::downloadProgress, this, &FileDownloader::updateDownloadProgress);
	connect(reply, &QNetworkReply::readyRead, this, &FileDownloader::writeToFile);
	connect(reply, &QNetworkReply::finished, this, &FileDownloader::fileDownloaded);
	connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &FileDownloader::networkError);
	connect(reply, &QNetworkReply::sslErrors, this, &FileDownloader::sslErrors);
	connect(this, &FileDownloader::abort, reply, &QNetworkReply::abort);
	connect(reply, &QNetworkReply::finished, networkAccessManager, &QObject::deleteLater);
	emit startedDownload(_fileName);
}

void FileDownloader::updateDownloadProgress(qint64 bytesReceived, qint64) {
	emit downloadProgress(bytesReceived);
}

void FileDownloader::fileDownloaded() {
	auto * reply = dynamic_cast<QNetworkReply *>(sender());

	emit downloadFinished();

	const auto err = reply->error();
	
	if (err == QNetworkReply::NetworkError::NoError) {
		if (Config::extendedLogging) {
			LOGINFO("Uncompressing file")
		}
		const QByteArray data = reply->readAll(); // the rest
		_outputFile->write(data);
		_outputFile->close();
		
		delete _outputFile;
		_outputFile = nullptr;

		uncompressAndHash();		
	} else {
		const auto fileSize = _outputFile->size();

		_outputFile->close();
		
		_outputFile->remove();
		
		delete _outputFile;
		_outputFile = nullptr;
		
		if (_retried) {
			LOGERROR("Unknown Error: " << reply->error() << ", " << q2s(reply->errorString()))
			if (reply->error() != QNetworkReply::OperationCanceledError) {
				ErrorReporting::report(QString("Unknown Error during download: %1, %2 (%3)").arg(reply->error()).arg(reply->errorString()).arg(_url.toString()));
			}
			emit fileFailed(DownloadError::UnknownError);
		} else {
			delete _outputFile;
			_outputFile = nullptr;
			
			_retried = true;
			_blockErrors = true;

			_url = _fallbackUrl;
			
			emit downloadProgress(-fileSize);

			emit retry();
		}
	}
	reply->deleteLater();
}

void FileDownloader::determineFileSize() {
	const auto reply = dynamic_cast<QNetworkReply *>(sender());

	const auto err = reply->error();
	
	if (err != QNetworkReply::NoError) {
		if (_url != _fallbackUrl) {
			_url = _fallbackUrl;
			requestFileSize();
		} else {
			emit fileFailed(DownloadError::NetworkError);
		}
		reply->deleteLater();
		return;
	}
	
	const qlonglong filesize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
	reply->deleteLater();
	_filesize = filesize;

	if (_finished) {
		emit fileSizeDetermined();
	} else {
		emit totalBytes(_filesize);
	}
}

void FileDownloader::writeToFile() {
	auto * reply = dynamic_cast<QNetworkReply *>(sender());
	const QByteArray data = reply->readAll();
	_outputFile->write(data);
	const QFileDevice::FileError err = _outputFile->error();
	if (err != QFileDevice::NoError) {
		reply->abort();
		emit downloadFinished();
		emit fileFailed(err == QFileDevice::ResizeError || err == QFileDevice::ResourceError ? DownloadError::DiskSpaceError : DownloadError::UnknownError);
	}
}

void FileDownloader::networkError(QNetworkReply::NetworkError err) {
	if (_blockErrors) return;
	
	emit downloadFinished();
	
	if (err == QNetworkReply::NetworkError::OperationCanceledError) {
		emit fileFailed(DownloadError::CanceledError);
	} else {
		emit fileFailed(DownloadError::NetworkError);
	}
}

void FileDownloader::sslErrors(const QList<QSslError> & errors) {
	auto * reply = dynamic_cast<QNetworkReply *>(sender());
	
	for (const auto & err : errors) {
		if (err.error() == QSslError::SelfSignedCertificate) {
			reply->ignoreSslErrors();
			continue;
		}
		LOGINFO(err.error() << " - " << q2s(err.errorString()) << " - " << q2s(err.certificate().issuerDisplayName()))
	}
}

void FileDownloader::uncompressAndHash() {
	QtConcurrent::run([this]() {
		// try a sleep in case file handle is closed with some delay
		// for some reason it happens sometimes that uncompressing fails, but we don't have any meaningful error message
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		
		QFileInfo fi(_fileName);
		const QString fileNameBackup = _fileName;

		auto suffix = fi.suffix();
		
		// compressed files always end with .z
		// in this case, uncompress, drop file extension and proceeed		
		if (suffix.compare("z", Qt::CaseInsensitive) == 0) {
			try {
				Compression::uncompress(_targetDirectory + "/" + _fileName, true); // remove compressed download now
			} catch (boost::iostreams::zlib_error & e) {
				LOGERROR("Exception: " << e.what())
				ErrorReporting::report(QString("Uncompressing of %1 failed: %2 (%3)").arg(_fileName).arg(e.what()).arg(_url.toString()));
			}
			_fileName.chop(2);
		}

		fi = QFileInfo(_fileName);
		suffix = fi.suffix();

		// zip case
		if (suffix.compare("zip", Qt::CaseInsensitive) == 0) {
			handleZip();
			
			return;
		}
		
		const bool b = Hashing::checkHash(_targetDirectory + "/" + _fileName, _hash);
		if (b) {
			if (_fileName.startsWith("vc") && _fileName.endsWith(".exe")) {
				handleVcRedist();
			} else if (_fileName == "directx_Jun2010_redist.exe") {
				handleDirectX();
			} else {
				emit fileSucceeded();
			}
		} else {
			LOGERROR("Hash invalid: " << _fileName.toStdString())
			emit fileFailed(DownloadError::HashError);
			ErrorReporting::report(QString("Hash invalid: %1 (%2)").arg(_fileName).arg(_url.toString()));
		}
	});
}

void FileDownloader::handleZip() {
	const auto fullPath = _targetDirectory + "/" + _fileName;
	{
		const bool b = Hashing::checkHash(fullPath, _hash);
		if (!b) {
			LOGERROR("Hash invalid: " << _fileName.toStdString())
			emit fileFailed(DownloadError::HashError);
			ErrorReporting::report(QString("Hash invalid: %1 (%2)").arg(_fileName).arg(_url.toString()));
			return;
		}
	}

	{
		zipper::Unzipper unzipper(q2s(fullPath));
		const bool b = unzipper.extract(q2s(_targetDirectory));
		if (!b) {
			LOGERROR("Unzipping failed: " << _fileName.toStdString())
			emit fileFailed(DownloadError::UnknownError);
			ErrorReporting::report(QString("Unzipping failed: %1 (%2)").arg(_fileName).arg(_url.toString()));
			return;
		}				
	}

	QFile(fullPath).remove();

	if (!QFileInfo::exists(_targetDirectory + "/.manifest")) {
		LOGERROR("Archive doesn't contain manifest: " << _fileName.toStdString())
		emit fileFailed(DownloadError::UnknownError);
		ErrorReporting::report(QString("Archive doesn't contain manifest: %1 (%2)").arg(_fileName).arg(_url.toString()));
		return;
	}

	QFile manifest(_targetDirectory + "/.manifest");
	if (!manifest.open(QIODevice::ReadOnly)) {
		LOGERROR("Manifest can't be opened: " << _fileName.toStdString())
		emit fileFailed(DownloadError::UnknownError);
		return;
	}

	QList<QPair<QString, QString>> files;
	
	QTextStream ts(&manifest);

	while (!ts.atEnd()) {
		const QString line = ts.readLine();
		
		if (line.isEmpty()) continue;

		const auto split = line.split(";");

		if (split.count() != 2) continue;

		files.append(qMakePair(split[0], split[1]));
	}
	manifest.close();

	QFile(_targetDirectory + "/.manifest").remove();

	emit unzippedArchive(_fileName + ".z", files);
	emit fileSucceeded();
}

void FileDownloader::handleVcRedist() {
#ifdef Q_OS_WIN
	if (Config::extendedLogging) {
		LOGINFO("Starting Visual Studio Redistributable")
	}
	SHELLEXECUTEINFO shExecInfo = { 0 };
	shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExecInfo.hwnd = nullptr;
	shExecInfo.lpVerb = "runas";
	char file[1024];
	QString qf = _targetDirectory + "/" + _fileName;
	qf = qf.replace("\0", "");
	strcpy(file, qf.toUtf8().constData());
	shExecInfo.lpFile = file;
	shExecInfo.lpParameters = "/q /norestart";
	char directory[1024];
	strcpy(directory, _targetDirectory.replace("\0", "").toUtf8().constData());
	shExecInfo.lpDirectory = directory;
	shExecInfo.nShow = SW_SHOWNORMAL;
	shExecInfo.hInstApp = nullptr;
	ShellExecuteEx(&shExecInfo);
	const int result = WaitForSingleObject(shExecInfo.hProcess, INFINITE);
	if (result != 0) {
		LOGERROR("Execute failed: " << _fileName.toStdString())
		emit fileFailed(DownloadError::UnknownError);
	} else {
		if (Config::extendedLogging) {
			LOGINFO("Download succeeded")
		}
		emit fileSucceeded();
	}
#endif
}

void FileDownloader::handleDirectX() {
#ifdef Q_OS_WIN
	if (Config::extendedLogging) {
		LOGINFO("Starting DirectX Redistributable")
	}
	bool dxSuccess = true;
	{
		SHELLEXECUTEINFO shExecInfo = { 0 };
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExecInfo.hwnd = nullptr;
		shExecInfo.lpVerb = "runas";
		char file[1024];
		QString qf = _targetDirectory + "/" + _fileName;
		qf = qf.replace("\0", "");
		strcpy(file, qf.toUtf8().constData());
		shExecInfo.lpFile = file;
		char parameters[1024];
		qf = "/Q /T:\"" + _targetDirectory + "/directX\"";
		qf = qf.replace("\0", "");
		strcpy(parameters, qf.toUtf8().constData());
		shExecInfo.lpParameters = parameters;
		char directory[1024];
		strcpy(directory, _targetDirectory.replace("\0", "").toUtf8().constData());
		shExecInfo.lpDirectory = directory;
		shExecInfo.nShow = SW_SHOWNORMAL;
		shExecInfo.hInstApp = nullptr;
		ShellExecuteEx(&shExecInfo);
		const int result = WaitForSingleObject(shExecInfo.hProcess, INFINITE);
		if (result != 0) {
			dxSuccess = false;
			LOGERROR("Execute failed: " << _fileName.toStdString())
			emit fileFailed(DownloadError::UnknownError);
		}
	}
	if (dxSuccess) {
		SHELLEXECUTEINFO shExecInfo = { 0 };
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExecInfo.hwnd = nullptr;
		shExecInfo.lpVerb = "runas";
		char file[1024];
		QString qf = _targetDirectory + "/directX/DXSETUP.exe";
		qf = qf.replace("\0", "");
		strcpy(file, qf.toUtf8().constData());
		shExecInfo.lpFile = file;
		shExecInfo.lpParameters = "/silent";
		char directory[1024];
		qf = _targetDirectory + "/directX";
		qf = qf.replace("\0", "");
		strcpy(directory, qf.toUtf8().constData());
		shExecInfo.lpDirectory = directory;
		shExecInfo.nShow = SW_SHOWNORMAL;
		shExecInfo.hInstApp = nullptr;
		ShellExecuteEx(&shExecInfo);
		const int result = WaitForSingleObject(shExecInfo.hProcess, INFINITE);
		if (result != 0) {
			dxSuccess = false;
			LOGERROR("Execute failed: " << _fileName.toStdString())
			emit fileFailed(DownloadError::UnknownError);
		}
	}
	if (dxSuccess) {
		if (Config::extendedLogging) {
			LOGINFO("Download succeeded")
		}
		emit fileSucceeded();
	}
	QDir(_targetDirectory + "/directX/").removeRecursively();
	Config::IniParser->setValue("INSTALLATION/DirectX", true);
#endif
}
