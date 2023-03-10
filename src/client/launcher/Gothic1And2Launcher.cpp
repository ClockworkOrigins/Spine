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

#include "launcher/Gothic1And2Launcher.h"

#include "InstallMode.h"
#include "LibraryFilterModel.h"
#include "SpineConfig.h"

#include "client/IconCache.h"

#include "client/widgets/UpdateLanguage.h"

#include "common/GameType.h"
#include "common/MessageStructs.h"

#include "gui/DontShowAgainDialog.h"

#include "https/Https.h"

#include "launcher/LauncherFactory.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/Hashing.h"
#include "utils/WindowsExtensions.h"

#include "widgets/MainWindow.h"
#include "widgets/SubmitCompatibilityDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QDirIterator>
#include <QFile>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QPushButton>
#include <QQueue>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QSplashScreen>
#include <QStandardItemModel>
#include <QtConcurrentRun>

using namespace spine;
using namespace spine::client;
using namespace spine::gui;
using namespace spine::https;
using namespace spine::launcher;
using namespace spine::utils;

namespace {
	
	struct ModVersion {
		common::GameType gothicVersion;
		int majorVersion;
		int minorVersion;
		int patchVersion;
		ModVersion() : gothicVersion(), majorVersion(), minorVersion(), patchVersion() {
		}
		ModVersion(int i1, int i2, int i3, int i4) : gothicVersion(static_cast<common::GameType>(i1)), majorVersion(i2), minorVersion(i3), patchVersion(i4) {
		}
	};

	struct Patch {
		int32_t modID;
		std::string name;

		Patch() : modID(-1) {
		}
		Patch(std::string m, std::string n) : modID(std::stoi(m)), name(n) {
		}
	};

}

void Gothic1And2Launcher::init() {
	ILauncher::init();

	_patchCounter = 0;
	_hideIncompatible = true;
	_gmpCounterBackup = -1;
	_zSpyActivated = false;

	_running = false;

	_networkAccessManager = new QNetworkAccessManager(this);

	qRegisterMetaType<std::vector<int32_t>>("std::vector<int32_t>");

	connect(this, &Gothic1And2Launcher::receivedModStats, this, &Gothic1And2Launcher::updateModInfoView);
	connect(this, &Gothic1And2Launcher::receivedCompatibilityList, this, &Gothic1And2Launcher::updateCompatibilityList);
	
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "CREATE TABLE IF NOT EXISTS patchConfigs(ModID INT NOT NULL, PatchID INT NOT NULL, Enabled INT NOT NULL);", err);

	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS usedFiles (File TEXT PRIMARY KEY);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS last_directory (Path TEXT PRIMARY KEY);", err);
	
	const auto files = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM usedFiles;", err);
	if (!files.empty()) {
		_lastBaseDir = QString::fromStdString(Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM last_directory LIMIT 1;", err, 0));
	}
	for (const std::string & s : files) {
		_copiedFiles.append(QString::fromStdString(s));
	}
	if (!_lastBaseDir.isEmpty()) {
		for (const QString & file : _copiedFiles) {
			QFile(_lastBaseDir + "/" + file).remove();
		}
	}
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles;", err);
	_copiedFiles.clear();

	if (!_lastBaseDir.isEmpty()) {
		QDirIterator itBackup(_lastBaseDir, QStringList() << "*.spbak", QDir::Files, QDirIterator::Subdirectories);
		QStringList backupFiles;
		while (itBackup.hasNext()) {
			itBackup.next();
			QString backupFileName = itBackup.filePath();
			if (!backupFileName.isEmpty()) {
				backupFiles.append(backupFileName);
			}
		}
		for (QString backupFile : backupFiles) {
			QFile f(backupFile);
			backupFile.chop(6);
			f.rename(backupFile);
		}
		removeEmptyDirs();
	}
}

bool Gothic1And2Launcher::supportsModAndIni(int32_t modID, const QString & iniFile) const {
	if (modID == -1) {
		auto dir = _directory;

		if (!dir.endsWith("/") && !dir.endsWith("\\")) {
			dir += "/";
		}
		
		return !_directory.isEmpty() && iniFile.contains(dir, Qt::CaseInsensitive);
	}

	Database::DBError err;
	const int gvInt = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(modID) + " LIMIT 1;", err, 0);
	const auto gv = static_cast<common::GameType>(gvInt);

	return supportsGame(gv);
}

void Gothic1And2Launcher::setDirectory(const QString & directory) {
	_directory = directory;

	QSettings systempackIni(QString("%1/system/Systempack.ini").arg(_directory), QSettings::IniFormat);
	systempackIni.setValue("GAME/SaveDATinASCII", 0);

	const auto outputPath = QStringLiteral("%1/_work/data/Scripts/Content/Cutscene/%2");
	
	if (QFileInfo::exists(outputPath.arg(_directory).arg("OU.LSC"))) {
		QFile::rename(outputPath.arg(_directory).arg("OU.LSC"), outputPath.arg(_directory).arg("OU.CSL"));
	}

	auto b = QDir(directory).mkpath(QStringLiteral("%1/_work/Data/Anims/_compiled").arg(directory));
	Q_UNUSED(b)

	b = QDir(directory).mkpath(QStringLiteral("%1/_work/Data/Meshes/_compiled").arg(directory));
	Q_UNUSED(b)

	b = QDir(directory).mkpath(QStringLiteral("%1/_work/Data/Textures/_compiled").arg(directory));
	Q_UNUSED(b)
}

void Gothic1And2Launcher::setHideIncompatible(bool enabled) {
	_hideIncompatible = enabled;
}

void Gothic1And2Launcher::createWidget() {
	ILauncher::createWidget();

	{
		_startSpacerButton = new QPushButton(QApplication::tr("StartSpacer"), _widget);
		_startSpacerButton->setProperty("library", true);
		UPDATELANGUAGESETTEXT(_startSpacerButton, "StartSpacer");
		_upperLayout->insertWidget(1, _startSpacerButton, 0, Qt::AlignLeft);
		_startSpacerButton->hide();
		connect(_startSpacerButton, &QPushButton::released, this, &Gothic1And2Launcher::startSpacer);
	}

	_adminInfoLabel = new QLabel(QApplication::tr("GothicAdminNote"), _widget);
	_adminInfoLabel->setProperty("library", true);
	_adminInfoLabel->setAlignment(Qt::AlignCenter);
	_adminInfoLabel->setWordWrap(true);
	_adminInfoLabel->setProperty("adminInfo", true);
	_adminInfoLabel->hide();

	_errorInfoLabel = new QLabel(QApplication::tr("NeitherSystempackNorUnionWarning"), _widget);
	_errorInfoLabel->setProperty("library", true);
	_errorInfoLabel->setAlignment(Qt::AlignCenter);
	_errorInfoLabel->setWordWrap(true);
	_errorInfoLabel->setProperty("adminInfo", true);
	_errorInfoLabel->hide();

	_nameLabel = new QLabel(_widget);
	_nameLabel->setProperty("library", true);
	_nameLabel->setAlignment(Qt::AlignCenter);
	_versionLabel = new QLabel(_widget);
	_versionLabel->setProperty("library", true);
	_versionLabel->setAlignment(Qt::AlignCenter);
	_teamLabel = new QLabel(_widget);
	_teamLabel->setProperty("library", true);
	_teamLabel->setAlignment(Qt::AlignCenter);
	_contactLabel = new QLabel(_widget);
	_contactLabel->setProperty("library", true);
	_contactLabel->setAlignment(Qt::AlignCenter);
	_contactLabel->setOpenExternalLinks(true);
	_homepageLabel = new QLabel(_widget);
	_homepageLabel->setProperty("library", true);
	_homepageLabel->setAlignment(Qt::AlignCenter);
	_homepageLabel->setOpenExternalLinks(true);
	
	_layout->insertWidget(4, _adminInfoLabel);
	_layout->insertWidget(5, _errorInfoLabel);
	_layout->insertWidget(6, _nameLabel);
	_layout->insertWidget(7, _versionLabel);
	_layout->insertWidget(8, _teamLabel);
	_layout->insertWidget(9, _contactLabel);
	_layout->insertWidget(10, _homepageLabel);

	_compileScripts = new QCheckBox(QApplication::tr("CompileScripts"), _widget);
	_compileScripts->setProperty("library", true);
	UPDATELANGUAGESETTEXT(_compileScripts, "CompileScripts");
	_compileScripts->hide();
	connect(_compileScripts, &QCheckBox::stateChanged, [this]() {
		const auto launcher = LauncherFactory::getInstance()->getLauncher(getGothicVersion() == common::GameType::Gothic ? common::GameType::Gothic2 : common::GameType::Gothic);
		const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->_compileScripts->setChecked(_compileScripts->isChecked());
	});

	_startupWindowed = new QCheckBox(QApplication::tr("StartupWindowed"), _widget);
	_startupWindowed->setProperty("library", true);
	UPDATELANGUAGESETTEXT(_startupWindowed, "StartupWindowed");
	_startupWindowed->hide();
	connect(_startupWindowed, &QCheckBox::stateChanged, [this]() {
		const auto launcher = LauncherFactory::getInstance()->getLauncher(getGothicVersion() == common::GameType::Gothic ? common::GameType::Gothic2 : common::GameType::Gothic);
		const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->_startupWindowed->setChecked(_startupWindowed->isChecked());
	});

	_convertTextures = new QCheckBox(QApplication::tr("ConvertTextures"), _widget);
	_convertTextures->setProperty("library", true);
	UPDATELANGUAGESETTEXT(_convertTextures, "ConvertTextures");
	_convertTextures->hide();
	connect(_convertTextures, &QCheckBox::stateChanged, [this]() {
		const auto launcher = LauncherFactory::getInstance()->getLauncher(getGothicVersion() == common::GameType::Gothic ? common::GameType::Gothic2 : common::GameType::Gothic);
		const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->_convertTextures->setChecked(_convertTextures->isChecked());
	});

	_noSound = new QCheckBox(QApplication::tr("DeactivateSound"), _widget);
	_noSound->setProperty("library", true);
	UPDATELANGUAGESETTEXT(_noSound, "DeactivateSound");
	_noSound->hide();
	connect(_noSound, &QCheckBox::stateChanged, [this]() {
		const auto launcher = LauncherFactory::getInstance()->getLauncher(getGothicVersion() == common::GameType::Gothic ? common::GameType::Gothic2 : common::GameType::Gothic);
		const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->_noSound->setChecked(_noSound->isChecked());
	});

	_noMusic = new QCheckBox(QApplication::tr("DeactivateMusic"), _widget);
	_noMusic->setProperty("library", true);
	UPDATELANGUAGESETTEXT(_noMusic, "DeactivateMusic");
	_noMusic->hide();
	connect(_noMusic, &QCheckBox::stateChanged, [this]() {
		const auto launcher = LauncherFactory::getInstance()->getLauncher(getGothicVersion() == common::GameType::Gothic ? common::GameType::Gothic2 : common::GameType::Gothic);
		const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->_noMusic->setChecked(_noMusic->isChecked());
	});

	auto * hl = new QHBoxLayout();
	{
		_patchGroup = new QGroupBox(QApplication::tr("PatchesAndTools"), _widget);
		UPDATELANGUAGESETTITLE(_patchGroup, "PatchesAndTools");
		_patchGroup->setProperty("library", true);

		{
			_patchLayout = new QGridLayout();
			_patchLayout->setColumnStretch(2, 1);

			auto * vb = new QVBoxLayout();
			auto * sa = new QScrollArea(_patchGroup);
			auto * cw = new QWidget(sa);
			vb->setAlignment(Qt::AlignTop);
			cw->setLayout(_patchLayout);
			sa->setWidget(cw);
			sa->setWidgetResizable(true);
			sa->setProperty("default", true);
			cw->setProperty("default", true);

			vb->addWidget(sa);
		
			_patchGroup->setLayout(vb);
			_patchGroup->hide();
		}

		_pdfGroup = new QGroupBox(QApplication::tr("PDFs"), _widget);
		UPDATELANGUAGESETTITLE(_pdfGroup, "PDFs");
		_pdfGroup->setProperty("library", true);
		_pdfLayout = new QVBoxLayout();
		_pdfGroup->setLayout(_pdfLayout);
		_pdfGroup->hide();

		hl->addWidget(_patchGroup);
		hl->addWidget(_pdfGroup);
	}
	auto * zSpyLayout = new QHBoxLayout();
	{
		_zSpyLabel = new QLabel(QApplication::tr("zSpyLevel"), _widget);
		_zSpyLabel->setProperty("library", true);
		_zSpyLabel->hide();
		UPDATELANGUAGESETTEXT(_zSpyLabel, "zSpyLevel");

		_zSpyLevel = new QSlider(Qt::Orientation::Horizontal, _widget);
		_zSpyLevel->setProperty("library", true);
		_zSpyLevel->setMinimum(0);
		_zSpyLevel->setMaximum(10);
		_zSpyLevel->setPageStep(1);
		_zSpyLevel->hide();

		zSpyLayout->addWidget(_zSpyLabel);
		zSpyLayout->addWidget(_zSpyLevel);
	}
	
	_layout->addWidget(_compileScripts);
	_layout->addWidget(_startupWindowed);
	_layout->addWidget(_convertTextures);
	_layout->addWidget(_noSound);
	_layout->addWidget(_noMusic);
	_layout->addLayout(zSpyLayout);
	_layout->addLayout(hl);
}

