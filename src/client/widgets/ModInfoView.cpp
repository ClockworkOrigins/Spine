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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#include "widgets/ModInfoView.h"

#include <thread>

#include "Config.h"
#include "Conversion.h"
#include "Database.h"
#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "RtfToHtmlConverter.h"
#include "ScreenshotManager.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

#ifdef Q_OS_WIN
	#include "WindowsExtensions.h"
#endif

#include "common/MessageStructs.h"

#ifdef Q_OS_WIN
	#include "gamepad/KeyMapping.h"
	#include "gamepad/XBoxController.h"
#endif

#include "security/Hash.h"

#include "widgets/AchievementView.h"
#include "widgets/DownloadProgressDialog.h"

#ifdef Q_OS_WIN
	#include "widgets/GamepadSettingsWidget.h"
#endif
#include "widgets/GeneralSettingsWidget.h"
#include "widgets/LocationSettingsWidget.h"
#include "widgets/NewCombinationDialog.h"
#include "widgets/RatingWidget.h"
#include "widgets/SubmitCompatibilityDialog.h"

#include "clockUtils/log/Log.h"
#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSettings>
#include <QSlider>
#include <QSplashScreen>
#include <QtConcurrentRun>
#include <QTime>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <shellapi.h>
#endif

namespace spine {
namespace widgets {
namespace {
	
	struct ModVersion {
		common::GothicVersion gothicVersion;
		int majorVersion;
		int minorVersion;
		int patchVersion;
		ModVersion() : gothicVersion(), majorVersion(), minorVersion(), patchVersion() {
		}
		ModVersion(int i1, int i2, int i3, int i4) : gothicVersion(common::GothicVersion(i1)), majorVersion(i2), minorVersion(i3), patchVersion(i4) {
		}
	};

	struct Patch {
		int32_t modID;
		std::string name;

		Patch() : modID(-1), name() {
		}
		Patch(std::string m, std::string n) : modID(std::stoi(m)), name(n) {
		}
	};

}

	ModInfoView::ModInfoView(bool onlineMode, QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, LocationSettingsWidget * locationSettingsWidget, GamepadSettingsWidget * gamepadSettingsWidget, QSettings * iniParser, QWidget * par) : QWidget(par), _mainWindow(mainWindow), _nameLabel(nullptr), _versionLabel(nullptr), _teamLabel(nullptr), _contactLabel(nullptr), _homepageLabel(nullptr), _patchGroup(nullptr), _patchLayout(nullptr), _pdfGroup(nullptr), _pdfLayout(nullptr), _achievementLabel(nullptr), _scoresLabel(nullptr), _iniFile(), _isInstalled(false), _modID(-1), _gothicDirectory(), _gothic2Directory(), _username(), _timer(new QTime()), _patchList(), _pdfList(), _checkboxPatchIDMapping(), _developerModeActive(false), _listenSocket(nullptr), _socket(nullptr), _gothicIniBackup(), _systempackIniBackup(), _zSpyActivated(false), _language(), _copiedFiles(), _lastBaseDir(), _showAchievements(true), _hideIncompatible(true), _ratingWidget(), _screenshotManager(new ScreenshotManager(locationSettingsWidget, this)), _gamepad(nullptr), _gamepadSettingsWigdet(gamepadSettingsWidget), _iniParser(iniParser), _onlineMode(onlineMode), _networkAccessManager(new QNetworkAccessManager(this)), _patchCounter(0), _gmpCounterBackup(-1) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hbl = new QHBoxLayout();

			_startButton = new QPushButton(QApplication::tr("StartMod"), this);
			_startButton->setProperty("library", true);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _startButton, "StartMod");
			_startButton->setFixedWidth(150);

			hbl->addWidget(_startButton, 0, Qt::AlignLeft);

