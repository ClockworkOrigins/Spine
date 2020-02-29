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

#include "launcher/Gothic1Launcher.h"

#include "InstallMode.h"
#include "SpineConfig.h"
#include "SteamProcess.h"

#include "utils/Config.h"
#include "utils/Database.h"
#include "utils/DownloadQueue.h"
#include "utils/FileDownloader.h"
#include "utils/Hashing.h"
#include "utils/MultiFileDownloader.h"
#include "utils/WindowsExtensions.h"

#include "widgets/LocationSettingsWidget.h"

#include "clockUtils/log/Log.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QTextStream>

using namespace spine::client;
using namespace spine::common;
using namespace spine::launcher;
using namespace spine::utils;
using namespace spine::widgets;

Gothic1Launcher::Gothic1Launcher() : _startedWithSteam(false) {
	connect(this, &Gothic1Launcher::updatedPath, this, &Gothic1Launcher::patchCheck, Qt::QueuedConnection);

	_defaultIcon = QPixmap::fromImage(QImage(":Gothic.ico"));
}

bool Gothic1Launcher::supportsGame(GameType gothic) const {
	return gothic == GameType::Gothic;
}

void Gothic1Launcher::setDirectory(const QString & directory) {
	Gothic1And2Launcher::setDirectory(directory);
	
#ifdef Q_OS_WIN
	QString programFiles = QProcessEnvironment::systemEnvironment().value("ProgramFiles", "Foobar123");
	programFiles = programFiles.replace("\\", "/");
	QString programFilesx86 = QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", "Foobar123");
	programFilesx86 = programFilesx86.replace("\\", "/");
	if (directory.contains(programFiles) || directory.contains(programFilesx86)) {
		if (!IsRunAsAdmin()) {
			QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("UpdateAdminInfo"), QApplication::tr("GeneralAdminNote").arg(QApplication::tr("Gothic")), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::No);
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
	if (QFileInfo::exists(_directory + "/System/GothicGame.ini")) {
		QFile(_directory + "/System/GothicGame.ini").remove();
	}

	if (!QDir(_directory + "/saves").exists()) {
		bool b = QDir(_directory).mkdir("saves");
		Q_UNUSED(b);
	}

	// fix Union.ini
	{
		QString text;
		if (QFileInfo::exists(_directory + "/System/Union.ini")) {
			QFile f(_directory + "/System/Union.ini");
			f.open(QIODevice::ReadOnly);
			QTextStream ts(&f);
			text = ts.readAll();
		}
		if (text.count("PluginList") > 1) {
			QFile(_directory + "/System/Union.ini").remove();
		}
	}

	emit updatedPath();
}

void Gothic1Launcher::patchCheck() {		
	if (!QFileInfo::exists(_directory + "/System/Gothic.exe")) return;

	QMap<QString, QString> fileList;
	fileList.insert("system/Autopan.flt", "3b72d25d0ddeb6085657ec74b51cf5c03dc61c9f26ed75faa6ed4033ab051082e3b232b310f67bbc1e9eaf063451fe098da456e8a89699e49abbca99ac1005cb");
	fileList.insert("system/Capture.flt", "5e5bf1c6639c13b188108b0ae6ca54c7ae8703d6902c6b2c0875c3769e123a9b90247562e71591bbce0ada652c3f37cf6b36ffdfe00730e8ec458349ef8023f9");
	fileList.insert("system/Chorus.flt", "c8fb28be71c6fb548390fbff75febe695f91177a43d72b52f68dd35f99ff7d0333b93f2e3fad4e4c3e7de5134bbf43abf03f09e7df93d1346450e9c49d9ed2d7");
	fileList.insert("system/Compress.flt", "7f331d73b4d4e96d032bb23c2ff13b15add7903f8c734854c84c0de1eb61626e23b2d8e9eb4c8e3a8999ab61efa9c0565cfed8c4f72fc3513f9fe360e424aa09");
	fileList.insert("system/Flange.flt", "43999bab74468f371265c154b7af38bc5c374b88ef760ee732faec7673efa63bd7aab4f65053bc20f7ed7d90a1950d4f107cf9b8153a1324440364735ab85394");
	fileList.insert("system/Gothic.exe", "cd4860c8956ce1193e1b8f0f86c90d698e9561f66694d5b0413464a37cbdeead75732a81ff6fa240f187aea667c3bc97eb6bca2ba0ee1668523f2ed2cd54bf74");
	fileList.insert("system/GothicMod.exe", "4732680584b2df955044918ea51d412fdf4a5ba0c5a3daa592491f3d1ee941a2922b47c9553a2f0628edf1109bf247f3cd895895a5ac471708449f5720231b62");
	fileList.insert("system/Highpass.flt", "dea358a7a053618a3a84aaac162d0f9b9c623c36098ba73a8da44a4ce489c86fbc66c67ece4a64e6a6ac7d0b21bb802d2aab626cbaaf691c065504670a61dcf4");
	fileList.insert("system/Laginter.flt", "6ef5f42eac2b7365a43452914fff236f5cb82c8174cb1fe636e46fbfb162c166046d6ed0dd076deea222aa76d64d5316de7d4e2dbc71036425a678c82641d0a0");
	fileList.insert("system/Lowpass.flt", "bc767ba5c9fb89ae090f12237ecdb7c319071077fcab91662ade69ec3e43af0205055df56496d78913d372f32a8254b5f8473bd5421035bc07d6cb4cdce9fd5a");
	fileList.insert("system/Mdelay.flt", "99aae659082c683919f8a0f89d50d44bbb9a0d54890b1756ca232f0316f03381e27690af7bcafc6520ebe6dff18846a9d285a349bc2d8ec39bce0fa095c1827c");
	fileList.insert("system/Mp3dec.asi", "1c80c8e21c099df8875b403a1f9d2be8afecca24ff473f862d8645642ceea4e3a6e4babb26bdca1ef1caf853785c64d343153b3f1c16a786e3c32e0244b1c34a");
	fileList.insert("system/Mss32.dll", "feaa427b8ea557979a3325c07d552162b74380216b70aff8b865514822c2e4da68fea94159b22d2bb75cbec935d9502cee4ee655fa3908ffae1de065e107717e");
	fileList.insert("system/Mssv12.asi", "42fca8246d728696cf975cf7fe62aebd38840ee4711315bc3f00e0e05393670580a1ef891597d3dd0ee2b633b8de229dfbc7c2264f3732d7f19ba7f8493feecf");
	fileList.insert("system/Mssv24.asi", "d2682d369213f5410ce887b56d4b483e4cb12f3aeb4aed2703f3e4cf1c7f120c91d1209f0843c39973359a4e721160b7fa62760bcdba9c29fea6be9a809954ac");
	fileList.insert("system/Mssv29.asi", "7831b46be45af6f734e5b4a3e33f711fc3e3936b060aac0a65d565895c2faa72b137de5b7b83c5e49d4aea420a4aa76f4cc1140ce0fc79db0381f15118adcd7a");
	fileList.insert("system/Parmeq.flt", "f98f2405a123c7ce527d2f7ccf7ffaae1ae90d2a8318e0be4f80670b6c6d57695634a52299d37b4dfc6dfb25571f65886958900b7fd47f97f162dc02c1069089");
	fileList.insert("system/Reverb1.flt", "cddf0853a7af3a86ab5207adbb9690e5f09dd6199c84f9da292db96b3b3e26970ac1d5345603165b9e05dc0afaa48cca63025b3f431db75d616334e5116a5992");
	fileList.insert("system/Reverb2.flt", "1435c55da55a2ea51c3088ca8659ab8ab23c266a8b6b7b75870a49b367d077e8b17285d514ace59ed601bea6c6188ff800d0ad8e5a40d38574eb5820873d92e9");
	fileList.insert("system/Reverb3.flt", "fa4487994deeb6e873f88a5e61b9b2cad86c4d49f6edc2e3193d73adf84e7c44c59169b025283be9f79a8c718b43f72175cf5c729e26fd7f43c651d313d40965");
	fileList.insert("system/Sdelay.flt", "eccb01caee8cd74ced99a49c0ad45c9b90598dcbd73ec29b42889c88c5a3bb61a4af1e1e1186927f7de8cf192f38b397c105e2b0cf06628cfd65da6ea1bfdb7d");
	fileList.insert("system/Shelfeq.flt", "748e4264d52df72b096b0f06ca4cc5bfcd21d7219df841f53a4c9f84a8cf08370d7511ea44c921fce4207382bae8bac4205b7b56d661144ea845765bb39eadba");
	fileList.insert("system/Shw32.dll", "2d0f51e7d3b10c5f42bb913452e4e77a46e43110dd881cacd38d1440182dac770d3dd4362bd7874733bc67d3ae634978148ce8449adef24935b8d0bc8dde260e");
	fileList.insert("system/Vdfs32g.dll", "d9319060d758fd402bd685bc99e608cbd93ee237e61b15c30e4405117f248c0281819207c7741f09a254fa5745ab436762077df4d2b07ea53c3edf29b3bf4f55");
	fileList.insert("VDFS.CFG", "dbb18ddaf3e58541724839453a43d369005a5e67b420dc3599c39dbeb4c163e8bfe76e94ef6fffc30e45d5d2236b18aaae61e6574f884068404827496935dd91");
	fileList.insert("Miles/Mssa3d.m3d", "406d7179a721fc1dc8fa1dfd52fac510d937e9df06d699d2788a74daad5ccb2ebe216a36304fd28a476e359933210de774ab8ac93eebcef28763dd5a88d2ea6e");
	fileList.insert("Miles/Mssa3d2.m3d", "fb365389037f425e3a5b3d032890c68552e5657823c44bb272d55874de8e95381fb763ec0e10df61c4983f141b30509a910a45006cddefeb3893bec51517096a");
	fileList.insert("Miles/Mssdolby.m3d", "02ede9938eac7854bc27f23305d47b219899560f0ffce3aa3fefd8a81059d03a2a2e26ba75d6347935a9350cb554d08912503a039bafc4ec5530e850f93e32ba");
	fileList.insert("Miles/Mssds3dh.m3d", "e029d46605ea053ec69c64d06c56088daf68628253ec42021f968638e81a8427538cd23fc9fd6696c2bc597907c07780ca53a16cd6e38586f78c2b62e9e8683b");
	fileList.insert("Miles/Mssdx7sh.m3d", "d06086168cb73b1ed05096ad4c114fb924dc7b30c5b0d48832f1d41a8e8fef2d03e7d36b41a5f760ebb829ac8ba5f43e9cad00efccf10e4706e0887e5ad247d4");
	fileList.insert("Miles/Mssdx7sl.m3d", "b213599f9268cf19abebbc08fe69d83e50b13c9952fdb72ee4e64dd2389a2c6ec41fd03a7217a1e6ce8fb87c5f296b241f90d1c42086be42b766f4a0a6408de2");
	fileList.insert("Miles/Mssdx7sn.m3d", "bd64ec7e76207dc1de21539a5a71fa481a5f9216c17246505aaa6a4c393f25df2497fbfc764d3fbe2665f2fe4d7d2bf276bb999b00b070ce81d67f1aa1d6790d");
	fileList.insert("Miles/Msseax.m3d", "5291636e5fd990c99ce6dd2a67470ef35c2d3a45772beb4cef3b659920bea689c7f61c2d3551278584782041d554c4d6938482a0b41d63e6949866cca82b833c");
	fileList.insert("Miles/Msseax2.m3d", "e4c53835ce6933b41eb361bdb29fe945c0aa87378a83f344ce2d191fa3b0eee7112d137c90e3883c14707ede767d7fb3a5e8a10783890d4123c9fa1104814eae");
	fileList.insert("Miles/Mssfast.m3d", "2f4b2106d83ba7fc348da1af95bfa74a230c6e99c1c305ec221a6889096043f581827b8ebd9693f4d55cecd5a00220f41b085f743cfb87cb9f1b5202d9c91139");
	fileList.insert("Miles/Mssrsx.m3d", "3c1d7aec82da23e116c6652db615780e5d305894541272de2ae7ee9c85d2d12e7a13980d51ee552491e9b7e7f48aa2df95b4c421bca782d9266752e72eccf639");
	fileList.insert("system/BugslayerUtil.dll", "c0dec407fa0d8d16cfbae351646651932ae91310b29e569b3dfd044629228e1ce43d4dd1871686958b37db67e9f3a344812e79efd6e67696b9d9d765797bcddf");
	fileList.insert("system/binkw32.dll", "cff0d1e1571e1bae40f309cc7f438805eeba64761b54ad04e877bcb0b02168f9d93abfa8fee5e4c3f37ce8725a26d57a31bdfe1a54eb8ac227f9c23228dd53f8");
	fileList.insert("system/gedialogs.dll", "eccfc666021b223dd79b98ee849e112c145357d41707aebad459b3c93b679e4c968ac088c48656b7810f056f572e1546a2033478c1a64cfaefd14d8f655c6ae2");
	fileList.insert("system/IMAGEHL2.dll", "64f2ea18af80b67ad570baa1e43542a9c3ca851232e1d14a9e1ed1796ce2261b33fa2f873507b1dfb5d98a3688c195e283d069e8c2e23cf5ea104d6e95ab157a");
	fileList.insert("system/mallocwin32debug.dll", "72ef3859256706ea517e59dae52d8a93e8ab1c7a3777c3e74af1346f29d94991a337b9b5cab2d7f10b812f062f32b9aba6ed1b315a39e726e650fdbd285fc224");
	fileList.insert("system/MSDBI.dll", "1b110d089258f26ec9dc77a0fb74e5adb1d010ae385ac4fc8af838dfdb383ef271b9010d895ae4699cd928aa5c5685fea207ddd521e89681da6cfe8ab55af0a6");
	fileList.insert("system/Mss32.dll", "feaa427b8ea557979a3325c07d552162b74380216b70aff8b865514822c2e4da68fea94159b22d2bb75cbec935d9502cee4ee655fa3908ffae1de065e107717e");
	fileList.insert("system/paths.d", "4e239ae79b6039f55ba4fec21a66becc36a2a53148a8d78ebb0232af01ec22db63645c6d717c4165f591dd76d18ebb1a5f27d9789ffae2632f644d420c810252");

	MultiFileDownloader * mfd = new MultiFileDownloader(this);

	for (auto it = fileList.begin(); it != fileList.end(); ++it) {
		QFile f(_directory + "/" + it.key());
		if (Config::extendedLogging) {
			LOGINFO("Checking G1 file " << it.key().toStdString());
		}
		const bool b = utils::Hashing::checkHash(_directory + "/" + it.key(), it.value()); // TODO: hash check is performed twice, here and in FileDownloader
		if (!b) {
			QFileInfo fi(it.key());
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g1/" + it.key()), _directory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
			mfd->addFileDownloader(fd);
			if (Config::extendedLogging) {
				LOGWARN("Hashcheck failed");
			}
		}
	}

	DownloadQueue::getInstance()->add(mfd);

	Database::DBError err;
	const auto ids = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 57 OR ModID = 37;", err);
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "57"; }) == ids.end()) {
		emit installMod(57, -1, InstallMode::Silent);
	}
	if (std::find_if(ids.begin(), ids.end(), [](const std::string & s) { return s == "37"; }) == ids.end()) {
		emit installMod(37, -1, InstallMode::Silent);
	}

	if (QFileInfo::exists(_directory + "/System/ddraw.dll")) {
		QFile::remove(_directory + "/System/ddraw.dll");
	}
}

