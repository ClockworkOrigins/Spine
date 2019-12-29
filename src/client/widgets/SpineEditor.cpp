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

#include "widgets/SpineEditor.h"

#include "Config.h"
#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

#include "models/SpineEditorModel.h"

#include "utils/Compression.h"
#include "utils/Conversion.h"
#include "utils/Hashing.h"

#include "widgets/AchievementSpineSettingsWidget.h"
#include "widgets/DownloadProgressDialog.h"
#include "widgets/GamepadSpineSettingsWidget.h"
#include "widgets/GeneralSpineSettingsWidget.h"
#include "widgets/LeGoSpineSettingsWidget.h"
#include "widgets/ScoreSpineSettingsWidget.h"
#include "widgets/WaitSpinner.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSet>
#include <QSettings>
#include <QTabWidget>
#include <QTextStream>
#include <QVBoxLayout>

#include <QtConcurrentRun>

namespace spine {
namespace widgets {

	enum EditorTabs {
		General,
		LeGo,
		Achievement,
		Score,
		Gamepad
	};

	SpineEditor::SpineEditor(QMainWindow * mainWindow) : QDialog(nullptr), _model(new models::SpineEditorModel(this)), _tabWidget(nullptr), _generalSpineSettingsWidget(nullptr), _achievementSpineSettingsWidget(nullptr), _scoreSpineSettingsWidget(nullptr), _gamepadSpineSettingsWidget(nullptr), _legoSpineSettingsWidget(nullptr), _installSpineButton(nullptr), _updateSpineButton(nullptr), _installIkarusButton(nullptr), _updateIkarusButton(nullptr), _installLeGoButton(nullptr), _updateLeGoButton(nullptr), _mainWindow(mainWindow), _modList() {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		_tabWidget = new QTabWidget(this);
		l->addWidget(_tabWidget);

		_generalSpineSettingsWidget = new GeneralSpineSettingsWidget(_model, _tabWidget);
		_tabWidget->addTab(_generalSpineSettingsWidget, QApplication::tr("General"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, EditorTabs::General, "General");
		connect(_model, SIGNAL(changed()), _generalSpineSettingsWidget, SLOT(updateFromModel()));

		_legoSpineSettingsWidget = new LeGoSpineSettingsWidget(_model, _tabWidget);
		_tabWidget->addTab(_legoSpineSettingsWidget, QApplication::tr("LeGo"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, EditorTabs::LeGo, "LeGo");
		connect(_model, SIGNAL(changed()), _legoSpineSettingsWidget, SLOT(updateFromModel()));

		_achievementSpineSettingsWidget = new AchievementSpineSettingsWidget(_model, _tabWidget);
		_tabWidget->addTab(_achievementSpineSettingsWidget, QApplication::tr("AchievementModule"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, EditorTabs::Achievement, "AchievementModule");
		connect(_model, SIGNAL(changed()), _achievementSpineSettingsWidget, SLOT(updateFromModel()));
		_tabWidget->setTabEnabled(EditorTabs::Achievement, false);

		_scoreSpineSettingsWidget = new ScoreSpineSettingsWidget(_model, _tabWidget);
		_tabWidget->addTab(_scoreSpineSettingsWidget, QApplication::tr("ScoresModule"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, EditorTabs::Score, "ScoresModule");
		connect(_model, SIGNAL(changed()), _scoreSpineSettingsWidget, SLOT(updateFromModel()));
		_tabWidget->setTabEnabled(EditorTabs::Score, false);

		_gamepadSpineSettingsWidget = new GamepadSpineSettingsWidget(_model, _tabWidget);
		_tabWidget->addTab(_gamepadSpineSettingsWidget, QApplication::tr("GamepadModule"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, EditorTabs::Score, "GamepadModule");
		connect(_model, SIGNAL(changed()), _gamepadSpineSettingsWidget, SLOT(updateFromModel()));
		_tabWidget->setTabEnabled(EditorTabs::Gamepad, false);

		connect(_generalSpineSettingsWidget, SIGNAL(changedAchievementState(int)), this, SLOT(achievementStateChanged(int)));
		connect(_generalSpineSettingsWidget, SIGNAL(changedAchievementState(int)), _legoSpineSettingsWidget, SLOT(achievementStateChanged(int)));
		connect(_generalSpineSettingsWidget, SIGNAL(changedScoreState(int)), this, SLOT(scoreStateChanged(int)));
		connect(_generalSpineSettingsWidget, SIGNAL(changedGamepadState(int)), this, SLOT(gamepadStateChanged(int)));
		connect(_generalSpineSettingsWidget, SIGNAL(changedGamepadState(int)), _legoSpineSettingsWidget, SLOT(gamepadStateChanged(int)));
		connect(_model, SIGNAL(changed()), this, SLOT(updateFromModel()));

		_installSpineButton = new QPushButton(QApplication::tr("InstallSpineScripts"), this);
		UPDATELANGUAGESETTEXT(_installSpineButton, "InstallSpineScripts");
		connect(_installSpineButton, SIGNAL(released()), this, SLOT(installSpineScripts()));
		_installSpineButton->hide();
		l->addWidget(_installSpineButton);

		_updateSpineButton = new QPushButton(QApplication::tr("UpdateSpineScripts"), this);
		UPDATELANGUAGESETTEXT(_updateSpineButton, "UpdateSpineScripts");
		connect(_updateSpineButton, SIGNAL(released()), this, SLOT(updateSpineScripts()));
		_updateSpineButton->hide();
		l->addWidget(_updateSpineButton);

		_installIkarusButton = new QPushButton(QApplication::tr("InstallIkarusScripts"), this);
		UPDATELANGUAGESETTEXT(_installIkarusButton, "InstallIkarusScripts");
		connect(_installIkarusButton, SIGNAL(released()), this, SLOT(installIkarusScripts()));
		_installIkarusButton->hide();
		l->addWidget(_installIkarusButton);

		_updateIkarusButton = new QPushButton(QApplication::tr("UpdateIkarusScripts"), this);
		UPDATELANGUAGESETTEXT(_updateIkarusButton, "UpdateIkarusScripts");
		connect(_updateIkarusButton, SIGNAL(released()), this, SLOT(updateIkarusScripts()));
		_updateIkarusButton->hide();
		l->addWidget(_updateIkarusButton);

		_installLeGoButton = new QPushButton(QApplication::tr("InstallLeGoScripts"), this);
		UPDATELANGUAGESETTEXT(_installLeGoButton, "InstallLeGoScripts");
		connect(_installLeGoButton, SIGNAL(released()), this, SLOT(installLeGoScripts()));
		_installLeGoButton->hide();
		l->addWidget(_installLeGoButton);

		_updateLeGoButton = new QPushButton(QApplication::tr("UpdateLeGoScripts"), this);
		UPDATELANGUAGESETTEXT(_updateLeGoButton, "UpdateLeGoScripts");
		connect(_updateLeGoButton, SIGNAL(released()), this, SLOT(updateLeGoScripts()));
		_updateLeGoButton->hide();
		l->addWidget(_updateLeGoButton);

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Save | QDialogButtonBox::StandardButton::Discard | QDialogButtonBox::StandardButton::Apply, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Save);
		b->setText(QApplication::tr("Save"));
		UPDATELANGUAGESETTEXT(b, "Save");

		connect(b, SIGNAL(clicked()), this, SIGNAL(accepted()));
		connect(b, SIGNAL(clicked()), this, SLOT(accept()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		b = dbb->button(QDialogButtonBox::StandardButton::Discard);
		b->setText(QApplication::tr("Discard"));
		UPDATELANGUAGESETTEXT(b, "Discard");

		connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
		connect(b, SIGNAL(clicked()), this, SLOT(reject()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		b = dbb->button(QDialogButtonBox::StandardButton::Apply);
		b->setText(QApplication::tr("Submit"));
		UPDATELANGUAGESETTEXT(b, "Submit");
		b->setToolTip(QApplication::tr("SubmitScripts"));
		UPDATELANGUAGESETTOOLTIP(b, "SubmitScripts");
		connect(b, SIGNAL(released()), this, SLOT(submit()));

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(QApplication::tr("SpineEditor"));
		UPDATELANGUAGESETWINDOWTITLE(this, "SpineEditor");

		restoreSettings();

		loadMods();
	}

	SpineEditor::~SpineEditor() {
		saveSettings();
	}

	models::SpineEditorModel * SpineEditor::getModel() const {
		return _model;
	}

	int SpineEditor::exec() {
		_model->load();
		checkSpineVersion();
		checkIkarusVersion();
		checkIkarusInitialized();
		checkLeGoVersion();
		checkLeGoInitialized();
		return QDialog::exec();
	}

	void SpineEditor::achievementStateChanged(int checkState) {
		_tabWidget->setTabEnabled(EditorTabs::Achievement, checkState == Qt::CheckState::Checked);
	}

	void SpineEditor::scoreStateChanged(int checkState) {
		_tabWidget->setTabEnabled(EditorTabs::Score, checkState == Qt::CheckState::Checked);
	}

	void SpineEditor::gamepadStateChanged(int checkState) {
		_tabWidget->setTabEnabled(EditorTabs::Gamepad, checkState == Qt::CheckState::Checked);
	}

	void SpineEditor::updateFromModel() {
		// ???
	}

	void SpineEditor::accept() {
		_generalSpineSettingsWidget->save();
		_legoSpineSettingsWidget->save();
		_achievementSpineSettingsWidget->save();
		_scoreSpineSettingsWidget->save();
		_gamepadSpineSettingsWidget->save();
		_model->save();
		QDialog::accept();
	}

	void SpineEditor::reject() {
		_model->load();
		QDialog::reject();
	}

	void SpineEditor::submit() {
		// save to update model
		_generalSpineSettingsWidget->save();
		_achievementSpineSettingsWidget->save();
		_scoreSpineSettingsWidget->save();
		
		if (!(_model->getModules() & common::SpineModules::Achievements) && !(_model->getModules() & common::SpineModules::Scores)) {
			QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("NoScriptFeatures"), QApplication::tr("NoScriptFeaturesText"), QMessageBox::StandardButton::Ok);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.exec();
			return;
		}
		QStringList modList;
		for (auto s : _modList) {
			modList.append(s2q(s.name));
		}
		QString modname = QInputDialog::getItem(this, QApplication::tr("Modname"), QApplication::tr("EnterModname"), modList);
		if (modname.isEmpty()) {
			return;
		}

		WaitSpinner spinner(QApplication::tr("SubmittingFeatures"), this);

		QFutureWatcher<clockUtils::ClockError> watcher(this);
		QEventLoop loop;
		connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));

		const QFuture<clockUtils::ClockError> future = QtConcurrent::run([this, &modname]() {
			common::SubmitScriptFeaturesMessage ssfm;
			common::SendModsForEditorMessage::ModForEditor mfe;
			for (const auto & s : _modList) {
				if (s2q(s.name) == modname) {
					mfe = s;
					break;
				}
			}
			ssfm.modID = mfe.modID;
			ssfm.language = Config::Language.toStdString();
			ssfm.username = Config::Username.toStdString();
			ssfm.password = Config::Password.toStdString();
			if (_model->getModules() & common::SpineModules::Achievements) {
				QSet<QString> imageNames;
				for (const models::AchievementModel & am : _model->getAchievements()) {
					common::SubmitScriptFeaturesMessage::Achievement a;
					a.name = q2s(am.name);
					a.description = q2s(am.description);
					a.hidden = am.hidden;
					a.lockedImageName = (QFileInfo(am.lockedImage).fileName() + "." + QFileInfo(am.lockedImage).suffix()).toStdString();
					a.unlockedImageName = (QFileInfo(am.unlockedImage).fileName() + "." + QFileInfo(am.unlockedImage).suffix()).toStdString();
					a.maxProgress = am.maxProgress;
					ssfm.achievements.push_back(a);
					if (!am.lockedImage.isEmpty()) {
						imageNames.insert(am.lockedImage);
					}
					if (!am.unlockedImage.isEmpty()) {
						imageNames.insert(am.unlockedImage);
					}
				}
				for (const QString & imageFile : imageNames) {
					QString imageHash;
					for (const auto & p : mfe.images) {
						QString af = QFileInfo(s2q(p.first)).fileName();
						af.chop(2);
						if (af == imageFile) {
							imageHash = s2q(p.second);
							break;
						}
					}
					QDirIterator itImage(_model->getPath() + "/_work/data/Textures/", QStringList() << imageFile, QDir::Files, QDirIterator::Subdirectories);
					if (itImage.hasNext()) {
						itImage.next();
						QImage img(itImage.filePath());
						if (!img.isNull()) {
							bool b = img.save(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/tmpAchSP.png");
							Q_UNUSED(b);
						}
						QString hashSum;
						utils::Hashing::hash(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/tmpAchSP.png", hashSum);
						if (imageHash != hashSum) {
							utils::Compression::compress(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/tmpAchSP.png", false);

							QFile compressedFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/tmpAchSP.png.z");
							if (compressedFile.open(QIODevice::ReadOnly)) {
								QByteArray byteArr = compressedFile.readAll();
								std::vector<uint8_t> buffer(byteArr.length());
								memcpy(&buffer[0], byteArr.data(), byteArr.length());
								ssfm.achievementImages.emplace_back(std::make_pair(QFileInfo(imageFile).fileName().toStdString(), q2s(hashSum)), buffer);
							}
							compressedFile.close();
							compressedFile.remove();
						}
					}
				}
			}
			if (_model->getModules() & common::SpineModules::Scores) {
				for (const models::ScoreModel & sm : _model->getScores()) {
					common::SubmitScriptFeaturesMessage::Score s;
					s.name = q2s(sm.name);
					ssfm.scores.push_back(s);
				}
			}
			const std::string serialized = ssfm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (Config::Username.isEmpty()) {
				cErr = clockUtils::ClockError::INVALID_USAGE;
			}
			if (clockUtils::ClockError::SUCCESS == cErr) {
				cErr = sock.writePacket(serialized);
			}
			return cErr;
		});
		watcher.setFuture(future);
		loop.exec();
		const clockUtils::ClockError cErr = future.result();
		if (cErr == clockUtils::ClockError::SUCCESS) {
			QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("ScriptSubmissionComplete"), QApplication::tr("ScriptSubmissionCompleteText"), QMessageBox::StandardButton::Ok);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.exec();
		} else {
			QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("ScriptSubmissionError"), QApplication::tr("ScriptSubmissionErrorText"), QMessageBox::StandardButton::Ok);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.exec();
		}
	}

	void SpineEditor::installSpineScripts() {
		static QList<QPair<QString, QString>> spineFiles = {
			{ "Spine.src.z", "0dbf6e8c3bf73e164e0db0c89eac582fb3473e18a1c91e5bfc2d94bf4ff0c95674f205d4dbf81d9f90592f909577beba0f19ea35008969753e8c4e7bcbcaaf33" },
			{ "Spine_Achievements.d.z", "bffa318b99639d2ada465a322b37cd50a57c52673ac66758993e946dc4b1d86da193778767e1eb27e91ee9066fb3b45dd2cd7ed5945d0445cff24979b81df689" },
			{ "Spine_API.d.z", "a3ea16983544768f953a2cfc4f41919ef394754a33335862661ea0b9561f992da4329f34f84af5132f9dd581c34d4e8eac65e9fef8651447582b51ad393c7c7b" },
			{ "Spine_Constants.d.z", "d2434fa4f817f31f8bf2b03e9805b9dda4e30b5bcfad9f71c9ee792ad7e1df4275aee0b43eec281216d20da9b8f5ddf3609e73fee241da976a5936fd33ce8fa4" },
			{ "Spine_Friends.d.z", "bc16f258031a5fb8b1f55b30f4e660295587a4c34da49fe9bffd5dedcc4717edc7d417bda8ba154d80cefb47dae43912e29d3876a68736f345f119fdcf4f996b" },
			{ "Spine_Gamepad.d.z", "490afa62bcdaa382de6d6e348708a48683ab78ec09f668871afd96f0fb5cb8eb7ef29d73c83b25c347a80c54bb421f3e3c13d319091924f8e5657a0f9c190d78" },
			{ "Spine_Message.d.z", "e6ede77bd7443a7e475e895ff251e2976a2a5308c16635fadfe0854bd58b4e538baf605f9e4cd4213eeccbf8d89118c1e7074eebf88a83598e859811bef18108" },
			{ "Spine_Multiplayer.d.z", "1278cc23b4ff4d0236bb151a9cb62b2e73dabb18e1564d2da93cf81238fcc3582ae3314ccf0cab630afec29598a5b0129542ee3fa97cea6ad474e565f2147acf" },
			{ "Spine_OverallSave.d.z", "873324c3f76972945da95b6c5b72815f0897ef57d0ebe9661730894bbc2c113e02ca0dbf77d84c38ef767f1e6c839702fc8cfb4c53a7ef3f3d629f1c936e992f" },
			{ "Spine_Scores.d.z", "bf8c538edb7b95b7ff286e65bd2ed969fb4a7035bdb9a4bf5d03f2f9e770a8f8487ba9ab464b69de0750e66c6ed1d03152490b3ac2c6fab74dc11d2d9c6bbef4" },
			{ "Spine_Statistics.d.z", "c23bd0f3305820c7ac5def8e23667f86464b2bb576f36f4074c82d1c0588b564fd25065cc6c9398089f759624f8fcc95c25a20c455b7cc9bd540740a9079279d" },
			{ "Spine_Statistics_UserDefined.d.z", "ad34af22f3b2c695a6f4ab34079fab262f89399c4cdf17d6a2c167d9ab06d33d0ad28f9fac3d1ea55a4016a5beff06d2172702d6abef537abb570779dd605701" },
			{ "Spine_TestMode.d.z", "fbaab320f5d8d2f0f0bc3ad89a45cf619fda4f8e3986961036e2c64833ed04261f143947ab1fa1ccef1fdf907cabd099b0c4ce12bb6f763cb480aaff2386738c" },
			{ "Spine_UserConstants.d.z", "bdc1c4ef20ab6388ae66550b6a27ffb4f52dd28522243c4efccfe4e3cb3241d76d07e83201dade73d1d6c7f73bb176a2de573b00bc8a1f1948bc71218c3932cc" },
			{ "Spine_Utils.d.z", "84df11c2081401c7accbf3a27555095db4fc391808099d42c627f1241d7ac089c3d4f3bb200f2bb880df0f2ad75bfffef8960c6aad80b6e946fa7ac1e0248f4d" }
		};
		MultiFileDownloader * mfd = new MultiFileDownloader(this);
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
		for (const QPair<QString, QString> & p : spineFiles) {
			QFileInfo fi(p.first);
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/scripts/Spine/" + p.first), _model->getPath() + "/_work/data/Scripts/Content/Spine/" + fi.path(), fi.fileName(), p.second, mfd);
			mfd->addFileDownloader(fd);
		}
		DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
		if (progressDlg.hasDownloadSucceeded()) {
			QDirIterator it(_model->getPath() + "/_work/data/Scripts/Content/", QStringList() << "Gothic.src", QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) {
				it.next();
				QFile f(it.filePath());
				if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QFile outFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Gothic.src");
					if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
						QTextStream outStream(&outFile);
						QTextStream ts(&f);
						while (!ts.atEnd()) {
							QString line = ts.readLine();
							if (line.contains("AI_Constants.d", Qt::CaseInsensitive)) {
								outStream << line << "\n";
								outStream << "Spine\\Spine.src\n";
							} else {
								outStream << line << "\n";
							}
						}
					}
				}
				f.close();
				QFile(it.filePath()).remove();
				QFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Gothic.src").rename(it.filePath());
			}
		}
	}

	void SpineEditor::updateSpineScripts() {
		static QList<QPair<QString, QString>> spineFiles = {
			{ "Spine.src.z", "0dbf6e8c3bf73e164e0db0c89eac582fb3473e18a1c91e5bfc2d94bf4ff0c95674f205d4dbf81d9f90592f909577beba0f19ea35008969753e8c4e7bcbcaaf33" },
			{ "Spine_Achievements.d.z", "bffa318b99639d2ada465a322b37cd50a57c52673ac66758993e946dc4b1d86da193778767e1eb27e91ee9066fb3b45dd2cd7ed5945d0445cff24979b81df689" },
			{ "Spine_API.d.z", "a3ea16983544768f953a2cfc4f41919ef394754a33335862661ea0b9561f992da4329f34f84af5132f9dd581c34d4e8eac65e9fef8651447582b51ad393c7c7b" },
			{ "Spine_Constants.d.z", "d2434fa4f817f31f8bf2b03e9805b9dda4e30b5bcfad9f71c9ee792ad7e1df4275aee0b43eec281216d20da9b8f5ddf3609e73fee241da976a5936fd33ce8fa4" },
			{ "Spine_Friends.d.z", "bc16f258031a5fb8b1f55b30f4e660295587a4c34da49fe9bffd5dedcc4717edc7d417bda8ba154d80cefb47dae43912e29d3876a68736f345f119fdcf4f996b" },
			{ "Spine_Gamepad.d.z", "490afa62bcdaa382de6d6e348708a48683ab78ec09f668871afd96f0fb5cb8eb7ef29d73c83b25c347a80c54bb421f3e3c13d319091924f8e5657a0f9c190d78" },
			{ "Spine_Message.d.z", "e6ede77bd7443a7e475e895ff251e2976a2a5308c16635fadfe0854bd58b4e538baf605f9e4cd4213eeccbf8d89118c1e7074eebf88a83598e859811bef18108" },
			{ "Spine_Multiplayer.d.z", "1278cc23b4ff4d0236bb151a9cb62b2e73dabb18e1564d2da93cf81238fcc3582ae3314ccf0cab630afec29598a5b0129542ee3fa97cea6ad474e565f2147acf" },
			{ "Spine_OverallSave.d.z", "873324c3f76972945da95b6c5b72815f0897ef57d0ebe9661730894bbc2c113e02ca0dbf77d84c38ef767f1e6c839702fc8cfb4c53a7ef3f3d629f1c936e992f" },
			{ "Spine_Scores.d.z", "bf8c538edb7b95b7ff286e65bd2ed969fb4a7035bdb9a4bf5d03f2f9e770a8f8487ba9ab464b69de0750e66c6ed1d03152490b3ac2c6fab74dc11d2d9c6bbef4" },
			{ "Spine_Statistics.d.z", "c23bd0f3305820c7ac5def8e23667f86464b2bb576f36f4074c82d1c0588b564fd25065cc6c9398089f759624f8fcc95c25a20c455b7cc9bd540740a9079279d" },
			{ "Spine_Statistics_UserDefined.d.z", "ad34af22f3b2c695a6f4ab34079fab262f89399c4cdf17d6a2c167d9ab06d33d0ad28f9fac3d1ea55a4016a5beff06d2172702d6abef537abb570779dd605701" },
			{ "Spine_TestMode.d.z", "fbaab320f5d8d2f0f0bc3ad89a45cf619fda4f8e3986961036e2c64833ed04261f143947ab1fa1ccef1fdf907cabd099b0c4ce12bb6f763cb480aaff2386738c" },
			{ "Spine_Utils.d.z", "84df11c2081401c7accbf3a27555095db4fc391808099d42c627f1241d7ac089c3d4f3bb200f2bb880df0f2ad75bfffef8960c6aad80b6e946fa7ac1e0248f4d" }
		};
		MultiFileDownloader * mfd = new MultiFileDownloader(this);
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
		for (const QPair<QString, QString> & p : spineFiles) {
			QFileInfo fi(p.first);
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/scripts/Spine/" + p.first), _model->getPath() + "/_work/data/Scripts/Content/Spine/" + fi.path(), fi.fileName(), p.second, mfd);
			mfd->addFileDownloader(fd);
		}
		DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
	}

	void SpineEditor::installIkarusScripts() {
		updateIkarusScripts();
		QDirIterator it(_model->getPath() + "/_work/data/Scripts/Content/", QStringList() << "Gothic.src", QDir::Files, QDirIterator::Subdirectories);
		if (it.hasNext()) {
			it.next();
			QFile f(it.filePath());
			if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QFile outFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Gothic.src");
				if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
					QTextStream outStream(&outFile);
					QTextStream ts(&f);
					while (!ts.atEnd()) {
						QString line = ts.readLine();
						if (line.contains("_intern\\Classes.d", Qt::CaseInsensitive)) {
							outStream << line << "\n\n";
							if (_model->getGothicVersion() == common::GothicVersion::GOTHIC) {
								outStream << "Ikarus\\ENGINECLASSES_G1\\*.d\n";
								outStream << "Ikarus\\IKARUS_CONST_G1.d\n";
							} else {
								outStream << "Ikarus\\ENGINECLASSES_G2\\*.d\n";
								outStream << "Ikarus\\IKARUS_CONST_G2.d\n";
							}
							outStream << "Ikarus\\IKARUS.d\n";
							outStream << "Ikarus\\FLOAT.d\n\n";
						} else {
							outStream << line << "\n";
						}
					}
				}
			}
			f.close();
			QFile(it.filePath()).remove();
			QFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Gothic.src").rename(it.filePath());
		}
	}

	void SpineEditor::updateIkarusScripts() {
		static QList<QPair<QString, QString>> ikarusFiles = {
			{ "EngineClasses_G1/Misc.d.z", "7d800ee00062fe9c0203f0543da6100162aa5085b48074c6ccd340f9932e7f119b95f634d78d685c80ffef3ff33fff2409fff55517ce04b47bc84637bb0b592e" },
			{ "EngineClasses_G1/oCAiHuman.unverified.d.z", "c757efa1714024586e8429987fbbf80db52c9184f5e00409120d788e6f8595de574aaece72480f16f775121a4bae65bc599255ba32b5dc3090512d80905b5bf0" },
			{ "EngineClasses_G1/oCGame.d.z", "18776278b9c28b10b7a9a55f293f74c06c86c2e559f1914991e5c10f2bb53886d8dc1983dc77496a8c4dbf74c258fa58dd97f4056955b7f7f10773fc4e207e5a" },
			{ "EngineClasses_G1/oCInfoManager.d.z", "e52623623273dac30d218b6bca1b9bcca47dbf90c290f0c5d9438caae24e05ab45627f2b938642bd5d34f329b60b917ad5aa3b02aa6b500ab3b261f640500c8e" },
			{ "EngineClasses_G1/oCItem.d.z", "5c7aa5154faaab79d2e497f5c922a32084bd1f2820a2fc0e1dee61e9237dcb6b16155d702e432f1421dd32856b0b6ee6b29f607f7a9a142a3b245f1eee946d1e" },
			{ "EngineClasses_G1/oCMob.d.z", "45a3113bfd269a16392533501d0a17aeb5f5c5007d77119c9573aa6051a3f445040d6dc361b921ea04fb0df51ff26bd1a96072a93f5d8e619b8b547025868eaa" },
			{ "EngineClasses_G1/oCNpc.d.z", "bc81bdaf73062d3c6fad49bf54ba7f868013d3e2e7e95eb294b61e27dcf2cbc349987ed6651d4a8b3df124077b8df452304dd2ab8bb2f965e58c49c257c94c6d" },
			{ "EngineClasses_G1/zCCamera.unverified.d.z", "44d33c96731b03938dba23f8d9bef01d114b1a53918e9f6a2061a04488d425e2bb259b2044faeb8e3ddd84f9268e8af18b533fe140cec3ce400b8b6fa4f56c11" },
			{ "EngineClasses_G1/zCMenu.d.z", "e054ae89774269465e9017c421e653a04b2a902503512d39cac458ac5c569e6924fffded2e8ea6113f63324c6f8d9d51f523ce785be28544b25da57cad74a26f" },
			{ "EngineClasses_G1/zCOption.d.z", "2c7b028bc546553f7c690f4113a7f1bd3875eb90229c9237ca0e4108a9a5e7cee8a548220b4eede4c6ab3afdd868f945c2406cea9eaf27687262288a392c0fd3" },
			{ "EngineClasses_G1/zCParser.d.z", "e58de04393ff1b30b9d9490eaca9da448ecafba2b31c71fdbcee84d6fe4cb43cf421b4f4ccdc429580932c42222791f36c998418f218ec4fc00c389487eecdab" },
			{ "EngineClasses_G1/zCSkyController.d.z", "b29ed1bb59cf01754c4c478791c03486756e7ca80a706195a3ebf74911624538ee7c0b2f8b4ca3a0716ebabd9b716c8900f2130e7931ea24123a73fa3b494b5e" },
			{ "EngineClasses_G1/zCTrigger.d.z", "8fbb0900f5da31f15dae6aae5e203050c7e5ad20e778fd74b016e5cef8dcd4bc773e3f6adeec8a171f7adca87b9d07946cd7d5fa7f8a2af42953164000370328" },
			{ "EngineClasses_G1/zCWaynet.d.z", "aee08cb0fad75e7ff80f1d93b80232aada83545fc61985759653bcab8aaa13e5ba3f6583b4a877b66f6b9f6d4d2bfee67ad87da8339415b2fc6d9d89df8d3667" },
			{ "EngineClasses_G1/zCWorld.d.z", "a1b9cb4e054a5f29c9f89a2bd831b19f2b7b4793690bfef9d8d12e9b43139c7e42f46e56a78ac7623b81fc47598baaa10214fcd61a90160a79317cb99a3cc252" },
			{ "EngineClasses_G1/zCZoneZFog.d.z", "2c921a5e20543f6056d96615b82852c45a730ed678ef49000da88990e6585b2e6202a4b0849a3b79c480bda647368baaea1430cf11e1b023ec9b93f0215086db" },
			{ "EngineClasses_G2/Misc.d.z", "ab8a09c712cc9e3f7316ed3b3f2825c78d9afeb2adf0b79e288e62f5fc7a276eb34c7d3a67ae015f5e9210d6dbdefa07d4d1d0764a550e34047770169c0aa266" },
			{ "EngineClasses_G2/oCAiHuman.d.z", "d3096b4e1678be460c744f48cf1db0064573eee647c7ddfe840337516ff92f72ed713f77b726bb906ecf7da60e466e67de3f7f0d7c0d8f95cd00d02c1edda051" },
			{ "EngineClasses_G2/oCGame.d.z", "591376f6862dbcc57772912eb50a4046e18e4e510047bbe2e9698b105cd52f92b4314029aacc75c9356d50948cbdd25ab2d13c12a3fc197e432a89a9c43fa520" },
			{ "EngineClasses_G2/oCInfoManager.d.z", "e52623623273dac30d218b6bca1b9bcca47dbf90c290f0c5d9438caae24e05ab45627f2b938642bd5d34f329b60b917ad5aa3b02aa6b500ab3b261f640500c8e" },
			{ "EngineClasses_G2/oCItem.d.z", "95c77d223323e3941e26cb06d7205971882f443aaa7f8e83e767d242dbcc3c86ec71f16311eb3b979a9b455410974d58d693cb8038ba040cb8c4076c47eb2055" },
			{ "EngineClasses_G2/oCMob.d.z", "62596cc453e56b074e5f66c2d656ce418e7b8713662e61b753042a34105402b10cd3e20033145dfe6e3be08190e52f8e938119fa21df496299655118dd2f32db" },
			{ "EngineClasses_G2/oCNpc.d.z", "bc5b3305d2016019c53014afeb8919f77607c6941e3418100157d6ba1154ebca5f7c43905321ee698870d6ee62558503f5f8f893f7ea7755766ce37ea10ab125" },
			{ "EngineClasses_G2/oCZoneMusic.d.z", "a01f8cc835c208bf3f3e918ef54601e8cdea0e955e0ad53d9a7ce08ada8f8f1993031f7422476a9a15c4bb7eb3e0c5345028547b57dda1634151c6acf8530bfb" },
			{ "EngineClasses_G2/zCCamera.d.z", "44d33c96731b03938dba23f8d9bef01d114b1a53918e9f6a2061a04488d425e2bb259b2044faeb8e3ddd84f9268e8af18b533fe140cec3ce400b8b6fa4f56c11" },
			{ "EngineClasses_G2/zCConsole.d.z", "f4d776fc9e2e9d3c639011e19bdb15d4af3307c1f45d89ca8fc1cd5ae560546181ef17cb51d250637f9135c5570fb21794f98d977fd3c3379e8d7736714b7593" },
			{ "EngineClasses_G2/zCMenu.d.z", "afca9305afb1c667250c58e940ec39c21e1a73ee0f4ee8bd0690416c0d0a383a3ff62727f3a9a78f85e2453f99b2bcc4d55a1fcccac2de34c63fb2e1822ccf48" },
			{ "EngineClasses_G2/zCMesh.d.z", "4dc22d317ff9681de66f9b58a99c8e7f02d7adefae986388f6041527678d87ba7157bd4d06c76111bf56a2aab85f4784ebbd6730f15131a19174086eaae24b4d" },
			{ "EngineClasses_G2/zCOption.d.z", "2c7b028bc546553f7c690f4113a7f1bd3875eb90229c9237ca0e4108a9a5e7cee8a548220b4eede4c6ab3afdd868f945c2406cea9eaf27687262288a392c0fd3" },
			{ "EngineClasses_G2/zCParser.d.z", "8fbd88379940c547e0069cd48d1e50eb3380ee5ec26c47b130067d3a6e06438e2a60323458752c0f17ba30b118aca1c6cfe8ee7a680eec1b2e18b4e162a7da3b" },
			{ "EngineClasses_G2/zCSkyController.d.z", "630bf2f123e287d2ef32010b266a6235d2b6efcd877b3afcb4738a1daf9649cf4ca38b1ec78757cc40046330d42bb1793d19fe570d6a7ae04f96b75acf00ebf2" },
			{ "EngineClasses_G2/zCTexture.d.z", "5ceac7ee28b59f602d5db0007a136d76d584fcaa681e662292485d03cdf358c8cf5ba4053a4f1a85eddc06563476597b9c6c74e06d8139cc3e34e9dcbe16adbd" },
			{ "EngineClasses_G2/zCTrigger.d.z", "75145e831e68b1c680ebe9720d33e062061d62029da808b85edf6cdef194f25c66c3f3eeff6b4449421cd302b46ba4a4d93ff1adc6373c7fc7b4a67fc8eada0f" },
			{ "EngineClasses_G2/zCWaynet.d.z", "8458d84f345666c0156b10c750788e1256df45c0d6593c337fb432cdecfff1a8c30481e7e5a3abab521456006cac6ff8c3924ef50a669e6ad1b0e31a3bf45093" },
			{ "EngineClasses_G2/zCWorld.d.z", "f9223252d72905088a7843c8108ad7fc2f214017f0d41b8f2666e3240799f4c341d3646d0c74fe57715e5dc7eb24d6fa847b61781db535bebd823c62f1469794" },
			{ "EngineClasses_G2/zCZoneZFog.d.z", "55fc6521ac5cca7f13a719a793f14117980a837ddc2cdb90f8f0e951c390961f3e212931549402f77b49ff65b04866bbc78a298cd88b731759876c440e4725ba" },
			{ "float.d.z", "f0fe97ee9c8d7b95d4567409431909f99cf5e35af151b58e3a8cbc7d4ed990a0cdbfc2f07c4d8836893a15200d26291422c4bd7c0740c9c64129620e7c8ae1a4" },
			{ "Ikarus.d.z", "e4859e2c1dd8dd49cf6f99760dd31241afc7896fef9af6cf1b8ba819dba2f98c86b0b811774e9c40d453f741e284576a50457c969e30cdaac571a903ae0a1ebe" },
			{ "Ikarus_Const_G1.d.z", "353de6a0db3c578e81347394f1c39d06565fca504fea17fd22e927bd7f1722eacba9de738571d688147c57334ab939b9b879fefafc5f5cec63a6a691d842a4ab" },
			{ "Ikarus_Const_G2.d.z", "c47641026eb6ad649b290b4d4810234a33e50413e739df46741fd2c17b6163402144a9e75da1749b6cd4724b67de5f1147a4d38f569ed055d9078ef7cf9a1e87" },
			{ "Ikarus_Doc.d.z", "feb6ece349eb94c5daadd03e301b25fd7515e53ab56c9da8c6e10ae08831b063a98af5579b3570ac4211b3fe4d7799340fa81a79d796acf88950c94d8564b58c" }
		};

		QList<QPair<QString, QString>> realFiles;
		const QString forbiddenSuffix = _model->getGothicVersion() == common::GothicVersion::GOTHIC ? "_G2" : "_G1";
		
		for (const auto & p : ikarusFiles) {
			if (!p.first.contains(forbiddenSuffix)) {
				realFiles.append(p);
			}
		}

		MultiFileDownloader * mfd = new MultiFileDownloader(this);
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
		for (const QPair<QString, QString> & p : realFiles) {
			QFileInfo fi(p.first);
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/scripts/Ikarus/" + p.first), _model->getPath() + "/_work/data/Scripts/Content/Ikarus/" + fi.path(), fi.fileName(), p.second, mfd);
			mfd->addFileDownloader(fd);
		}
		DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
	}

	void SpineEditor::installLeGoScripts() {
		static QList<QPair<QString, QString>> legoFiles = {
			{ "AI_Function.d.z", "5970553074f681d540acebd417ff48df3daf5d97731a84f36dc56add8fa5dc410a8df5af80775e2b701edf8aee0bfcc1641f3e2d7b373fcd964c5ba70915173f" },
			{ "Anim8.d.z", "0b152b3edca6395cf667f126bdc028567d7d94c311429d6fdc5a06ad695b4d7215aa1bc59cc7bb3c2a68f576e48e2f41da5b6c3c495a297c5b2198b43b7a83be" },
			{ "Bars.d.z", "dc12a585adf6c927a38e1cfa3cd9aa64dab2a686ccd65dbd1dae5615667bd833231e6d96115dd878749ad9c77d97b46cee415e68bd5b90c24d36b3dcfbe93b87" },
			{ "BinaryMachines.d.z", "3a3f7c31ded303e99a4a24db38696103217c2d16c0f1873faeb8dc916b7852caa49d61c1f5135ec61c6b7dc3661e992a5d4897692b879589bcfafb8f17e20b72" },
			{ "Bloodsplats.d.z", "c96a9867bb05811d7d75a2850a5839335e607df9a79ea3aa57c3ad23bae5e8848edc8bb2b176b3cc9af42333dd9881b18afb451ac92909488d7239b0201c11c6" },
			{ "Buffs.d.z", "84e73043cda3c28525801197fc9087df03f57528f6cfe4fd54b33932394fe16aded281a4e3e9c9f7f316c757b5b4b0f0191d8465b44dc490f559622ed2e404a8" },
			{ "Buttons.d.z", "d3362a628aa4749ffb01414cc1f7bff48e43cc73ee09fe3bd512ccfcc6b14357a2d40481c4bb4937c82a45abbe2a5967e2afbf4a8ada04a29bf53c0b6f6298bc" },
			{ "ConsoleCommands.d.z", "56d6c10c6eac37102ff9e4636d3041c226837feb6f4ce4dcc07063bfd0f25d5b9c9aa062ec3275d76db324573f7d61131719eaa9feee1032c717e083f2a672bf" },
			{ "Cursor.d.z", "2c6aa86ce177d9b6a29849efb0de03ae683a52fe723f8a1de7828ac4bb9c3e17cb4d8fc6d23703883ac9af083c823a340cd7591b08ffe16abe59f3c25574a0d1" },
			{ "Daedalus_Hooks.d.z", "d0779485a8f7d03f11c9b40211eca44370d21fb8a52dfdd37d2aaac943f6f9d6b0756e2591301527b5ffec1872d0e3de784ffee39cca468cef69bee8d908d9d6" },
			{ "Dialoggestures.d.z", "4d0a2e9510e49e9a5f904d6b3ef3b7beb21ee0d2ed4fc99a96ba49aded28ec8550720b7b0b0b58f8fad4a0283abbad20578afb8ed61722c620c2841307a2ba2f" },
			{ "Draw3D.d.z", "51cd0574219e43b0d9b9745145afe8bb87e464ad50a41afa00346087689bfc464035de96be187a929c12696a7e89034b94f9e99697fbc89e229c0efb83c86dc6" },
			{ "EngineAdr_G1.d.z", "f374013acabcacda1f6093c6d3fee4017a2b206e469044f962fc2ae9d60645486969a1075ea36148d302e47c47befe4d167e1baac090e7bb510bc7f9abec9785" },
			{ "EngineAdr_G2.d.z", "cdc14fe1c80db80b21b1e4a94740cf2d37ab8f89e858aa2a312b944ee0104cf693f70fd8b4a4a5de2989bcab204d9153e993d26cb3662ac318d2b11a7b6bc15b" },
			{ "EventHandler.d.z", "4d6a68deefffcdafb424e11aa60047f6bfe925d409534325b09ad7c1c655239036c44ad5ee49657b03e378cfe6bfe73e9443d14f6b66c91471a878ecff97e139" },
			{ "Focusnames.d.z", "b498febac0e27b4d42fcbe2d4c334b2250efc85da934e5f777af22d0a50ba9fb8aac8a300a3a5d5fd09f0e3987646eef87c55329004cc6cbd83a1688ad814f34" },
			{ "FrameFunctions.d.z", "005c8d3a76ec880737deafff6c79ccb7312a843d51f56eff3ed7252a7414b3344401fc9b1e8c50fda293d2fc517c7f33d4aeb8a8b8448db8728c85b1b90b92d6" },
			{ "Gamestate.d.z", "92b38a7367836b8aee9b7c5cd952cdb700ecc4c6a05d481c804d8e5be67ad7ea8d58fbb13dcd8fc57d31343653d8c18f947fcb2ae7d219caf8c554b13a61e5b0" },
			{ "Hashtable.d.z", "2bb17464af38fcdeae3a76f6cb5ae555ce03b7d03e68529887058b58001d0d967b2076ed117820a0249757cca7fcf7f7f5d200919ee3228bc7822f4f8ced4940" },
			{ "Header.src.z", "72f1e2c92f630118647d1eceef0599ef31d3d17429974b37e3f990d04796c35c25bb4def2d03cd5b71f88cb0be024712cdc45f88e0aba86fc3c8795af97a7fd1" },
			{ "HookEngine.d.z", "6cc363349fef2e27fcad6122064b5acf47cb402893ca6d405a0b92f691c1824b703ba6785e8d2742b0ff55575e1867b3481fbcc2eb469eb258cbddb4c5d28f3c" },
			{ "Int64.d.z", "39623d799133accb477d74c81edf85c7dd40407d2da372fcde9ee8a7df4029e9862376468d61e26a8449d09f5d47f8d3d0d1321a22745f661dbe272bd15fe95c" },
			{ "Interface.d.z", "d17f2caa3ad386b8fe53a6405c09861dd4d92ca9605057255e5e26431101d6cd953267b6ef1d187f689768e788dfd969dbf0d2075a998097958294f2f688d6b7" },
			{ "ItemHelper.d.z", "6f6ed716a7d8e564148aa47538c2dca66794be22194e1acc986e42949956510887f142de868932df763dc5b74766710fa2e1058c450a49efbabd18d1ea43e577" },
			{ "LeGo.d.z", "c82390cc6db05711d6bd9c39bd0548d0f2c085d6647f3a5c8cb7ab69b16b87380998510eae82c4c076146501ad79b848b85b9ac641b57dccad495216add854ef" },
			{ "List.d.z", "e590d7621abc204ff22665d2ba6788dee9d4ce97193ffad6707b58433d42076df8cb8d50f8cb90bf1554068addd7188d6b6f74a20f8bfd3aa6afffb60dbdddab" },
			{ "Locals.d.z", "a698237d0a02ba1b962cd8503d2f09ef8bb087b41318c4a29d7521e11647fa4c59ea49f887e2491a0f93d5c026928d64128c566db130f45599f2faeea94983e9" },
			{ "Misc.d.z", "095b47cbad88e3542885d5bb77e3562c3bce884d70ded90ab3d1118281fb9db4036ab68e5b809cdc848f51da9408ad939fceea10d09a9214ffa3034a84bdeccf" },
			{ "Names.d.z", "2f472dc0021590a0b61d1c4b155cd57a12fb00b201a9114ecb3b3993974816b95ac22a2bcaea4d52e704e345ec296f4a3e00b939876167fdb4979373027eb0a0" },
			{ "PermMem.d.z", "b9bd2c129b36ca6b47e3d64845534a8bf849f38c2eff3463f874a1e0365e9ba2465dcea6f38ac595b31ddd397c21468803e9b7539c2dc4b6ca3948fef905d10a" },
			{ "PermMem_Structs.d.z", "8648c26c1e607a59eec899307b29fd44ad968ad5014e3546452f8b8007b06a507eeb92f35e119291ecf0418ba471e3c14e3078e1094262c3b67f3d503544daa0" },
			{ "Queue.d.z", "bebfeda2face5215592aff7e7b9fa150c1e9e1b36015a26249f76e8e8c3167d725ce1b29b6be2798adaa58f5c7b244aeec0b94be54b394b9faff3de144693044" },
			{ "Random.d.z", "f9773bb2e7453be6475356172a5d4149cb22b4fce08cd0e79a276eafd6d14a749c57255d6dc5ffe226c6b621ae78ae7d2eb050811eb5b7ec056c49b6747e1270" },
			{ "Render.d.z", "230ea949d59f96b81d0eca16a4a44eb5e0f2a66bf3ca57c82b63614583a159ffaaffddd144dde3fd41df3cd08d8e1b7f736e0eb4116e7f020b33726d2253fd10" },
			{ "Saves.d.z", "3a8e20ee995b28ccabc1342557dbd249c3d4da9f59d2f4542affca89fefa64494da0623d915207d6d4bf748360c3e3867c62613bdc98cb5fc3eee983d68fb14c" },
			{ "Sprite.d.z", "dfc751611c861a5b680aa57d7705e2269162ab296a139035c28e7b56ca836da131e0ed3cdac3a1e61075d6baab83c62484f453a4a57ae7b2dd9e549c9373d17e" },
			{ "StringBuilder.d.z", "2810d730a8171519a24abc36f9b7b71f01bc0b294e2cee65fb3ff9625981d0c68fd54bf81fcc2262b93b537a0db784c0e648b4f46c526c5ba9f9142cfc7b3c78" },
			{ "Talents.d.z", "c3d5f984ca20027defe97b67cdaae6700fc512b15b685b58b41fb2013c66b282f133c10d26b86ad8ab4bb23630c29f2e3831d2e726e463ec9a2051029cc68d03" },
			{ "Timer.d.z", "1594210d0a32aae2b387f764499cfb001024a8b71ff7eb91efab0436e68335c762441fd13a12be82fd197b0845712b9ccad9a8fc8e4cb1884c617a368040823e" },
			{ "Trialoge.d.z", "159a8531459fb4905f10d077ff07d86c7cba10404b549bd34594cc7521c4636f695bcec496f36c4a89d9871d746094ae909a8bfadc57f85b7a2a92c8191b2741" },
			{ "Userconst.d.z", "cae2787286512167bafa9a4fe399c08b111451abbb0dbc324361ea1a6d1f34058aa5840d06fa646887d94b045e41b822324e8725b5ca7f9fadf809baf066d588" },
			{ "View.d.z", "65794fd65c844cd7c974271e4647ccaa890c8a7f2d261278931067a2a6b127d16c28702bbf7f419b6fa07c9a6901c257a711bc34f10306f659d8dad6047e638e" },
			{ "_Hashtable.d.z", "1ba581f38d3689c01c1805d7f98ffa5a8d593a35ffb0c82f39e097998ec92496f2ff7b1f2bf2ab1b2dbba91a0afae1f58cf217323add91ee6dbf86aab0cb16f6" }
		};
		MultiFileDownloader * mfd = new MultiFileDownloader(this);
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
		for (const QPair<QString, QString> & p : legoFiles) {
			QFileInfo fi(p.first);
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/scripts/LeGo/" + p.first), _model->getPath() + "/_work/data/Scripts/Content/LeGo/" + fi.path(), fi.fileName(), p.second, mfd);
			mfd->addFileDownloader(fd);
		}
		DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
		if (progressDlg.hasDownloadSucceeded()) {
			QDirIterator it(_model->getPath() + "/_work/data/Scripts/Content/", QStringList() << "Gothic.src", QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) {
				it.next();
				QFile f(it.filePath());
				if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QFile outFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Gothic.src");
					if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
						QTextStream outStream(&outFile);
						QTextStream ts(&f);
						while (!ts.atEnd()) {
							QString line = ts.readLine();
							if (line.contains("AI_Constants.d", Qt::CaseInsensitive)) {
								outStream << "\n\nLeGo\\Header.src\n\n";
								outStream << line << "\n";
							} else {
								outStream << line << "\n";
							}
						}
					}
				}
				f.close();
				QFile(it.filePath()).remove();
				QFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Gothic.src").rename(it.filePath());
			}
		}
	}