			_startSpacerButton = new QPushButton(QApplication::tr("StartSpacer"), this);
			_startSpacerButton->setProperty("library", true);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _startSpacerButton, "StartSpacer");
			hbl->addWidget(_startSpacerButton, 0, Qt::AlignLeft);
			_startSpacerButton->hide();
			connect(_startSpacerButton, SIGNAL(released()), this, SLOT(startSpacer()));

			hbl->addSpacing(25);

			_playTimeLabel = new QLabel(this);
			_playTimeLabel->setProperty("library", true);
			_playTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

			hbl->addWidget(_playTimeLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
			hbl->addStretch(1);

			if (_onlineMode) {
				_ratingWidget = new RatingWidget(this);
				_ratingWidget->setProperty("library", true);
				hbl->addWidget(_ratingWidget, 1, Qt::AlignRight);
				_ratingWidget->setEditable(true);
				_ratingWidget->setVisible(false);
			}

			l->addLayout(hbl);
		}

		{
			QHBoxLayout * hbl = new QHBoxLayout();

			_installDate = new QLabel(this);
			_installDate->setProperty("library", true);
			_installDate->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

			hbl->addWidget(_installDate, 0, Qt::AlignLeft | Qt::AlignVCenter);

			hbl->addSpacing(25);

			_lastPlayedDate = new QLabel(this);
			_lastPlayedDate->setProperty("library", true);
			hbl->addWidget(_lastPlayedDate);

			hbl->addStretch(1);

			l->addLayout(hbl);
		}

		_adminInfoLabel = new QLabel(QApplication::tr("GothicAdminNote"), this);
		_adminInfoLabel->setProperty("library", true);
		_adminInfoLabel->setAlignment(Qt::AlignCenter);
		_adminInfoLabel->setWordWrap(true);
		_adminInfoLabel->setProperty("adminInfo", true);
		_adminInfoLabel->hide();

		_nameLabel = new QLabel(this);
		_nameLabel->setProperty("library", true);
		_nameLabel->setAlignment(Qt::AlignCenter);
		_versionLabel = new QLabel(this);
		_versionLabel->setProperty("library", true);
		_versionLabel->setAlignment(Qt::AlignCenter);
		_teamLabel = new QLabel(this);
		_teamLabel->setProperty("library", true);
		_teamLabel->setAlignment(Qt::AlignCenter);
		_contactLabel = new QLabel(this);
		_contactLabel->setProperty("library", true);
		_contactLabel->setAlignment(Qt::AlignCenter);
		_contactLabel->setOpenExternalLinks(true);
		_homepageLabel = new QLabel(this);
		_homepageLabel->setProperty("library", true);
		_homepageLabel->setAlignment(Qt::AlignCenter);
		_homepageLabel->setOpenExternalLinks(true);

		_achievementLabel = new QLabel(this);
		_achievementLabel->setProperty("library", true);
		_achievementLabel->setAlignment(Qt::AlignCenter);
		_achievementLabel->hide();
		connect(_achievementLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(prepareAchievementView()));

		_scoresLabel = new QLabel(this);
		_scoresLabel->setProperty("library", true);
		_scoresLabel->setAlignment(Qt::AlignCenter);
		_scoresLabel->hide();
		connect(_scoresLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(prepareScoreView()));

		_compileScripts = new QCheckBox(QApplication::tr("CompileScripts"), this);
		_compileScripts->setProperty("library", true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, _compileScripts, "CompileScripts");
		_compileScripts->hide();

		_startupWindowed = new QCheckBox(QApplication::tr("StartupWindowed"), this);
		_startupWindowed->setProperty("library", true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, _startupWindowed, "StartupWindowed");
		_startupWindowed->hide();

		_convertTextures = new QCheckBox(QApplication::tr("ConvertTextures"), this);
		_convertTextures->setProperty("library", true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, _convertTextures, "ConvertTextures");
		_convertTextures->hide();

		_noSound = new QCheckBox(QApplication::tr("DeactivateSound"), this);
		_noSound->setProperty("library", true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, _noSound, "DeactivateSound");
		_noSound->hide();

		_noMusic = new QCheckBox(QApplication::tr("DeactivateMusic"), this);
		_noMusic->setProperty("library", true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, _noMusic, "DeactivateMusic");
		_noMusic->hide();

		QHBoxLayout * hl = new QHBoxLayout();
		{
			_patchGroup = new QGroupBox(QApplication::tr("PatchesAndTools"), this);
			_patchGroup->setProperty("library", true);
			_patchLayout = new QGridLayout();
			_patchLayout->setColumnStretch(2, 1);
			_patchGroup->setLayout(_patchLayout);
			_patchGroup->hide();

			_pdfGroup = new QGroupBox(QApplication::tr("PDFs"), this);
			_pdfGroup->setProperty("library", true);
			_pdfLayout = new QVBoxLayout();
			_pdfGroup->setLayout(_pdfLayout);
			_pdfGroup->hide();

			hl->addWidget(_patchGroup);
			hl->addWidget(_pdfGroup);
		}
		QHBoxLayout * zSpyLayout = new QHBoxLayout();
		{
			_zSpyLabel = new QLabel(QApplication::tr("zSpyLevel"), this);
			_zSpyLabel->setProperty("library", true);
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _zSpyLabel, "zSpyLevel");

			_zSpyLevel = new QSlider(Qt::Orientation::Horizontal, this);
			_zSpyLevel->setProperty("library", true);
			_zSpyLevel->setMinimum(0);
			_zSpyLevel->setMaximum(10);
			_zSpyLevel->setPageStep(1);

			zSpyLayout->addWidget(_zSpyLabel);
			zSpyLayout->addWidget(_zSpyLevel);
		}

		l->addWidget(_adminInfoLabel);
		l->addWidget(_nameLabel);
		l->addWidget(_versionLabel);
		l->addWidget(_teamLabel);
		l->addWidget(_contactLabel);
		l->addWidget(_homepageLabel);
		l->addWidget(_achievementLabel);
		l->addWidget(_scoresLabel);
		l->addWidget(_compileScripts);
		l->addWidget(_startupWindowed);
		l->addWidget(_convertTextures);
		l->addWidget(_noSound);
		l->addWidget(_noMusic);
		l->addLayout(zSpyLayout);
		l->addLayout(hl);

		setLayout(l);

		connect(_startButton, SIGNAL(clicked()), this, SLOT(startMod()));

		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "CREATE TABLE IF NOT EXISTS patchConfigs(ModID INT NOT NULL, PatchID INT NOT NULL, Enabled INT NOT NULL);", err);

		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS usedFiles (File TEXT PRIMARY KEY);", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS lastUsedBaseDir (Path TEXT PRIMARY KEY);", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS installDates (ModID INT PRIMARY KEY, InstallDate INT NOT NULL);", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS scoreCache (ModID INT NOT NULL, Identifier INT NOT NULL, Score INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS achievementCache (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS achievementProgressCache (ModID INT NOT NULL, Identifier INT NOT NULL, Progress INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "CREATE TABLE IF NOT EXISTS overallSaveDataCache (ModID INT NOT NULL, Entry TEXT NOT NULL, Value TEXT NOT NULL, PRIMARY KEY (ModID, Entry));", err);

		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modScores (ModID INT NOT NULL, Username TEXT NOT NULL, Identifier INT NOT NULL, Score INT NOT NULL, PRIMARY KEY (ModID, Username, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievements (ModID INT NOT NULL, Username TEXT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Username, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievementList (ModID INT NOT NULL, Identifier INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievementProgressMax (ModID INT NOT NULL, Identifier INT NOT NULL, Max INT NOT NULL, PRIMARY KEY (ModID, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS modAchievementProgress (ModID INT NOT NULL, Username TEXT NOT NULL, Identifier INT NOT NULL, Current INT NOT NULL, PRIMARY KEY (ModID, Username, Identifier));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS overallSaveData (Username TEXT NOT NULL, ModID INT NOT NULL, Entry TEXT NOT NULL, Value TEXT NOT NULL, PRIMARY KEY (Username, ModID, Entry));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS playTimes (ModID INT NOT NULL, Username TEXT NOT NULL, Duration INT NOT NULL, PRIMARY KEY (Username, ModID));", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "CREATE TABLE IF NOT EXISTS sync (Enabled INT PRIMARY KEY);", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT OR IGNORE INTO sync (Enabled) VALUES (0);", err);

		Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "CREATE TABLE IF NOT EXISTS lastPlayed (ModID INT NOT NULL, Ini TEXT NOT NULL, PRIMARY KEY (ModID, Ini));", err);
		Q_ASSERT(!err.error);
		
		std::vector<std::string> files = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM usedFiles;", err);
		if (!files.empty()) {
			_lastBaseDir = QString::fromStdString(Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM lastUsedBaseDir LIMIT 1;", err, 0));
		}
		for (const std::string & s : files) {
			_copiedFiles.insert(QString::fromStdString(s));
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
			removeEmptyDirs(_lastBaseDir);
		}

		qRegisterMetaType<std::vector<int32_t>>("std::vector<int32_t>");
		qRegisterMetaType<common::ModStats>("common::ModStats");

		connect(this, SIGNAL(receivedModStats(common::ModStats)), this, SLOT(updateModInfoView(common::ModStats)));
		connect(this, SIGNAL(receivedCompatibilityList(int, std::vector<int32_t>, std::vector<int32_t>)), this, SLOT(updateCompatibilityList(int, std::vector<int32_t>, std::vector<int32_t>)));
		connect(this, SIGNAL(restartAsAdmin()), this, SLOT(restartSpineAsAdmin()));
		connect(this, SIGNAL(updatedG1Path()), this, SLOT(patchCheckG1()), Qt::QueuedConnection);
		connect(this, SIGNAL(updatedG2Path()), this, SLOT(patchCheckG2()), Qt::QueuedConnection);
		connect(this, &ModInfoView::errorMessage, this, &ModInfoView::showErrorMessage);

		_hideIncompatible = generalSettingsWidget->getHideIncompatible();
		connect(generalSettingsWidget, SIGNAL(changedHideIncompatible(bool)), this, SLOT(setHideIncompatible(bool)));

		restoreSettings();
	}

	ModInfoView::~ModInfoView() {
		saveSettings();
	}

	void ModInfoView::setIniFile(QString file) {
		_iniFile = file;
		QSettings iniParser(file, QSettings::IniFormat);
		iniParser.beginGroup("INFO");
		QString title = iniParser.value("Title", "").toString();
		QString version = iniParser.value("Version", "").toString();
		QString team = iniParser.value("Authors", "").toString();
		QString homepage = iniParser.value("Webpage", "").toString();
		QString contact = iniParser.value("Contact", "").toString();
		QString description = iniParser.value("Description", "").toString();
		iniParser.endGroup();
		if (!title.isEmpty()) {
			_nameLabel->setText(title);
		}
		if (!version.isEmpty()) {
			_versionLabel->setText(version);
		}
		if (!team.isEmpty()) {
			_teamLabel->setText(team);
		} else {
			_teamLabel->setText("");
		}
		_teamLabel->setVisible(!team.isEmpty());
		if (!contact.isEmpty() && _onlineMode) {
			_contactLabel->setText("<a href=\"mailto:" + contact + "\">" + contact + "</a>");
		} else {
			_contactLabel->setText("");
		}
		_contactLabel->setVisible(!contact.isEmpty());
		if (_onlineMode) {
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
		if (!description.isEmpty()) {
			QString desc = description;
			if (desc.startsWith("!<symlink>")) {
				desc = desc.replace("!<symlink>", "");
				QFileInfo fi(file);
				QFile f(fi.absolutePath() + "/" + desc);
				if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream ts(&f);
					desc = ts.readAll();
				} else {
					desc = "";
				}
			}
			desc = RtfToHtmlConverter::convert(desc);
			emit descriptionChanged(desc);
		} else {
			emit descriptionChanged("");
		}
		QString programFiles = QProcessEnvironment::systemEnvironment().value("ProgramFiles", "Foobar123");
		programFiles = programFiles.replace("\\", "/");
		QString programFilesx86 = QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", "Foobar123");
		programFilesx86 = programFilesx86.replace("\\", "/");
		if (_iniFile.contains(programFiles) || _iniFile.contains(programFilesx86)) {
#ifdef Q_OS_WIN
			_startButton->setEnabled(IsRunAsAdmin());
#else
			_startButton->setEnabled(true);
#endif
			_adminInfoLabel->setText(QApplication::tr("GothicAdminInfo").arg(title));
			_adminInfoLabel->show();
		} else {
			_startButton->setEnabled(true);
			_adminInfoLabel->hide();
		}
		_installDate->hide();
		_lastPlayedDate->hide();
	}

	void ModInfoView::setIsInstalled(bool isInstalled) {
		_isInstalled = isInstalled;
		if (!isInstalled) {
			setModID("-1");
		}
	}

	void ModInfoView::setModID(QString modID) {
		_modID = modID.toInt();

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
		_achievementLabel->hide();
		_scoresLabel->hide();
		_playTimeLabel->setText("");
		if (_onlineMode) {
			_ratingWidget->setVisible(_modID != -1);
		}
		_installDate->setText("");
		_lastPlayedDate->setText("");

		if (_modID != -1) {
			if (_onlineMode) {
				_ratingWidget->setModID(_modID);
				_ratingWidget->setModName(_nameLabel->text());
			}
			Database::DBError err;
			const ModVersion modGv = Database::queryNth<ModVersion, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion, MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err, 0);
			std::vector<int> date = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT InstallDate FROM installDates WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err);
			if (date.empty()) {
				const int currentDate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO installDates (ModID, InstallDate) VALUES (" + std::to_string(_modID) + ", " + std::to_string(currentDate) +");", err);
				date.push_back(currentDate);
			}
			_installDate->setText(QApplication::tr("Installed").arg(QDate(1970, 1, 1).addDays(date[0] / 24).toString("dd.MM.yyyy")));
			_installDate->show();
			
			{
				QDirIterator it(Config::MODDIR + "/mods/" + QString::number(_modID) + "/", QStringList() << "*.pdf", QDir::Filter::Files);
				while (it.hasNext()) {
					it.next();
					QLabel * l = new QLabel("<a href=\"file:///" + it.filePath().toHtmlEscaped() + R"(" style="color: #181C22">)" + QFileInfo(it.fileName()).fileName() + "</a>", _pdfGroup);
					l->setProperty("library", true);
					l->setOpenExternalLinks(true);
					_pdfLayout->addWidget(l);
					_pdfList.append(l);
				}
			}
			if (!_pdfList.empty()) {
				QLabel * l = new QLabel(_pdfGroup);
				l->setProperty("library", true);
				_pdfLayout->addWidget(l);
				_pdfLayout->setStretchFactor(l, 1);
				_pdfList.append(l);
				_pdfGroup->show();
			}
			QString usedBaseDir;
			if (modGv.gothicVersion == common::GothicVersion::GOTHIC) {
				usedBaseDir = _gothicDirectory;
			} else if (modGv.gothicVersion == common::GothicVersion::GOTHIC2) {
				usedBaseDir = _gothic2Directory;
			} else if (modGv.gothicVersion == common::GothicVersion::GOTHICINGOTHIC2) {
				usedBaseDir = _gothic2Directory;
			}
			QString programFiles = QProcessEnvironment::systemEnvironment().value("ProgramFiles", "Foobar123");
			programFiles = programFiles.replace("\\", "/");
			QString programFilesx86 = QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", "Foobar123");
			programFilesx86 = programFilesx86.replace("\\", "/");
			if (usedBaseDir.contains(programFiles) || usedBaseDir.contains(programFilesx86)) {
#ifdef Q_OS_WIN
				_startButton->setEnabled(IsRunAsAdmin());
#else
				_startButton->setEnabled(true);
#endif
				const QString gothicName = modGv.gothicVersion == common::GothicVersion::GOTHIC ? QApplication::tr("Gothic") : QApplication::tr("Gothic2");
				_adminInfoLabel->setText(QApplication::tr("GothicAdminInfo").arg(gothicName));
				_adminInfoLabel->show();
			} else {
				_startButton->setEnabled(true);
				_adminInfoLabel->hide();
			}
			_versionLabel->setText(QString("%1.%2.%3").arg(modGv.majorVersion).arg(modGv.minorVersion).arg(modGv.patchVersion));
			updateModStats();
		} else {
			Database::DBError err;
			const common::GothicVersion gothicVersion = getGothicVersion(getUsedBaseDir());
			std::vector<Patch> patches = Database::queryAll<Patch, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, Name FROM patches;", err);
			for (const Patch & p : patches) {
				const common::GothicVersion gv = common::GothicVersion(std::stoi(Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(p.modID) + " LIMIT 1;", err, 0)));
				if (gv == gothicVersion || ((gothicVersion == common::GothicVersion::GOTHIC || gothicVersion == common::GothicVersion::GOTHIC2 || gothicVersion == common::GothicVersion::GOTHICINGOTHIC2) && gv == common::GothicVersion::Gothic1And2)) {
					QCheckBox * cb = new QCheckBox(s2q(p.name), this);
					cb->setProperty("library", true);
					_patchLayout->addWidget(cb);
					_patchList.append(cb);
					const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT Enabled FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + " AND PatchID = " + std::to_string(p.modID) + " LIMIT 1;", err);
					cb->setChecked(count);
					_checkboxPatchIDMapping.insert(std::make_pair(cb, p.modID));
					connect(cb, SIGNAL(stateChanged(int)), this, SLOT(changedPatchState()));
					_patchGroup->show();
					_patchGroup->setToolTip(QApplication::tr("PatchesAndToolsTooltip").arg(_nameLabel->text()));
					_pdfGroup->hide();
				}
			}
		}
	}

	void ModInfoView::setGothicDirectory(QString directory) {
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
		if (QFile(_gothicDirectory + "/System/Gothic.exe").exists()) {
			QFile(_gothicDirectory + "/System/GothicGame.ini").remove();
		}
		_gothicDirectory = directory;
		if (QFile(_gothicDirectory + "/System/GothicGame.ini").exists()) {
			QFile(_gothicDirectory + "/System/GothicGame.ini").remove();
		}

		if (!QDir(_gothicDirectory + "/saves").exists()) {
			bool b = QDir(_gothicDirectory).mkdir("saves");
			Q_UNUSED(b);
		}

		emit updatedG1Path();
	}

	void ModInfoView::setGothic2Directory(QString directory) {
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
		if (QFile(_gothic2Directory + "/System/Gothic2.exe").exists()) {
			QFile(_gothic2Directory + "/System/GothicGame.ini").remove();
		}
		_gothic2Directory = directory;
		if (QFile(_gothic2Directory + "/System/GothicGame.ini").exists()) {
			QFile(_gothic2Directory + "/System/GothicGame.ini").remove();
		}

		emit updatedG2Path();
	}

	void ModInfoView::loggedIn(QString username, QString password) {
		_username = username;
		_password = password;
		if (_onlineMode) {
			_ratingWidget->setUsername(username, password);
		}
		if (!_username.isEmpty()) {
			tryCleanCaches();
			synchronizeOfflineData();
		}
	}

	void ModInfoView::setDeveloperMode(bool active) {
		_developerModeActive = active;
		_compileScripts->setVisible(active);
		_startupWindowed->setVisible(active);
		_startSpacerButton->setVisible(active);
		_convertTextures->setVisible(active);
		_noSound->setVisible(active);
		_noMusic->setVisible(active);
		_zSpyLabel->setVisible(active);
		_zSpyLevel->setVisible(active);
	}

	void ModInfoView::setZSpyActivated(bool active) {
		_zSpyActivated = active;
	}

	void ModInfoView::setLanguage(QString language) {
		_language = language;
	}

	void ModInfoView::setShowAchievements(bool showAchievements) {
		_showAchievements = showAchievements;
	}

	void ModInfoView::setHideIncompatible(bool enabled) {
		_hideIncompatible = enabled;
	}

	void ModInfoView::startMod() {
		if (_iniFile.isEmpty()) {
			return;
		}

		_mainWindow->setDisabled(true);
		_oldWindowState = _mainWindow->windowState();
		_mainWindow->setWindowState(Qt::WindowState::WindowMinimized);

		QString splashImage = ":/SpineSplash.png";
		_splashTextColor = Qt::black;

		QSettings splashParser(_iniFile, QSettings::IniFormat);
		QString splashImageStr = splashParser.value("SETTINGS/Splash", "").toString();
		if (!splashImageStr.isEmpty()) {
			splashImage = getUsedBaseDir() + "/" + splashImageStr;
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
		splash.setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		splash.show();
		connect(this, SIGNAL(changeSplashMessage(QString, int, QColor)), &splash, SLOT(showMessage(QString, int, QColor)));
		QFutureWatcher<bool> watcher(this);
		QEventLoop loop;
		connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
		QStringList backgroundExecutables;
		QString usedBaseDir;
		QString usedExecutable;
		bool newGMP = false;

		QTime t;
		t.start();
		QFuture<bool> future = QtConcurrent::run<bool>(this, &ModInfoView::prepareModStart, &usedBaseDir, &usedExecutable, &backgroundExecutables, &newGMP);
		watcher.setFuture(future);
		loop.exec();
		if (!future.result()) {
			_mainWindow->setEnabled(true);
			_mainWindow->setWindowState(_oldWindowState);
			return;
		}
		QProcess * process = new QProcess(this);
		for (const QString & backgroundProcess : backgroundExecutables) {
			QProcess * bp = new QProcess(process);
			connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), bp, SLOT(terminate()));
			bp->setWorkingDirectory(usedBaseDir + "/" + QFileInfo(backgroundProcess).path());
			bp->start("\"" + usedBaseDir + "/" + backgroundProcess + "\"");
		}
		connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(errorOccurred(QProcess::ProcessError)));
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedMod(int, QProcess::ExitStatus)));
		process->setWorkingDirectory(usedBaseDir + "/System/");
		if (GeneralSettingsWidget::extendedLogging) {
			LOGINFO("Preparation duration: " << t.elapsed());
		}
		_timer->start();
		QStringList args;
		args << "-game:" + _iniFile.split("/").back();
		if (_zSpyActivated && _zSpyLevel->value() > 0) {
			QProcess::startDetached("\"" + usedBaseDir + "/_work/tools/zSpy/zSpy.exe\"");
			args << "-zlog:" + QString::number(_zSpyLevel->value()) + ",s";
			LOGINFO("Started zSpy");
		}
		if (!_username.isEmpty() && _onlineMode) {
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
				common::SendUserInfosMessage suim;
				suim.username = _username.toStdString();
				suim.password = _password.toStdString();
				suim.hash = security::Hash::calculateSystemHash().toStdString();
				suim.mac = security::Hash::getMAC().toStdString();
				QSettings gothicIniParser(usedBaseDir + "/System/Gothic.ini", QSettings::IniFormat);
				{
					QString zVidResFullscreenX = gothicIniParser.value("VIDEO/zVidResFullscreenX", "0").toString();
					QString zVidResFullscreenY = gothicIniParser.value("VIDEO/zVidResFullscreenY", "0").toString();
					suim.settings.emplace_back("zVidResFullscreenX", zVidResFullscreenX.toStdString());
					suim.settings.emplace_back("zVidResFullscreenY", zVidResFullscreenY.toStdString());
					gothicIniParser.beginGroup("KEYS");
					QStringList entries = gothicIniParser.allKeys();
					for (const QString & s : entries) {
						QString value = gothicIniParser.value(s, "").toString();
						if (!value.isEmpty()) {
							suim.settings.emplace_back(s.toStdString(), value.toStdString());
						}
					}
					gothicIniParser.endGroup();
				}
				const std::string serialized = suim.SerializePublic();
				sock.writePacket(serialized);
			}
		}
		if (_onlineMode) {
			std::thread([]() {
				clockUtils::sockets::TcpSocket sock;
				if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
					common::UpdatePlayingTimeMessage uptm;
					uptm.dayOfTheWeek = QDate::currentDate().dayOfWeek();
					uptm.hour = QTime::currentTime().hour();
					const std::string serialized = uptm.SerializePublic();
					sock.writePacket(serialized);
				}
			}).detach();
		}
		if (_developerModeActive && _compileScripts->isChecked()) {
			args << "-zreparse";
		}
		if (_developerModeActive && _startupWindowed->isChecked()) {
			args << "-zwindow";
		}
		if (_developerModeActive && _convertTextures->isChecked()) {
			args << "-ztexconvert";
		}
		if (_developerModeActive && _noSound->isChecked()) {
			args << "-znosound";
		}
		if (_developerModeActive && _noMusic->isChecked()) {
			args << "-znomusic";
		}
