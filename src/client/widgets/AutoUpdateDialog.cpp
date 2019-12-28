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

#include "widgets/AutoUpdateDialog.h"

#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"
#include "WindowsExtensions.h"

#include "common/MessageStructs.h"

#include "widgets/DownloadProgressDialog.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <ShellAPI.h>
#endif

namespace spine {
namespace widgets {

	AutoUpdateDialog::AutoUpdateDialog(QMainWindow * mainWindow) : QDialog(), _mainWindow(mainWindow), _manuallyChecking(false) {
		QVBoxLayout * l = new QVBoxLayout();

		QLabel * lbl = new QLabel(QApplication::tr("CheckingForUpdates"), this);
		UPDATELANGUAGESETTEXT(lbl, "CheckingForUpdates");

		l->addWidget(lbl);

		setLayout(l);

		setWindowTitle(QApplication::tr("CheckForUpdates"));
		UPDATELANGUAGESETWINDOWTITLE(this, "CheckForUpdates");

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	}

	int AutoUpdateDialog::exec() {
		_manuallyChecking = true;
		checkForUpdate();
		return QDialog::Accepted;
	}

	void AutoUpdateDialog::checkForUpdate() {
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage checkForUpdate #1: " << getPRAMValue());
#endif
		clockUtils::sockets::TcpSocket sock;
		if (sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000) != clockUtils::ClockError::SUCCESS) {
			accept();
			return;
		}

		common::UpdateRequestMessage urm;

		urm.majorVersion = VERSION_MAJOR;
		urm.minorVersion = VERSION_MINOR;
		urm.patchVersion = VERSION_PATCH;

		std::string serialized = urm.SerializeBlank();
		if (sock.writePacket(serialized) != clockUtils::ClockError::SUCCESS) {
			accept();
			return;
		}
		if (sock.receivePacket(serialized) != clockUtils::ClockError::SUCCESS) {
			accept();
			return;
		}
		common::Message * m = common::Message::DeserializeBlank(serialized);
		if (!m || m->type != common::MessageType::UPDATEFILES) {
			delete m;
			accept();
			return;
		}
		common::UpdateFilesMessage * ufm = dynamic_cast<common::UpdateFilesMessage *>(m);

		if (ufm->files.empty()) {
			if (_manuallyChecking) {
				QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("VersionUpToDate"), QApplication::tr("VersionUpToDateText"), QMessageBox::StandardButton::Ok);
				resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				resultMsg.exec();
			}
			delete ufm;
			accept();
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage checkForUpdate #3: " << getPRAMValue());
#endif
			return;
		}
		QString exeFileName = qApp->applicationDirPath() + "/" + qApp->applicationName();
#ifdef Q_OS_WIN
		if (!IsRunAsAdmin()) {
			QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("UpdateAdminInfo"), QApplication::tr("UpdateAdminInfoText"), QMessageBox::StandardButton::Ok);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.exec();

			const int result = int(::ShellExecuteA(nullptr, "runas", exeFileName.toUtf8().constData(), nullptr, nullptr, SW_SHOWNORMAL));
			if (result > 32) { // no error
				qApp->quit();
			}
			return;
		}
#endif
		MultiFileDownloader * mfd = new MultiFileDownloader(this);
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
		QStringList removedFiles;
		QStringList removeFiles;
		for (const auto p : ufm->files) {
			QString filename = QString::fromStdString(p.first);
			if (QFileInfo(filename).suffix() == "z") {
				filename.resize(filename.size() - 2);
			}
			QFile f(qApp->applicationDirPath() + "/../" + filename);
			QString shortFilename = filename;
			shortFilename.resize(shortFilename.length() - 1);
			if (QFile(qApp->applicationDirPath() + "/../" + shortFilename).exists()) {
				QFile(qApp->applicationDirPath() + "/../" + shortFilename).remove();
			}
			if (f.rename(qApp->applicationDirPath() + "/../" + shortFilename)) {
				removedFiles << filename;
				removeFiles << shortFilename;
			}
			QFileInfo fi(QString::fromStdString(p.first));
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/Spine/" + QString::fromStdString(p.first)), qApp->applicationDirPath() + "/../" + fi.path(), fi.fileName(), QString::fromStdString(p.second), mfd);
			mfd->addFileDownloader(fd);
		}
		delete ufm;
		DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
		if (progressDlg.hasDownloadSucceeded()) {
			QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("UpdateRestartInfo"), QApplication::tr("UpdateRestartInfoText"), QMessageBox::StandardButton::Ok);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.exec();
			QProcess::startDetached(qApp->applicationDirPath() + "/" + qApp->applicationName(), removeFiles, qApp->applicationDirPath());
			qApp->quit();
		} else {
			for (const QString & s : removedFiles) {
				const QString & filename = s;
				QString shortFilename = filename;
				shortFilename.chop(1);
				QFile f(qApp->applicationDirPath() + "/../" + shortFilename);
				if (f.exists()) {
					QFile(qApp->applicationDirPath() + "/../" + filename).remove();
					f.rename(qApp->applicationDirPath() + "/../" + filename);
				}
			}
		}
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage checkForUpdate #2: " << getPRAMValue());
#endif
	}

} /* namespace widgets */
} /* namespace spine */