	void SpineEditor::updateLeGoScripts() {
		static QList<QPair<QString, QString>> legoFiles = {
			{ "AI_Function.d.z", "5970553074f681d540acebd417ff48df3daf5d97731a84f36dc56add8fa5dc410a8df5af80775e2b701edf8aee0bfcc1641f3e2d7b373fcd964c5ba70915173f" },
			{ "Anim8.d.z", "0b152b3edca6395cf667f126bdc028567d7d94c311429d6fdc5a06ad695b4d7215aa1bc59cc7bb3c2a68f576e48e2f41da5b6c3c495a297c5b2198b43b7a83be" },
			{ "Bars.d.z", "dc12a585adf6c927a38e1cfa3cd9aa64dab2a686ccd65dbd1dae5615667bd833231e6d96115dd878749ad9c77d97b46cee415e68bd5b90c24d36b3dcfbe93b87" },
			{ "BinaryMachines.d.z", "3a3f7c31ded303e99a4a24db38696103217c2d16c0f1873faeb8dc916b7852caa49d61c1f5135ec61c6b7dc3661e992a5d4897692b879589bcfafb8f17e20b72" },
			{ "Bloodsplats.d.z", "c96a9867bb05811d7d75a2850a5839335e607df9a79ea3aa57c3ad23bae5e8848edc8bb2b176b3cc9af42333dd9881b18afb451ac92909488d7239b0201c11c6" },
			{ "Buffs.d.z", "84e73043cda3c28525801197fc9087df03f57528f6cfe4fd54b33932394fe16aded281a4e3e9c9f7f316c757b5b4b0f0191d8465b44dc490f559622ed2e404a8" },
			{ "Buttons.d.z", "d3362a628aa4749ffb01414cc1f7bff48e43cc73ee09fe3bd512ccfcc6b14357a2d40481c4bb4937c82a45abbe2a5967e2afbf4a8ada04a29bf53c0b6f6298bc" },
			{ "ConsoleCommands.d.z", "56d6c10c6eac37102ff9e4636d3041c226837feb6f4ce4dcc07063bfd0f25d5b9c9aa062ec3275d76db324573f7d61131719eaa9feee1032c717e083f2a672bf" },
			{ "Cursor.d.z", "2c6aa86ce177d9b6a29849efb0de03ae683a52fe723f8a1de7828ac4bb9c3e17cb4d8fc6d23703883ac9af083c823a340cd7591b08ffe16abe59f3c25574a0d1" },
			{ "Daedalus_Hooks.d.z", "d0779485a8f7d03f11c9b40211eca44370d21fb8a52dfdd37d2aaac943f6f9d6b0756e2591301527b5ffec1872d0e3de784ffee39cca468cef69bee8d908d9d6" },
			{ "Dialoggestures.d.z", "4d0a2e9510e49e9a5f904d6b3ef3b7beb21ee0d2ed4fc99a96ba49aded28ec8550720b7b0b0b58f8fad4a0283abbad20578afb8ed61722c620c2841307a2ba2f" },
			{ "Draw3D.d.z", "51cd0574219e43b0d9b9745145afe8bb87e464ad50a41afa00346087689bfc464035de96be187a929c12696a7e89034b94f9e99697fbc89e229c0efb83c86dc6" },
			{ "EngineAdr_G1.d.z", "f374013acabcacda1f6093c6d3fee4017a2b206e469044f962fc2ae9d60645486969a1075ea36148d302e47c47befe4d167e1baac090e7bb510bc7f9abec9785" },
			{ "EngineAdr_G2.d.z", "cdc14fe1c80db80b21b1e4a94740cf2d37ab8f89e858aa2a312b944ee0104cf693f70fd8b4a4a5de2989bcab204d9153e993d26cb3662ac318d2b11a7b6bc15b" },
			{ "EventHandler.d.z", "4d6a68deefffcdafb424e11aa60047f6bfe925d409534325b09ad7c1c655239036c44ad5ee49657b03e378cfe6bfe73e9443d14f6b66c91471a878ecff97e139" },
			{ "Focusnames.d.z", "b498febac0e27b4d42fcbe2d4c334b2250efc85da934e5f777af22d0a50ba9fb8aac8a300a3a5d5fd09f0e3987646eef87c55329004cc6cbd83a1688ad814f34" },
			{ "FrameFunctions.d.z", "005c8d3a76ec880737deafff6c79ccb7312a843d51f56eff3ed7252a7414b3344401fc9b1e8c50fda293d2fc517c7f33d4aeb8a8b8448db8728c85b1b90b92d6" },
			{ "Gamestate.d.z", "92b38a7367836b8aee9b7c5cd952cdb700ecc4c6a05d481c804d8e5be67ad7ea8d58fbb13dcd8fc57d31343653d8c18f947fcb2ae7d219caf8c554b13a61e5b0" },
			{ "Hashtable.d.z", "2bb17464af38fcdeae3a76f6cb5ae555ce03b7d03e68529887058b58001d0d967b2076ed117820a0249757cca7fcf7f7f5d200919ee3228bc7822f4f8ced4940" },
			{ "Header.src.z", "72f1e2c92f630118647d1eceef0599ef31d3d17429974b37e3f990d04796c35c25bb4def2d03cd5b71f88cb0be024712cdc45f88e0aba86fc3c8795af97a7fd1" },
			{ "HookEngine.d.z", "6cc363349fef2e27fcad6122064b5acf47cb402893ca6d405a0b92f691c1824b703ba6785e8d2742b0ff55575e1867b3481fbcc2eb469eb258cbddb4c5d28f3c" },
			{ "Int64.d.z", "39623d799133accb477d74c81edf85c7dd40407d2da372fcde9ee8a7df4029e9862376468d61e26a8449d09f5d47f8d3d0d1321a22745f661dbe272bd15fe95c" },
			{ "Interface.d.z", "d17f2caa3ad386b8fe53a6405c09861dd4d92ca9605057255e5e26431101d6cd953267b6ef1d187f689768e788dfd969dbf0d2075a998097958294f2f688d6b7" },
			{ "ItemHelper.d.z", "6f6ed716a7d8e564148aa47538c2dca66794be22194e1acc986e42949956510887f142de868932df763dc5b74766710fa2e1058c450a49efbabd18d1ea43e577" },
			{ "LeGo.d.z", "c82390cc6db05711d6bd9c39bd0548d0f2c085d6647f3a5c8cb7ab69b16b87380998510eae82c4c076146501ad79b848b85b9ac641b57dccad495216add854ef" },
			{ "List.d.z", "e590d7621abc204ff22665d2ba6788dee9d4ce97193ffad6707b58433d42076df8cb8d50f8cb90bf1554068addd7188d6b6f74a20f8bfd3aa6afffb60dbdddab" },
			{ "Locals.d.z", "a698237d0a02ba1b962cd8503d2f09ef8bb087b41318c4a29d7521e11647fa4c59ea49f887e2491a0f93d5c026928d64128c566db130f45599f2faeea94983e9" },
			{ "Misc.d.z", "095b47cbad88e3542885d5bb77e3562c3bce884d70ded90ab3d1118281fb9db4036ab68e5b809cdc848f51da9408ad939fceea10d09a9214ffa3034a84bdeccf" },
			{ "Names.d.z", "2f472dc0021590a0b61d1c4b155cd57a12fb00b201a9114ecb3b3993974816b95ac22a2bcaea4d52e704e345ec296f4a3e00b939876167fdb4979373027eb0a0" },
			{ "PermMem.d.z", "b9bd2c129b36ca6b47e3d64845534a8bf849f38c2eff3463f874a1e0365e9ba2465dcea6f38ac595b31ddd397c21468803e9b7539c2dc4b6ca3948fef905d10a" },
			{ "PermMem_Structs.d.z", "8648c26c1e607a59eec899307b29fd44ad968ad5014e3546452f8b8007b06a507eeb92f35e119291ecf0418ba471e3c14e3078e1094262c3b67f3d503544daa0" },
			{ "Queue.d.z", "bebfeda2face5215592aff7e7b9fa150c1e9e1b36015a26249f76e8e8c3167d725ce1b29b6be2798adaa58f5c7b244aeec0b94be54b394b9faff3de144693044" },
			{ "Random.d.z", "f9773bb2e7453be6475356172a5d4149cb22b4fce08cd0e79a276eafd6d14a749c57255d6dc5ffe226c6b621ae78ae7d2eb050811eb5b7ec056c49b6747e1270" },
			{ "Render.d.z", "230ea949d59f96b81d0eca16a4a44eb5e0f2a66bf3ca57c82b63614583a159ffaaffddd144dde3fd41df3cd08d8e1b7f736e0eb4116e7f020b33726d2253fd10" },
			{ "Saves.d.z", "3a8e20ee995b28ccabc1342557dbd249c3d4da9f59d2f4542affca89fefa64494da0623d915207d6d4bf748360c3e3867c62613bdc98cb5fc3eee983d68fb14c" },
			{ "Sprite.d.z", "dfc751611c861a5b680aa57d7705e2269162ab296a139035c28e7b56ca836da131e0ed3cdac3a1e61075d6baab83c62484f453a4a57ae7b2dd9e549c9373d17e" },
			{ "StringBuilder.d.z", "2810d730a8171519a24abc36f9b7b71f01bc0b294e2cee65fb3ff9625981d0c68fd54bf81fcc2262b93b537a0db784c0e648b4f46c526c5ba9f9142cfc7b3c78" },
			{ "Talents.d.z", "c3d5f984ca20027defe97b67cdaae6700fc512b15b685b58b41fb2013c66b282f133c10d26b86ad8ab4bb23630c29f2e3831d2e726e463ec9a2051029cc68d03" },
			{ "Timer.d.z", "1594210d0a32aae2b387f764499cfb001024a8b71ff7eb91efab0436e68335c762441fd13a12be82fd197b0845712b9ccad9a8fc8e4cb1884c617a368040823e" },
			{ "Trialoge.d.z", "159a8531459fb4905f10d077ff07d86c7cba10404b549bd34594cc7521c4636f695bcec496f36c4a89d9871d746094ae909a8bfadc57f85b7a2a92c8191b2741" },
			{ "Userconst.d.z", "cae2787286512167bafa9a4fe399c08b111451abbb0dbc324361ea1a6d1f34058aa5840d06fa646887d94b045e41b822324e8725b5ca7f9fadf809baf066d588" },
			{ "View.d.z", "65794fd65c844cd7c974271e4647ccaa890c8a7f2d261278931067a2a6b127d16c28702bbf7f419b6fa07c9a6901c257a711bc34f10306f659d8dad6047e638e" },
			{ "_Hashtable.d.z", "1ba581f38d3689c01c1805d7f98ffa5a8d593a35ffb0c82f39e097998ec92496f2ff7b1f2bf2ab1b2dbba91a0afae1f58cf217323add91ee6dbf86aab0cb16f6" }
		};
		MultiFileDownloader * mfd = new MultiFileDownloader(this);
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
		for (const QPair<QString, QString> & p : legoFiles) {
			QFileInfo fi(p.first);
			FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/scripts/LeGo/" + p.first), _model->getPath() + "/_work/data/Scripts/Content/LeGo/" + fi.path(), fi.fileName(), p.second, mfd);
			mfd->addFileDownloader(fd);
		}
		DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
		progressDlg.setCancelButton(nullptr);
		progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		progressDlg.exec();
	}