#ifdef Q_OS_WIN
		if (newGMP) {
			usedExecutable = "gml.exe";
		}
		QtConcurrent::run([usedExecutable]() {
			QString tmp = usedExecutable;
			HANDLE hProcess = GetProcHandle(q2s(tmp).c_str());
			while (hProcess == INVALID_HANDLE_VALUE) {
				hProcess = GetProcHandle(q2s(tmp).c_str());
			}

			HMODULE hKernel32 = GetModuleHandle("kernel32");
			// Procedures in kernel32.dll are loaded at the same address in all processes
			// so find the address in our own process, then use it in the target process
			FARPROC pSetProcessDEPPolicy = GetProcAddress(hKernel32, "SetProcessDEPPolicy");
			HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pSetProcessDEPPolicy), nullptr /* disable DEP */, 0, nullptr);
			if (hThread == nullptr) {
			  // handle/report error
			}
			WaitForSingleObject(hThread, 10000);
			CloseHandle(hThread);
			CloseHandle(hProcess);
		});
#endif
		if (newGMP) {
			process->start("\"" + usedBaseDir + "/" + usedExecutable + "\"", args);
		} else {
			process->start("\"" + usedBaseDir + "/System/" + usedExecutable + "\"", args);
		}
		LOGINFO("Started " << usedExecutable.toStdString());
		_screenshotManager->start(_modID);
#ifdef Q_OS_WIN
		{
			QSettings gothicIniParser(usedBaseDir + "/System/Gothic.ini", QSettings::IniFormat);
			if (_gamepadSettingsWigdet->isEnabled()) {
				QMap<QString, gamepad::GamePadButton> buttonMapping = _gamepadSettingsWigdet->getKeyMapping();
				std::map<gamepad::GamePadButton, gamepad::DIK_KeyCodes> realButtonMapping;
				gamepad::GamePadButton previousSpellButton = gamepad::GamePadButton::GamePadButton_Max;
				gamepad::GamePadButton nextSpellButton = gamepad::GamePadButton::GamePadButton_Max;
				gamepad::GamePadButton drawSpellButton = gamepad::GamePadButton::GamePadButton_Max;
				for (auto it = buttonMapping.begin(); it != buttonMapping.end(); ++it) {
					if (it.key() == "keyDrawMeleeWeapon") {
						realButtonMapping.insert(std::make_pair(it.value(), gamepad::DIK_KeyCodes::KEY_1));
					} else if (it.key() == "keyDrawRangedWeapon") {
						realButtonMapping.insert(std::make_pair(it.value(), gamepad::DIK_KeyCodes::KEY_2));
					} else if (it.key() == "keyDrawSpell") {
						drawSpellButton = it.value();
					} else if (it.key() == "keyPreviousSpell") {
						previousSpellButton = it.value();
					} else if (it.key() == "keyNextSpell") {
						nextSpellButton = it.value();
					} else if (it.key() == "keyEnter") {
						realButtonMapping.insert(std::make_pair(it.value(), gamepad::DIK_KeyCodes::KEY_RETURN));
					} else {
						QString key = gothicIniParser.value("KEYS/" + it.key(), "").toString();
						if (!key.isEmpty()) {
							if (key.size() > 2) {
								key.resize(2);
							}
							if (key.isEmpty()) {
								continue;
							}
							realButtonMapping.insert(std::make_pair(it.value(), gamepad::DIK_KeyCodes(key.toInt(nullptr, 16))));
						}
					}
				}
				_gamepad = new gamepad::GamePadXbox(_gamepadSettingsWigdet->getIndex(), _gamepadSettingsWigdet->getKeyDelay(), realButtonMapping, previousSpellButton, nextSpellButton, drawSpellButton);
			}
		}