void Gothic1And2Launcher::setDeveloperMode(bool enabled) {
	ILauncher::setDeveloperMode(enabled);
	
	_compileScripts->setVisible(enabled);
	_startupWindowed->setVisible(enabled);
	_startSpacerButton->setVisible(enabled);
	_convertTextures->setVisible(enabled);
	_noSound->setVisible(enabled);
	_noMusic->setVisible(enabled);
	_zSpyLabel->setVisible(enabled);
	_zSpyLevel->setVisible(enabled);
}

void Gothic1And2Launcher::updateCompatibilityList(int modID, std::vector<int32_t> incompatiblePatches, std::vector<int32_t> forbiddenPatches) {
	if (_projectID != modID) return;

	for (QCheckBox * cb : _patchList) {
		_patchLayout->removeWidget(cb);
		delete cb;
	}
	_patchList.clear();
	_checkboxPatchIDMapping.clear();

	Database::DBError err;
	const auto modGv = Database::queryNth<ModVersion, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion, MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = " + std::to_string(_projectID) + " LIMIT 1;", err, 0);

	const auto contains = [](const std::vector<int32_t> & list, int32_t id) {
		const auto it = std::find_if(list.begin(), list.end(), [id](const int32_t & a) {
			return a == id;
		});
		return it != list.end();
	};

	const auto patches = Database::queryAll<Patch, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, Name FROM patches WHERE ModID != 228 ORDER BY Name ASC;", err);
	for (const Patch & p : patches) {
		const common::GameType gv = static_cast<common::GameType>(std::stoi(Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(p.modID) + " LIMIT 1;", err, 0)));
		if ((gv == modGv.gothicVersion || ((modGv.gothicVersion == common::GameType::Gothic || modGv.gothicVersion == common::GameType::Gothic2 || modGv.gothicVersion == common::GameType::GothicInGothic2) && gv == common::GameType::Gothic1And2)) && (!contains(incompatiblePatches, p.modID) || !_hideIncompatible) && !contains(forbiddenPatches, p.modID)) {
			auto * cb = new QCheckBox(s2q(p.name), _widget);
			cb->setProperty("library", true);
			_patchLayout->addWidget(cb, _patchCounter / 2, _patchCounter % 2);
			++_patchCounter;
			_patchList.append(cb);
			const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT Enabled FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + " AND PatchID = " + std::to_string(p.modID) + " LIMIT 1;", err);
			cb->setChecked(count);
			_checkboxPatchIDMapping.insert(cb, p.modID);
			connect(cb, &QCheckBox::stateChanged, this, &Gothic1And2Launcher::changedPatchState, Qt::QueuedConnection);
			cb->setProperty("patchID", p.modID);
		} else if (contains(forbiddenPatches, p.modID)) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + " AND PatchID = " + std::to_string(p.modID) + ";", err);
		}
	}

	updatePatchCheckboxes();
	
	if (_patchCounter % 2 == 1) {
		++_patchCounter;
	}
	if (!_patchList.empty()) {
		_patchGroup->show();
		_patchGroup->setToolTip(QApplication::tr("PatchesAndToolsTooltip").arg(_nameLabel->text()));
	}
}

void Gothic1And2Launcher::startSpacer() {
	QString usedExecutable;
	if (QFileInfo::exists(_directory + "/System/Spacer.exe")) {
		usedExecutable = "Spacer.exe";
	} else if (QFileInfo::exists(_directory + "/System/Spacer2_exe.exe")) {
		usedExecutable = "Spacer2_exe.exe";
	} else if (QFileInfo::exists(_directory + "/System/Spacer2.exe")) {
		usedExecutable = "Spacer2.exe";
	}
	// check overrides and backup original values!
	_gothicIniBackup.clear();
	_systempackIniBackup.clear();
	QSettings iniParser(_iniFile, QSettings::IniFormat);
	{
		QSettings gothicIniParser(_directory + "/System/Gothic.ini", QSettings::IniFormat);
		iniParser.beginGroup("OVERRIDES");
		QStringList entries = iniParser.allKeys();
		for (const QString & s : entries) {
			const QString & entry = s;
			QString section = entry.split(".").front();
			QString key = entry.split(".").back();
			// backup original value
			QString val = gothicIniParser.value(section + "/" + key, "---").toString();
			_gothicIniBackup.append(std::make_tuple(section, key, val));
			val = iniParser.value(entry, "---").toString();
			if (val != "---") {
				gothicIniParser.setValue(section + "/" + key, val);
			}
		}
		iniParser.endGroup();
	}
	{
		QSettings systempackIniParser(_directory + "/System/Systempack.ini", QSettings::IniFormat);
		iniParser.beginGroup("OVERRIDES_SP");
		QStringList entries = iniParser.allKeys();
		for (const QString & s : entries) {
			const QString & entry = s;
			QString section = entry.split(".").front();
			QString key = entry.split(".").back();
			// backup original value
			QString val = systempackIniParser.value(section + "/" + key, "---").toString();
			_systempackIniBackup.append(std::make_tuple(section, key, val));
			val = iniParser.value(entry, "---").toString();
			if (val != "---") {
				systempackIniParser.setValue(section + "/" + key, val);
			}
		}
		iniParser.endGroup();
	}
	auto * process = new QProcess(this);
	connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Gothic1And2Launcher::finishedSpacer);
	connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), process, &Gothic1And2Launcher::deleteLater);
	process->setWorkingDirectory(_directory + "/System/");
	QStringList args;
	args << "-zmaxframerate:50";
	process->start("\"" + _directory + "/System/" + usedExecutable + "\"", args);
	_widget->setDisabled(true);
}

void Gothic1And2Launcher::finishedSpacer() {
	{
		QSettings gothicIniParser(_directory + "/System/Gothic.ini", QSettings::IniFormat);
		for (auto & t : _gothicIniBackup) {
			const QString section = std::get<0>(t);
			const QString key = std::get<1>(t);
			const QString value = std::get<2>(t);
			if (value != "---") {
				gothicIniParser.setValue(section + "/" + key, value);
			}
		}
	}
	{
		QSettings systempackIniParser(_directory + "/System/Systempack.ini", QSettings::IniFormat);
		for (auto & t : _systempackIniBackup) {
			const QString section = std::get<0>(t);
			const QString key = std::get<1>(t);
			const QString value = std::get<2>(t);
			if (value != "---") {
				systempackIniParser.setValue(section + "/" + key, value);
			}
		}
	}
	_gothicIniBackup.clear();
	_systempackIniBackup.clear();

	_widget->setEnabled(true);
}

void Gothic1And2Launcher::removeEmptyDirs() const {
	bool deleted;
	do {
		deleted = false;
		QDirIterator it(_lastBaseDir, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoSymLinks, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			const QString path = it.filePath();
			QDir dir(path);
			if (dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).isEmpty() && canRemoveDir(dir.absolutePath()) ) {
				deleted |= dir.rmdir(path);
			}
		}
	} while (deleted);
}

bool Gothic1And2Launcher::canRemoveDir(const QString & path) {
	if (path.contains("saves_", Qt::CaseInsensitive))
		return false;

	if (path.contains("_work/data/meshes/_compiled", Qt::CaseInsensitive))
		return false;

	if (path.contains("_work/data/anims/_compiled", Qt::CaseInsensitive))
		return false;

	if (path.contains("_work/data/textures/_compiled", Qt::CaseInsensitive))
		return false;

	return true;
}

void Gothic1And2Launcher::restoreSettings() {
	Config::IniParser->beginGroup("DEVELOPER");
		bool b = Config::IniParser->value("CompileScripts", false).toBool();
		_compileScripts->setChecked(b);
		b = Config::IniParser->value("WindowedMode", false).toBool();
		_startupWindowed->setChecked(b);
		const int i = Config::IniParser->value("zSpyLevel", 10).toInt();
		_zSpyLevel->setValue(i);
		b = Config::IniParser->value("ConvertTextures", false).toBool();
		_convertTextures->setChecked(b);
		b = Config::IniParser->value("NoSound", false).toBool();
		_noSound->setChecked(b);
		b = Config::IniParser->value("NoMusic", false).toBool();
		_noMusic->setChecked(b);
	Config::IniParser->endGroup();
}

void Gothic1And2Launcher::saveSettings() {
	Config::IniParser->beginGroup("DEVELOPER");
		Config::IniParser->setValue("CompileScripts", _compileScripts->isChecked());
		Config::IniParser->setValue("WindowedMode", _startupWindowed->isChecked());
		Config::IniParser->setValue("zSpyLevel", _zSpyLevel->value());
		Config::IniParser->setValue("ConvertTextures", _convertTextures->isChecked());
		Config::IniParser->setValue("NoSound", _noSound->isChecked());
		Config::IniParser->setValue("NoMusic", _noMusic->isChecked());
	Config::IniParser->endGroup();
}

