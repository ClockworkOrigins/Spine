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
// Copyright 2019 Clockwork Origins

#include "launcher/Gothic2Launcher.h"

#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"

#include "utils/Config.h"
#include "utils/Database.h"
#include "utils/Hashing.h"
#include "utils/WindowsExtensions.h"

#include "widgets/DownloadProgressDialog.h"
#include "widgets/GeneralSettingsWidget.h"
#include "widgets/MainWindow.h"

#include "clockUtils/log/Log.h"

#include <QAbstractButton>
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QTextStream>

using namespace spine;
using namespace spine::launcher;
using namespace spine::utils;

Gothic2Launcher::Gothic2Launcher() {
	connect(this, &Gothic2Launcher::updatedPath, this, &Gothic2Launcher::patchCheck, Qt::QueuedConnection);
}

bool Gothic2Launcher::supports(common::GothicVersion gothic) const {
	return gothic == common::GothicVersion::GOTHIC2;
}

void Gothic2Launcher::setDirectory(const QString & directory) {
	Gothic1And2Launcher::setDirectory(directory);
	
#ifdef Q_OS_WIN
	QString programFiles = QProcessEnvironment::systemEnvironment().value("ProgramFiles", "Foobar123");
	programFiles = programFiles.replace("\\", "/");
	QString programFilesx86 = QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", "Foobar123");
	programFilesx86 = programFilesx86.replace("\\", "/");
	if (directory.contains(programFiles) || directory.contains(programFilesx86)) {
		if (!IsRunAsAdmin()) {
			QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("UpdateAdminInfo"), QApplication::tr("GeneralAdminNote").arg(QApplication::tr("Gothic2")), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::No);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.button(QMessageBox::StandardButton::No)->setText(QApplication::tr("No"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			if (resultMsg.exec() == QMessageBox::StandardButton::Ok) {
				emit restartAsAdmin();
				return;
			}
		}
	}
#endif
	if (QFileInfo::exists(directory + "/System/GothicGame.ini")) {
		QFile(directory + "/System/GothicGame.ini").remove();
	}

	// fix Union.ini
	{
		QString text;
		if (QFileInfo::exists(directory + "/System/Union.ini")) {
			QFile f(directory + "/System/Union.ini");
			f.open(QIODevice::ReadOnly);
			QTextStream ts(&f);
			text = ts.readAll();
		}
		if (text.count("PluginList") > 1) {
			QFile(directory + "/System/Union.ini").remove();
		}
	}

	emit updatedPath();
}

void Gothic2Launcher::patchCheck() {
	if (!QFileInfo::exists(_directory + "/System/Gothic2.exe")) return;

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage patchCheckG2 #1: " << getPRAMValue());
#endif

	QMap<QString, QString> fileList;
	fileList.insert("System/ar.exe", "495fdfc1797428a184bea293f96f46e7eb148ea56de4b7e4f628be1f4a9a8165c08b03c7e5245df6076fba8508ad7b521b6630ff0e33ad7fcbec7e4d2e4f10e3");
	fileList.insert("System/BinkW32.dll", "e6d1c3f5e33ff8fc56b4798a6155ae76411ba9a234bea599338b7af424051943b1a2e666baa6935975df3d0354ba435962d1281b88b1ea17a77b1fbeb2cecca2");
	fileList.insert("System/BugslayerUtil.dll", "c0dec407fa0d8d16cfbae351646651932ae91310b29e569b3dfd044629228e1ce43d4dd1871686958b37db67e9f3a344812e79efd6e67696b9d9d765797bcddf");
	fileList.insert("System/GeDialogs.dll", "eccfc666021b223dd79b98ee849e112c145357d41707aebad459b3c93b679e4c968ac088c48656b7810f056f572e1546a2033478c1a64cfaefd14d8f655c6ae2");
	fileList.insert("System/Gothic2.exe", "3d67b2d941f461be826c5f92d64c7a55ac3e5f3d723f11462266b770d1abe6bedc230432a896670e2e928921635f0990187202ede6eb23ea00a7e77237a8c96f");
	fileList.insert("System/ImageHl2.dll", "64f2ea18af80b67ad570baa1e43542a9c3ca851232e1d14a9e1ed1796ce2261b33fa2f873507b1dfb5d98a3688c195e283d069e8c2e23cf5ea104d6e95ab157a");
	fileList.insert("System/MsDbi.dll", "1b110d089258f26ec9dc77a0fb74e5adb1d010ae385ac4fc8af838dfdb383ef271b9010d895ae4699cd928aa5c5685fea207ddd521e89681da6cfe8ab55af0a6");
	fileList.insert("System/Mss32.dll", "a6c3a366760baaa321bf69ed4646f012cfbfb39ac49e2307cd06baa1f10fa1b7500e5441e8f03af4e47daa5aa479637dce7c3fa7e0adfbda9287acaa1adaada6");
	fileList.insert("System/Paths.d", "f3f7e6f4981c24e3dc1dd540df18d6a69d2f759e58550ec227640c11cf507cb990fad1875e0888c3b8ff8051082c879a4b3f62a263833e5c499d0e87039d56b5");
	fileList.insert("System/Shw32.dll", "2d0f51e7d3b10c5f42bb913452e4e77a46e43110dd881cacd38d1440182dac770d3dd4362bd7874733bc67d3ae634978148ce8449adef24935b8d0bc8dde260e");
	fileList.insert("System/vdfs32g.dll", "07251602e7a7fdce39b00e6c5be98c625c07dcb3dd66c867aee941eb9c2450df907e25c8e4e23fa90997c38f83bc4b5dd5fe76702693619e07208c7a7dd1e7d6");
	fileList.insert("vdfs.cfg", "dbb18ddaf3e58541724839453a43d369005a5e67b420dc3599c39dbeb4c163e8bfe76e94ef6fffc30e45d5d2236b18aaae61e6574f884068404827496935dd91");
	fileList.insert("Miles/MssA3D.m3d", "bc3657fcd76206defcda297a3fff5ec14dd1dd86f3181f83c20800ad4e431249aad3f74012918915007c36fcdd5da0dee3025776d26a344fa1ab344d8e1ca419");
	fileList.insert("Miles/MssDS3D.m3d", "9ad6b69c39a05f3abe51dabdb5cd6f3f0d76cb8e2e3d936ad3c600a44ce0d275853043bd990eec383e4dc71fa9daf887d37680543538e5438587367efb74961d");
	fileList.insert("Miles/MssDX7.m3d", "878aa07688a8474164758b1d619031050eb8a81ca6e53b427492cec65bfa406eb2c7f0e17c678fbe13ba023db49c43aa25daa40e3da6b420610b42c419a69510");
	fileList.insert("Miles/MssEAX.m3d", "3771335f15a8ad862940342543fd1d912246fae8bf10cde0ca0174248a98175ec9fff6c7c7a3b60e6c7891287b7bf3485a32c7680d081544e8c2db198485ab48");
	fileList.insert("Miles/MssRSX.m3d", "cf58176faa0429a40dc641609eeb297665752c226cf7d1dd58120365964727ba827be9771698e2137df555c99c6ea72b3d4a190ac0082afe383aec20f309a6b1");
	fileList.insert("Miles/MssSoft.m3d", "949ce71af730262719ba6c442df07e3074ee512a2c55560ce5d09de9d5de179c08fba485317a508ea887b7ecc9278b1b1fd4a1cf728cfb7a2ff3f602660406c2");
	fileList.insert("_work/data/Video/Portal_Raven.bik", "9d4f6dc897ce5db1737bfcdf10d792dc324130ff5c9e42fa1d4d36d1909b3f15822b0595607add4fccc48cf3edebf9fcb7ac8278fb0a08cc27a3aa6db0b4a17e");

	MultiFileDownloader * mfd = new MultiFileDownloader(this);
	connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
	connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));

	bool start = false;

	for (auto it = fileList.begin(); it != fileList.end(); ++it) {
		if (Config::extendedLogging) {
			LOGINFO("Checking G2 file " << it.key().toStdString());
		}
		const bool b = utils::Hashing::checkHash(_directory + "/" + it.key(), it.value()); // TODO: hash check is performed twice, here and in FileDownloader
		if (!b) {
			if (Config::extendedLogging) {
				LOGWARN("Hashcheck failed");
			}
			start = true;
			QFileInfo fi(it.key());
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g2/" + it.key()), _directory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
			mfd->addFileDownloader(fd);
		}
	}
	if (start) {
		widgets::DownloadProgressDialog progressDlg(mfd, "PatchingG2", 0, 100, 0, widgets::MainWindow::getInstance());
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
	} else {
		mfd->deleteLater();
	}

	Database::DBError err;
	const auto ids = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 40 OR ModID = 36 OR ModID = 116 OR ModID = 314;", err);
	
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "40"; }) == ids.end()) {
		emit installMod(40);
	}
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "36"; }) == ids.end()) {
		emit installMod(36);
	}
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "116"; }) == ids.end()) {
		emit installMod(116);
	}
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "314"; }) == ids.end()) {
		emit installMod(314);
	}

	if (QFileInfo::exists(_directory + "/System/ddraw.dll")) {
		QFile::remove(_directory + "/System/ddraw.dll");
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage patchCheckG2 #2: " << getPRAMValue());
#endif
}