#endif
	}

	void ModInfoView::updatedMod(int modID) {
		if (_modID != modID) return;

		setModID(QString::number(modID));
	}

	void ModInfoView::errorOccurred(QProcess::ProcessError error) {
		QProcess * process = dynamic_cast<QProcess *>(sender());
		LOGERROR("Error Code: " << error);
		if (process) {
			LOGERROR("Some error occurred: " << process->errorString().toStdString());
		}
	}

	void ModInfoView::finishedMod(int, QProcess::ExitStatus status) {
		_screenshotManager->stop();
#ifdef Q_OS_WIN
		delete _gamepad;
		_gamepad = nullptr;
#endif
		LOGINFO("Finished Mod");
		if (GeneralSettingsWidget::extendedLogging) {
			LOGINFO("Resetting ini files");
		}
		const QString usedBaseDir = getUsedBaseDir();
		{
			QSettings gothicIniParser(usedBaseDir + "/System/Gothic.ini", QSettings::IniFormat);
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
			QFile(usedBaseDir + "/System/Gothic.ini").remove();
			QFile(usedBaseDir + "/System/Gothic.ini.spbak").rename(usedBaseDir + "/System/Gothic.ini");
		} else {
			QFile(usedBaseDir + "/System/Gothic.ini.spbak").remove();
		}
		{
			QSettings systempackIniParser(usedBaseDir + "/System/Systempack.ini", QSettings::IniFormat);
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
		_gothicIniBackup.clear();
		_systempackIniBackup.clear();
		int duration = _timer->elapsed();
		duration = duration / 1000; // to seconds
		duration = duration / 60; // to minutes
		delete _listenSocket;
		_listenSocket = nullptr;
		delete _socket;
		_socket = nullptr;
		if (duration > 0) {
			if (_onlineMode && _modID != -1) {
				Database::DBError err;
				std::vector<std::string> patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + ";", err);
				for (const std::string & patchIDString : patches) {
					std::vector<std::string> res = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "SELECT Compatible FROM ownCompatibilityVotes WHERE ModID = " + std::to_string(_modID) + " AND PatchID = " + patchIDString + " LIMIT 1;", err);
					if (res.empty()) {
						const std::string patchName = Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", err);
						NewCombinationDialog dlg(QApplication::tr("NewCombinationDetected"), QApplication::tr("NewCombinationDetectedText").arg(_nameLabel->text()).arg(s2q(patchName)), _iniParser, this);
						if (dlg.canShow()) {
							if (dlg.exec() == QDialog::Accepted) {
								SubmitCompatibilityDialog submitDlg(_language, _username, _password, _modID, std::stoi(patchIDString), getGothicVersion());
								submitDlg.exec();
							}
						}
						break;
					}
				}

				std::thread([this, duration]() {
					common::UpdatePlayTimeMessage uptm;
					uptm.username = _username.toStdString();
					uptm.password = _password.toStdString();
					uptm.modID = _modID;
					uptm.duration = duration;
					std::string serialized = uptm.SerializePublic();
					clockUtils::sockets::TcpSocket sock;
					if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
						sock.writePacket(serialized);
						// everything worked fine so far, so add patches!
						Database::DBError dbErr;
						std::vector<std::string> patches2 = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + ";", dbErr);
						for (const std::string & patchIDString : patches2) {
							const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", dbErr);
							if (count == 0) {
								continue;
							}

							const int patchID = std::stoi(patchIDString);
							uptm.modID = patchID;
							serialized = uptm.SerializePublic();
							sock.writePacket(serialized);
						}
						updateModStats();
					}
				}).detach();
			} else {
				Database::DBError err;
				Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO playTimes (ModID, Username, Duration) VALUES (" + std::to_string(_modID) + ", '" + _username.toStdString() + "', " + std::to_string(duration) + ");", err);
				if (err.error) {
					Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE playTimes SET Duration = Duration + " + std::to_string(duration) + " WHERE ModID = " + std::to_string(_modID) + " AND Username = '" + _username.toStdString() + "';", err);
				}
			}
		}
		removeModFiles();

		// more of a hack in order to fix corruption/deletion of Gothic2.exe due to some mod/patch
		Database::DBError err;
		const ModVersion modGv = Database::queryNth<ModVersion, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion, MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err, 0);

		if (modGv.gothicVersion == common::GothicVersion::GOTHIC) {
			setGothicDirectory(_gothicDirectory);
		} else if (modGv.gothicVersion == common::GothicVersion::GOTHIC2 || modGv.gothicVersion == common::GothicVersion::GOTHICINGOTHIC2) {
			setGothic2Directory(_gothic2Directory);
		}

		_mainWindow->setEnabled(true);
		_mainWindow->setWindowState(_oldWindowState);

		Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "DELETE FROM lastPlayed;", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "INSERT INTO lastPlayed (ModID, Ini) VALUES (" + std::to_string(_modID) + ", '" + _iniFile.toStdString() + "');", err);
	}

	void ModInfoView::changedPatchState() {
		QCheckBox * cb = dynamic_cast<QCheckBox *>(sender());
		assert(cb);
		if (cb) {
			Database::DBError err;
			if (cb->isChecked()) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(_modID) + ", " + std::to_string(_checkboxPatchIDMapping[cb]) + ", 1);", err);
			} else {
				Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + " AND PatchID = " + std::to_string(_checkboxPatchIDMapping[cb]) + ";", err);
			}
		}
	}

	void ModInfoView::updateModInfoView(common::ModStats ms) {
		const QString timeString = timeToString(ms.duration);
		_playTimeLabel->setText(timeString);

		_achievementLabel->setText(R"(<a href="Foobar" style="color: #181C22">)" + QApplication::tr("AchievementText").arg(ms.achievedAchievements).arg(ms.allAchievements).arg(ms.achievedAchievements * 100 / ((ms.allAchievements) ? ms.allAchievements : 1)) + "</a>");
		_achievementLabel->setVisible(ms.allAchievements > 0);

		_scoresLabel->setText(R"(<a href="Blafoo" style="color: #181C22">)" + ((ms.bestScoreRank > 0) ? QApplication::tr("BestScoreText").arg(s2q(ms.bestScoreName)).arg(ms.bestScoreRank).arg(ms.bestScore) : QApplication::tr("NoBestScore")) + "</a>");
		_scoresLabel->setVisible(ms.bestScoreRank != -1);

		if (ms.lastTimePlayed == -1) {
			_lastPlayedDate->hide();
		} else {
			_lastPlayedDate->setText(QApplication::tr("LastPlayed").arg(QDate(1970, 1, 1).addDays(ms.lastTimePlayed / 24).toString("dd.MM.yyyy")));
			_lastPlayedDate->show();
		}
	}

	void ModInfoView::restartSpineAsAdmin() {
#ifdef Q_OS_WIN
		const QString exeFileName = qApp->applicationDirPath() + "/" + qApp->applicationName();
		const int result = int(::ShellExecuteA(nullptr, "runas", exeFileName.toUtf8().constData(), nullptr, nullptr, SW_SHOWNORMAL));
		if (result > 32) { // no error
			qApp->quit();
		}
#endif
	}

	void ModInfoView::prepareAchievementView() {
		emit openAchievementView(_modID, _nameLabel->text());
	}

	void ModInfoView::prepareScoreView() {
		emit openScoreView(_modID, _nameLabel->text());
	}

	void ModInfoView::patchCheckG1() {
		if (!QFile(_gothicDirectory + "/System/Gothic.exe").exists()) {
			return;
		}
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
		connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
		connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));

		bool start = false;

		for (auto it = fileList.begin(); it != fileList.end(); ++it) {
			QFile f(_gothicDirectory + "/" + it.key());
			if (GeneralSettingsWidget::extendedLogging) {
				LOGINFO("Checking G1 file " << it.key().toStdString());
			}
			if (f.open(QFile::ReadOnly)) {
				QCryptographicHash hash(QCryptographicHash::Sha512);
				if (hash.addData(&f)) {
					QString hashSum = QString::fromLatin1(hash.result().toHex());
					if (hashSum != it.value()) {
						start = true;
						QFileInfo fi(it.key());
						FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g1/" + it.key()), _gothicDirectory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
						mfd->addFileDownloader(fd);
						if (GeneralSettingsWidget::extendedLogging) {
							LOGWARN("Hashes don't match: " << hashSum.toStdString() << " vs. " << it.value().toStdString());
						}
					}
				} else {
					if (GeneralSettingsWidget::extendedLogging) {
						LOGWARN("Hashing failed");
					}
					start = true;
					QFileInfo fi(it.key());
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g1/" + it.key()), _gothicDirectory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
					mfd->addFileDownloader(fd);
				}
			} else {
				if (GeneralSettingsWidget::extendedLogging) {
					LOGWARN("Couldn't open file");
				}
				start = true;
				QFileInfo fi(it.key());
				FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g1/" + it.key()), _gothicDirectory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
				mfd->addFileDownloader(fd);
			}
		}
		if (start) {
			DownloadProgressDialog progressDlg(mfd, "PatchingG1", 0, 100, 0, _mainWindow);
			progressDlg.setCancelButton(nullptr);
			progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			progressDlg.exec();
		} else {
			mfd->deleteLater();
		}

		Database::DBError err;
		const std::vector<std::string> ids = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 57 LIMIT 1;", err);
		if (ids.empty()) {
			emit installMod(57);
		}
	}

	void ModInfoView::patchCheckG2() {
		if (!QFile(_gothic2Directory + "/System/Gothic2.exe").exists()) {
			return;
		}
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
			QFile f(_gothic2Directory + "/" + it.key());
			if (GeneralSettingsWidget::extendedLogging) {
				LOGINFO("Checking G2 file " << it.key().toStdString());
			}
			if (f.open(QFile::ReadOnly)) {
				QCryptographicHash hash(QCryptographicHash::Sha512);
				if (hash.addData(&f)) {
					QString hashSum = QString::fromLatin1(hash.result().toHex());
					if (hashSum != it.value()) {
						if (GeneralSettingsWidget::extendedLogging) {
							LOGWARN("Hashes don't match: " << hashSum.toStdString() << " vs. " << it.value().toStdString());
						}
						start = true;
						QFileInfo fi(it.key());
						FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g2/" + it.key()), _gothic2Directory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
						mfd->addFileDownloader(fd);
					}
				} else {
					if (GeneralSettingsWidget::extendedLogging) {
						LOGWARN("Hashing failed");
					}
					start = true;
					QFileInfo fi(it.key());
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g2/" + it.key()), _gothic2Directory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
					mfd->addFileDownloader(fd);
				}
			} else {
				if (GeneralSettingsWidget::extendedLogging) {
					LOGWARN("Couldn't open file");
				}
				start = true;
				QFileInfo fi(it.key());
				FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g2/" + it.key()), _gothic2Directory + "/" + fi.path(), fi.fileName(), it.value(), mfd);
				mfd->addFileDownloader(fd);
			}
		}
		if (start) {
			DownloadProgressDialog progressDlg(mfd, "PatchingG2", 0, 100, 0, _mainWindow);
			progressDlg.setCancelButton(nullptr);
			progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			progressDlg.exec();
		} else {
			mfd->deleteLater();
		}

		Database::DBError err;
		const std::vector<std::string> ids = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 40 LIMIT 1;", err);
		if (ids.empty()) {
			emit installMod(40);
		}
	}

	void ModInfoView::updateCompatibilityList(int modID, std::vector<int32_t> incompatiblePatches, std::vector<int32_t> forbiddenPatches) {
		if (_modID != modID) {
			return;
		}
		for (QCheckBox * cb : _patchList) {
			_patchLayout->removeWidget(cb);
			delete cb;
		}
		_patchList.clear();

		Database::DBError err;
		const ModVersion modGv = Database::queryNth<ModVersion, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion, MajorVersion, MinorVersion, PatchVersion FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err, 0);

		const auto contains = [](const std::vector<int32_t> & list, int32_t id) {
			const auto it = std::find_if(list.begin(), list.end(), [id](const int32_t & a) {
				return a == id;
			});
			return it != list.end();
		};

		std::vector<Patch> patches = Database::queryAll<Patch, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, Name FROM patches WHERE ModID != 228 ORDER BY Name ASC;", err);
		for (Patch p : patches) {
			const common::GothicVersion gv = common::GothicVersion(std::stoi(Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(p.modID) + " LIMIT 1;", err, 0)));
			if ((gv == modGv.gothicVersion || ((modGv.gothicVersion == common::GothicVersion::GOTHIC || modGv.gothicVersion == common::GothicVersion::GOTHIC2 || modGv.gothicVersion == common::GothicVersion::GOTHICINGOTHIC2) && gv == common::GothicVersion::Gothic1And2)) && (!contains(incompatiblePatches, p.modID) || !_hideIncompatible) && !contains(forbiddenPatches, p.modID)) {
				QCheckBox * cb = new QCheckBox(s2q(p.name), this);
				cb->setProperty("library", true);
				_patchLayout->addWidget(cb, _patchCounter / 2, _patchCounter % 2);
				++_patchCounter;
				_patchList.append(cb);
				const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT Enabled FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + " AND PatchID = " + std::to_string(p.modID) + " LIMIT 1;", err);
				cb->setChecked(count);
				_checkboxPatchIDMapping.insert(std::make_pair(cb, p.modID));
				connect(cb, SIGNAL(stateChanged(int)), this, SLOT(changedPatchState()));
				cb->setProperty("patchID", p.modID);
			}
		}
		if (_patchCounter % 2 == 1) {
			++_patchCounter;
		}
		if (!_patchList.empty()) {
			_patchGroup->show();
			_patchGroup->setToolTip(QApplication::tr("PatchesAndToolsTooltip").arg(_nameLabel->text()));
		}
	}

	void ModInfoView::startSpacer() {
		const QString usedBaseDir = getUsedBaseDir();
		QString usedExecutable;
		if (QFile(usedBaseDir + "/System/Spacer.exe").exists()) {
			usedExecutable = "Spacer.exe";
		} else if (QFile(usedBaseDir + "/System/Spacer2_exe.exe").exists()) {
			usedExecutable = "Spacer2_exe.exe";
		} else if (QFile(usedBaseDir + "/System/Spacer2.exe").exists()) {
			usedExecutable = "Spacer2.exe";
		}
		// check overrides and backup original values!
		_gothicIniBackup.clear();
		_systempackIniBackup.clear();
		QSettings iniParser(_iniFile, QSettings::IniFormat);
		{
			QSettings gothicIniParser(usedBaseDir + "/System/Gothic.ini", QSettings::IniFormat);
			iniParser.beginGroup("OVERRIDES");
			QStringList entries = iniParser.allKeys();
			for (const QString & s : entries) {
				const QString & entry = s;
				QString section = entry.split(".").front();
				QString key = entry.split(".").back();
				// backup original value
				QString val = gothicIniParser.value(section + "/" + key, "---").toString();
				_gothicIniBackup.emplace_back(section, key, val);
				val = iniParser.value(entry, "---").toString();
				if (val != "---") {
					gothicIniParser.setValue(section + "/" + key, val);
				}
			}
			iniParser.endGroup();
		}
		{
			QSettings systempackIniParser(usedBaseDir + "/System/Systempack.ini", QSettings::IniFormat);
			iniParser.beginGroup("OVERRIDES_SP");
			QStringList entries = iniParser.allKeys();
			for (const QString & s : entries) {
				const QString & entry = s;
				QString section = entry.split(".").front();
				QString key = entry.split(".").back();
				// backup original value
				QString val = systempackIniParser.value(section + "/" + key, "---").toString();
				_systempackIniBackup.emplace_back(section, key, val);
				val = iniParser.value(entry, "---").toString();
				if (val != "---") {
					systempackIniParser.setValue(section + "/" + key, val);
				}
			}
			iniParser.endGroup();
		}
		QProcess * process = new QProcess(this);
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedSpacer()));
		connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
		process->setWorkingDirectory(usedBaseDir + "/System/");
		QStringList args;
		args << "-zmaxframerate:50";
		process->start("\"" + usedBaseDir + "/System/" + usedExecutable + "\"", args);
		setDisabled(true);
	}

	void ModInfoView::finishedSpacer() {
		const QString usedBaseDir = getUsedBaseDir();
		{
			QSettings gothicIniParser(usedBaseDir + "/System/Gothic.ini", QSettings::IniFormat);
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
			QSettings systempackIniParser(usedBaseDir + "/System/Systempack.ini", QSettings::IniFormat);
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

		setEnabled(true);
	}

	void ModInfoView::showErrorMessage(QString msg) {
		QMessageBox msgBox(QMessageBox::Icon::Information, QApplication::tr("ErrorOccurred"), msg, QMessageBox::StandardButton::Ok);
		msgBox.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msgBox.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msgBox.exec();
	}

	QStringList ModInfoView::getGothicFiles() const {
		static QStringList gothicFiles = {
			"Data/fonts.VDF",
			"Data/meshes.VDF",
			"Data/sound.VDF",
			"Data/sound_patch2.VDF",
			"Data/speech.VDF",
			"Data/speech_patch2.VDF",
			"Data/textures.VDF",
			"Data/textures_apostroph_patch_neu.VDF",
			"Data/textures_patch.VDF",
			"_work/data/Music/dungeon/_OGY.sty",
			"_work/data/Music/dungeon/_OLM.sty",
			"_work/data/Music/dungeon/Dungeons.dls",
			"_work/data/Music/dungeon/OGY_Day_Fgt.sgt",
			"_work/data/Music/dungeon/OGY_Day_Std.sgt",
			"_work/data/Music/dungeon/OLM_Day_Fgt.sgt",
			"_work/data/Music/dungeon/OLM_Day_Std.sgt",
			"_work/data/Music/dungeon/OLM_Day_Thr.sgt",
			"_work/data/Music/menu_men/Menu.sgt",
			"_work/data/Music/menu_men/menu.sty",
			"_work/data/Music/menu_men/menu_MEN.dls",
			"_work/data/Music/orchestra/__Orchestra.dls",
			"_work/data/Music/orchestra/_BAN.sty",
			"_work/data/Music/orchestra/_CAM.sty",
			"_work/data/Music/orchestra/_FOC.sty",
			"_work/data/Music/orchestra/_NCI.sty",
			"_work/data/Music/orchestra/_OC.sty",
			"_work/data/Music/orchestra/_ORC.sty",
			"_work/data/Music/orchestra/_OW.sty",
			"_work/data/Music/orchestra/_PSI.sty",
			"_work/data/Music/orchestra/BAN_Day_Std.sgt",
			"_work/data/Music/orchestra/CAM_Day_Fgt.sgt",
			"_work/data/Music/orchestra/CAM_Day_Std.sgt",
			"_work/data/Music/orchestra/CAM_Day_Thr.sgt",
			"_work/data/Music/orchestra/CAM_Ngt_Std.sgt",
			"_work/data/Music/orchestra/Default.sgt",
			"_work/data/Music/orchestra/FOC_Day_Std.sgt",
			"_work/data/Music/orchestra/NCI_Day_Fgt.sgt",
			"_work/data/Music/orchestra/NCI_Day_Std.sgt",
			"_work/data/Music/orchestra/OC_Day_Fgt.sgt",
			"_work/data/Music/orchestra/OC_Day_Std.sgt",
			"_work/data/Music/orchestra/ORC_Day_Std.sgt",
			"_work/data/Music/orchestra/OW_Day_Std.sgt",
			"_work/data/Music/orchestra/PSI_Day_Fgt.sgt",
			"_work/data/Music/orchestra/PSI_Day_Std.sgt",
			"_work/data/Music/orchestra/PSI_Day_Thr.sgt",
			"_work/data/Music/orchestra/PSI_Ngt_Std.sgt",
			"_work/data/Video/credits.bik",
			"_work/data/Video/extro.bik",
			"_work/data/Video/greatPrayer.bik",
			"_work/data/Video/intro.bik",
			"_work/data/Video/logo1.bik",
			"_work/data/Video/logo2.bik",
			"_work/data/Video/oreheap.bik",
			"_work/data/Video/playerout.bik"
		};
		return gothicFiles;
	}

	void ModInfoView::removeGothicFiles() {
		if (_isInstalled) {
			Database::DBError err;
			const common::GothicVersion mid = common::GothicVersion(Database::queryNth<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err, 0).front());
			if (mid == common::GothicVersion::GOTHICINGOTHIC2 && !_gothic2Directory.isEmpty()) {
				const QString usedBaseDir = _gothic2Directory;
				static QStringList gothicFiles = getGothicFiles();
				for (QString file : gothicFiles) {
					if (file.endsWith(".vdf", Qt::CaseSensitivity::CaseInsensitive)) {
						file = file.insert(file.length() - 4, "_G1");
					}
					QFile f(usedBaseDir + "/" + file);
					f.remove();
				}

				QDirIterator itBackup(usedBaseDir, QStringList() << "*.spbak", QDir::Files, QDirIterator::Subdirectories);
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
					backupFile.resize(backupFile.size() - 6);
					f.rename(backupFile);
				}
			}
		}
	}

	void ModInfoView::removeModFiles() {
		if (!_lastBaseDir.isEmpty()) {
			QSet<QString> removed;
			Database::DBError err;
			Database::open(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "BEGIN TRANSACTION;", err);
			for (const QString & file : _copiedFiles) {
				if (QFile(_lastBaseDir + "/" + file).exists()) {
					if (QFile(_lastBaseDir + "/" + file).remove()) {
						Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + file.toStdString() + "';", err);
						removed.insert(file);
						if (GeneralSettingsWidget::extendedLogging) {
							LOGINFO("Removed file " << file.toStdString());
						}
					} else {
#ifdef Q_OS_WIN
						if (QFileInfo(_lastBaseDir + "/" + file).isSymLink()) {
							removeSymlink(_lastBaseDir + "/" + file);
						}
#endif
						if (GeneralSettingsWidget::extendedLogging) {
							LOGINFO("Couldn't remove file " << file.toStdString());
						}
					}
				} else if (QDir(_lastBaseDir + "/" + file).exists()) {
					if (QDir(_lastBaseDir + "/" + file).remove(_lastBaseDir + "/" + file)) {
						Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + file.toStdString() + "';", err);
						removed.insert(file);
						if (GeneralSettingsWidget::extendedLogging) {
							LOGINFO("Removed file " << file.toStdString());
						}
					} else {
						if (GeneralSettingsWidget::extendedLogging) {
							LOGINFO("Couldn't remove file " << file.toStdString());
						}
					}
				} else {
					LOGINFO("Couldn't remove file " << file.toStdString());
				}
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "END TRANSACTION;", err);
			Database::close(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
			for (const QString & file : removed) {
				_copiedFiles.erase(file);
			}

			if (GeneralSettingsWidget::extendedLogging) {
				LOGINFO("Not removed files: " << _copiedFiles.size());
			}

			removeEmptyDirs(_lastBaseDir);

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
					LOGINFO("Couldn't rename file: " << q2s(backupFile));
				}
			}
		}

		QString usedBaseDir = getUsedBaseDir();
		if (!usedBaseDir.isEmpty()) {
			QDirIterator it(usedBaseDir + "/Data", QStringList() << "*mod", QDir::Files);
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
			if (_isInstalled || _developerModeActive) {
				QFile(usedBaseDir + "/System/SpineAPI.dll").remove();
				QFile(usedBaseDir + "/System/m2etis.dll").remove();
				QFile(usedBaseDir + "/Data/Spine.vdf").remove();
			}
		}
	}

	QString ModInfoView::getUsedBaseDir() const {
		QString usedBaseDir;
		if (_isInstalled) {
			Database::DBError err;
			const common::GothicVersion mid = common::GothicVersion(Database::queryNth<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err, 0).front());
			if (mid == common::GothicVersion::GOTHIC && !_gothicDirectory.isEmpty()) {
				usedBaseDir = _gothicDirectory;
			} else if (mid == common::GothicVersion::GOTHIC2 && !_gothic2Directory.isEmpty()) {
				usedBaseDir = _gothic2Directory;
			} else if (mid == common::GothicVersion::GOTHICINGOTHIC2 && !_gothicDirectory.isEmpty() && !_gothic2Directory.isEmpty()) {
				usedBaseDir = _gothic2Directory;
			}
		} else {
			if (!_gothicDirectory.isEmpty() && _iniFile.contains(_gothicDirectory + "/")) {
				usedBaseDir = _gothicDirectory;
			} else if (!_gothic2Directory.isEmpty() && _iniFile.contains(_gothic2Directory + "/")) {
				usedBaseDir = _gothic2Directory;
			}
		}
		return usedBaseDir;
	}

	common::GothicVersion ModInfoView::getGothicVersion() const {
		return getGothicVersion(getUsedBaseDir());
	}

	common::GothicVersion ModInfoView::getGothicVersion(QString path) const {
		return QFile(path + "/System/Gothic2.exe").exists() ? common::GothicVersion::GOTHIC2 : (QFile(path + "/System/Gothic.exe").exists()) ? common::GothicVersion::GOTHIC : common::GothicVersion(1);
	}

	void ModInfoView::acceptedConnection(clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError err) {
		if (err == clockUtils::ClockError::SUCCESS) {
			delete _socket;
			_socket = sock;
			sock->receiveCallback(std::bind(&ModInfoView::receivedMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}
	}

	void ModInfoView::receivedMessage(std::vector<uint8_t> message, clockUtils::sockets::TcpSocket * socket, clockUtils::ClockError err) {
		if (err == clockUtils::ClockError::SUCCESS) {
			try {
				std::string serialized(message.begin(), message.end());
				common::Message * msg = common::Message::DeserializeBlank(serialized);
				if (msg) {
					if (msg->type == common::MessageType::REQUESTUSERNAME) {
						common::SendUsernameMessage sum;
						sum.username = _username.toStdString();
						sum.password = _password.toStdString();
						sum.modID = _modID;
						serialized = sum.SerializeBlank();
						socket->writePacket(serialized);
					} else if (msg->type == common::MessageType::REQUESTSCORES) {
						common::RequestScoresMessage * rsm = dynamic_cast<common::RequestScoresMessage *>(msg);
						if (_isInstalled) {
							if (rsm) {
								rsm->modID = _modID;
								if (_onlineMode) {
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = rsm->SerializePublic();
										if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
											if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
												common::Message * newMsg = common::Message::DeserializePublic(serialized);
												serialized = newMsg->SerializeBlank();
												delete newMsg;
												socket->writePacket(serialized);
											} else {
												socket->writePacket("empty");
											}
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									Database::DBError dbErr;
									std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Username, Score FROM modScores WHERE ModID = " + std::to_string(_modID) + " ORDER BY Score DESC", dbErr);
									common::SendScoresMessage ssm;
									std::map<int, std::vector<std::pair<std::string, int32_t>>> scores;
									for (auto vec : lastResults) {
										int32_t identifier = int32_t(std::stoi(vec[0]));
										std::string username = vec[1];
										int32_t score = int32_t(std::stoi(vec[2]));
										if (!username.empty()) {
											scores[identifier].push_back(std::make_pair(username, score));
										}
									}
									for (auto & score : scores) {
										ssm.scores.emplace_back(score.first, score.second);
									}
									serialized = ssm.SerializeBlank();
									socket->writePacket(serialized);
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							common::SendScoresMessage ssm;
							serialized = ssm.SerializeBlank();
							socket->writePacket(serialized);
						}
					} else if (msg->type == common::MessageType::UPDATESCORE) {
						common::UpdateScoreMessage * usm = dynamic_cast<common::UpdateScoreMessage *>(msg);
						if (_isInstalled) {
							if (usm) {
								if (_onlineMode) {
									usm->modID = _modID;
									usm->username = _username.toStdString();
									usm->password = _password.toStdString();
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = usm->SerializePublic();
										if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
											cacheScore(usm);
										} else {
											removeScore(usm);
										}
									} else {
										cacheScore(usm);
									}
								} else {
									Database::DBError dbErr;
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(_modID) + ", " + std::to_string(usm->identifier) + ", '" + _username.toStdString() + "', " + std::to_string(usm->score) + ");", dbErr);
									if (dbErr.error) {
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modScores SET Score = " + std::to_string(usm->score) + " WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + std::to_string(usm->identifier) + " AND Username = '" + _username.toStdString() + "';", dbErr);
									}
								}
							}
						}
					} else if (msg->type == common::MessageType::REQUESTACHIEVEMENTS) {
						common::RequestAchievementsMessage * ram = dynamic_cast<common::RequestAchievementsMessage *>(msg);
						if (_isInstalled) {
							if (ram && !_username.isEmpty()) {
								if (_onlineMode) {
									ram->modID = _modID;
									ram->username = _username.toStdString();
									ram->password = _password.toStdString();
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = ram->SerializePublic();
										if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
											if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
												common::SendAchievementsMessage * sam = dynamic_cast<common::SendAchievementsMessage *>(common::Message::DeserializePublic(serialized));
												sam->showAchievements = _showAchievements;
												serialized = sam->SerializeBlank();
												socket->writePacket(serialized);
												delete sam;
											} else {
												socket->writePacket("empty");
											}
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									Database::DBError dbErr;
									std::vector<std::string> lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(_modID) + " AND Username = '" = _username.toStdString() + "';", dbErr);

									common::SendAchievementsMessage sam;
									for (const std::string & s : lastResults) {
										int32_t identifier = int32_t(std::stoi(s));
										sam.achievements.push_back(identifier);
									}
									auto lastResultsVec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Max FROM modAchievementProgressMax WHERE ModID = " + std::to_string(_modID) + ";", dbErr);
									for (auto vec : lastResultsVec) {
										lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + vec[0] + " AND Username = '" + _username.toStdString() +  "' LIMIT 1;", dbErr);
										if (lastResults.empty()) {
											sam.achievementProgress.emplace_back(std::stoi(vec[0]), std::make_pair(0, std::stoi(vec[1])));
										} else {
											sam.achievementProgress.emplace_back(std::stoi(vec[0]), std::make_pair(std::stoi(lastResults[0]), std::stoi(vec[1])));
										}
									}
									serialized = sam.SerializeBlank();
									socket->writePacket(serialized);
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							common::SendAchievementsMessage sam;
							sam.showAchievements = _showAchievements;
							serialized = sam.SerializeBlank();
							socket->writePacket(serialized);
						}
					} else if (msg->type == common::MessageType::UNLOCKACHIEVEMENT) {
						common::UnlockAchievementMessage * uam = dynamic_cast<common::UnlockAchievementMessage *>(msg);
						if (_isInstalled) {
							if (uam) {
								if (_onlineMode) {
									int duration = _timer->elapsed();
									duration = duration / 1000; // to seconds
									duration = duration / 60;

									uam->modID = _modID;
									uam->username = _username.toStdString();
									uam->password = _password.toStdString();
									uam->duration = duration;
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = uam->SerializePublic();
										if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
											cacheAchievement(uam);
										} else {
											removeAchievement(uam);
										}
									} else {
										cacheAchievement(uam);
									}
								} else {
									Database::DBError dbErr;
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(_modID) + ", " + std::to_string(uam->identifier) + ", '" + _username.toStdString() + ");", dbErr);
								}
							}
						}
					} else if (msg->type == common::MessageType::UPDATEACHIEVEMENTPROGRESS) {
						common::UpdateAchievementProgressMessage * uapm = dynamic_cast<common::UpdateAchievementProgressMessage *>(msg);
						if (_isInstalled) {
							if (uapm) {
								if (_onlineMode) {
									uapm->modID = _modID;
									uapm->username = _username.toStdString();
									uapm->password = _password.toStdString();
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = uapm->SerializePublic();
										if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
											cacheAchievementProgress(uapm);
										} else {
											removeAchievementProgress(uapm);
										}
									} else {
										cacheAchievementProgress(uapm);
									}
								} else {
									Database::DBError dbErr;
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(_modID) + ", " + std::to_string(uapm->identifier) + ", '" + _username.toStdString() + "', " + std::to_string(uapm->progress) + ");", dbErr);
									if (dbErr.error) {
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE modAchievementProgress SET Current = " + std::to_string(uapm->progress) + " WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + std::to_string(uapm->identifier) + " AND Username = '" + _username.toStdString() + "';", dbErr);
									}
								}
							}
						}
					} else if (msg->type == common::MessageType::SEARCHMATCH) {
						if (_isInstalled && _onlineMode) {
							common::SearchMatchMessage * smm = dynamic_cast<common::SearchMatchMessage *>(msg);
							if (smm) {
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									smm->modID = _modID;
									smm->username = _username.toStdString();
									smm->password = _password.toStdString();
									serialized = smm->SerializePublic();
									if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
										if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
											common::Message * newMsg = common::Message::DeserializePublic(serialized);
											serialized = newMsg->SerializeBlank();
											delete newMsg;
											socket->writePacket(serialized);
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							common::FoundMatchMessage fmm;
							serialized = fmm.SerializeBlank();
							socket->writePacket(serialized);
						}
					} else if (msg->type == common::MessageType::REQUESTOVERALLSAVEPATH) {
						common::RequestOverallSavePathMessage * rospm = dynamic_cast<common::RequestOverallSavePathMessage *>(msg);
						if (_isInstalled) {
							if (rospm) {
								QString overallSavePath = _lastBaseDir + "/saves_" + QFileInfo(_iniFile).baseName() + "/" + QString::number(_modID) + ".spsav";
								common::SendOverallSavePathMessage sospm;
								sospm.path = overallSavePath.toStdString();
								socket->writePacket(sospm.SerializeBlank());
							} else {
								socket->writePacket("empty");
							}
						} else {
							QString overallSavePath = getUsedBaseDir() + "/saves_" + QFileInfo(_iniFile).baseName() + "/spineTest.spsav";
							common::SendOverallSavePathMessage sospm;
							sospm.path = overallSavePath.toStdString();
							socket->writePacket(sospm.SerializeBlank());
						}
					} else if (msg->type == common::MessageType::REQUESTOVERALLSAVEDATA) {
						common::RequestOverallSaveDataMessage * rom = dynamic_cast<common::RequestOverallSaveDataMessage *>(msg);
						if (_isInstalled) {
							if (rom && !_username.isEmpty()) {
								if (_onlineMode) {
									rom->modID = _modID;
									rom->username = _username.toStdString();
									rom->password = _password.toStdString();
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = rom->SerializePublic();
										if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
											if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
												serialized = common::Message::DeserializePublic(serialized)->SerializeBlank();
												socket->writePacket(serialized);
											} else {
												socket->writePacket("empty");
											}
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									common::SendOverallSaveDataMessage som;
									Database::DBError dbErr;
									std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Entry, Value FROM overallSaveData WHERE ModID = " + std::to_string(_modID) + " AND Username = '" = _username.toStdString() + "';", dbErr);
									for (auto vec : lastResults) {
										if (vec.size() == 2) {
											som.data.emplace_back(vec[0], vec[1]);
										}
									}
									serialized = som.SerializeBlank();
									socket->writePacket(serialized);
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							common::SendOverallSaveDataMessage som;
							serialized = som.SerializeBlank();
							socket->writePacket(serialized);
						}
					} else if (msg->type == common::MessageType::UPDATEOVERALLSAVEDATA) {
						common::UpdateOverallSaveDataMessage * uom = dynamic_cast<common::UpdateOverallSaveDataMessage *>(msg);
						if (_isInstalled) {
							if (uom) {
								if (_onlineMode) {
									uom->modID = _modID;
									uom->username = _username.toStdString();
									uom->password = _password.toStdString();
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = uom->SerializePublic();
										if (clockUtils::ClockError::SUCCESS != sock.writePacket(serialized)) {
											cacheOverallSaveData(uom);
										} else {
											removeOverallSaveData(uom);
										}
									} else {
										cacheOverallSaveData(uom);
									}
								} else {
									Database::DBError dbErr;
									Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(_modID) + ", '" + _username.toStdString() + "', '" + uom->entry + "', '" + uom->value + "');", dbErr);
									if (dbErr.error) {
										Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "UPDATE overallSaveData SET Value = '" + uom->value + "' WHERE ModID = " + std::to_string(_modID) + " AND Entry = '" + uom->entry + "' AND Username = '" + _username.toStdString() + "';", dbErr);
									}
								}
							}
						}
					} else if (msg->type == common::MessageType::REQUESTALLFRIENDS) {
						common::RequestAllFriendsMessage * rafm = dynamic_cast<common::RequestAllFriendsMessage *>(msg);
						if (_onlineMode) {
							if (rafm && !_username.isEmpty()) {
								rafm->username = _username.toStdString();
								rafm->password = _password.toStdString();
								clockUtils::sockets::TcpSocket sock;
								if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
									serialized = rafm->SerializePublic();
									if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
										if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
											common::SendAllFriendsMessage * safm = dynamic_cast<common::SendAllFriendsMessage *>(common::Message::DeserializePublic(serialized));
											serialized = safm->SerializeBlank();
											socket->writePacket(serialized);
											delete safm;
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									socket->writePacket("empty");
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							common::SendAllFriendsMessage safm;
							serialized = safm.SerializeBlank();
							socket->writePacket(serialized);
						}
					} else if (msg->type == common::MessageType::UPDATECHAPTERSTATS) {
						common::UpdateChapterStatsMessage * ucsm = dynamic_cast<common::UpdateChapterStatsMessage *>(msg);
						if (_isInstalled) {
							if (ucsm) {
								if (_onlineMode) {
									ucsm->modID = _modID;
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = ucsm->SerializePublic();
										sock.writePacket(serialized);
									}
								}
							}
						}
					} else if (msg->type == common::MessageType::ISACHIEVEMENTUNLOCKED) {
						common::IsAchievementUnlockedMessage * iaum = dynamic_cast<common::IsAchievementUnlockedMessage *>(msg);
						if (_isInstalled) {
							if (iaum) {
								iaum->username = q2s(_username);
								iaum->password = q2s(_password);
								if (_onlineMode) {
									clockUtils::sockets::TcpSocket sock;
									if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
										serialized = iaum->SerializePublic();
										if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
											if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
												common::Message * newMsg = common::Message::DeserializePublic(serialized);
												serialized = newMsg->SerializeBlank();
												delete newMsg;
												socket->writePacket(serialized);
											} else {
												socket->writePacket("empty");
											}
										} else {
											socket->writePacket("empty");
										}
									} else {
										socket->writePacket("empty");
									}
								} else {
									Database::DBError dbErr;
									std::vector<std::vector<std::string>> lastResults = Database::queryAll<std::vector<std::string>, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(iaum->modID) + " AND Identifier = " + std::to_string(iaum->achievementID) + "", dbErr);
									common::SendAchievementUnlockedMessage saum;
									saum.unlocked = !lastResults.empty();
									serialized = saum.SerializeBlank();
									socket->writePacket(serialized);
								}
							} else {
								socket->writePacket("empty");
							}
						} else {
							common::SendScoresMessage ssm;
							serialized = ssm.SerializeBlank();
							socket->writePacket(serialized);
						}
					}
				}
				delete msg;
			} catch (boost::archive::archive_exception &) {
				socket->writePacket("empty");
				return;
			}
		}
	}

	void ModInfoView::updateModStats() {
		if (!_onlineMode) {
			return;
		}
		int modID = _modID;
		_ratingWidget->setModID(modID);
		_ratingWidget->setModName(_nameLabel->text());
		std::thread([this, modID]() {
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				{
					common::RequestSingleModStatMessage rsmsm;
					rsmsm.modID = _modID;
					rsmsm.username = _username.toStdString();
					rsmsm.password = _password.toStdString();
					rsmsm.language = _language.toStdString();
					std::string serialized = rsmsm.SerializePublic();
					sock.writePacket(serialized);
					if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
						try {
							common::Message * m = common::Message::DeserializePublic(serialized);
							if (m) {
								common::SendSingleModStatMessage * ssmsm = dynamic_cast<common::SendSingleModStatMessage *>(m);
								emit receivedModStats(ssmsm->mod);
							}
							delete m;
						} catch (...) {
							emit receivedCompatibilityList(modID, {}, {});
							return;
						}
					} else {
						qDebug() << "Error occurred: " << int(cErr);
					}
				}
				{
					common::RequestCompatibilityListMessage rclm;
					rclm.modID = modID;
					std::string serialized = rclm.SerializePublic();
					sock.writePacket(serialized);
					if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
						try {
							common::Message * m = common::Message::DeserializePublic(serialized);
							if (m) {
								common::SendCompatibilityListMessage * sclm = dynamic_cast<common::SendCompatibilityListMessage *>(m);
								emit receivedCompatibilityList(modID, sclm->impossiblePatches, sclm->forbiddenPatches);
							}
							delete m;
						} catch (...) {
							emit receivedCompatibilityList(modID, {}, {});
							return;
						}
					} else {
						qDebug() << "Error occurred: " << int(cErr);
					}
				}
			} else {
				emit receivedCompatibilityList(modID, {}, {});
			}
		}).detach();
	}

	bool ModInfoView::isAllowedSymlinkSuffix(QString suffix) const {
		suffix = suffix.toLower();
		const bool canSymlink = (suffix == "mod" || suffix == "vdf" || suffix == "sty" || suffix == "sgt" || suffix == "dls" || suffix == "bik" || suffix == "dds" || suffix == "jpg" || suffix == "png" || suffix == "mi" || suffix == "hlsl" || suffix == "h" || suffix == "vi" || suffix == "exe" || suffix == "dll" || suffix == "bin" || suffix == "mtl" || suffix == "obj" || suffix == "txt" || suffix == "rtf" || suffix == "obj" || suffix == "ico" || suffix == "ini" || suffix == "bak" || suffix == "gsp" || suffix == "pdb" || suffix == "config" || suffix == "fx" || suffix == "3ds" || suffix == "mcache" || suffix == "fxh");
		if (!canSymlink) {
			LOGINFO("Copying extension: " << suffix.toStdString());
		}
		return canSymlink;
	}

	bool ModInfoView::prepareModStart(QString * usedBaseDir, QString * usedExecutable, QStringList * backgroundExecutables, bool * newGMP) {
		_gmpCounterBackup = -1;

		LOGINFO("Starting Ini: " << _iniFile.toStdString());
		emitSplashMessage(QApplication::tr("RemovingOldFiles"));
		for (const QString & f : _copiedFiles) {
			QFile(_lastBaseDir + "/" + f).remove();
			Database::DBError err;
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + f.toStdString() + "';", err);
		}
		_copiedFiles.clear();

		Database::DBError err;
		const std::vector<std::string> patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + ";", err);
		bool clockworkRenderer = false;
		bool systempack = false;
		for (const std::string & patchID : patches) {
			const std::string patchName = Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchID + " LIMIT 1;", err);
			if (patchName == "D3D11 Renderer Clockwork Edition") {
				clockworkRenderer = true;
			} else if (patchName.find("Systempack") != std::string::npos) {
				systempack = true;
			}
		}
		int normalsCounter = -1;
		if (_isInstalled) {
			emitSplashMessage(QApplication::tr("DetermingCorrectGothicPath"));
			if (GeneralSettingsWidget::extendedLogging) {
				LOGINFO("Installed Mod");
			}
			const common::GothicVersion mid = common::GothicVersion(Database::queryNth<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err, 0).front());
			bool success = true;
			if (mid == common::GothicVersion::GOTHIC && !_gothicDirectory.isEmpty()) {
				*usedBaseDir = _gothicDirectory;
				*usedExecutable = "GothicMod.exe";
			} else if (mid == common::GothicVersion::GOTHIC2 && !_gothic2Directory.isEmpty()) {
				*usedBaseDir = _gothic2Directory;
				*usedExecutable = "Gothic2.exe";
			} else if (mid == common::GothicVersion::GOTHICINGOTHIC2 && !_gothicDirectory.isEmpty() && !_gothic2Directory.isEmpty()) {
				*usedBaseDir = _gothic2Directory;
				*usedExecutable = "Gothic2.exe";

				// in this case, prepare Gothic 2 directory with Gothic 1 files
				static QStringList gothicFiles = getGothicFiles();

				for (QString file : gothicFiles) {
					QString sourceFileName = file;
					QString targetFileName = file;
					if (file.endsWith(".vdf", Qt::CaseSensitivity::CaseInsensitive)) {
						targetFileName = file.insert(file.length() - 4, "_G1");
					}
					QFile f(_gothicDirectory + "/" + sourceFileName);
					QFileInfo fi(*usedBaseDir + "/" + targetFileName);
					QDir dir = fi.absoluteDir();
					success = dir.mkpath(dir.absolutePath());
					if (!success) {
						LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString());
						break;
					}
					// backup old file
					if (QFile::exists(*usedBaseDir + "/" + targetFileName)) {
						if (QFile::exists(*usedBaseDir + "/" + targetFileName + ".spbak")) {
							QFile::remove(*usedBaseDir + "/" + targetFileName);
						} else {
							QFile::rename(*usedBaseDir + "/" + targetFileName, *usedBaseDir + "/" + targetFileName + ".spbak");
						}
					}
					const QString suffix = QFileInfo(file).suffix();
#ifdef Q_OS_WIN
					if (IsRunAsAdmin() && isAllowedSymlinkSuffix(suffix)) {
						success = makeSymlink(_gothicDirectory + "/" + sourceFileName, *usedBaseDir + "/" + targetFileName);
					} else {
						success = f.copy(*usedBaseDir + "/" + targetFileName);
					}
#else
					success = f.copy(*usedBaseDir + "/" + targetFileName);
#endif
					if (!success) {
						LOGERROR("Couldn't copy file: " << sourceFileName.toStdString() << " " << f.errorString().toStdString());
						removeGothicFiles();
						break;
					}
					_copiedFiles.insert(file);
				}
			}

			emitSplashMessage(QApplication::tr("RemovingBackups"));
			_lastBaseDir = *usedBaseDir;
			// everything that's left here can be removed I guess
			QDirIterator itBackup(*usedBaseDir, QStringList() << "*.spbak", QDir::Files, QDirIterator::Subdirectories);
			while (itBackup.hasNext()) {
				itBackup.next();
				QFile(itBackup.filePath()).remove();
			}
			LOGINFO("Starting " << usedExecutable->toStdString() << " in " << usedBaseDir->toStdString());
			if (!usedExecutable->isEmpty() && success) {
				emitSplashMessage(QApplication::tr("CopyingModfiles"));
				std::vector<std::pair<std::string, std::string>> files = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File, Hash FROM modfiles WHERE ModID = " + std::to_string(_modID) + ";", err);
				if (err.error || files.empty()) {
					return false;
				}
				for (const std::pair<std::string, std::string> & file : files) {
					// SP-55 skipping .mod files, the necessary ones will be copied to Data later, so only one copy is necessary
					QString filename = QString::fromStdString(file.first);
					if (filename.contains("Data/modvdf/")) {
						continue;
					}
					if (GeneralSettingsWidget::extendedLogging) {
						LOGINFO("Copying file " << file.first);
					}
#ifdef Q_OS_WIN
					if (IsRunAsAdmin()) {
						if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements")) {
							bool b = QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements");
							Q_UNUSED(b);
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
							const QString targetName = clockworkRenderer ? *usedBaseDir + "/" + checkPath + QString::number(normalsCounter) : "/System/GD3D11/textures/replacements/" + match.captured(1);
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(_modID) + "/System/GD3D11/textures/replacements/" + match.captured(1), targetName);
							_copiedFiles.insert("/System/GD3D11/textures/replacements/" + match.captured(1));
							continue;
						}
					} else {
#endif
						if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements")) {
							bool b = QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements");
							Q_UNUSED(b);
						}
						if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_Original")) {
							bool b = QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_Original");
							Q_UNUSED(b);
						}
						// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
						const QString checkPath = "System/GD3D11/textures/replacements/Normalmaps_";
						if (filename.contains(checkPath, Qt::CaseInsensitive)) {
							if (normalsCounter == -1) {
								normalsCounter++;
								if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter))) {
									QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter));
								}
							}
							QRegularExpression regex("System/GD3D11/textures/replacements/(Normalmaps_[^/]+)/", QRegularExpression::CaseInsensitiveOption);
							QRegularExpressionMatch match = regex.match(filename);
							// backup old file
							bool copy = true;
							QFile f(Config::MODDIR + "/mods/" + QString::number(_modID) + "/" + filename);
							if (QFile::exists(*usedBaseDir + "/" + filename)) {
								QFile checkFile(*usedBaseDir + "/" + filename);
								if (checkFile.open(QIODevice::ReadOnly)) {
									QCryptographicHash hash(QCryptographicHash::Sha512);
									hash.addData(&checkFile);
									const QString hashSum = QString::fromLatin1(hash.result().toHex());
									if (hashSum == QString::fromStdString(file.second)) {
										copy = false;
										if (GeneralSettingsWidget::extendedLogging) {
											LOGINFO("Skipping file");
										}
									}
								}
								checkFile.close();
								if (copy) {
									if (QFile::exists(*usedBaseDir + "/" + filename + ".spbak")) {
										QFile::remove(*usedBaseDir + "/" + filename);
									} else {
										QFile::rename(*usedBaseDir + "/" + filename, *usedBaseDir + "/" + filename + ".spbak");
									}
								}
							}
							QString suffix = QFileInfo(filename).suffix();
							QString changedFile = filename;
							if (clockworkRenderer) {
								changedFile = changedFile.replace(match.captured(1), "Normalmap_" + QString::number(normalsCounter), Qt::CaseInsensitive);
							}
							if (copy) {
								success = f.copy(*usedBaseDir + "/" + changedFile);
							}
							if (!success) {
								LOGERROR("Couldn't copy file: " << filename.toStdString() << " " << f.errorString().toStdString());
								break;
							}
							if (copy) {
								_copiedFiles.insert(changedFile);
							}
							continue;
						}
