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

#include "widgets/IntegrityCheckDialog.h"

#include <thread>

#include "SpineConfig.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/Hashing.h"

#include "clockUtils/log/Log.h"

#include <QApplication>
#include <QDirIterator>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QtConcurrentRun>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

IntegrityCheckDialog::IntegrityCheckDialog(QMainWindow * mainWindow,  QWidget * par) : QProgressDialog(QApplication::tr("CheckIntegrityChecking").arg("-"), QApplication::tr("Cancel"), 0, 100, par), _taskbarProgress(nullptr), _running(false) {
	connect(this, &IntegrityCheckDialog::updateText, this, &IntegrityCheckDialog::setText, Qt::BlockingQueuedConnection);
	connect(this, &IntegrityCheckDialog::updateValue, this, &IntegrityCheckDialog::setValue, Qt::BlockingQueuedConnection);
	connect(this, &IntegrityCheckDialog::updateCount, this, &IntegrityCheckDialog::setMaximum, Qt::BlockingQueuedConnection);
	connect(this, &IntegrityCheckDialog::canceled, this, &IntegrityCheckDialog::cancelCheck);

#ifdef Q_OS_WIN
	QWinTaskbarButton * button = new QWinTaskbarButton(this);
	button->setWindow(mainWindow->windowHandle());

	_taskbarProgress = button->progress();
	_taskbarProgress->setMinimum(0);
	_taskbarProgress->setMaximum(100);
	_taskbarProgress->setValue(0);
	
	connect(this, &IntegrityCheckDialog::updateValue, _taskbarProgress, &QWinTaskbarProgress::setValue, Qt::BlockingQueuedConnection);
	connect(this, &IntegrityCheckDialog::updateCount, _taskbarProgress, &QWinTaskbarProgress::setMaximum, Qt::BlockingQueuedConnection);
#endif

	setWindowTitle(QApplication::tr("CheckIntegrity"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

int IntegrityCheckDialog::exec(int projectID) {
	_running = true;
#ifdef Q_OS_WIN
	_taskbarProgress->show();
#endif
	QFutureWatcher<void> watcher;
	QFuture<void> f = QtConcurrent::run([this, projectID]() {
		process(projectID);
	});
	connect(&watcher, &QFutureWatcher<void>::finished, this, &QProgressDialog::accept);
	watcher.setFuture(f);
	QProgressDialog::exec();
	f.waitForFinished();
#ifdef Q_OS_WIN
	_taskbarProgress->hide();
#endif
	return QDialog::Accepted;
}

void IntegrityCheckDialog::setGothicDirectory(QString path) {
	_gothicDirectory = path;
}

void IntegrityCheckDialog::setGothic2Directory(QString path) {
	_gothic2Directory = path;
}

void IntegrityCheckDialog::setText(QString text) {
	setLabelText(QApplication::tr("CheckIntegrityChecking").arg(text));
}

void IntegrityCheckDialog::cancelCheck() {
	_running = false;
}

void IntegrityCheckDialog::closeEvent(QCloseEvent * evt) {
	_running = false;
	QProgressDialog::closeEvent(evt);
}

void IntegrityCheckDialog::process(int projectID) {
	QMap<QString, QString> gothicFileList;
	gothicFileList.insert("system/Autopan.flt", "3b72d25d0ddeb6085657ec74b51cf5c03dc61c9f26ed75faa6ed4033ab051082e3b232b310f67bbc1e9eaf063451fe098da456e8a89699e49abbca99ac1005cb");
	gothicFileList.insert("system/BugslayerUtil.dll", "c0dec407fa0d8d16cfbae351646651932ae91310b29e569b3dfd044629228e1ce43d4dd1871686958b37db67e9f3a344812e79efd6e67696b9d9d765797bcddf");
	gothicFileList.insert("system/binkw32.dll", "cff0d1e1571e1bae40f309cc7f438805eeba64761b54ad04e877bcb0b02168f9d93abfa8fee5e4c3f37ce8725a26d57a31bdfe1a54eb8ac227f9c23228dd53f8");
	gothicFileList.insert("system/gedialogs.dll", "eccfc666021b223dd79b98ee849e112c145357d41707aebad459b3c93b679e4c968ac088c48656b7810f056f572e1546a2033478c1a64cfaefd14d8f655c6ae2");
	gothicFileList.insert("system/IMAGEHL2.dll", "64f2ea18af80b67ad570baa1e43542a9c3ca851232e1d14a9e1ed1796ce2261b33fa2f873507b1dfb5d98a3688c195e283d069e8c2e23cf5ea104d6e95ab157a");
	gothicFileList.insert("system/mallocwin32debug.dll", "72ef3859256706ea517e59dae52d8a93e8ab1c7a3777c3e74af1346f29d94991a337b9b5cab2d7f10b812f062f32b9aba6ed1b315a39e726e650fdbd285fc224");
	gothicFileList.insert("system/MSDBI.dll", "1b110d089258f26ec9dc77a0fb74e5adb1d010ae385ac4fc8af838dfdb383ef271b9010d895ae4699cd928aa5c5685fea207ddd521e89681da6cfe8ab55af0a6");
	gothicFileList.insert("system/Mss32.dll", "feaa427b8ea557979a3325c07d552162b74380216b70aff8b865514822c2e4da68fea94159b22d2bb75cbec935d9502cee4ee655fa3908ffae1de065e107717e");
	gothicFileList.insert("system/Capture.flt", "5e5bf1c6639c13b188108b0ae6ca54c7ae8703d6902c6b2c0875c3769e123a9b90247562e71591bbce0ada652c3f37cf6b36ffdfe00730e8ec458349ef8023f9");
	gothicFileList.insert("system/Chorus.flt", "c8fb28be71c6fb548390fbff75febe695f91177a43d72b52f68dd35f99ff7d0333b93f2e3fad4e4c3e7de5134bbf43abf03f09e7df93d1346450e9c49d9ed2d7");
	gothicFileList.insert("system/Compress.flt", "7f331d73b4d4e96d032bb23c2ff13b15add7903f8c734854c84c0de1eb61626e23b2d8e9eb4c8e3a8999ab61efa9c0565cfed8c4f72fc3513f9fe360e424aa09");
	gothicFileList.insert("system/Flange.flt", "43999bab74468f371265c154b7af38bc5c374b88ef760ee732faec7673efa63bd7aab4f65053bc20f7ed7d90a1950d4f107cf9b8153a1324440364735ab85394");
	gothicFileList.insert("system/Gothic.exe", "cd4860c8956ce1193e1b8f0f86c90d698e9561f66694d5b0413464a37cbdeead75732a81ff6fa240f187aea667c3bc97eb6bca2ba0ee1668523f2ed2cd54bf74");
	gothicFileList.insert("system/GothicMod.exe", "4732680584b2df955044918ea51d412fdf4a5ba0c5a3daa592491f3d1ee941a2922b47c9553a2f0628edf1109bf247f3cd895895a5ac471708449f5720231b62");
	gothicFileList.insert("system/Highpass.flt", "dea358a7a053618a3a84aaac162d0f9b9c623c36098ba73a8da44a4ce489c86fbc66c67ece4a64e6a6ac7d0b21bb802d2aab626cbaaf691c065504670a61dcf4");
	gothicFileList.insert("system/Laginter.flt", "6ef5f42eac2b7365a43452914fff236f5cb82c8174cb1fe636e46fbfb162c166046d6ed0dd076deea222aa76d64d5316de7d4e2dbc71036425a678c82641d0a0");
	gothicFileList.insert("system/Lowpass.flt", "bc767ba5c9fb89ae090f12237ecdb7c319071077fcab91662ade69ec3e43af0205055df56496d78913d372f32a8254b5f8473bd5421035bc07d6cb4cdce9fd5a");
	gothicFileList.insert("system/Mdelay.flt", "99aae659082c683919f8a0f89d50d44bbb9a0d54890b1756ca232f0316f03381e27690af7bcafc6520ebe6dff18846a9d285a349bc2d8ec39bce0fa095c1827c");
	gothicFileList.insert("system/Mp3dec.asi", "1c80c8e21c099df8875b403a1f9d2be8afecca24ff473f862d8645642ceea4e3a6e4babb26bdca1ef1caf853785c64d343153b3f1c16a786e3c32e0244b1c34a");
	gothicFileList.insert("system/Mss32.dll", "feaa427b8ea557979a3325c07d552162b74380216b70aff8b865514822c2e4da68fea94159b22d2bb75cbec935d9502cee4ee655fa3908ffae1de065e107717e");
	gothicFileList.insert("system/Mssv12.asi", "42fca8246d728696cf975cf7fe62aebd38840ee4711315bc3f00e0e05393670580a1ef891597d3dd0ee2b633b8de229dfbc7c2264f3732d7f19ba7f8493feecf");
	gothicFileList.insert("system/Mssv24.asi", "d2682d369213f5410ce887b56d4b483e4cb12f3aeb4aed2703f3e4cf1c7f120c91d1209f0843c39973359a4e721160b7fa62760bcdba9c29fea6be9a809954ac");
	gothicFileList.insert("system/Mssv29.asi", "7831b46be45af6f734e5b4a3e33f711fc3e3936b060aac0a65d565895c2faa72b137de5b7b83c5e49d4aea420a4aa76f4cc1140ce0fc79db0381f15118adcd7a");
	gothicFileList.insert("system/Parmeq.flt", "f98f2405a123c7ce527d2f7ccf7ffaae1ae90d2a8318e0be4f80670b6c6d57695634a52299d37b4dfc6dfb25571f65886958900b7fd47f97f162dc02c1069089");
	gothicFileList.insert("system/Reverb1.flt", "cddf0853a7af3a86ab5207adbb9690e5f09dd6199c84f9da292db96b3b3e26970ac1d5345603165b9e05dc0afaa48cca63025b3f431db75d616334e5116a5992");
	gothicFileList.insert("system/Reverb2.flt", "1435c55da55a2ea51c3088ca8659ab8ab23c266a8b6b7b75870a49b367d077e8b17285d514ace59ed601bea6c6188ff800d0ad8e5a40d38574eb5820873d92e9");
	gothicFileList.insert("system/Reverb3.flt", "fa4487994deeb6e873f88a5e61b9b2cad86c4d49f6edc2e3193d73adf84e7c44c59169b025283be9f79a8c718b43f72175cf5c729e26fd7f43c651d313d40965");
	gothicFileList.insert("system/Sdelay.flt", "eccb01caee8cd74ced99a49c0ad45c9b90598dcbd73ec29b42889c88c5a3bb61a4af1e1e1186927f7de8cf192f38b397c105e2b0cf06628cfd65da6ea1bfdb7d");
	gothicFileList.insert("system/Shelfeq.flt", "748e4264d52df72b096b0f06ca4cc5bfcd21d7219df841f53a4c9f84a8cf08370d7511ea44c921fce4207382bae8bac4205b7b56d661144ea845765bb39eadba");
	gothicFileList.insert("system/Vdfs32g.dll", "d9319060d758fd402bd685bc99e608cbd93ee237e61b15c30e4405117f248c0281819207c7741f09a254fa5745ab436762077df4d2b07ea53c3edf29b3bf4f55");
	gothicFileList.insert("VDFS.CFG", "dbb18ddaf3e58541724839453a43d369005a5e67b420dc3599c39dbeb4c163e8bfe76e94ef6fffc30e45d5d2236b18aaae61e6574f884068404827496935dd91");
	gothicFileList.insert("Miles/Mssa3d.m3d", "406d7179a721fc1dc8fa1dfd52fac510d937e9df06d699d2788a74daad5ccb2ebe216a36304fd28a476e359933210de774ab8ac93eebcef28763dd5a88d2ea6e");
	gothicFileList.insert("Miles/Mssa3d2.m3d", "fb365389037f425e3a5b3d032890c68552e5657823c44bb272d55874de8e95381fb763ec0e10df61c4983f141b30509a910a45006cddefeb3893bec51517096a");
	gothicFileList.insert("Miles/Mssdolby.m3d", "02ede9938eac7854bc27f23305d47b219899560f0ffce3aa3fefd8a81059d03a2a2e26ba75d6347935a9350cb554d08912503a039bafc4ec5530e850f93e32ba");
	gothicFileList.insert("Miles/Mssds3dh.m3d", "e029d46605ea053ec69c64d06c56088daf68628253ec42021f968638e81a8427538cd23fc9fd6696c2bc597907c07780ca53a16cd6e38586f78c2b62e9e8683b");
	gothicFileList.insert("Miles/Mssdx7sh.m3d", "d06086168cb73b1ed05096ad4c114fb924dc7b30c5b0d48832f1d41a8e8fef2d03e7d36b41a5f760ebb829ac8ba5f43e9cad00efccf10e4706e0887e5ad247d4");
	gothicFileList.insert("Miles/Mssdx7sl.m3d", "b213599f9268cf19abebbc08fe69d83e50b13c9952fdb72ee4e64dd2389a2c6ec41fd03a7217a1e6ce8fb87c5f296b241f90d1c42086be42b766f4a0a6408de2");
	gothicFileList.insert("Miles/Mssdx7sn.m3d", "bd64ec7e76207dc1de21539a5a71fa481a5f9216c17246505aaa6a4c393f25df2497fbfc764d3fbe2665f2fe4d7d2bf276bb999b00b070ce81d67f1aa1d6790d");
	gothicFileList.insert("Miles/Msseax.m3d", "5291636e5fd990c99ce6dd2a67470ef35c2d3a45772beb4cef3b659920bea689c7f61c2d3551278584782041d554c4d6938482a0b41d63e6949866cca82b833c");
	gothicFileList.insert("Miles/Msseax2.m3d", "e4c53835ce6933b41eb361bdb29fe945c0aa87378a83f344ce2d191fa3b0eee7112d137c90e3883c14707ede767d7fb3a5e8a10783890d4123c9fa1104814eae");
	gothicFileList.insert("Miles/Mssfast.m3d", "2f4b2106d83ba7fc348da1af95bfa74a230c6e99c1c305ec221a6889096043f581827b8ebd9693f4d55cecd5a00220f41b085f743cfb87cb9f1b5202d9c91139");
	gothicFileList.insert("Miles/Mssrsx.m3d", "3c1d7aec82da23e116c6652db615780e5d305894541272de2ae7ee9c85d2d12e7a13980d51ee552491e9b7e7f48aa2df95b4c421bca782d9266752e72eccf639");
	gothicFileList.insert("system/paths.d", "4e239ae79b6039f55ba4fec21a66becc36a2a53148a8d78ebb0232af01ec22db63645c6d717c4165f591dd76d18ebb1a5f27d9789ffae2632f644d420c810252");
	QMap<QString, QString> gothic2FileList;
	gothic2FileList.insert("System/ar.exe", "495fdfc1797428a184bea293f96f46e7eb148ea56de4b7e4f628be1f4a9a8165c08b03c7e5245df6076fba8508ad7b521b6630ff0e33ad7fcbec7e4d2e4f10e3");
	gothic2FileList.insert("System/BinkW32.dll", "e6d1c3f5e33ff8fc56b4798a6155ae76411ba9a234bea599338b7af424051943b1a2e666baa6935975df3d0354ba435962d1281b88b1ea17a77b1fbeb2cecca2");
	gothic2FileList.insert("System/BugslayerUtil.dll", "c0dec407fa0d8d16cfbae351646651932ae91310b29e569b3dfd044629228e1ce43d4dd1871686958b37db67e9f3a344812e79efd6e67696b9d9d765797bcddf");
	gothic2FileList.insert("System/GeDialogs.dll", "eccfc666021b223dd79b98ee849e112c145357d41707aebad459b3c93b679e4c968ac088c48656b7810f056f572e1546a2033478c1a64cfaefd14d8f655c6ae2");
	gothic2FileList.insert("System/Gothic2.exe", "3d67b2d941f461be826c5f92d64c7a55ac3e5f3d723f11462266b770d1abe6bedc230432a896670e2e928921635f0990187202ede6eb23ea00a7e77237a8c96f");
	gothic2FileList.insert("System/ImageHl2.dll", "64f2ea18af80b67ad570baa1e43542a9c3ca851232e1d14a9e1ed1796ce2261b33fa2f873507b1dfb5d98a3688c195e283d069e8c2e23cf5ea104d6e95ab157a");
	gothic2FileList.insert("System/MsDbi.dll", "1b110d089258f26ec9dc77a0fb74e5adb1d010ae385ac4fc8af838dfdb383ef271b9010d895ae4699cd928aa5c5685fea207ddd521e89681da6cfe8ab55af0a6");
	gothic2FileList.insert("System/Mss32.dll", "a6c3a366760baaa321bf69ed4646f012cfbfb39ac49e2307cd06baa1f10fa1b7500e5441e8f03af4e47daa5aa479637dce7c3fa7e0adfbda9287acaa1adaada6");
	gothic2FileList.insert("System/Paths.d", "f3f7e6f4981c24e3dc1dd540df18d6a69d2f759e58550ec227640c11cf507cb990fad1875e0888c3b8ff8051082c879a4b3f62a263833e5c499d0e87039d56b5");
	gothic2FileList.insert("System/Shw32.dll", "2d0f51e7d3b10c5f42bb913452e4e77a46e43110dd881cacd38d1440182dac770d3dd4362bd7874733bc67d3ae634978148ce8449adef24935b8d0bc8dde260e");
	gothic2FileList.insert("System/vdfs32g.dll", "07251602e7a7fdce39b00e6c5be98c625c07dcb3dd66c867aee941eb9c2450df907e25c8e4e23fa90997c38f83bc4b5dd5fe76702693619e07208c7a7dd1e7d6");
	gothic2FileList.insert("System/Vdfs32g.exe", "a59e98f3e48afe0da058b4636b4b76475528990b424aa28026c81c0a5a6fb4894f8247f7ef2035b5b1d1ff0359da8845dd92b38faa1ea49b3cebf98de2a76ec4");
	gothic2FileList.insert("vdfs.cfg", "dbb18ddaf3e58541724839453a43d369005a5e67b420dc3599c39dbeb4c163e8bfe76e94ef6fffc30e45d5d2236b18aaae61e6574f884068404827496935dd91");
	gothic2FileList.insert("Miles/MssA3D.m3d", "bc3657fcd76206defcda297a3fff5ec14dd1dd86f3181f83c20800ad4e431249aad3f74012918915007c36fcdd5da0dee3025776d26a344fa1ab344d8e1ca419");
	gothic2FileList.insert("Miles/MssDS3D.m3d", "9ad6b69c39a05f3abe51dabdb5cd6f3f0d76cb8e2e3d936ad3c600a44ce0d275853043bd990eec383e4dc71fa9daf887d37680543538e5438587367efb74961d");
	gothic2FileList.insert("Miles/MssDX7.m3d", "878aa07688a8474164758b1d619031050eb8a81ca6e53b427492cec65bfa406eb2c7f0e17c678fbe13ba023db49c43aa25daa40e3da6b420610b42c419a69510");
	gothic2FileList.insert("Miles/MssEAX.m3d", "3771335f15a8ad862940342543fd1d912246fae8bf10cde0ca0174248a98175ec9fff6c7c7a3b60e6c7891287b7bf3485a32c7680d081544e8c2db198485ab48");
	gothic2FileList.insert("Miles/MssRSX.m3d", "cf58176faa0429a40dc641609eeb297665752c226cf7d1dd58120365964727ba827be9771698e2137df555c99c6ea72b3d4a190ac0082afe383aec20f309a6b1");
	gothic2FileList.insert("Miles/MssSoft.m3d", "949ce71af730262719ba6c442df07e3074ee512a2c55560ce5d09de9d5de179c08fba485317a508ea887b7ecc9278b1b1fd4a1cf728cfb7a2ff3f602660406c2");
	Database::DBError err;

	std::string statement = "SELECT ModID, File, Hash FROM modfiles;";

	if (projectID > 0) {
		statement = "SELECT ModID, File, Hash FROM modfiles WHERE ModID = " + std::to_string(projectID) + ";";
	}
	
	std::vector<ModFile> files = Database::queryAll<ModFile, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, statement, err);
	int amount = files.size();
	if (!_gothicDirectory.isEmpty()) {
		amount += gothicFileList.size();
	}
	if (!_gothic2Directory.isEmpty()) {
		amount += gothic2FileList.size();
	}
	emit updateCount(amount + 1);
	int count = 0;
	int step = amount / 200;
	step = 1;
	if (step == 0) {
		step = 1;
	}
	QSet<QString> allFiles;

	auto projectDir = Config::DOWNLOADDIR + "/mods/";

	if (projectID > 0) {
		projectDir += QString::number(projectID) + "/";
	}
	
	QDirIterator it(projectDir, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString path = it.next();
		if (!path.contains(".spsav")) {
			allFiles.insert(path);
		}
	}

	QSet<int> d3d11Versions;
	
	for (const ModFile & file : files) {
		QFileInfo fi(file.file);
		if (count % step == 0) {
			emit updateText(fi.fileName());
		}

		const bool b = utils::Hashing::checkHash(Config::DOWNLOADDIR + "/mods/" + QString::number(file.modID) + "/" + file.file, file.hash);
		if (!b && !file.file.contains("directx_Jun2010_redist.exe", Qt::CaseInsensitive)) {
			_corruptFiles.append(file);

			if (file.file.contains("GD3D11", Qt::CaseInsensitive) && !d3d11Versions.contains(file.modID)) {
				d3d11Versions.insert(file.modID);

				{
					ModFile mf;
					mf.modID = file.modID;
					mf.file = "D3D11.zip";
					_corruptFiles.append(mf);
				}
				{
					ModFile mf;
					mf.modID = file.modID;
					mf.file = "archive.zip";
					_corruptFiles.append(mf);
				}
			}
		}

		for (auto it2 = allFiles.begin(); it2 != allFiles.end(); ++it2) {
			const auto path = Config::DOWNLOADDIR + "/mods/" + QString::number(file.modID) + "/" + file.file;
			const auto refPath = *it2;

			if (refPath.compare(path, Qt::CaseInsensitive) != 0) continue;
			
			allFiles.erase(it2);

			break;
		}
		count++;
		if (count % step == 0) {
			emit updateValue(count);
		}
		if (!_running) {
			emit rejected();
			break;
		}
	}
	if (!_gothicDirectory.isEmpty()) {
		for (auto it2 = gothicFileList.constBegin(); it2 != gothicFileList.constEnd(); ++it2) {
			QFileInfo fi(it2.key());
			emit updateText(fi.fileName());

			const bool b = utils::Hashing::checkHash(_gothicDirectory + "/" + it2.key(), it2.value());
			if (!b) {
				_corruptGothicFiles.append(ModFile(it2.key(), it2.value()));
			}

			count++;
			emit updateValue(count);
			if (!_running) {
				emit rejected();
				break;
			}
		}
	}
	if (!_gothic2Directory.isEmpty()) {
		for (auto it2 = gothic2FileList.constBegin(); it2 != gothic2FileList.constEnd(); ++it2) {
			QFileInfo fi(it2.key());
			emit updateText(fi.fileName());

			const bool b = utils::Hashing::checkHash(_gothic2Directory + "/" + it2.key(), it2.value());
			if (!b) {
				_corruptGothic2Files.append(ModFile(it2.key(), it2.value()));
			}

			count++;
			emit updateValue(count);
			if (!_running) {
				emit rejected();
				break;
			}
		}
	}
	if (_running) {
		for (const QString & file : allFiles) {
			QFile(file).remove();
		}

		if (Config::extendedLogging) {
			for (const auto & mf : _corruptFiles) {
				LOGINFO("Integrity Check - Hash invalid: " << q2s(mf.file) << " (" << mf.modID << ")");
			}
		}
	}
}