void Gothic1And2Launcher::updateView(int modID, const QString & iniFile) {
	if (iniFile.isEmpty()) {
		_iniFile = getDefaultIni(modID);
	} else {
		_iniFile = iniFile;
	}

	QSettings iniParser(_iniFile, QSettings::IniFormat);
	iniParser.beginGroup("INFO");
		const QString title = iniParser.value("Title", "").toString();
		const QString version = iniParser.value("Version", "").toString();
		const QString team = iniParser.value("Authors", "").toString();
		QString homepage = iniParser.value("Webpage", "").toString();
		const QString contact = iniParser.value("Contact", "").toString();
		const QString description = iniParser.value("Description", "").toString();
		const bool requiresAdmin = iniParser.value("RequiresAdmin", false).toBool();
	iniParser.endGroup();

	updateCommonView(modID, title);

	if (!title.isEmpty()) {
		_nameLabel->setText(title);
	}
	if (!version.isEmpty()) {
		_versionLabel->setText(version);
	}
	_teamLabel->setText(team.isEmpty() ? team : "");

	_teamLabel->setVisible(!team.isEmpty());

	if (!contact.isEmpty() && Config::OnlineMode) {
		_contactLabel->setText("<a href=\"mailto:" + contact + "\">" + contact + "</a>");
	} else {
		_contactLabel->setText("");
	}
	_contactLabel->setVisible(!contact.isEmpty());

	if (Config::OnlineMode) {
		const QUrl url(homepage);
		const QNetworkRequest request(url);
		QNetworkReply * reply = _networkAccessManager->get(request);
		reply->waitForReadyRead(2000);
		if (reply->error() != QNetworkReply::NoError) {
			homepage.clear();
		}
	} else {
		homepage.clear();
	}
	if (!homepage.isEmpty()) {
		_homepageLabel->setText("<a href=\"" + homepage + "\">" + homepage + "</a>");
	} else {
		_homepageLabel->setText("");
	}
	_homepageLabel->setVisible(!homepage.isEmpty());

	QString programFiles = QProcessEnvironment::systemEnvironment().value("ProgramFiles", "Foobar123");
	programFiles = programFiles.replace("\\", "/");
	QString programFilesx86 = QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", "Foobar123");
	programFilesx86 = programFilesx86.replace("\\", "/");
	if (_iniFile.contains(programFiles, Qt::CaseInsensitive) || _iniFile.contains(programFilesx86, Qt::CaseInsensitive) || requiresAdmin) {
#ifdef Q_OS_WIN
		_startButton->setEnabled(IsRunAsAdmin() && !_runningUpdates.contains(_projectID));
		_adminInfoLabel->setVisible(!IsRunAsAdmin());
#else
		_startButton->setEnabled(!_runningUpdates.contains(_projectID));
		_adminInfoLabel->setVisible(false);
#endif
		_adminInfoLabel->setText(requiresAdmin ? QApplication::tr("GothicAdminInfoRequiresAdmin").arg(title) : QApplication::tr("GothicAdminInfo").arg(title));
	} else {
		_startButton->setEnabled(!_runningUpdates.contains(_projectID));
		_adminInfoLabel->hide();
	}

	// more stuff ^^

	for (QCheckBox * cb : _patchList) {
		_patchLayout->removeWidget(cb);
		delete cb;
	}
	_patchList.clear();
	for (QLabel * l : _pdfList) {
		_pdfLayout->removeWidget(l);
		delete l;
	}
	_pdfList.clear();
	_checkboxPatchIDMapping.clear();
	_patchGroup->hide();
	_pdfGroup->hide();

	if (_projectID != -1) {
		Database::DBError err;
		const ModVersion modGv = Database::queryNth<ModVersion, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion, MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = " + std::to_string(_projectID) + " LIMIT 1;", err, 0);
		
		{
			QDirIterator it(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID) + "/", QStringList() << "*.pdf", QDir::Filter::Files);
			while (it.hasNext()) {
				it.next();
				auto * l = new QLabel("<a href=\"file:///" + it.filePath().toHtmlEscaped() + R"(" style="color: #181C22">)" + QFileInfo(it.fileName()).fileName() + "</a>", _pdfGroup);
				l->setProperty("library", true);
				l->setOpenExternalLinks(true);
				_pdfLayout->addWidget(l);
				_pdfList.append(l);
			}
		}
		if (!_pdfList.empty()) {
			auto * l = new QLabel(_pdfGroup);
			l->setProperty("library", true);
			_pdfLayout->addWidget(l);
			_pdfLayout->setStretchFactor(l, 1);
			_pdfList.append(l);
			_pdfGroup->show();
		}
		_versionLabel->setText(QString("%1.%2.%3").arg(modGv.majorVersion).arg(modGv.minorVersion).arg(modGv.patchVersion));

		updateCompatibilityList(_projectID, {}, {});
	} else {
		Database::DBError err;
		const auto patches = Database::queryAll<Patch, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, Name FROM patches;", err);

		const auto gothicVersion = getGothicVersion();
		
		for (const Patch & p : patches) {
			const common::GameType gv = static_cast<common::GameType>(std::stoi(Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(p.modID) + " LIMIT 1;", err, 0)));
			if (gv == gothicVersion || ((gothicVersion == common::GameType::Gothic || gothicVersion == common::GameType::Gothic2 || gothicVersion == common::GameType::GothicInGothic2) && gv == common::GameType::Gothic1And2)) {
				auto * cb = new QCheckBox(s2q(p.name), _widget);
				cb->setProperty("library", true);
				_patchLayout->addWidget(cb);
				_patchList.append(cb);
				const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT Enabled FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + " AND PatchID = " + std::to_string(p.modID) + " LIMIT 1;", err);
				cb->setChecked(count);
				_checkboxPatchIDMapping.insert(cb, p.modID);
				connect(cb, &QCheckBox::stateChanged, this, &Gothic1And2Launcher::changedPatchState, Qt::QueuedConnection);
				_patchGroup->show();
				_patchGroup->setToolTip(QApplication::tr("PatchesAndToolsTooltip").arg(_nameLabel->text()));
				_pdfGroup->hide();
			}
		}

		updatePatchCheckboxes();
	}
	
	if (_projectID == 36 || _projectID == 37 || _projectID == 116) {
		_startButton->setText(QApplication::tr("StartGame"));
		UPDATELANGUAGESETTEXT(_startButton, "StartGame");
	} else {
		_startButton->setText(QApplication::tr("StartMod"));
		UPDATELANGUAGESETTEXT(_startButton, "StartMod");
	}
}

void Gothic1And2Launcher::start() {
	if (_iniFile.isEmpty()) return;

	widgets::MainWindow::getInstance()->setDisabled(true);
	_oldWindowState = widgets::MainWindow::getInstance()->windowState();
	widgets::MainWindow::getInstance()->setWindowState(Qt::WindowState::WindowMinimized);

	QString splashImage = ":/SpineSplash.png";
	_splashTextColor = Qt::black;

	QSettings splashParser(_iniFile, QSettings::IniFormat);
	QString splashImageStr = splashParser.value("SETTINGS/Splash", "").toString();
	if (!splashImageStr.isEmpty()) {
		splashImage = _directory + "/" + splashImageStr;
	}
	QString splashColorStr = splashParser.value("SETTINGS/Splash", "").toString();
	if (!splashColorStr.isEmpty()) {
		_splashTextColor = QColor(splashColorStr);
	}

	QSplashScreen splash;
	QPixmap splashPixmap(splashImage);
	if (splashPixmap.isNull()) {
		splashPixmap = QPixmap(":/SpineSplash.png"); // fallback
	}
	splash.setPixmap(splashPixmap);
	splash.setWindowFlags(splash.windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	splash.show();
	connect(this, &Gothic1And2Launcher::changeSplashMessage, &splash, &QSplashScreen::showMessage);
	QFutureWatcher<bool> watcher(this);
	QEventLoop loop;
	connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
	QStringList backgroundExecutables;
	QSet<QString> dependencies;
	QString usedExecutable;
	bool newGMP = false;
	Renderer renderer = Renderer::None;

	QElapsedTimer t;
	t.start();
	QFuture<bool> future = QtConcurrent::run<bool>(this, &Gothic1And2Launcher::prepareModStart, &usedExecutable, &backgroundExecutables, &newGMP, &dependencies, &renderer);
	watcher.setFuture(future);
	loop.exec();

	if (!future.result()) {
		splash.hide();
		if (!dependencies.isEmpty()) {
			Https::postAsync(DATABASESERVER_PORT, "getModnameForIDs", QString("{ \"Language\": \"%1\", \"ModIDs\": [ %2 ] }").arg(Config::Language).arg(dependencies.values().join(',')), [this, dependencies](const QJsonObject & json, int status) {
				QString msg = QApplication::tr("DependenciesMissing");
				if (status != 200) {
					for (const auto & p : dependencies) {
						msg += "\n- " + p;
					}
				} else {
					const auto it = json.find("Names");
					if (it != json.end()) {
						const auto jsonArray = it->toArray();
						for (const auto & i : jsonArray) {
							msg += "\n- " + i.toString();
						}
					}
				}
				emit errorMessage(msg, true, [this, dependencies] {
					for (const auto & s : dependencies) {
						const auto id = s.toInt();

						emit installMod(id, -1, InstallMode::Silent);
					}
				});
			});
		}
		widgets::MainWindow::getInstance()->setEnabled(true);
		widgets::MainWindow::getInstance()->setWindowState(_oldWindowState);
		return;
	}
	auto * process = new QProcess(this);
	for (const QString & backgroundProcess : backgroundExecutables) {
		auto * bp = new QProcess(process);
		connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), bp, &QProcess::terminate);
		bp->setWorkingDirectory(_directory + "/" + QFileInfo(backgroundProcess).path());
		bp->start("\"" + _directory + "/" + backgroundProcess + "\"", QStringList());
	}
	connect(process, &QProcess::errorOccurred, this, &Gothic1And2Launcher::errorOccurred);
	connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Gothic1And2Launcher::finishedMod, Qt::QueuedConnection);
	process->setWorkingDirectory(_directory + "/System/");
	if (Config::extendedLogging) {
		LOGINFO("Preparation duration: " << t.elapsed())
	}
	QStringList args;
	args << "-game:" + _iniFile.split("/").back();
	if (_zSpyActivated && _zSpyLevel->value() > 0) {
		QProcess::startDetached("\"" + _directory + "/_work/tools/zSpy/zSpy.exe\"", QStringList());
		args << "-zlog:" + QString::number(_zSpyLevel->value()) + ",s";
		LOGINFO("Started zSpy")
	}
	if (!Config::Username.isEmpty() && Config::OnlineMode) {
		QJsonObject json;
		QJsonArray jsonArray;

		{
			QSettings gothicIniParser(_directory + "/System/Gothic.ini", QSettings::IniFormat);
			QString zVidResFullscreenX = gothicIniParser.value("VIDEO/zVidResFullscreenX", "0").toString();
			QString zVidResFullscreenY = gothicIniParser.value("VIDEO/zVidResFullscreenY", "0").toString();

			{
				QJsonObject entry;
				entry["Entry"] = "zVidResFullscreenX";
				entry["Value"] = zVidResFullscreenX;

				jsonArray << entry;
			}
			{
				QJsonObject entry;
				entry["Entry"] = "zVidResFullscreenY";
				entry["Value"] = zVidResFullscreenY;

				jsonArray << entry;
			}
			
			gothicIniParser.beginGroup("KEYS");
			QStringList entries = gothicIniParser.allKeys();
			for (const QString & s : entries) {
				QString value = gothicIniParser.value(s, "").toString();
				if (!value.isEmpty()) {
					QJsonObject entry;
					entry["Entry"] = s;
					entry["Value"] = value;

					jsonArray << entry;
				}
			}
			gothicIniParser.endGroup();
		}
		json["Settings"] = jsonArray;
		sendUserInfos(json);
	}
	if (_developerMode && _compileScripts->isChecked()) {
		args << "-zreparse";
	}
	if (_developerMode && _startupWindowed->isChecked()) {
		args << "-zwindow";
	}
	if (_developerMode && _convertTextures->isChecked()) {
		args << "-ztexconvert";
	}
	if (_developerMode && _noSound->isChecked()) {
		args << "-znosound";
	}
	if (_developerMode && _noMusic->isChecked()) {
		args << "-znomusic";
	}
#ifdef Q_OS_WIN
	if (newGMP) {
		usedExecutable = "gml.exe";
	}

	_running = true;