#ifdef Q_OS_WIN
					}
#endif
					QFile f(Config::MODDIR + "/mods/" + QString::number(_modID) + "/" + filename);
					QFileInfo fi(*usedBaseDir + "/" + filename);
					QDir dir = fi.absoluteDir();
					success = dir.mkpath(dir.absolutePath());
					if (!success) {
						LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString());
						break;
					}
					// backup old file
					bool copy = true;
					if (QFile::exists(*usedBaseDir + "/" + filename)) {
						QFile checkFile(*usedBaseDir + "/" + filename);
						if (checkFile.open(QIODevice::ReadOnly)) {
							QCryptographicHash hash(QCryptographicHash::Sha512);
							hash.addData(&checkFile);
							const QString hashSum = QString::fromLatin1(hash.result().toHex());
							if (hashSum == QString::fromStdString(file.second)) {
								copy = false;
								if (GeneralSettingsWidget::extendedLogging) {
									LOGINFO("Skipping file");
								}
							}
						}
						checkFile.close();
						if (copy) {
							if (QFile::exists(*usedBaseDir + "/" + filename + ".spbak")) {
								QFile::remove(*usedBaseDir + "/" + filename);
							} else {
								QFile::rename(*usedBaseDir + "/" + filename, *usedBaseDir + "/" + filename + ".spbak");
							}
						}
					}
					const QString suffix = QFileInfo(filename).suffix();
					if (copy) {
#ifdef Q_OS_WIN
						if (IsRunAsAdmin() && isAllowedSymlinkSuffix(suffix)) {
							success = makeSymlink(Config::MODDIR + "/mods/" + QString::number(_modID) + "/" + filename, *usedBaseDir + "/" + filename);
						} else {
							success = f.copy(*usedBaseDir + "/" + filename);
						}
#else
						success = f.copy(*usedBaseDir + "/" + filename);
#endif
					}
					if (!success) {
						LOGERROR("Couldn't copy file: " << filename.toStdString() << " " << f.errorString().toStdString());
						break;
					}
					if (copy) {
						_copiedFiles.insert(filename);
					}
				}
				if (!success) {
					removeModFiles();
					LOGERROR("Failed copying mod data");
					return false;
				}
				checkToolCfg(Config::MODDIR + "/mods/" + QString::number(_modID), usedBaseDir, backgroundExecutables, newGMP);
			}
		} else {
			emitSplashMessage(QApplication::tr("DetermingCorrectGothicPath"));
			if (!_gothicDirectory.isEmpty() && _iniFile.contains(_gothicDirectory + "/")) {
				*usedBaseDir = _gothicDirectory;
				*usedExecutable = "GothicMod.exe";
			} else if (!_gothic2Directory.isEmpty() && _iniFile.contains(_gothic2Directory + "/")) {
				*usedBaseDir = _gothic2Directory;
				*usedExecutable = "Gothic2.exe";
			}
			_lastBaseDir = *usedBaseDir;
			LOGERROR("Starting " << usedExecutable->toStdString() << " in " << usedBaseDir->toStdString());
		}
		if (systempack) {
			if (QFile::exists(*usedBaseDir + "/System/vdfs32g.exe")) {
				QFile::rename(*usedBaseDir + "/System/vdfs32g.exe", *usedBaseDir + "/System/vdfs32g.exe.spbak");
				_copiedFiles.insert("System/vdfs32g.exe");
			}
		}
		emitSplashMessage(QApplication::tr("CopyingPatchfiles"));
		// everything worked fine so far, so add patches!
		if (!err.error && !patches.empty()) {
			for (const std::string & patchIDString : patches) {
				const int patchID = std::stoi(patchIDString);
				std::vector<std::pair<std::string, std::string>> patchFiles = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File, Hash FROM modfiles WHERE ModID = " + std::to_string(patchID) + ";", err);
				if (err.error || patchFiles.empty()) {
					continue;
				}
				if (GeneralSettingsWidget::extendedLogging) {
					LOGINFO("Applying patch " << patchID);
				}
				bool raisedNormalCounter = false;
				QSet<QString> skippedBases;
				for (const std::pair<std::string, std::string> & file : patchFiles) {
					QString filename = QString::fromStdString(file.first);
#ifdef Q_OS_WIN
					if (IsRunAsAdmin()) {
						if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements")) {
							bool b = QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements");
							Q_UNUSED(b);
						}
						// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
						QString checkPath = "System/GD3D11/Data";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.insert(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/Meshes";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.insert(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/shaders";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.insert(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/textures/infos";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.insert(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/textures/RainDrops";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.insert(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/textures/replacements/Normalmaps_Original";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.insert(checkPath);
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
							const QString targetName = clockworkRenderer ? *usedBaseDir + "/" + checkPath + QString::number(normalsCounter) : "/System/GD3D11/textures/replacements/" + match.captured(1);
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/System/GD3D11/textures/replacements/" + match.captured(1), targetName);
							_copiedFiles.insert("/System/GD3D11/textures/replacements/" + match.captured(1));
							continue;
						}
					} else {
#endif
						if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements")) {
							bool b = QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements");
							Q_UNUSED(b);
						}
						if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_Original")) {
							bool b = QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_Original");
							Q_UNUSED(b);
						}
						// D3D11 special treatment... don't set symlink for every file, but set symlink to base dir
						const QString checkPath = "System/GD3D11/textures/replacements/Normalmaps_";
						if (filename.contains(checkPath, Qt::CaseInsensitive)) {
							if (!raisedNormalCounter) {
								raisedNormalCounter = true;
								normalsCounter++;
								if (!QDir().exists(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter))) {
									QDir().mkpath(*usedBaseDir + "/System/GD3D11/textures/replacements/Normalmaps_" + QString::number(normalsCounter));
								}
							}
							QRegularExpression regex("System/GD3D11/textures/replacements/(Normalmaps_[^/]+)/", QRegularExpression::CaseInsensitiveOption);
							QRegularExpressionMatch match = regex.match(filename);
							// backup old file
							bool copy = true;
							QFile f(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + filename);
							QString changedFile = filename;
							if (clockworkRenderer && match.captured(1).compare("Normalmaps_Original", Qt::CaseInsensitive) != 0) {
								changedFile = changedFile.replace(match.captured(1), "Normalmaps_" + QString::number(normalsCounter), Qt::CaseInsensitive);
							}
							Q_ASSERT(QFileInfo::exists(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + filename));
							if (QFile::exists(*usedBaseDir + "/" + changedFile)) {
								QFile checkFile(*usedBaseDir + "/" + changedFile);
								if (checkFile.open(QIODevice::ReadOnly)) {
									QCryptographicHash hash(QCryptographicHash::Sha512);
									hash.addData(&checkFile);
									const QString hashSum = QString::fromLatin1(hash.result().toHex());
									if (hashSum == QString::fromStdString(file.second)) {
										copy = false;
										if (GeneralSettingsWidget::extendedLogging) {
											LOGINFO("Skipping file");
										}
									}
								}
								checkFile.close();
								if (copy) {
									if (QFile::exists(*usedBaseDir + "/" + changedFile + ".spbak")) {
										QFile::remove(*usedBaseDir + "/" + changedFile);
									} else {
										QFile::rename(*usedBaseDir + "/" + changedFile, *usedBaseDir + "/" + changedFile + ".spbak");
									}
								}
							}
							if (copy) {
								const bool b = f.copy(*usedBaseDir + "/" + changedFile);
								_copiedFiles.insert(changedFile);
								Q_ASSERT(b);
							}
							continue;
						}
#ifdef Q_OS_WIN
					}
#endif
					QFile f(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + filename);
					QFileInfo fi(*usedBaseDir + "/" + filename);
					QDir dir = fi.absoluteDir();
					bool success = dir.mkpath(dir.absolutePath());
					if (!success) {
						LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString());
						break;
					}
					// backup old file, if already backed up, don't patch
					if (_copiedFiles.find(filename) == _copiedFiles.end() && ((QFile::exists(*usedBaseDir + "/" + filename) && !QFile::exists(*usedBaseDir + "/" + filename + ".spbak")) || !QFile::exists(*usedBaseDir + "/" + filename))) {
						if (GeneralSettingsWidget::extendedLogging) {
							LOGINFO("Copying file " << file.first);
						}
						bool copy = true;
						if (QFile::exists(*usedBaseDir + "/" + filename)) {
							QFile checkFile(*usedBaseDir + "/" + filename);
							if (checkFile.open(QIODevice::ReadOnly)) {
								QCryptographicHash hash(QCryptographicHash::Sha512);
								hash.addData(&checkFile);
								const QString hashSum = QString::fromLatin1(hash.result().toHex());
								if (hashSum == QString::fromStdString(file.second)) {
									copy = false;
								}
							}
						}
						if (copy) {
							QFile::rename(*usedBaseDir + "/" + filename, *usedBaseDir + "/" + filename + ".spbak");
							const QString suffix = QFileInfo(filename).suffix();
#ifdef Q_OS_WIN
							if (IsRunAsAdmin() && isAllowedSymlinkSuffix(suffix)) {
								success = makeSymlink(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + filename, *usedBaseDir + "/" + filename);
							} else {
								success = f.copy(*usedBaseDir + "/" + filename);
							}
#else
							success = f.copy(*usedBaseDir + "/" + filename);
#endif
							if (!success) {
								std::vector<std::string> name = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", err);
								Q_ASSERT(!name.empty());
								emit errorMessage(QApplication::tr("PatchIncomplete").arg(s2q(name[0])));
								return false;
							}
						}
						_copiedFiles.insert(filename);
					}
				}
				checkToolCfg(Config::MODDIR + "/mods/" + QString::number(patchID), usedBaseDir, backgroundExecutables, newGMP);
			}
		}
		if (usedExecutable->isEmpty()) {
			removeModFiles();
			LOGERROR("No executable found");
			return false;
		}
		emitSplashMessage(QApplication::tr("RemovingOldFiles"));
		// this shouldn't be necessary here! Test whether it can be removed
		QDirIterator it(*usedBaseDir + "/Data", QStringList() << "*.mod", QDir::Files);
		QStringList files;
		while (it.hasNext()) {
			it.next();
			QString fileName = it.filePath();
			const QString partialName = fileName.replace(*usedBaseDir + "/", "");
			if (!fileName.isEmpty() && _copiedFiles.find(partialName) == _copiedFiles.end()) {
				QFile f(fileName);
				f.remove();
			}
		}
		emitSplashMessage(QApplication::tr("CopyingModfiles"));
		QSettings iniParser(_iniFile, QSettings::IniFormat);
		{
			QString modFiles = iniParser.value("FILES/VDF", "").toString();
			QString executable = iniParser.value("FILES/Executable", "").toString();
			if (!executable.isEmpty()) {
				*usedExecutable = executable;
			}
			QString sourceDir = *usedBaseDir;
			if (_isInstalled) {
				QFileInfo fi(_iniFile);
				QDir sd = fi.absolutePath() + "/..";
				sourceDir = sd.absolutePath();
			}
			QStringList l = modFiles.split(" ", QString::SplitBehavior::SkipEmptyParts);
			for (const QString & s : l) {
				const QString suffix = QFileInfo(s).suffix();
#ifdef Q_OS_WIN
				if (IsRunAsAdmin() && isAllowedSymlinkSuffix(suffix)) {
					makeSymlink(sourceDir + "/Data/modvdf/" + s, *usedBaseDir + "/Data/" + s);
				} else {
					QFile copy(sourceDir + "/Data/modvdf/" + s);
					copy.copy(*usedBaseDir + "/Data/" + s);
				}
#else
				QFile copy(sourceDir + "/Data/modvdf/" + s);
				copy.copy(*usedBaseDir + "/Data/" + s);
#endif
				_copiedFiles.insert("Data/" + s);
			}
			// check overrides and backup original values!
			_gothicIniBackup.clear();
			_systempackIniBackup.clear();
			emitSplashMessage(QApplication::tr("OverridingIni"));
			QFile(*usedBaseDir + "/System/Gothic.ini").copy(*usedBaseDir + "/System/Gothic.ini.spbak");
			{
				QSettings gothicIniParser(*usedBaseDir + "/System/Gothic.ini", QSettings::IniFormat);
				iniParser.beginGroup("OVERRIDES");
				QStringList entries = iniParser.allKeys();
				for (const QString & s : entries) {
					const QString & entry = s;
					QString section = entry.split(".").front();
					QString key = entry.split(".").back();
					// backup original value
					QString val = gothicIniParser.value(section + "/" + key, "---").toString();
					_gothicIniBackup.emplace_back(section, key, val);
					val = iniParser.value(entry, "---").toString();
					if (val != "---") {
						gothicIniParser.setValue(section + "/" + key, val);
					}
				}
				iniParser.endGroup();
			}
			{
				QSettings systempackIniParser(*usedBaseDir + "/System/Systempack.ini", QSettings::IniFormat);
				iniParser.beginGroup("OVERRIDES_SP");
				QStringList entries = iniParser.allKeys();
				for (const QString & s : entries) {
					const QString & entry = s;
					QString section = entry.split(".").front();
					QString key = entry.split(".").back();
					// backup original value
					QString val = systempackIniParser.value(section + "/" + key, "---").toString();
					_systempackIniBackup.emplace_back(section, key, val);
					val = iniParser.value(entry, "---").toString();
					if (val != "---") {
						systempackIniParser.setValue(section + "/" + key, val);
					}
				}
				iniParser.endGroup();
			}
		}
		// if mod is installed or developer mode is active, copy SpineAPI.dll to modification dir
		if (_isInstalled || _developerModeActive) {
			QFile(*usedBaseDir + "/System/SpineAPI.dll").remove();
			QFile(*usedBaseDir + "/System/m2etis.dll").remove();
			QFile(*usedBaseDir + "/Data/Spine.vdf").remove();
			{
				QFile spineAPI(qApp->applicationDirPath() + "/SpineAPI.dll");
				spineAPI.copy(*usedBaseDir + "/System/SpineAPI.dll");
			}
			{
				QFile spineAPI(qApp->applicationDirPath() + "/m2etis.dll");
				spineAPI.copy(*usedBaseDir + "/System/m2etis.dll");
			}
			{
				QFile spineAPI(qApp->applicationDirPath() + "/../media/Spine.vdf");
				spineAPI.copy(*usedBaseDir + "/Data/Spine.vdf");
			}
			_listenSocket = new clockUtils::sockets::TcpSocket();
			_listenSocket->listen(LOCAL_PORT, 1, true, std::bind(&ModInfoView::acceptedConnection, this, std::placeholders::_1, std::placeholders::_2));
		}
		Database::open(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "BEGIN TRANSACTION;", err);
		for (const QString & file : _copiedFiles) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO usedFiles (File) VALUES ('" + file.toStdString() + "');", err);
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, err);
		std::vector<std::string> res = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM lastUsedBaseDir LIMIT 1;", err);
		if (res.empty()) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO lastUsedBaseDir (Path) VALUES ('" + _lastBaseDir.toStdString() + "');", err);
		} else {
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE lastUsedBaseDir SET Path = '" + _lastBaseDir.toStdString() + "';", err);
		}
		{
			QString startName = QFileInfo(_iniFile).fileName();
			startName = startName.split(".").front();
			const QString saveDir = *usedBaseDir + "/saves_" + startName;
			QDir saveDirectory(saveDir);
			if (!saveDirectory.exists()) {
				bool b = saveDirectory.mkpath(saveDirectory.absolutePath());
				Q_UNUSED(b);
			}
		}
		{
			// remove all m3d files from System dir
			QDirIterator dirIt(*usedBaseDir + "/System", QStringList() << "*.m3d", QDir::Files);
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
			if (_modID == 218) {
				const QString skoOptionsPath = QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA") + "/SKOLauncher/";
				const QString skoOptions = skoOptionsPath + "Options.json";
				QDir skoOptionsDir(skoOptionsPath);
				if (!skoOptionsDir.exists()) {
					bool b = skoOptionsDir.mkpath(skoOptionsDir.absolutePath());
					Q_UNUSED(b);
				}
				if (!QFileInfo::exists(skoOptions)) {
					QFile skoOptionsFile(skoOptions);
					if (skoOptionsFile.open(QIODevice::WriteOnly)) {
						QTextStream skoOptionsStream(&skoOptionsFile);
						skoOptionsStream << "{\n";
						skoOptionsStream << "\t\"Nickname\": \"" << _username << "\",\n";
						skoOptionsStream << "\t\"Path\": \"" << *usedBaseDir << "/System\"\n";
						skoOptionsStream << "}\n";
					}
				}
			}
		}
		return true;
	}

	void ModInfoView::checkToolCfg(QString path, QString * usedBaseDir, QStringList * backgroundExecutables, bool * newGMP) {
		if (QFileInfo::exists(path + "/tool.cfg")) {
			QSettings configParser(path + "/tool.cfg", QSettings::IniFormat);
			QString executable = configParser.value("CONFIG/BackgroundProcess", "").toString();
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
					registrySettings.setValue("server_nick", _username);
					registrySettings.setValue("server_port", configParser.value("GMP/Port").toInt());
					registrySettings.setValue("server_url", configParser.value("GMP/IP").toString());
				}
				{
					QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\Public domain\GMP Launcher\Gothic)", QSettings::NativeFormat);
					registrySettings.setValue("working_directory", *usedBaseDir);
				}
				*newGMP = true;