	void SpineEditor::closeEvent(QCloseEvent * evt) {
		QDialog::closeEvent(evt);
		reject();
	}

	void SpineEditor::checkSpineVersion() {
		QDirIterator it(_model->getPath() + "/_work/data/Scripts/Content/", QStringList() << "Spine_Constants.d", QDir::Files, QDirIterator::Subdirectories);
		if (it.hasNext()) {
			it.next();
			QFile f(it.filePath());
			if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream ts(&f);
				while (!ts.atEnd()) {
					QString line = ts.readLine();
					if (line.contains("SPINE_VERSION_STRING")) {
						line = line.replace(QRegExp("[ \t\"]"), "");
						line = line.split("=").back();
						line = line.split(";").front();
						if (line != QString::fromStdString(SPINE_SCRIPT_VERSION)) {
							_updateSpineButton->show();
						} else {
							_updateSpineButton->hide();
						}
						break;
					}
				}
			}
			_installSpineButton->hide();
		} else {
			_installSpineButton->show();
		}
	}

	void SpineEditor::checkIkarusVersion() {
		QDirIterator it(_model->getPath() + "/_work/data/Scripts/Content/", QStringList() << "Ikarus.d", QDir::Files, QDirIterator::Subdirectories);
		if (it.hasNext()) {
			it.next();
			QFile f(it.filePath());
			if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream ts(&f);
				while (!ts.atEnd()) {
					QString line = ts.readLine();
					if (line.contains("Version")) {
						line = line.replace(QRegExp("[ \t]"), "");
						line = line.split(":").back();
						if (line != QString::fromStdString(IKARUS_VERSION)) {
							_updateIkarusButton->show();
						} else {
							_updateIkarusButton->hide();
						}
						break;
					}
				}
			}
			_installIkarusButton->hide();
		} else {
			_installIkarusButton->show();
		}
	}

	void SpineEditor::checkIkarusInitialized() {}

	void SpineEditor::checkLeGoVersion() {
		QDirIterator it(_model->getPath() + "/_work/data/Scripts/Content/", QStringList() << "LeGo.d", QDir::Files, QDirIterator::Subdirectories);
		if (it.hasNext()) {
			it.next();
			QFile f(it.filePath());
			if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream ts(&f);
				while (!ts.atEnd()) {
					QString line = ts.readLine();
					if (line.contains("LeGo_Version")) {
						line = line.replace(QRegExp("[ \t\"]"), "");
						line = line.split("=").back();
						line = line.split(";").front();
						line = line.replace("LeGo", "");
						if (line != QString::fromStdString(LEGO_VERSION)) {
							_updateLeGoButton->show();
						} else {
							_updateLeGoButton->hide();
						}
						break;
					}
				}
			}
			_installLeGoButton->hide();
		} else {
			_installLeGoButton->show();
		}
	}

	void SpineEditor::checkLeGoInitialized() {}

	void SpineEditor::restoreSettings() {
		const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/SpineEditorGeometry", QByteArray()).toByteArray();
		if (!restoreGeometry(arr)) {
			Config::IniParser->remove("WINDOWGEOMETRY/SpineEditorGeometry");
		}
	}

	void SpineEditor::saveSettings() {
		Config::IniParser->setValue("WINDOWGEOMETRY/SpineEditorGeometry", saveGeometry());
	}

	void SpineEditor::loadMods() {
		QtConcurrent::run([this]() {
			common::RequestModsForEditorMessage rmfem;
			rmfem.username = Config::Username.toStdString();
			rmfem.password = Config::Password.toStdString();
			rmfem.language = Config::Language.toStdString();
			std::string serialized = rmfem.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendModsForEditorMessage * smfem = dynamic_cast<common::SendModsForEditorMessage *>(m);
							_modList = smfem->modList;
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << int(cErr);
				}
			}
		});
	}

} /* namespace widgets */
} /* namespace spine */