#endif

	if (newGMP) {
		if (QFileInfo::exists(_directory + "/" + usedExecutable) && QFileInfo(_directory + "/" + usedExecutable).isExecutable()) {
			process->start("\"" + _directory + "/" + usedExecutable + "\"", args);
		} else {
			startCommon();
			finishedMod(0, QProcess::CrashExit);
			return;
		}
	} else {
		if (usedExecutable.compare(getExecutable(), Qt::CaseInsensitive) == 0 && canBeStartedWithSteam()) {
			if (renderer != Renderer::Clockwork) {
				// Steam starts Gothic from the main directory, while Spine launches it from the System folder. That's basically no problem, but won't work when using the renderer as GD3D11 folder is expected in the working directory
				// not necessary to do for Clockwork renderer as it uses the proper folder
				linkOrCopyFolder(_directory + "/System/GD3D11", _directory + "/GD3D11");
			}
			startViaSteam(args);
		} else {
			if (QFileInfo::exists(_directory + "/System/" + usedExecutable) && QFileInfo(_directory + "/System/" + usedExecutable).isExecutable()) {
				process->start("\"" + _directory + "/System/" + usedExecutable + "\"", args);
			} else {
				startCommon();
				finishedMod(0, QProcess::CrashExit);
				return;
			}
		}
	}
	LOGINFO("Started " << usedExecutable.toStdString())

	startCommon();
}

void Gothic1And2Launcher::finishedMod(int, QProcess::ExitStatus status) {
	_running = false;
	
	LOGINFO("Finished Mod")
	if (Config::extendedLogging) {
		LOGINFO("Resetting ini files")
	}
	{
		QSettings gothicIniParser(_directory + "/System/Gothic.ini", QSettings::IniFormat);
		for (auto & t : _gothicIniBackup) {
			const QString section = std::get<0>(t);
			const QString key = std::get<1>(t);
			const QString value = std::get<2>(t);
			if (value != "---") {
				gothicIniParser.setValue(section + "/" + key, value);
			}
		}
	}
	if (status == QProcess::ExitStatus::CrashExit) {
		QFile(_directory + "/System/Gothic.ini").remove();
		QFile(_directory + "/System/Gothic.ini.spbak").rename(_directory + "/System/Gothic.ini");
	} else {
		QFile(_directory + "/System/Gothic.ini.spbak").remove();
	}
	{
		QSettings systempackIniParser(_directory + "/System/Systempack.ini", QSettings::IniFormat);
		for (auto & t : _systempackIniBackup) {
			const QString section = std::get<0>(t);
			const QString key = std::get<1>(t);
			const QString value = std::get<2>(t);
			if (value != "---") {
				systempackIniParser.setValue(section + "/" + key, value);
			}
		}
	}
	if (_gmpCounterBackup != -1) {
		QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\Public domain\GMP Launcher\Server)", QSettings::NativeFormat);
		registrySettings.setValue("size", _gmpCounterBackup);
		_gmpCounterBackup = -1;
	}
	if (QFileInfo::exists(_directory + "/System/pre.load")) {
		QString text;
		{
			QFile f(_directory + "/System/pre.load");
			f.open(QIODevice::ReadOnly);
			QTextStream ts(&f);
			text = ts.readAll();
		}
		for (const auto & s : _systempackPreLoads) {
			text = text.remove("\n" + s);
		}
		{
			QFile f(_directory + "/System/pre.load");
			f.open(QIODevice::WriteOnly);
			QTextStream ts(&f);
			ts << text;
		}
		_systempackPreLoads.clear();
	}
	
	if (!_unionPlugins.isEmpty() && QFileInfo::exists(_directory + "/System/Union.ini")) {
		QString text;
		{
			QFile f(_directory + "/System/Union.ini");
			f.open(QIODevice::ReadOnly);
			QTextStream ts(&f);
			text = ts.readAll();
		}
		const QRegularExpression regExp("(PluginList\\s=[^\n]*)\n");
		const QRegularExpressionMatch match = regExp.match(text);
		QString replaceText = match.captured(1);
		replaceText = replaceText.trimmed();
		if (!replaceText.isEmpty()) {
			text = text.replace(replaceText, "PluginList =");
			{
				QFile f(_directory + "/System/Union.ini");
				f.open(QIODevice::WriteOnly);
				QTextStream ts(&f);
				ts << text;
			}
		}
	}
	
	if (!_unionPlugins.isEmpty() && QFileInfo::exists(_directory + "/System/Systempack.ini")) {
		QString text;
		{
			QFile f(_directory + "/System/Systempack.ini");
			f.open(QIODevice::ReadOnly);
			QTextStream ts(&f);
			text = ts.readAll();
		}
		const QRegularExpression regExp("(PluginList[\\s]*=[^\n]*)\n");
		const QRegularExpressionMatch match = regExp.match(text);
		QString replaceText = match.captured(1);
		LOGINFO("Replacing #2: " << q2s(replaceText))
		replaceText = replaceText.trimmed();
		if (!replaceText.isEmpty()) {
			text = text.replace(replaceText, "PluginList=");
			{
				QFile f(_directory + "/System/Systempack.ini");
				f.open(QIODevice::WriteOnly);
				QTextStream ts(&f);
				ts << text;
			}
		}
	}
	
	_unionPlugins.clear();

	if (QFileInfo::exists(_directory + "/Data/_delete_me.vdf")) {
		QFile::remove(_directory + "/Data/_delete_me.vdf");
	}

	_gothicIniBackup.clear();
	_systempackIniBackup.clear();
	_skippedFiles.clear();

	auto duration = _timer->elapsed();

	stopCommon();

	if (duration > 0) {
		if (Config::OnlineMode && _projectID != -1) {
			Database::DBError err;
			const auto patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + ";", err);
			for (const std::string & patchIDString : patches) {
				const auto res = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "SELECT Compatible FROM ownCompatibilityVotes WHERE ModID = " + std::to_string(_projectID) + " AND PatchID = " + patchIDString + " LIMIT 1;", err);
				if (res.empty()) {
					const auto patchName = Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", err);
					DontShowAgainDialog dlg(QApplication::tr("NewCombinationDetected"), QApplication::tr("NewCombinationDetectedText").arg(_nameLabel->text()).arg(s2q(patchName)), "NEWCOMBINATIONDIALOG/DontShowAgain", _widget);
					if (dlg.canShow()) {
						if (dlg.exec() == QDialog::Accepted) {
							widgets::SubmitCompatibilityDialog submitDlg(_projectID, std::stoi(patchIDString), getGothicVersion());
							submitDlg.exec();
						}
					}
					break;
				}
			}
		} else {
			Database::DBError err;
			Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO playTimes (ModID, Username, Duration) VALUES (" + std::to_string(_projectID) + ", '" + Config::Username.toStdString() + "', " + std::to_string(duration) + ");", err);
			if (err.error) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE playTimes SET Duration = Duration + " + std::to_string(duration) + " WHERE ModID = " + std::to_string(_projectID) + " AND Username = '" + Config::Username.toStdString() + "';", err);
			}
		}
	}

	modFinished();
	
	removeModFiles();

	if (QFileInfo::exists(_directory + "/System/BugslayerUtilG.dll")) {
		QFile::rename(_directory + "/System/BugslayerUtilG.dll", _directory + "/System/BugslayerUtil.dll");
	}


	setDirectory(_directory);

	widgets::MainWindow::getInstance()->setEnabled(true);
	widgets::MainWindow::getInstance()->setWindowState(_oldWindowState);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "DELETE FROM lastPlayed;", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "INSERT INTO lastPlayed (ModID, Ini) VALUES (" + std::to_string(_projectID) + ", '" + _iniFile.toStdString() + "');", err);
}

void Gothic1And2Launcher::changedPatchState() {
	auto * cb = dynamic_cast<QCheckBox *>(sender());
	assert(cb);
	if (cb) {
		Database::DBError err;
		if (cb->isChecked()) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(_projectID) + ", " + std::to_string(_checkboxPatchIDMapping[cb]) + ", 1);", err);
		} else {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + " AND PatchID = " + std::to_string(_checkboxPatchIDMapping[cb]) + ";", err);
		}
	}

	updatePatchCheckboxes();
}

void Gothic1And2Launcher::removeModFiles() {
	if (!_lastBaseDir.isEmpty()) {
		QSet<QString> removed;
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "BEGIN TRANSACTION;", err);
		for (const QString & file : _copiedFiles) {
			if (QFileInfo::exists(_lastBaseDir + "/" + file)) {
				if (QFile(_lastBaseDir + "/" + file).remove()) {
					Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + file.toStdString() + "';", err);
					removed.insert(file);
					if (Config::extendedLogging) {
						LOGINFO("Removed file " << file.toStdString())
					}
				} else {
#ifdef Q_OS_WIN
					if (QFileInfo(_lastBaseDir + "/" + file).isSymLink()) {
						removeSymlink(_lastBaseDir + "/" + file);
					}
#endif
					if (Config::extendedLogging) {
						LOGINFO("Couldn't remove file " << file.toStdString())
					}
				}
			} else if (QDir(_lastBaseDir + "/" + file).exists()) {
				if (QDir(_lastBaseDir + "/" + file).remove(_lastBaseDir + "/" + file)) {
					Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + file.toStdString() + "';", err);
					removed.insert(file);
					if (Config::extendedLogging) {
						LOGINFO("Removed file " << file.toStdString())
					}
				} else {
					if (Config::extendedLogging) {
						LOGINFO("Couldn't remove file " << file.toStdString())
					}
				}
			} else {
				LOGINFO("Couldn't remove file " << file.toStdString())
			}
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
		for (const QString & file : removed) {
			_copiedFiles.removeOne(file);
		}

		if (Config::extendedLogging) {
			LOGINFO("Not removed files: " << _copiedFiles.size())
		}

		removeEmptyDirs();

		QDirIterator itBackup(_lastBaseDir, QStringList() << "*.spbak", QDir::Files, QDirIterator::Subdirectories);
		QStringList backupFiles;
		while (itBackup.hasNext()) {
			itBackup.next();
			QString backupFileName = itBackup.filePath();
			if (!backupFileName.isEmpty()) {
				backupFiles.append(backupFileName);
			}
		}
		for (QString backupFile : backupFiles) {
			QFile f(backupFile);
			backupFile.chop(6);
			bool b = f.rename(backupFile);
			if (!b) {
				LOGINFO("Couldn't rename file: " << q2s(backupFile))
			}
		}
	}

	if (!_directory.isEmpty()) {
		QDirIterator it(_directory + "/Data", QStringList() << "*mod", QDir::Files);
		QStringList files;
		while (it.hasNext()) {
			it.next();
			QString fileName = it.filePath();
			if (!fileName.isEmpty()) {
				QFile f(fileName);
				f.remove();
			}
		}
		// if mod is installed or developer mode is active, remove SpineAPI.dll from modification dir
		if (_projectID != -1 || _developerMode) {
			QFile(_directory + "/System/SpineAPI.dll").remove();
			QFile(_directory + "/Data/Spine.vdf").remove();
		}
	}
}