#endif
			} else if (configParser.contains("G2O/IP")) {
#ifdef Q_OS_WIN
				const QString ip = configParser.value("G2O/IP", "127.0.0.1").toString();
				const int port = configParser.value("G2O/Port", 12345).toInt();
				const QString favoriteString = QString("<servers><server><ip>123.456.789.0</ip><port>12345</port></server></servers>").arg(ip).arg(port);
				QFile outFile(*usedBaseDir + "/../favorite.xml");
				outFile.open(QIODevice::WriteOnly);
				QTextStream ts(&outFile);
				ts << favoriteString;

				{
					QSettings registrySettings(R"(HKEY_CURRENT_USER\Software\G2O)", QSettings::NativeFormat);
					if (!registrySettings.contains("nickname")) {
						registrySettings.setValue("nickname", _username);
					}
				}
#endif
			} else {
				QString ini = configParser.value("CONFIG/WriteIni", "").toString();
				const QString clearIni = configParser.value("CONFIG/ClearIni", "").toString();
				if (!ini.isEmpty()) {
					QSettings newIni(*usedBaseDir + "/" + ini, QSettings::IniFormat);
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
							e = e.replace("@USERNAME@", _username);
							newIni.setValue(list[0] + "/" + list[1], e);
						}
					}
					configParser.endGroup();
				}
				QString file = configParser.value("CONFIG/WriteFile", "").toString();
				const QString clearFile = configParser.value("CONFIG/ClearFile", "").toString();
				if (!file.isEmpty()) {
					QFile f(*usedBaseDir + "/" + file);
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
						e = e.replace("@USERNAME@", _username);
						f.write(e.toStdString().c_str(), e.length());
						f.write("\n");
					}
					f.close();
					configParser.endGroup();
				}
			}
		}
	}

	void ModInfoView::tryCleanCaches() {
		if (!_onlineMode) {
			return;
		}
		Database::DBError err;
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
			std::vector<std::vector<int>> scores = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM scoreCache;", err);
			for (auto t : scores) {
				common::UpdateScoreMessage usm;
				usm.modID = t[0];
				usm.identifier = t[1];
				usm.score = t[2];
				usm.username = _username.toStdString();
				usm.password = _password.toStdString();
				const std::string serialized = usm.SerializePublic();
				if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
					removeScore(&usm);
				}
			}
			std::vector<std::vector<int>> achievements = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM achievementCache;", err);
			for (auto t : achievements) {
				common::UnlockAchievementMessage uam;
				uam.modID = t[0];
				uam.identifier = t[1];
				uam.username = _username.toStdString();
				uam.password = _password.toStdString();
				const std::string serialized = uam.SerializePublic();
				if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
					removeAchievement(&uam);
				}
			}
			std::vector<std::vector<int>> achievementProgresses = Database::queryAll<std::vector<int>, int, int, int>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM achievementProgressCache;", err);
			for (auto t : achievementProgresses) {
				common::UpdateAchievementProgressMessage uapm;
				uapm.modID = t[0];
				uapm.identifier = t[1];
				uapm.progress = t[2];
				uapm.username = _username.toStdString();
				uapm.password = _password.toStdString();
				const std::string serialized = uapm.SerializePublic();
				if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
					removeAchievementProgress(&uapm);
				}
			}
			std::vector<std::vector<std::string>> overallSaveDatas = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "SELECT * FROM overallSaveDataCache;", err);
			for (auto t : overallSaveDatas) {
				common::UpdateOverallSaveDataMessage uom;
				uom.modID = std::stoi(t[0]);
				uom.entry = t[1];
				uom.value = t[2];
				uom.username = _username.toStdString();
				uom.password = _password.toStdString();
				const std::string serialized = uom.SerializePublic();
				if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
					removeOverallSaveData(&uom);
				}
			}
		}
	}

	void ModInfoView::cacheScore(common::UpdateScoreMessage * usm) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO scoreCache (ModID, Identifier, Score) VALUES (" + std::to_string(usm->modID) + ", " + std::to_string(usm->identifier) + ", " + std::to_string(usm->score) + ");", err);
		if (err.error) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE scoreCache SET Score = " + std::to_string(usm->score) + " WHERE ModID = " + std::to_string(usm->modID) + " AND Identifier = " + std::to_string(usm->identifier) + ";", err);
		}
	}

	void ModInfoView::removeScore(common::UpdateScoreMessage * usm) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM scoreCache WHERE ModID = " + std::to_string(usm->modID) + " AND Identifier = " + std::to_string(usm->identifier) + ";", err);
	}

	void ModInfoView::cacheAchievement(common::UnlockAchievementMessage * uam) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO achievementCache (ModID, Identifier) VALUES (" + std::to_string(uam->modID) + ", " + std::to_string(uam->identifier) + ");", err);
	}

	void ModInfoView::removeAchievement(common::UnlockAchievementMessage * uam) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM achievementCache WHERE ModID = " + std::to_string(uam->modID) + " AND Identifier = " + std::to_string(uam->identifier) + ";", err);
	}

	void ModInfoView::cacheAchievementProgress(common::UpdateAchievementProgressMessage * uapm) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO achievementProgressCache (ModID, Identifier, Progress) VALUES (" + std::to_string(uapm->modID) + ", " + std::to_string(uapm->identifier) + ", " + std::to_string(uapm->progress) + ");", err);
		if (err.error) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE achievementProgressCache SET Progress = " + std::to_string(uapm->progress) + " WHERE ModID = " + std::to_string(uapm->modID) + " AND Identifier = " + std::to_string(uapm->identifier) + ";", err);
		}
	}

	void ModInfoView::removeAchievementProgress(common::UpdateAchievementProgressMessage * uapm) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM achievementProgressCache WHERE ModID = " + std::to_string(uapm->modID) + " AND Identifier = " + std::to_string(uapm->identifier) + ";", err);
	}

	void ModInfoView::cacheOverallSaveData(common::UpdateOverallSaveDataMessage * uom) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "INSERT INTO overallSaveDataCache (ModID, Entry, Value) VALUES (" + std::to_string(uom->modID) + ", '" + uom->entry + "', '" + uom->value + "');", err);
		if (err.error) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "UPDATE overallSaveDataCache SET Value = '" + uom->value + "' WHERE ModID = " + std::to_string(uom->modID) + " AND Entry = '" + uom->entry + "';", err);
		}
	}

	void ModInfoView::removeOverallSaveData(common::UpdateOverallSaveDataMessage * uom) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM overallSaveDataCache WHERE ModID = " + std::to_string(uom->modID) + " AND Entry = '" + uom->entry + "';", err);
	}

	void ModInfoView::emitSplashMessage(QString message) {
		emit changeSplashMessage(message, Qt::AlignLeft, _splashTextColor);
	}

	void ModInfoView::restoreSettings() {
		bool b = _iniParser->value("DEVELOPER/CompileScripts", false).toBool();
		_compileScripts->setChecked(b);
		b = _iniParser->value("DEVELOPER/WindowedMode", false).toBool();
		_startupWindowed->setChecked(b);
		const int i = _iniParser->value("DEVELOPER/zSpyLevel", 10).toInt();
		_zSpyLevel->setValue(i);
		b = _iniParser->value("DEVELOPER/ConvertTextures", false).toBool();
		_convertTextures->setChecked(b);
		b = _iniParser->value("DEVELOPER/NoSound", false).toBool();
		_noSound->setChecked(b);
		b = _iniParser->value("DEVELOPER/NoMusic", false).toBool();
		_noMusic->setChecked(b);
	}

	void ModInfoView::saveSettings() {
		_iniParser->setValue("DEVELOPER/CompileScripts", _compileScripts->isChecked());
		_iniParser->setValue("DEVELOPER/WindowedMode", _startupWindowed->isChecked());
		_iniParser->setValue("DEVELOPER/zSpyLevel", _zSpyLevel->value());
		_iniParser->setValue("DEVELOPER/ConvertTextures", _convertTextures->isChecked());
		_iniParser->setValue("DEVELOPER/NoSound", _noSound->isChecked());
		_iniParser->setValue("DEVELOPER/NoMusic", _noMusic->isChecked());
	}

	void ModInfoView::removeEmptyDirs(QString baseDir) {
		bool deleted;
		do {
			deleted = false;
			QDirIterator it(_lastBaseDir, QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoSymLinks, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				const QString path = it.filePath();
				QDir dir(path);
				if (dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot).isEmpty() && !dir.absolutePath().contains("saves_", Qt::CaseInsensitive)) {
					deleted |= dir.rmdir(path);
				}
			}
		} while (deleted);
	}

	void ModInfoView::synchronizeOfflineData() {
		// this code is incredible slow due to around 2.500 SQL inserts
		std::thread([this]() {
			try {
				if (_onlineMode) {
					// update server from local data in case Sync flag is set
					Database::DBError err;
					const int sync = Database::queryNth<int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Enabled FROM sync LIMIT 1;", err);
					if (sync) {
						{
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM sync;", err);
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO sync (Enabled) VALUES (0);", err);
								common::UpdateOfflineDataMessage uodm;
								std::vector<std::vector<std::string>> achievements = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier FROM modAchievements WHERE Username = '" + _username.toStdString() + "';", err);
								for (auto vec : achievements) {
									common::UpdateOfflineDataMessage::AchievementData ad {};
									ad.modID = std::stoi(vec[0]);
									ad.identifier = std::stoi(vec[1]);
									std::vector<int> progress = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE Username = '" + _username.toStdString() + "' AND ModID = " + vec[0] + " AND Identifier = " + vec[1] + " LIMIT 1;", err);
									if (!progress.empty()) {
										ad.current = progress[0];
									}
									uodm.achievements.push_back(ad);
								}
								std::vector<std::vector<std::string>> scores = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier, Score FROM modScores WHERE Username = '" + _username.toStdString() + "';", err);
								for (auto vec : scores) {
									common::UpdateOfflineDataMessage::ScoreData sd {};
									sd.modID = std::stoi(vec[0]);
									sd.identifier = std::stoi(vec[1]);
									sd.score = std::stoi(vec[2]);
									uodm.scores.push_back(sd);
								}
								std::vector<std::vector<std::string>> overallSaveData = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Entry, Value FROM overallSaveData WHERE Username = '" + _username.toStdString() + "';", err);
								for (auto vec : overallSaveData) {
									common::UpdateOfflineDataMessage::OverallSaveData od;
									od.modID = std::stoi(vec[0]);
									od.entry = vec[1];
									od.value = vec[2];
									uodm.overallSaves.push_back(od);
								}
								std::vector<std::vector<int>> playTimes = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Duration FROM playTimes WHERE Username = '" + _username.toStdString() + "';", err);
								for (auto vec : playTimes) {
									uodm.playTimes.emplace_back(vec[0], vec[1]);
								}
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes WHERE Username = '" + _username.toStdString() + "';", err);
								const std::string serialized = uodm.SerializePublic();
								sock.writePacket(serialized);
							}
						}
						{
							// Load data from server
							clockUtils::sockets::TcpSocket sock;
							if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
								common::RequestOfflineDataMessage rodm;
								rodm.username = _username.toStdString();
								rodm.password = _password.toStdString();
								std::string serialized = rodm.SerializePublic();
								sock.writePacket(serialized);
								if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
									common::Message * msg = common::Message::DeserializePublic(serialized);
									if (msg) {
										common::SendOfflineDataMessage * sodm = dynamic_cast<common::SendOfflineDataMessage *>(msg);
										if (sodm) {
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes WHERE Username = '" + _username.toStdString() + "';", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementList;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementProgress;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievementProgressMax;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modAchievements;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM modScores;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM overallSaveData;", err);

											Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA synchronous = OFF;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA journal_mode = MEMORY;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "PRAGMA cache_size=10000;", err);
											Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
											for (auto ad : sodm->achievements) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementList (ModID, Identifier) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ");", err);

												if (ad.current > 0) {
													Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgress (ModID, Identifier, Username, Current) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ", '" + ad.username + "', " + std::to_string(ad.current) + ");", err);
												}
												if (ad.max > 0) {
													Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievementProgressMax (ModID, Identifier, Max) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ", " + std::to_string(ad.max) + ");", err);
												}
												if (ad.unlocked) {
													Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modAchievements (ModID, Identifier, Username) VALUES (" + std::to_string(ad.modID) + ", " + std::to_string(ad.identifier) + ", '" + ad.username + "');", err);
												}
											}
											for (auto sd : sodm->scores) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(sd.modID) + ", " + std::to_string(sd.identifier) + ", '" + sd.username + "', " + std::to_string(sd.score) + ");", err);
											}
											for (auto od : sodm->overallSaves) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO overallSaveData (ModID, Username, Entry, Value) VALUES (" + std::to_string(od.modID) + ", '" + od.username + "', '" + od.entry + "', '" + od.value + "');", err);
											}
											Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
											Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
										}
									}
									delete msg;
								}
							}
						}
					}
				}
			} catch (...) {
			}
		}).detach();
	}

} /* namespace widgets */
} /* namespace spine */