QString Gothic1Launcher::getExecutable() const {
	return "GothicMod.exe";
}

bool Gothic1Launcher::canBeStartedWithSteam() const {
	return LocationSettingsWidget::getInstance()->startGothicWithSteam() && !_developerModeActive; // can't start Gothic with Steam yet
}

void Gothic1Launcher::startViaSteam(QStringList arguments) {
#ifdef Q_OS_WIN
	QFile::remove(_directory + "/System/Gothic.exe.bak");
	QFile::rename(_directory + "/System/Gothic.exe", _directory + "/System/Gothic.exe.bak");
	QFile::rename(_directory + "/System/GothicMod.exe", _directory + "/System/Gothic.exe");

	_startedWithSteam = true;
	
	SteamProcess * sp = new SteamProcess(65540, "Gothic.exe", arguments);
	connect(sp, &SteamProcess::finished, this, &Gothic1Launcher::finishedMod);
	connect(sp, &SteamProcess::finished, sp, &SteamProcess::deleteLater);
	sp->start(5);
#endif
}

QPixmap Gothic1Launcher::getDefaultIcon() const {
	return _defaultIcon;
}

void Gothic1Launcher::modFinished() {
	if (!_startedWithSteam) return;

	_startedWithSteam = false;

	QFile::rename(_directory + "/System/Gothic.exe", _directory + "/System/GothicMod.exe");
	QFile::rename(_directory + "/System/Gothic.exe.bak", _directory + "/System/Gothic.exe");
}