void Gothic1And2Launcher::updateModStats() {
	if (!Config::OnlineMode) {
		emit receivedCompatibilityList(_projectID, {}, {});
		return;
	}
	int modID = _projectID;
	requestSingleProjectStats([this, modID](bool b) {
		if (b) {
			QJsonObject json;
			json["ProjectID"] = modID;
			Https::postAsync(DATABASESERVER_PORT, "requestCompatibilityList", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, modID](const QJsonObject & data, int) {
				std::vector<int32_t> impossiblePatches;
				std::vector<int32_t> forbiddenPatches;

				if (data.contains("ImpossiblePatches")) {
					QJsonArray jsonArray = data["ImpossiblePatches"].toArray();
					for (const auto jsonObject : jsonArray) {
						impossiblePatches.push_back(jsonObject.toString().toInt());
					}
				}

				if (data.contains("ForbiddenPatches")) {
					QJsonArray jsonArray = data["ForbiddenPatches"].toArray();
					for (const auto jsonObject : jsonArray) {
						forbiddenPatches.push_back(jsonObject.toString().toInt());
					}
				}

				emit receivedCompatibilityList(modID, impossiblePatches, forbiddenPatches);
			});
		} else {
			emit receivedCompatibilityList(modID, {}, {});
		}
	});
}

bool Gothic1And2Launcher::prepareModStart(QString * usedExecutable, QStringList * backgroundExecutables, bool * newGMP, QSet<QString> * dependencies, Renderer * renderer) {
	_gmpCounterBackup = -1;

	QSet<QString> forbidden;
	QSet<QString> included;
	QMap<QString, QStringList> dependencyMap;
	QMap<QString, QStringList> overrideFiles;
	collectDependencies(_projectID, dependencies, &forbidden, &dependencyMap, &included, &overrideFiles);

	LOGINFO("Starting Ini: " << _iniFile.toStdString())
	emitSplashMessage(QApplication::tr("RemovingOldFiles"));
	for (const QString & f : _copiedFiles) {
		QFile(_lastBaseDir + "/" + f).remove();
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + f.toStdString() + "';", err);
	}
	_copiedFiles.clear();

	Database::DBError err;
	auto patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + ";", err);
	bool clockworkRenderer = false;
	bool systempack = false;
	bool ninja = false;

	// disable all forbidden patches
	for (const QString & p : forbidden) {
		auto it = std::find(patches.begin(), patches.end(), p.toStdString());
		if (it != patches.end()) {
			patches.erase(it);
		}
	}

	// check if all dependencies are enabled/can be enabled
	for (auto it = dependencies->begin(); it != dependencies->end();) {
		const auto dependencyID = it->toStdString();
		auto it2 = std::find(patches.begin(), patches.end(), dependencyID);
		if (it2 != patches.end() || included.contains(s2q(dependencyID))) {
			it = dependencies->erase(it);
		} else {
			auto results = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = " + it->toStdString() + " LIMIT 1;", err);
			if (!err.error && !results.empty()) {
				// patch is not enabled, but installed => enable it
				patches.push_back(it->toStdString());
				it = dependencies->erase(it);
			} else {
				// otherwise unresolved dependency that will cause an error
				++it;
			}
		}
	}
	if (!dependencies->isEmpty())
		return false; // not all dependencies are met and can't automatically be enabled

	sortPatches(dependencyMap, patches);

	for (const std::string & patchID : patches) {
		const std::string patchName = Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchID + " LIMIT 1;", err);
		if (patchName == "D3D11 Renderer Clockwork Edition" || patchName == "D3D11 Renderer Convenient Edition") {
			clockworkRenderer = true;
			*renderer = patchName == "D3D11 Renderer Clockwork Edition" ? Renderer::Clockwork : Renderer::Classic;
		} else if (patchName.find("Systempack") != std::string::npos) {
			systempack = true;
		} else if (patchID == "314") {
			ninja = true;
		} else if (patchName.find("Renderer") != std::string::npos) {
			*renderer = Renderer::Classic;
		}
	}

	int normalsCounter = -1;

	*usedExecutable = getExecutable();
	
	if (_projectID != -1) {
		emitSplashMessage(QApplication::tr("DetermingCorrectGothicPath"));
		if (Config::extendedLogging) {
			LOGINFO("Installed Mod")
		}
		bool success = true;

		if (ninja) {
			prepareForNinja();
		}

		emitSplashMessage(QApplication::tr("RemovingBackups"));
		_lastBaseDir = _directory;
		// everything that's left here can be removed I guess
		QDirIterator itBackup(_directory, QStringList() << "*.spbak", QDir::Files, QDirIterator::Subdirectories);
		while (itBackup.hasNext()) {
			itBackup.next();
			QFile(itBackup.filePath()).remove();
		}
		LOGINFO("Starting " << usedExecutable->toStdString() << " in " << _directory.toStdString())
		if (!usedExecutable->isEmpty() && success) {
			emitSplashMessage(QApplication::tr("CopyingModfiles"));
			auto files = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File, Hash FROM modfiles WHERE ModID = " + std::to_string(_projectID) + ";", err);
			if (err.error || files.empty()) {
				return false;
			}
			for (const std::pair<std::string, std::string> & file : files) {
				// SP-55 skipping .mod files, the necessary ones will be copied to Data later, so only one copy is necessary
				QString filename = QString::fromStdString(file.first);
				if (filename.contains("Data/modvdf/") || canSkipFile(filename)) {
					continue;
				}
				if (Config::extendedLogging) {
					LOGINFO("Copying file " << file.first)
				}
#ifdef Q_OS_WIN
				if (IsRunAsAdmin()) {
					if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements")) {
						bool b = QDir().mkpath(_directory + "/System/GD3D11/textures/replacements");
						Q_UNUSED(b)
					}
					// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
					const QString checkPath = "System/GD3D11/textures/replacements/Normalmaps_";
					if (filename.contains(checkPath, Qt::CaseInsensitive)) {
						if (normalsCounter == -1) {
							normalsCounter++;
						} else {
							continue;
						}
						QRegularExpression regex("System/GD3D11/textures/replacements/(Normalmaps_[^/]+)/", QRegularExpression::CaseInsensitiveOption);
						QRegularExpressionMatch match = regex.match(filename);
						const QString targetName = clockworkRenderer ? _directory + "/" + checkPath + QString::number(normalsCounter) : "/System/GD3D11/textures/replacements/" + match.captured(1);
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID) + "/System/GD3D11/textures/replacements/" + match.captured(1), targetName);
						_copiedFiles.append("/System/GD3D11/textures/replacements/" + match.captured(1));
						continue;
					}
				} else {
#endif
					if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements")) {
						bool b = QDir().mkpath(_directory + "/System/GD3D11/textures/replacements");
						Q_UNUSED(b)
					}
					if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements/Normalmaps_Original")) {
						bool b = QDir().mkpath(_directory + "/System/GD3D11/textures/replacements/Normalmaps_Original");
						Q_UNUSED(b)
					}
					// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
					const QString checkPath = "System/GD3D11/textures/replacements/Normalmaps_";
					if (filename.contains(checkPath, Qt::CaseInsensitive)) {
						if (normalsCounter == -1) {
							normalsCounter++;
							if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter))) {
								QDir().mkpath(_directory + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter));
							}
						}
						QRegularExpression regex("System/GD3D11/textures/replacements/(Normalmaps_[^/]+)/", QRegularExpression::CaseInsensitiveOption);
						QRegularExpressionMatch match = regex.match(filename);
						// backup old file
						bool copy = true;
						QFile f(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID) + "/" + filename);
						if (QFileInfo::exists(_directory + "/" + filename)) {
							const bool b = utils::Hashing::checkHash(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID) + "/" + filename, QString::fromStdString(file.second));
							if (b) {
								copy = false;
								if (Config::extendedLogging) {
									LOGINFO("Skipping file")
								}
								_skippedFiles.append(filename);
							}
							if (copy) {
								if (QFileInfo::exists(_directory + "/" + filename + ".spbak")) {
									QFile::remove(_directory + "/" + filename);
								} else {
									QFile::rename(_directory + "/" + filename, _directory + "/" + filename + ".spbak");
								}
							}
						}
						QString suffix = QFileInfo(filename).suffix();
						QString changedFile = filename;
						if (clockworkRenderer) {
							changedFile = changedFile.replace(match.captured(1), "Normalmap_" + QString::number(normalsCounter), Qt::CaseInsensitive);
						}
						if (copy) {
							f.close();
							success = linkOrCopyFile(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID) + "/" + filename, _directory + "/" + changedFile);
						}
						if (!success) {
							LOGERROR("Couldn't copy file: " << filename.toStdString() << " " << f.errorString().toStdString())
							break;
						}
						if (copy) {
							_copiedFiles.append(changedFile);
						}
						continue;
					}
#ifdef Q_OS_WIN
				}
#endif
				QFileInfo fi(_directory + "/" + filename);
				QDir dir = fi.absoluteDir();
				success = dir.mkpath(dir.absolutePath());
				if (!success) {
					LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString())
					break;
				}
				// backup old file
				bool copy = true;
				if (QFileInfo::exists(_directory + "/" + filename)) {
					const bool b = utils::Hashing::checkHash(_directory + "/" + filename, QString::fromStdString(file.second));
					if (b) {
						copy = false;
						if (Config::extendedLogging) {
							LOGINFO("Skipping file")
						}
						_skippedFiles.append(filename);
					}
					if (copy) {
						if (QFileInfo::exists(_directory + "/" + filename + ".spbak")) {
							QFile::remove(_directory + "/" + filename);
						} else {
							QFile::rename(_directory + "/" + filename, _directory + "/" + filename + ".spbak");
						}
					}
				}
				if (copy) {
					success = linkOrCopyFile(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID) + "/" + filename, _directory + "/" + filename);
				}
				if (!success) {
					LOGERROR("Couldn't copy file: " << filename.toStdString())
					break;
				}
				if (copy) {
					_copiedFiles.append(filename);
				}
			}
			if (!success) {
				removeModFiles();
				LOGERROR("Failed copying mod data")
				return false;
			}
			checkToolCfg(Config::DOWNLOADDIR + "/mods/" + QString::number(_projectID), backgroundExecutables, newGMP);
			updatePlugins(_projectID);
		}
	} else {
		_lastBaseDir = _directory;
		LOGERROR("Starting " << usedExecutable->toStdString() << " in " << _directory.toStdString())
	}
	if (systempack) {
		if (QFileInfo::exists(_directory + "/System/vdfs32g.exe")) {
			QFile::rename(_directory + "/System/vdfs32g.exe", _directory + "/System/vdfs32g.exe.spbak");
			_copiedFiles.append("System/vdfs32g.exe");
		}
	}
	emitSplashMessage(QApplication::tr("CopyingPatchfiles"));
	// everything worked fine so far, so add patches!
	if (!err.error && !patches.empty()) {
		for (const std::string & patchIDString : patches) {
			const int patchID = std::stoi(patchIDString);
			auto patchFiles = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File, Hash FROM modfiles WHERE ModID = " + std::to_string(patchID) + ";", err);
			if (err.error || patchFiles.empty()) {
				continue;
			}
			if (Config::extendedLogging) {
				LOGINFO("Applying patch " << patchID)
			}
			bool raisedNormalCounter = false;
			QSet<QString> skippedBases;
			for (const std::pair<std::string, std::string> & file : patchFiles) {
				QString filename = QString::fromStdString(file.first);

				if (canSkipFile(filename)) continue;

				if (patchID == 320 && getGothicVersion() == common::GameType::Gothic2 && filename.contains("binkw32.dll", Qt::CaseInsensitive)) continue;
				
#ifdef Q_OS_WIN
				if (IsRunAsAdmin()) {
					if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements")) {
						bool b = QDir().mkpath(_directory + "/System/GD3D11/textures/replacements");
						Q_UNUSED(b)
					}
					// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
					QString checkPath = "System/GD3D11/Data";
					if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(_directory + "/" + checkPath).exists() || QFileInfo(_directory + "/" + checkPath).isSymLink())) {
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, _directory + "/" + checkPath);
						skippedBases.insert(checkPath);
						_copiedFiles.append(checkPath);
						continue;
					} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
						// just skip
						continue;
					}
					checkPath = "System/GD3D11/Meshes";
					if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(_directory + "/" + checkPath).exists() || QFileInfo(_directory + "/" + checkPath).isSymLink())) {
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, _directory + "/" + checkPath);
						skippedBases.insert(checkPath);
						_copiedFiles.append(checkPath);
						continue;
					} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
						// just skip
						continue;
					}
					checkPath = "System/GD3D11/shaders";
					if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(_directory + "/" + checkPath).exists() || QFileInfo(_directory + "/" + checkPath).isSymLink())) {
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, _directory + "/" + checkPath);
						skippedBases.insert(checkPath);
						_copiedFiles.append(checkPath);
						continue;
					} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
						// just skip
						continue;
					}
					checkPath = "System/GD3D11/textures/infos";
					if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(_directory + "/" + checkPath).exists() || QFileInfo(_directory + "/" + checkPath).isSymLink())) {
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, _directory + "/" + checkPath);
						skippedBases.insert(checkPath);
						_copiedFiles.append(checkPath);
						continue;
					} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
						// just skip
						continue;
					}
					checkPath = "System/GD3D11/textures/RainDrops";
					if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(_directory + "/" + checkPath).exists() || QFileInfo(_directory + "/" + checkPath).isSymLink())) {
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, _directory + "/" + checkPath);
						skippedBases.insert(checkPath);
						_copiedFiles.append(checkPath);
						continue;
					} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
						// just skip
						continue;
					}
					checkPath = "System/GD3D11/textures/replacements/Normalmaps_Original";
					if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(_directory + "/" + checkPath).exists() || QFileInfo(_directory + "/" + checkPath).isSymLink())) {
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, _directory + "/" + checkPath);
						skippedBases.insert(checkPath);
						_copiedFiles.append(checkPath);
						continue;
					} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
						// just skip
						continue;
					}
					checkPath = "System/GD3D11/textures/replacements/Normalmaps_";
					if (filename.contains(checkPath, Qt::CaseInsensitive)) {
						if (!raisedNormalCounter) {
							raisedNormalCounter = true;
							normalsCounter++;
						} else {
							continue;
						}
						QRegularExpression regex("System/GD3D11/textures/replacements/(Normalmaps_[^/]+)/", QRegularExpression::CaseInsensitiveOption);
						QRegularExpressionMatch match = regex.match(filename);
						const QString targetName = clockworkRenderer ? _directory + "/" + checkPath + QString::number(normalsCounter) : "/System/GD3D11/textures/replacements/" + match.captured(1);
						makeSymlinkFolder(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/System/GD3D11/textures/replacements/" + match.captured(1), targetName);
						_copiedFiles.append("/System/GD3D11/textures/replacements/" + match.captured(1));
						continue;
					}
				} else {
#endif
					if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements")) {
						bool b = QDir().mkpath(_directory + "/System/GD3D11/textures/replacements");
						Q_UNUSED(b)
					}
					if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements/Normalmaps_Original")) {
						bool b = QDir().mkpath(_directory + "/System/GD3D11/textures/replacements/Normalmaps_Original");
						Q_UNUSED(b)
					}
					// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
					const QString checkPath = "System/GD3D11/textures/replacements/Normalmaps_";
					if (filename.contains(checkPath, Qt::CaseInsensitive)) {
						if (!raisedNormalCounter) {
							raisedNormalCounter = true;
							normalsCounter++;
							if (!QDir().exists(_directory + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter))) {
								QDir().mkpath(_directory + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter));
							}
						}
						QRegularExpression regex("System/GD3D11/textures/replacements/(Normalmaps_[^/]+)/", QRegularExpression::CaseInsensitiveOption);
						QRegularExpressionMatch match = regex.match(filename);
						// backup old file
						bool copy = true;
						QFile f(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + filename);
						QString changedFile = filename;
						if (clockworkRenderer && match.captured(1).compare("Normalmaps_Original", Qt::CaseInsensitive) != 0) {
							changedFile = changedFile.replace(match.captured(1), "Normalmaps_" + QString::number(normalsCounter), Qt::CaseInsensitive);
						}
						Q_ASSERT(QFileInfo::exists(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + filename));
						if (QFileInfo::exists(_directory + "/" + changedFile)) {
							const bool b = utils::Hashing::checkHash(_directory + "/" + changedFile, QString::fromStdString(file.second));
							if (b) {
								copy = false;
								if (Config::extendedLogging) {
									LOGINFO("Skipping file")
								}
							}
							if (copy) {
								if (QFileInfo::exists(_directory + "/" + changedFile + ".spbak")) {
									QFile::remove(_directory + "/" + changedFile);
								} else {
									QFile::rename(_directory + "/" + changedFile, _directory + "/" + changedFile + ".spbak");
								}
							}
						}
						if (copy) {
							f.close();
							const bool b = linkOrCopyFile(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + filename, _directory + "/" + changedFile);
							_copiedFiles.append(changedFile);
							Q_ASSERT(b);
						}
						continue;
					}
#ifdef Q_OS_WIN
				}
#endif
				QFileInfo fi(_directory + "/" + filename);
				QDir dir = fi.absoluteDir();
				bool success = dir.mkpath(dir.absolutePath());
				if (!success) {
					LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString())
					break;
				}

				if (overrideFiles[s2q(patchIDString)].contains(filename, Qt::CaseInsensitive)) {
					const auto it = std::find_if(_copiedFiles.begin(), _copiedFiles.end(), [filename](const QString & f) {
						return f.compare(filename, Qt::CaseInsensitive) == 0;
					});

					if (it != _copiedFiles.end()) {
						_copiedFiles.erase(it);
					}
				}
				
				// backup old file, if already backed up, don't patch
				if (!_copiedFiles.contains(filename, Qt::CaseInsensitive) && !_skippedFiles.contains(filename, Qt::CaseInsensitive) && ((QFileInfo::exists(_directory + "/" + filename) && !QFileInfo::exists(_directory + "/" + filename + ".spbak")) || !QFileInfo::exists(_directory + "/" + filename))) {
					if (Config::extendedLogging) {
						LOGINFO("Copying file " << file.first)
					}
					bool copy = true;
					if (QFileInfo::exists(_directory + "/" + filename)) {
						const bool b = Hashing::checkHash(_directory + "/" + filename, QString::fromStdString(file.second));
						if (b) {
							copy = false;
						}
					}
					if (filename.contains("directx_Jun2010_redist.exe", Qt::CaseInsensitive) || filename.contains(QRegularExpression("vc*.exe", QRegularExpression::ExtendedPatternSyntaxOption))) {
						copy = false;
					}
					if (copy) {
						QFile::rename(_directory + "/" + filename, _directory + "/" + filename + ".spbak");
						success = linkOrCopyFile(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + filename, _directory + "/" + filename);
						if (!success) {
							if (Config::extendedLogging) {
								LOGINFO("Missing file " << file.first << " (" << patchIDString << ")" << " " << QFileInfo::exists(_directory + "/" + filename) << " - " << QFileInfo::exists(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID) + "/" + filename))
							}

							auto name = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", err);
							Q_ASSERT(!name.empty());
							emit errorMessage(QApplication::tr("PatchIncomplete").arg(s2q(name[0])), false, nullptr);
							return false;
						}
					}
					_copiedFiles.append(filename);
				}
			}
			checkToolCfg(Config::DOWNLOADDIR + "/mods/" + QString::number(patchID), backgroundExecutables, newGMP);
			updatePlugins(patchID);
		}
	}
	if (usedExecutable->isEmpty()) {
		removeModFiles();
		LOGERROR("No executable found")
		return false;
	}
	emitSplashMessage(QApplication::tr("RemovingOldFiles"));
	// this shouldn't be necessary here! Test whether it can be removed
	QDirIterator it(_directory + "/Data", QStringList() << "*.mod", QDir::Files);
	QStringList files;
	while (it.hasNext()) {
		it.next();
		QString fileName = it.filePath();
		const QString partialName = fileName.replace(_directory + "/", "");
		if (!fileName.isEmpty() && !_copiedFiles.contains(partialName, Qt::CaseInsensitive) && !_skippedFiles.contains(partialName, Qt::CaseInsensitive)) {
			QFile f(fileName);
			f.remove();
		}
	}
	emitSplashMessage(QApplication::tr("CopyingModfiles"));
	{
		QSettings iniParser(_iniFile, QSettings::IniFormat);
		QString modFiles = iniParser.value("FILES/VDF", "").toString();
		QString executable = stripRelativePath(iniParser.value("FILES/Executable", "").toString());
		if (!executable.isEmpty()) {
			*usedExecutable = executable;
		}
		QString sourceDir = _directory;
		if (_projectID != -1) {
			QFileInfo fi(_iniFile);
			QDir sd = fi.absolutePath() + "/..";
			sourceDir = sd.absolutePath();
		}
		QStringList l = modFiles.split(" ", Qt::SkipEmptyParts);
		for (const QString & s : l) {
			linkOrCopyFile(sourceDir + "/Data/modvdf/" + s, _directory + "/Data/" + s);
			_copiedFiles.append("Data/" + s);
		}
		// check overrides and backup original values!
		_gothicIniBackup.clear();
		_systempackIniBackup.clear();
		emitSplashMessage(QApplication::tr("OverridingIni"));
		QFile(_directory + "/System/Gothic.ini").copy(_directory + "/System/Gothic.ini.spbak");
		{
			QSettings gothicIniParser(_directory + "/System/Gothic.ini", QSettings::IniFormat);
			iniParser.beginGroup("OVERRIDES");
			QStringList entries = iniParser.allKeys();
			for (const QString & s : entries) {
				const QString & entry = s;
				QString section = entry.split(".").front();
				QString key = entry.split(".").back();
				// backup original value
				QString val = gothicIniParser.value(section + "/" + key, "---").toString();
				_gothicIniBackup.append(std::make_tuple(section, key, val));
				val = iniParser.value(entry, "---").toString();
				if (val != "---") {
					gothicIniParser.setValue(section + "/" + key, val);
				}
			}
			iniParser.endGroup();
		}
		{
			QSettings systempackIniParser(_directory + "/System/Systempack.ini", QSettings::IniFormat);
			iniParser.beginGroup("OVERRIDES_SP");
			QStringList entries = iniParser.allKeys();
			for (const QString & s : entries) {
				const QString & entry = s;
				QString section = entry.split(".").front();
				QString key = entry.split(".").back();
				// backup original value
				QString val = systempackIniParser.value(section + "/" + key, "---").toString();
				_systempackIniBackup.append(std::make_tuple(section, key, val));
				val = iniParser.value(entry, "---").toString();
				if (val != "---") {
					systempackIniParser.setValue(section + "/" + key, val);
				}
			}
			iniParser.endGroup();
		}
	}
	// if mod is installed or developer mode is active, copy SpineAPI.dll to modification dir
	if (_projectID != -1 || _developerMode) {
		QFile(_directory + "/System/SpineAPI.dll").remove();
		QFile(_directory + "/Data/Spine.vdf").remove();
		{
			linkOrCopyFile(qApp->applicationDirPath() + "/SpineAPI.dll", _directory + "/System/SpineAPI.dll");
		}
		{
			linkOrCopyFile(qApp->applicationDirPath() + "/../media/Spine.vdf", _directory + "/Data/Spine.vdf");
		}
	}
	Database::open(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "BEGIN TRANSACTION;", err);
	for (const QString & file : _copiedFiles) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO usedFiles (File) VALUES ('" + file.toStdString() + "');", err);
	}
	Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "END TRANSACTION;", err);
	Database::close(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
	auto res = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM last_directory LIMIT 1;", err);
	if (res.empty()) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO last_directory (Path) VALUES ('" + _lastBaseDir.toStdString() + "');", err);
	} else {
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE last_directory SET Path = '" + _lastBaseDir.toStdString() + "';", err);
	}
	{
		QString startName = QFileInfo(_iniFile).fileName();
		startName = startName.split(".").front();
		const QString saveDir = _directory + "/saves_" + startName;
		QDir saveDirectory(saveDir);
		if (!saveDirectory.exists()) {
			bool b = saveDirectory.mkpath(saveDirectory.absolutePath());
			Q_UNUSED(b)
		}
	}
	{
		// remove all m3d files from System dir
		QDirIterator dirIt(_directory + "/System", QStringList() << "*.m3d", QDir::Files);
		while (dirIt.hasNext()) {
			dirIt.next();
			QString fileName = dirIt.filePath();
			if (!fileName.isEmpty()) {
				QFile f(fileName);
				f.remove();
			}
		}
	}
	{
		// SKO needs Options file in LocalAppData
		if (_projectID == 218) {
			const QString skoOptionsPath = QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA") + "/SKOLauncher/";
			const QString skoOptions = skoOptionsPath + "Options.json";
			QDir skoOptionsDir(skoOptionsPath);
			if (!skoOptionsDir.exists()) {
				bool b = skoOptionsDir.mkpath(skoOptionsDir.absolutePath());
				Q_UNUSED(b)
			}
			if (!QFileInfo::exists(skoOptions)) {
				QFile skoOptionsFile(skoOptions);
				if (skoOptionsFile.open(QIODevice::WriteOnly)) {
					QTextStream skoOptionsStream(&skoOptionsFile);
					skoOptionsStream << "{\n";
					skoOptionsStream << "\t\"Nickname\": \"" << Config::Username << "\",\n";
					skoOptionsStream << "\t\"Path\": \"" << _directory << "/System\"\n";
					skoOptionsStream << "}\n";
				}
			}
		}
	}

	return true;
}

void Gothic1And2Launcher::checkToolCfg(QString path, QStringList * backgroundExecutables, bool * newGMP) {
	if (!QFileInfo::exists(path + "/tool.cfg")) return;
	
	QSettings configParser(path + "/tool.cfg", QSettings::IniFormat);
	const QString executable = stripRelativePath(configParser.value("CONFIG/BackgroundProcess", "").toString());
	if (!executable.isEmpty()) {
		backgroundExecutables->append(executable);
	}
	if (configParser.contains("GMP/IP")) {
#ifdef Q_OS_WIN
		{
			QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\Public domain\GMP Launcher\Server)", QSettings::NativeFormat);
			_gmpCounterBackup = registrySettings.value("size", -1).toInt();
			registrySettings.setValue("size", 1);
		}
		{
			QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\Public domain\GMP Launcher\Server\1)", QSettings::NativeFormat);
			registrySettings.setValue("server_name", _nameLabel->text());
			registrySettings.setValue("server_nick", Config::Username);
			registrySettings.setValue("server_port", configParser.value("GMP/Port").toInt());
			registrySettings.setValue("server_url", configParser.value("GMP/IP").toString());
		}
		{
			QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\Public domain\GMP Launcher\Gothic)", QSettings::NativeFormat);
			registrySettings.setValue("working_directory", _directory);
		}
		*newGMP = true;
#endif
	} else if (configParser.contains("G2O/IP")) {
#ifdef Q_OS_WIN
		const QString ip = configParser.value("G2O/IP", "127.0.0.1").toString();
		const int port = configParser.value("G2O/Port", 12345).toInt();
		const QString favoriteString = QString("<servers><server><ip>123.456.789.0</ip><port>12345</port></server></servers>").arg(ip).arg(port);
		QFile outFile(_directory + "/../favorite.xml");
		outFile.open(QIODevice::WriteOnly);
		QTextStream ts(&outFile);
		ts << favoriteString;

		{
			QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\G2O)", QSettings::NativeFormat);
			if (!registrySettings.contains("nickname")) {
				registrySettings.setValue("nickname", Config::Username);
			}
		}
#endif
	} else {
		const QString ini = configParser.value("CONFIG/WriteIni", "").toString();
		const QString clearIni = configParser.value("CONFIG/ClearIni", "").toString();
		if (!ini.isEmpty()) {
			QSettings newIni(_directory + "/" + ini, QSettings::IniFormat);
			if (ini == clearIni) {
				newIni.clear();
			}
			configParser.beginGroup(ini);
			for (const QString & entry : configParser.allKeys()) {
				QStringList list = entry.split(".");
				if (list.size() == 2) {
					QString e = configParser.value(entry, "").toString();
					// modify e
					// in case e = @USERNAME@ => _username
					e = e.replace("@USERNAME@", Config::Username);
					newIni.setValue(list[0] + "/" + list[1], e);
				}
			}
			configParser.endGroup();
		}
		const QString file = configParser.value("CONFIG/WriteFile", "").toString();
		const QString clearFile = configParser.value("CONFIG/ClearFile", "").toString();
		if (!file.isEmpty()) {
			QFile f(_directory + "/" + file);
			if (file != clearFile) {
				f.open(QIODevice::ReadWrite);
			} else {
				f.open(QIODevice::WriteOnly);
			}
			configParser.beginGroup(file);
			for (const QString & entry : configParser.allKeys()) {
				QString e = configParser.value(entry, "").toString();
				// modify e
				// in case e = @USERNAME@ => _username
				e = e.replace("@USERNAME@", Config::Username);
				f.write(e.toStdString().c_str(), e.length());
				f.write("\n");
			}
			f.close();
			configParser.endGroup();
		}
	}
}

void Gothic1And2Launcher::collectDependencies(int modID, QSet<QString> * dependencies, QSet<QString> * forbidden, QMap<QString, QStringList> * dependencyMap, QSet<QString> * included, QMap<QString, QStringList> * overrideFiles) const {
	QQueue<QString> toCheck;
	toCheck.enqueue(QString::number(modID));

	Database::DBError err;
	auto patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(modID) + ";", err);

	for (const auto & p : patches) {
		toCheck.enqueue(s2q(p));
	}

	while (!toCheck.empty()) {
		auto id = toCheck.dequeue();
		auto path = Config::DOWNLOADDIR + "/mods/" + id;
		if (QFileInfo::exists(path + "/tool.cfg")) {
			QSettings configParser(path + "/tool.cfg", QSettings::IniFormat);

			auto required = configParser.value("DEPENDENCIES/Required", "").toString();
			
			auto split = required.split(',', Qt::SkipEmptyParts);
			for (const auto & s : split) {
				(*dependencyMap)[id].append(required);
				
				if (dependencies->contains(s))
					continue;
				
				dependencies->insert(s);
				toCheck.append(s);
			}
			
			auto blocked = configParser.value("DEPENDENCIES/Blocked", "").toString();
			
			split = blocked.split(',', Qt::SkipEmptyParts);
			for (const auto & s : split) {
				forbidden->insert(s);
			}
			
			auto includedIDs = configParser.value("DEPENDENCIES/Included", "").toString();
			
			split = includedIDs.split(',', Qt::SkipEmptyParts);
			for (const auto & s : split) {
				included->insert(s);
			}
			
			auto overrides = configParser.value("DEPENDENCIES/Overrides", "").toString();
			
			split = overrides.split(',', Qt::SkipEmptyParts);
			for (const auto & s : split) {
				(*overrideFiles)[id].append(s);
			}
		}			
	}
}

void Gothic1And2Launcher::prepareForNinja() {
	// base case
	{
		const auto bugslayerDll = _directory + "/System/BugslayerUtil.dll";
		if (QFileInfo::exists(bugslayerDll)) {
			if (QFileInfo::exists(_directory + "/System/BugslayerUtilG.dll")) {
				QFile::remove(_directory + "/System/BugslayerUtilG.dll");
			}
			const bool b = QFile::rename(bugslayerDll, _directory + "/System/BugslayerUtilG.dll");
			Q_ASSERT(b);
		}
	}
}

void Gothic1And2Launcher::updatePlugins(int modID) {
	const auto path = QString("%1/mods/%2").arg(Config::DOWNLOADDIR).arg(modID);

	if (!QFileInfo::exists(path + "/tool.cfg")) return;

	const QSettings configParser(path + "/tool.cfg", QSettings::IniFormat);

	{
		const auto systempackEntries = configParser.value("LOADER/SPpreload", "").toString();
				
		const auto split = systempackEntries.split(',', Qt::SkipEmptyParts);

		QFile f(_directory + "/System/pre.load");
		f.open(QIODevice::Append);
		QTextStream ts(&f);
		for (const auto & s : split) {
			ts << "\n" + s;
		}
		_systempackPreLoads << split;
	}
	{
		const auto unionEntries = configParser.value("LOADER/UnionIni", "").toString();
				
		const auto split = unionEntries.split(',', Qt::SkipEmptyParts);

		_unionPlugins << split;

		if (!_unionPlugins.isEmpty()) {
			QString text;
			{
				QFile f(_directory + "/System/Systempack.ini");
				f.open(QIODevice::ReadOnly);
				QTextStream ts(&f);
				text = ts.readAll();
			}
			const QRegularExpression regExp("(PluginList[\\s]*=[^\n]*)\n");
			const QRegularExpressionMatch match = regExp.match(text);
			QString replaceText = match.captured(1);
			LOGINFO("Replacing #1: " << q2s(replaceText))
			replaceText = replaceText.trimmed();
			if (replaceText.isEmpty()) {
				text += "\n[PLUGINS]\nPluginList= " + _unionPlugins.join(',') + "\n";
			} else {
				text = text.replace(replaceText, "PluginList= " + _unionPlugins.join(','));
			}
			{
				QFile f(_directory + "/System/Systempack.ini");
				f.open(QIODevice::WriteOnly);
				QTextStream ts(&f);
				ts << text;
			}
		}
	}
}

void Gothic1And2Launcher::sortPatches(const QMap<QString, QStringList> & dependencyMap, QList<std::string> & patches) const {
	for (auto it = dependencyMap.begin(); it != dependencyMap.end(); ++it) {
		const auto & currentID = it.key();
		const auto & dependencies = it.value();

		for (const auto & dependency : dependencies) {
			int currentIndex = std::numeric_limits<int>::max();
			for (int i = 0; i < patches.size(); i++) {
				const auto patchID = s2q(patches[i]);
				
				if (patchID == currentID) {
					currentIndex = i;
				}

				if (patchID == dependency) {
					if (currentIndex == std::numeric_limits<int>::max())
						break; // patch is in front of dependent project

					std::swap(patches[i], patches[currentIndex]);

					break;
				}
			}
		}
	}
}

bool Gothic1And2Launcher::linkOrCopyFolder(QString sourcePath, QString destinationPath) {
#ifdef Q_OS_WIN
	if (IsRunAsAdmin()) {
		const bool linked = makeSymlinkFolder(sourcePath, destinationPath);
		return linked;
	}

	return false; // not supported for none Admin mode
#else
	const bool copied = QFile::copy(sourcePath, destinationPath);
	return copied;
#endif
}

bool Gothic1And2Launcher::canSkipFile(const QString & filename) const {
	const QFileInfo fi(filename);
	const auto suffix = fi.suffix().toLower();
	return filename.compare("tool.cfg", Qt::CaseInsensitive) == 0 || suffix == "pdf";
}

void Gothic1And2Launcher::emitSplashMessage(QString message) {
	emit changeSplashMessage(message, Qt::AlignLeft, _splashTextColor);
}

void Gothic1And2Launcher::errorOccurred(QProcess::ProcessError error) {
	auto * process = dynamic_cast<QProcess *>(sender());
	LOGERROR("Error Code: " << error)
	if (process) {
		LOGERROR("Some error occurred: " << process->errorString().toStdString())
	}
}

QString Gothic1And2Launcher::getOverallSavePath() const {
	if (_projectID != -1) {
		return _directory + "/saves_" + QFileInfo(_iniFile).baseName() + "/" + QString::number(_projectID) + ".spsav";
	}

	return _directory + "/saves_" + QFileInfo(_iniFile).baseName() + "/spineTest.spsav";
}

void Gothic1And2Launcher::setZSpyActivated(bool enabled) {
	_zSpyActivated = enabled;
}

void Gothic1And2Launcher::parseMods() {
	if (Config::extendedLogging) {
		LOGINFO("Parsing Mods")
	}

	_parsedInis.clear();
	if (!_developerMode) {
		if (Config::extendedLogging) {
			LOGINFO("Parsing Installed Mods")
		}
		parseInstalledMods();
	}

	if (Config::extendedLogging) {
		LOGINFO("Parsing Mods in Gothic 1 Folder")
	}
	parseMods(_directory);
}

void Gothic1And2Launcher::parseMods(QString baseDir) {
	QStringList files;
	{
		QDirIterator it(baseDir + "/System", QStringList() << "*.ini", QDir::Files);
		while (it.hasNext()) {
			it.next();
			QString fileName = it.filePath();
			if (!fileName.isEmpty()) {
				files.append(fileName);
			}
		}
	}
	for (const QString & s : files) {
		auto it = _parsedInis.find(QFileInfo(s).fileName());
		if (it != _parsedInis.end()) {
			const bool b = utils::Hashing::checkHash(s, std::get<0>(it.value()));
			if (b) {
				// remove all files belonging to this mod AND skip it
				// this can happen in two cases:
				// 1. the mod is installed via Spine and manually (don't do such things, that's stupid)
				// 2. an error occurred and Spine couldn't remove mod files for some reason
				Database::DBError err;
				const auto fs = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File FROM modfiles WHERE ModID = " + std::to_string(std::get<1>(it.value())) + ";", err);
				for (const std::string & file : fs) {
					QFile tmpF(baseDir + "/" + QString::fromStdString(file));
					tmpF.remove();
				}

				QDirIterator itBackup(baseDir, QStringList() << "*.spbak", QDir::Files, QDirIterator::Subdirectories);
				QStringList backupFiles;
				while (itBackup.hasNext()) {
					itBackup.next();
					QString backupFileName = itBackup.filePath();
					if (!backupFileName.isEmpty()) {
						backupFiles.append(backupFileName);
					}
				}
				for (QString backupFile : backupFiles) {
					QFile f2(backupFile);
					backupFile.chop(6);
					f2.rename(backupFile);
				}
				QDirIterator dirIt(baseDir + "/Data", QStringList() << "*mod", QDir::Files);
				while (dirIt.hasNext()) {
					dirIt.next();
					QString fileName = dirIt.filePath();
					if (!fileName.isEmpty()) {
						QFile tmpF(fileName);
						tmpF.remove();
					}
				}
				continue;
			}
		}

		QSettings iniParser(s, QSettings::IniFormat);
		QString title = iniParser.value("INFO/Title", "").toString();

		if (title.isEmpty()) continue;

		const QString icon = iniParser.value("INFO/Icon", "").toString();
		QPixmap pixmap(baseDir + "/System/" + icon);
		if (pixmap.isNull()) {
			pixmap = getDefaultIcon();
		}
		pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
		while (title.startsWith(' ') || title.startsWith('\t')) {
			title = title.remove(0, 1);
		}
		auto * item = new QStandardItem(QIcon(pixmap), title);
		item->setData(s, LibraryFilterModel::IniFileRole);
		item->setData(false, LibraryFilterModel::InstalledRole);
		item->setData(false, LibraryFilterModel::HiddenRole);
		item->setData(-1, LibraryFilterModel::ModIDRole);
		item->setEditable(false);
		item->setData(static_cast<int>(getGothicVersion()), LibraryFilterModel::GameRole);
		
		_model->appendRow(item);
	}
}

void Gothic1And2Launcher::parseInstalledMods() {
	if (Config::extendedLogging) {
		LOGINFO("Checking Files in " << Config::DOWNLOADDIR.toStdString())
	}

	if (_directory.isEmpty() || !QDir(_directory).exists())
		return;

	QDirIterator it(Config::DOWNLOADDIR + "/mods", QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QString fileName = it.filePath();
		if (!fileName.isEmpty()) {
			parseIni(fileName);
		}
	}
}

void Gothic1And2Launcher::parseMod(QString folder) {
	QDirIterator it(folder, { "*.ini" }, QDir::Files, QDirIterator::Subdirectories);
	QStringList files;
	while (it.hasNext()) {
		it.next();
		QString fileName = it.filePath();
		if (!fileName.isEmpty()) {
			parseIni(fileName);
		}
	}
}

void Gothic1And2Launcher::parseIni(QString file) {
	QSettings iniParser(file, QSettings::IniFormat);
	QString title = iniParser.value("INFO/Title", "").toString();
	if (title.isEmpty()) {
		if (Config::extendedLogging) {
			LOGINFO("No title set in " << file.toStdString())
		}
		return;
	}
	const QString icon = iniParser.value("INFO/Icon", "").toString();
	QFileInfo fi(file);
	const QString iconPath = fi.absolutePath() + "/" + icon;

	QString modIDString = fi.absolutePath();
	QDir md(Config::DOWNLOADDIR + "/mods");
	modIDString.replace(md.absolutePath(), "");
	modIDString = modIDString.split("/", Qt::SkipEmptyParts).front();
	int32_t modID = modIDString.toInt();
	
	IconCache::getInstance()->cacheIcon(modID, iconPath);
	
	QPixmap pixmap = IconCache::getInstance()->getIcon(modID);

	Database::DBError err;
	common::GameType mid;
	bool found;
	if (!Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + modIDString.toStdString() + " LIMIT 1;", err).empty()) {
		mid = static_cast<common::GameType>(Database::queryNth<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + modIDString.toStdString() + " LIMIT 1;", err, 0).front());

		if (mid != getGothicVersion())
			return;
		
		found = true;
	} else {
		return;
	}
	if (pixmap.isNull()) {
		if (found) {
			pixmap = getDefaultIcon();
		}
	}
	pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
	while (title.startsWith(' ') || title.startsWith('\t')) {
		title = title.remove(0, 1);
	}
	auto * item = new QStandardItem(QIcon(pixmap), title);
	item->setData(file, LibraryFilterModel::IniFileRole);
	item->setData(true, LibraryFilterModel::InstalledRole);
	item->setData(modID, LibraryFilterModel::ModIDRole);
	item->setData(static_cast<int>(mid), LibraryFilterModel::GameRole);
	if (!Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM hiddenMods WHERE ModID = " + modIDString.toStdString() + " LIMIT 1;", err).empty()) {
		item->setData(true, LibraryFilterModel::HiddenRole);
	} else {
		item->setData(false, LibraryFilterModel::HiddenRole);
	}
	item->setEditable(false);
	_model->appendRow(item);

	QString hashSum;
	const bool b = Hashing::hash(file, hashSum);
	if (b) {
		_parsedInis.insert(fi.fileName(), std::make_tuple(hashSum, modID));
	}
	if (Config::extendedLogging) {
		LOGINFO("Listing Mod: " << title.toStdString())
	}
}

void Gothic1And2Launcher::updateModel(QStandardItemModel * model) {
	_model = model;
	
	parseMods();
}

void Gothic1And2Launcher::finishedInstallation(int modID, int packageID, bool success) {
	if (!success)
		return;

	if (packageID != -1)
		return;
	
	if (_developerMode)
		return;
	
	parseMod(QString("%1/mods/%2").arg(Config::DOWNLOADDIR).arg(modID));
	updateModStats();
}

void Gothic1And2Launcher::updatedProject(int projectID) {
	if (_developerMode)
		return;
	
	Database::DBError err;
	const auto mods = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, GothicVersion FROM mods WHERE (GothicVersion = " + std::to_string(static_cast<int>(common::GameType::Gothic)) + " OR GothicVersion = " + std::to_string(static_cast<int>(common::GameType::Gothic2)) + " OR GothicVersion = " + std::to_string(static_cast<int>(common::GameType::Gothic1And2)) + " OR GothicVersion = " + std::to_string(static_cast<int>(common::GameType::GothicInGothic2)) + ") AND ModID = " + std::to_string(projectID) + ";", err);

	if (mods.empty()) return;

	const auto mod = mods[0];

	const auto idxList = _model->match(_model->index(0, 0), LibraryFilterModel::ModIDRole, QVariant::fromValue(projectID), 2, Qt::MatchRecursive);

	if (!idxList.empty()) {
		_model->removeRow(idxList[0].row(), idxList[0].parent());
	}

	parseMod(QString("%1/mods/%2").arg(Config::DOWNLOADDIR).arg(projectID));
}

void Gothic1And2Launcher::updatePatchCheckboxes() {
	QSet<QString> dependencies;
	QSet<QString> forbidden;
	QSet<QString> included;
	QMap<QString, QStringList> dependencyMap;
	QMap<QString, QStringList> overrideFiles;
	collectDependencies(_projectID, &dependencies, &forbidden, &dependencyMap, &included, &overrideFiles);

	auto systempackUnionEnabled = included.contains("320") || included.contains("40") || included.contains("57") || included.contains("208");

	for (auto it = _checkboxPatchIDMapping.begin(); it != _checkboxPatchIDMapping.end(); ++it) {
		const auto idString = QString::number(it.value());

		auto isForbidden = forbidden.contains(idString);

		auto sl = dependencyMap.value(idString);

		for (const auto & id : sl) {
			if (!forbidden.contains(id))
				continue;

			isForbidden = true;
			
			break;
		}

		if (isForbidden) {
			if (it.key()->isChecked()) {
				it.key()->setChecked(false);
			}
			it.key()->setEnabled(false);
		} else {
			it.key()->setEnabled(true);
		}

		if (dependencies.contains(idString) && !it.key()->isChecked() && !isForbidden) {
			it.key()->setChecked(true);
		}

		const auto id = it.value();

		if (!it.key()->isChecked())
			continue;
		
		if (id == 320 || id == 40 || id == 57 || id == 208) {
			systempackUnionEnabled = true;
		}
	}

	_errorInfoLabel->setVisible(!systempackUnionEnabled);
}

QList<int32_t> Gothic1And2Launcher::getActiveProjects() const {
	auto activeProjects = ILauncher::getActiveProjects();
	
	Database::DBError dbErr;
	const auto patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_projectID) + ";", dbErr);
	for (const std::string & patchIDString : patches) {
		const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", dbErr);
		if (count == 0) continue;

		const int32_t patchID = std::stoi(patchIDString);
		activeProjects << patchID;
	}

	return activeProjects;
}

QString Gothic1And2Launcher::getDefaultIni(int projectID) const {
	const auto path = Config::DOWNLOADDIR + "/mods/" + QString::number(projectID) + "/System";
	QDirIterator it(path, { "*.ini" }, QDir::Files);
	while (it.hasNext()) {
		it.next();

		return it.filePath();
	}

	return QString();
}
