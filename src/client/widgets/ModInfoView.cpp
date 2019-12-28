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

#include "https/Https.h"

#include "launcher/Gothic1And2Launcher.h"
#include "launcher/LauncherFactory.h"

#include "security/Hash.h"

#include "utils/Conversion.h"
#include "utils/Hashing.h"

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
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QProcessEnvironment>
#include <QQueue>
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
			UPDATELANGUAGESETTITLE(generalSettingsWidget, _patchGroup, "PatchesAndTools");
			_patchGroup->setProperty("library", true);
			_patchLayout = new QGridLayout();
			_patchLayout->setColumnStretch(2, 1);
			_patchGroup->setLayout(_patchLayout);
			_patchGroup->hide();

			_pdfGroup = new QGroupBox(QApplication::tr("PDFs"), this);
			UPDATELANGUAGESETTITLE(generalSettingsWidget, _pdfGroup, "PDFs");
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
			removeEmptyDirs(_lastBaseDir);
		}

		qRegisterMetaType<std::vector<int32_t>>("std::vector<int32_t>");
		qRegisterMetaType<common::ModStats>("common::ModStats");

		connect(this, SIGNAL(receivedModStats(common::ModStats)), this, SLOT(updateModInfoView(common::ModStats)));
		connect(this, SIGNAL(receivedCompatibilityList(int, std::vector<int32_t>, std::vector<int32_t>)), this, SLOT(updateCompatibilityList(int, std::vector<int32_t>, std::vector<int32_t>)));
		connect(this, &ModInfoView::errorMessage, this, &ModInfoView::showErrorMessage);

		{
			const auto factory = launcher::LauncherFactory::getInstance();
			connect(factory, &launcher::LauncherFactory::restartAsAdmin, this, &ModInfoView::restartSpineAsAdmin);
			
			{
				const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC);
				const auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();

				connect(gothicLauncher.data(), &launcher::Gothic1And2Launcher::installMod, this, &ModInfoView::installMod);
			}
			{
				const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC2);
				const auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();

				connect(gothicLauncher.data(), &launcher::Gothic1And2Launcher::installMod, this, &ModInfoView::installMod);
			}
		}

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
		const QString title = iniParser.value("Title", "").toString();
		const QString version = iniParser.value("Version", "").toString();
		const QString team = iniParser.value("Authors", "").toString();
		QString homepage = iniParser.value("Webpage", "").toString();
		const QString contact = iniParser.value("Contact", "").toString();
		const QString description = iniParser.value("Description", "").toString();
		const bool requiresAdmin = iniParser.value("RequiresAdmin", false).toBool();
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
				const QFileInfo fi(file);
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
		if (_iniFile.contains(programFiles) || _iniFile.contains(programFilesx86) || requiresAdmin) {
#ifdef Q_OS_WIN
			_startButton->setEnabled(IsRunAsAdmin());
			_adminInfoLabel->setVisible(!IsRunAsAdmin());
#else
			_startButton->setEnabled(true);
			_adminInfoLabel->setVisible(false);
#endif
			_adminInfoLabel->setText(requiresAdmin ? QApplication::tr("GothicAdminInfoRequiresAdmin").arg(title) : QApplication::tr("GothicAdminInfo").arg(title));
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

		QSettings iniParser(_iniFile, QSettings::IniFormat);
		iniParser.beginGroup("INFO");
		const bool requiresAdmin = iniParser.value("RequiresAdmin", false).toBool();
		iniParser.endGroup();

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
			if (usedBaseDir.contains(programFiles) || usedBaseDir.contains(programFilesx86) || requiresAdmin) {
#ifdef Q_OS_WIN
				_startButton->setEnabled(IsRunAsAdmin());
				_adminInfoLabel->setVisible(!IsRunAsAdmin());
#else
				_startButton->setEnabled(true);
				_adminInfoLabel->setVisible(false);
#endif
				const QString gothicName = modGv.gothicVersion == common::GothicVersion::GOTHIC ? QApplication::tr("Gothic") : QApplication::tr("Gothic2");
				_adminInfoLabel->setText(requiresAdmin ? QApplication::tr("GothicAdminInfoRequiresAdmin").arg(gothicName) : QApplication::tr("GothicAdminInfo").arg(gothicName));
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
		_gothicDirectory = directory; // TODO: this part can be removed as soon as everything referencing it is moved to launcher class

		const auto factory = launcher::LauncherFactory::getInstance();
		const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC);
		auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();
		gothicLauncher->setDirectory(directory);
	}

	void ModInfoView::setGothic2Directory(QString directory) {
		_gothic2Directory = directory; // TODO: this part can be removed as soon as everything referencing it is moved to launcher class

		const auto factory = launcher::LauncherFactory::getInstance();
		const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC2);
		auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();
		gothicLauncher->setDirectory(directory);
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
		QSet<QString> dependencies;
		QString usedBaseDir;
		QString usedExecutable;
		bool newGMP = false;

		QTime t;
		t.start();
		QFuture<bool> future = QtConcurrent::run<bool>(this, &ModInfoView::prepareModStart, &usedBaseDir, &usedExecutable, &backgroundExecutables, &newGMP, &dependencies);
		watcher.setFuture(future);
		loop.exec();
		if (!future.result()) {
			splash.hide();
			if (!dependencies.isEmpty()) {
				https::Https::postAsync(DATABASESERVER_PORT, "getModnameForIDs", QString("{ \"Language\": \"%1\", \"ModIDs\": [ %2 ] }").arg(_language).arg(dependencies.toList().join(',')), [this, dependencies](const QJsonObject & json, int status) {
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
					emit errorMessage(msg);
				});
			}
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
		connect(process, &QProcess::errorOccurred, this, &ModInfoView::errorOccurred);
		connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &ModInfoView::finishedMod);
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
		if (QFileInfo::exists(usedBaseDir + "/System/pre.load")) {
			QString text;
			{
				QFile f(usedBaseDir + "/System/pre.load");
				f.open(QIODevice::ReadOnly);
				QTextStream ts(&f);
				text = ts.readAll();
			}
			for (const auto & s : _systempackPreLoads) {
				text = text.remove("\n" + s);
			}
			{
				QFile f(usedBaseDir + "/System/pre.load");
				f.open(QIODevice::WriteOnly);
				QTextStream ts(&f);
				ts << text;
			}
			_systempackPreLoads.clear();
		}	
		
		if (!_unionPlugins.isEmpty() && QFileInfo::exists(usedBaseDir + "/System/Union.ini")) {
			QString text;
			{
				QFile f(usedBaseDir + "/System/Union.ini");
				f.open(QIODevice::ReadOnly);
				QTextStream ts(&f);
				text = ts.readAll();
			}
			const QRegularExpression regExp("(PluginList\\s=[^\n]*)\n");
			const QRegularExpressionMatch match = regExp.match(text);
			QString replaceText = match.captured(1);
			LOGINFO("Replacing #2: " << q2s(replaceText));
			replaceText = replaceText.trimmed();
			if (!replaceText.isEmpty()) {
				text = text.replace(replaceText, "PluginList =");
				{
					QFile f(usedBaseDir + "/System/Union.ini");
					f.open(QIODevice::WriteOnly);
					QTextStream ts(&f);
					ts << text;
				}
			}
			_unionPlugins.clear();
		}

		if (QFileInfo::exists(usedBaseDir + "/Data/_delete_me.vdf")) {
			QFile::remove(usedBaseDir + "/Data/_delete_me.vdf");
		}
	
		_gothicIniBackup.clear();
		_systempackIniBackup.clear();
		_skippedFiles.clear();
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
	
		if (QFileInfo::exists(usedBaseDir + "/System/BugslayerUtilG.dll")) {
			QFile::rename(usedBaseDir + "/System/BugslayerUtilG.dll", usedBaseDir + "/System/BugslayerUtil.dll");
		}

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
		const QString timeString = utils::timeToString(ms.duration);
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

	void ModInfoView::updateCompatibilityList(int modID, std::vector<int32_t> incompatiblePatches, std::vector<int32_t> forbiddenPatches) {
		if (_modID != modID) return;

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

		const auto patches = Database::queryAll<Patch, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, Name FROM patches WHERE ModID != 228 ORDER BY Name ASC;", err);
		for (const Patch & p : patches) {
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
			} else if (contains(forbiddenPatches, p.modID)) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + " AND PatchID = " + std::to_string(p.modID) + ";", err);
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
		if (QFileInfo(usedBaseDir + "/System/Spacer.exe").exists()) {
			usedExecutable = "Spacer.exe";
		} else if (QFileInfo(usedBaseDir + "/System/Spacer2_exe.exe").exists()) {
			usedExecutable = "Spacer2_exe.exe";
		} else if (QFileInfo(usedBaseDir + "/System/Spacer2.exe").exists()) {
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
				_copiedFiles.removeOne(file);
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
							if (ram && _onlineMode && !_username.isEmpty()) {
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
								std::vector<std::string> lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier FROM modAchievements WHERE ModID = " + std::to_string(_modID) + ";", dbErr);

								common::SendAchievementsMessage sam;
								for (const std::string & s : lastResults) {
									int32_t identifier = int32_t(std::stoi(s));
									sam.achievements.push_back(identifier);
								}
								auto lastResultsVec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Identifier, Max FROM modAchievementProgressMax WHERE ModID = " + std::to_string(_modID) + ";", dbErr);
								for (auto vec : lastResultsVec) {
									lastResults = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT Current FROM modAchievementProgress WHERE ModID = " + std::to_string(_modID) + " AND Identifier = " + vec[0] + " LIMIT 1;", dbErr);
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
									uam->modID = _modID;
									
									cacheAchievement(uam);
									
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
			emit receivedCompatibilityList(_modID, {}, {});
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
		const bool canSymlink = suffix == "mod" || suffix == "vdf" || suffix == "sty" || suffix == "sgt" || suffix == "dls" || suffix == "bik" || suffix == "dds" || suffix == "jpg" || suffix == "png" || suffix == "mi" || suffix == "hlsl" || suffix == "h" || suffix == "vi" || suffix == "exe" || suffix == "dll" || suffix == "bin" || suffix == "mtl" || suffix == "obj" || suffix == "txt" || suffix == "rtf" || suffix == "obj" || suffix == "ico" || suffix == "ini" || suffix == "bak" || suffix == "gsp" || suffix == "pdb" || suffix == "config" || suffix == "fx" || suffix == "3ds" || suffix == "mcache" || suffix == "fxh";
		if (!canSymlink) {
			LOGINFO("Copying extension: " << suffix.toStdString());
		}
		return canSymlink;
	}

	bool ModInfoView::prepareModStart(QString * usedBaseDir, QString * usedExecutable, QStringList * backgroundExecutables, bool * newGMP, QSet<QString> * dependencies) {
		_gmpCounterBackup = -1;

		QSet<QString> forbidden;
		collectDependencies(_modID, dependencies, &forbidden);

		LOGINFO("Starting Ini: " << _iniFile.toStdString());
		emitSplashMessage(QApplication::tr("RemovingOldFiles"));
		for (const QString & f : _copiedFiles) {
			QFile(_lastBaseDir + "/" + f).remove();
			Database::DBError err;
			Database::execute(Config::BASEDIR.toStdString() + "/" + FIX_DATABASE, "DELETE FROM usedFiles WHERE File = '" + f.toStdString() + "';", err);
		}
		_copiedFiles.clear();

		Database::DBError err;
		std::vector<std::string> patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(_modID) + ";", err);
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
			auto it2 = std::find(patches.begin(), patches.end(), it->toStdString());
			if (it2 != patches.end()) {
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
		if (!dependencies->isEmpty()) return false; // not all dependencies are met and can't automatically be enabled

		for (const std::string & patchID : patches) {
			const std::string patchName = Database::queryNth<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchID + " LIMIT 1;", err);
			if (patchName == "D3D11 Renderer Clockwork Edition" || patchName == "D3D11 Renderer Convenient Edition") {
				clockworkRenderer = true;
			} else if (patchName.find("Systempack") != std::string::npos) {
				systempack = true;
			} else if (patchID == "314") {
				ninja = true;
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
					success = linkOrCopyFile(_gothicDirectory + "/" + sourceFileName, *usedBaseDir + "/" + targetFileName);
					if (!success) {
						LOGERROR("Couldn't copy file: " << sourceFileName.toStdString() << " " << f.errorString().toStdString());
						removeGothicFiles();
						break;
					}
					_copiedFiles.append(file);
				}
			}

			if (ninja) {
				prepareForNinja(usedBaseDir);
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
					if (filename.contains("Data/modvdf/") || canSkipFile(filename)) {
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
							_copiedFiles.append("/System/GD3D11/textures/replacements/" + match.captured(1));
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
							if (QFileInfo::exists(*usedBaseDir + "/" + filename)) {
								const bool b = utils::Hashing::checkHash(Config::MODDIR + "/mods/" + QString::number(_modID) + "/" + filename, QString::fromStdString(file.second));
								if (b) {
									copy = false;
									if (GeneralSettingsWidget::extendedLogging) {
										LOGINFO("Skipping file");
									}
									_skippedFiles.append(filename);
								}
								if (copy) {
									if (QFileInfo::exists(*usedBaseDir + "/" + filename + ".spbak")) {
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
								f.close();
								success = linkOrCopyFile(Config::MODDIR + "/mods/" + QString::number(_modID) + "/" + filename, *usedBaseDir + "/" + changedFile);
							}
							if (!success) {
								LOGERROR("Couldn't copy file: " << filename.toStdString() << " " << f.errorString().toStdString());
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
					QFileInfo fi(*usedBaseDir + "/" + filename);
					QDir dir = fi.absoluteDir();
					success = dir.mkpath(dir.absolutePath());
					if (!success) {
						LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString());
						break;
					}
					// backup old file
					bool copy = true;
					if (QFileInfo::exists(*usedBaseDir + "/" + filename)) {
						const bool b = utils::Hashing::checkHash(*usedBaseDir + "/" + filename, QString::fromStdString(file.second));
						if (b) {
							copy = false;
							if (GeneralSettingsWidget::extendedLogging) {
								LOGINFO("Skipping file");
							}
							_skippedFiles.append(filename);
						}
						if (copy) {
							if (QFileInfo::exists(*usedBaseDir + "/" + filename + ".spbak")) {
								QFile::remove(*usedBaseDir + "/" + filename);
							} else {
								QFile::rename(*usedBaseDir + "/" + filename, *usedBaseDir + "/" + filename + ".spbak");
							}
						}
					}
					if (copy) {
						success = linkOrCopyFile(Config::MODDIR + "/mods/" + QString::number(_modID) + "/" + filename, *usedBaseDir + "/" + filename);
					}
					if (!success) {
						LOGERROR("Couldn't copy file: " << filename.toStdString());
						break;
					}
					if (copy) {
						_copiedFiles.append(filename);
					}
				}
				if (!success) {
					removeModFiles();
					LOGERROR("Failed copying mod data");
					return false;
				}
				checkToolCfg(Config::MODDIR + "/mods/" + QString::number(_modID), usedBaseDir, backgroundExecutables, newGMP);
				updatePlugins(_modID, usedBaseDir);
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
				_copiedFiles.append("System/vdfs32g.exe");
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

					 if (canSkipFile(filename)) continue;
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
							_copiedFiles.append(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/Meshes";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.append(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/shaders";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.append(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/textures/infos";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.append(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/textures/RainDrops";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
							skippedBases.insert(checkPath);
							_copiedFiles.append(checkPath);
							continue;
						} else if (skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive)) {
							// just skip
							continue;
						}
						checkPath = "System/GD3D11/textures/replacements/Normalmaps_Original";
						if (!skippedBases.contains(checkPath) && filename.contains(checkPath, Qt::CaseInsensitive) && (!QDir(*usedBaseDir + "/" + checkPath).exists() || QFileInfo(*usedBaseDir + "/" + checkPath).isSymLink())) {
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + checkPath, *usedBaseDir + "/" + checkPath);
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
							const QString targetName = clockworkRenderer ? *usedBaseDir + "/" + checkPath + QString::number(normalsCounter) : "/System/GD3D11/textures/replacements/" + match.captured(1);
							makeSymlinkFolder(Config::MODDIR + "/mods/" + QString::number(patchID) + "/System/GD3D11/textures/replacements/" + match.captured(1), targetName);
							_copiedFiles.append("/System/GD3D11/textures/replacements/" + match.captured(1));
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
							if (QFileInfo::exists(*usedBaseDir + "/" + changedFile)) {
								const bool b = utils::Hashing::checkHash(*usedBaseDir + "/" + changedFile, QString::fromStdString(file.second));
								if (b) {
									copy = false;
									if (GeneralSettingsWidget::extendedLogging) {
										LOGINFO("Skipping file");
									}
								}
								if (copy) {
									if (QFileInfo::exists(*usedBaseDir + "/" + changedFile + ".spbak")) {
										QFile::remove(*usedBaseDir + "/" + changedFile);
									} else {
										QFile::rename(*usedBaseDir + "/" + changedFile, *usedBaseDir + "/" + changedFile + ".spbak");
									}
								}
							}
							if (copy) {
								f.close();
								const bool b = linkOrCopyFile(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + filename, *usedBaseDir + "/" + changedFile);
								_copiedFiles.append(changedFile);
								Q_ASSERT(b);
							}
							continue;
						}
#ifdef Q_OS_WIN
					}
#endif
					QFileInfo fi(*usedBaseDir + "/" + filename);
					QDir dir = fi.absoluteDir();
					bool success = dir.mkpath(dir.absolutePath());
					if (!success) {
						LOGERROR("Couldn't create dir: " << dir.absolutePath().toStdString());
						break;
					}
					// backup old file, if already backed up, don't patch
					qDebug() << _copiedFiles;
					if (!_copiedFiles.contains(filename, Qt::CaseInsensitive) && !_skippedFiles.contains(filename, Qt::CaseInsensitive) && ((QFileInfo::exists(*usedBaseDir + "/" + filename) && !QFileInfo::exists(*usedBaseDir + "/" + filename + ".spbak")) || !QFileInfo::exists(*usedBaseDir + "/" + filename))) {
						if (GeneralSettingsWidget::extendedLogging) {
							LOGINFO("Copying file " << file.first);
						}
						bool copy = true;
						if (QFileInfo::exists(*usedBaseDir + "/" + filename)) {
							const bool b = utils::Hashing::checkHash(*usedBaseDir + "/" + filename, QString::fromStdString(file.second));
							if (b) {
								copy = false;
							}
						}
						if (filename.contains("directx_Jun2010_redist.exe", Qt::CaseInsensitive) || filename.contains(QRegularExpression("vc*.exe", QRegularExpression::ExtendedPatternSyntaxOption))) {
							copy = false;
						}
						if (copy) {
							QFile::rename(*usedBaseDir + "/" + filename, *usedBaseDir + "/" + filename + ".spbak");
							success = linkOrCopyFile(Config::MODDIR + "/mods/" + QString::number(patchID) + "/" + filename, *usedBaseDir + "/" + filename);
							if (!success) {
								std::vector<std::string> name = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Name FROM patches WHERE ModID = " + patchIDString + " LIMIT 1;", err);
								Q_ASSERT(!name.empty());
								emit errorMessage(QApplication::tr("PatchIncomplete").arg(s2q(name[0])));
								return false;
							}
						}
						_copiedFiles.append(filename);
					}
				}
				checkToolCfg(Config::MODDIR + "/mods/" + QString::number(patchID), usedBaseDir, backgroundExecutables, newGMP);
				updatePlugins(patchID, usedBaseDir);
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
			if (!fileName.isEmpty() && !_copiedFiles.contains(partialName, Qt::CaseInsensitive) && !_skippedFiles.contains(partialName, Qt::CaseInsensitive)) {
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
				linkOrCopyFile(sourceDir + "/Data/modvdf/" + s, *usedBaseDir + "/Data/" + s);
				_copiedFiles.append("Data/" + s);
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
				linkOrCopyFile(qApp->applicationDirPath() + "/SpineAPI.dll", *usedBaseDir + "/System/SpineAPI.dll");
			}
			{
				linkOrCopyFile(qApp->applicationDirPath() + "/../media/Spine.vdf", *usedBaseDir + "/Data/Spine.vdf");
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
			const QString executable = configParser.value("CONFIG/BackgroundProcess", "").toString();
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
				const QString ini = configParser.value("CONFIG/WriteIni", "").toString();
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
				const QString file = configParser.value("CONFIG/WriteFile", "").toString();
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
								std::vector<std::vector<std::string>> achievements = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier FROM modAchievements;", err);
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
								std::vector<std::vector<std::string>> scores = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Identifier, Score FROM modScores;", err);
								for (auto vec : scores) {
									common::UpdateOfflineDataMessage::ScoreData sd {};
									sd.modID = std::stoi(vec[0]);
									sd.identifier = std::stoi(vec[1]);
									sd.score = std::stoi(vec[2]);
									uodm.scores.push_back(sd);
								}
								std::vector<std::vector<std::string>> overallSaveData = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Entry, Value FROM overallSaveData;", err);
								for (auto vec : overallSaveData) {
									common::UpdateOfflineDataMessage::OverallSaveData od;
									od.modID = std::stoi(vec[0]);
									od.entry = vec[1];
									od.value = vec[2];
									uodm.overallSaves.push_back(od);
								}
								std::vector<std::vector<int>> playTimes = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "SELECT ModID, Duration FROM playTimes;", err);
								for (auto vec : playTimes) {
									uodm.playTimes.emplace_back(vec[0], vec[1]);
								}
								Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM playTimes;", err);
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
											for (const auto & ad : sodm->achievements) {
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
											for (const auto & sd : sodm->scores) {
												Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO modScores (ModID, Identifier, Username, Score) VALUES (" + std::to_string(sd.modID) + ", " + std::to_string(sd.identifier) + ", '" + sd.username + "', " + std::to_string(sd.score) + ");", err);
											}
											for (const auto & od : sodm->overallSaves) {
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

	void ModInfoView::collectDependencies(int modID, QSet<QString> * dependencies, QSet<QString> * forbidden) {
		QQueue<QString> toCheck;
		toCheck.enqueue(QString::number(modID));

		Database::DBError err;
		std::vector<std::string> patches = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT PatchID FROM patchConfigs WHERE ModID = " + std::to_string(modID) + ";", err);

		for (const auto & p : patches) {
			toCheck.enqueue(s2q(p));
		}

		while (!toCheck.empty()) {
			auto id = toCheck.dequeue();
			auto path = Config::MODDIR + "/mods/" + id;
			if (QFileInfo::exists(path + "/tool.cfg")) {
				QSettings configParser(path + "/tool.cfg", QSettings::IniFormat);

				auto required = configParser.value("DEPENDENCIES/Required", "").toString();
				
				auto split = required.split(',', QString::SkipEmptyParts);
				for (const auto & s : split) {
					if (dependencies->contains(s)) continue;
					
					dependencies->insert(s);
					toCheck.append(s);
				}
				
				auto blocked = configParser.value("DEPENDENCIES/Blocked", "").toString();
				
				split = blocked.split(',', QString::SkipEmptyParts);
				for (const auto & s : split) {
					forbidden->insert(s);
				}
			}			
		}
	}

	void ModInfoView::prepareForNinja(QString * usedBaseDir) {
		// base case
		{
			const auto bugslayerDll = *usedBaseDir + "/System/BugslayerUtil.dll";
			if (QFileInfo::exists(bugslayerDll)) {
				if (QFileInfo::exists(*usedBaseDir + "/System/BugslayerUtilG.dll")) {
					QFile::remove(*usedBaseDir + "/System/BugslayerUtilG.dll");
				}
				const bool b = QFile::rename(bugslayerDll, *usedBaseDir + "/System/BugslayerUtilG.dll");
				Q_ASSERT(b);
			}
		}
	}

	void ModInfoView::updatePlugins(int modID, QString * usedBaseDir) {
		const auto path = QString("%1/mods/%2").arg(Config::MODDIR).arg(modID);
	
		if (!QFileInfo::exists(path + "/tool.cfg")) return;

		const QSettings configParser(path + "/tool.cfg", QSettings::IniFormat);

		{
			const auto systempackEntries = configParser.value("LOADER/SPpreload", "").toString();
					
			const auto split = systempackEntries.split(',', QString::SkipEmptyParts);

			QFile f(*usedBaseDir + "/System/pre.load");
			f.open(QIODevice::Append);
			QTextStream ts(&f);
			for (const auto & s : split) {
				ts << "\n" + s;
			}
			_systempackPreLoads << split;
		}
		{
			const auto unionEntries = configParser.value("LOADER/UnionIni", "").toString();
					
			const auto split = unionEntries.split(',', QString::SkipEmptyParts);

			_unionPlugins << split;

			if (!_unionPlugins.isEmpty()) {
				QString text;
				{
					QFile f(*usedBaseDir + "/System/Union.ini");
					f.open(QIODevice::ReadOnly);
					QTextStream ts(&f);
					text = ts.readAll();
				}
				const QRegularExpression regExp("(PluginList\\s=[^\n]*)\n");
				const QRegularExpressionMatch match = regExp.match(text);
				QString replaceText = match.captured(1);
				LOGINFO("Replacing #1: " << q2s(replaceText));
				replaceText = replaceText.trimmed();
				if (replaceText.isEmpty()) {
					text += "\n[PLUGINS]\nPluginList = " + _unionPlugins.join(',') + "\n";
				} else {
					text = text.replace(replaceText, "PluginList = " + _unionPlugins.join(','));
				}
				{
					QFile f(*usedBaseDir + "/System/Union.ini");
					f.open(QIODevice::WriteOnly);
					QTextStream ts(&f);
					ts << text;
				}
			}
		}
	}

	bool ModInfoView::linkOrCopyFile(QString sourcePath, QString destinationPath) {
#ifdef Q_OS_WIN
		const auto suffix = QFileInfo(sourcePath).suffix();
		if (IsRunAsAdmin() && isAllowedSymlinkSuffix(suffix)) {
			const bool linked = makeSymlink(sourcePath, destinationPath);
			return linked;
		}
	
		const bool linked = CreateHardLinkW(destinationPath.toStdWString().c_str(), sourcePath.toStdWString().c_str(), nullptr);
		if (!linked) {
			const bool copied = QFile::copy(sourcePath, destinationPath);
			return copied;
		}
		return linked;
#else
		const bool copied = QFile::copy(sourcePath, destinationPath);
		return copied;
#endif
	}

	bool ModInfoView::canSkipFile(const QString & filename) const {
		const QFileInfo fi(filename);
		const auto suffix = fi.suffix().toLower();
		return filename.compare("tool.cfg", Qt::CaseInsensitive) == 0 || suffix == "pdf";
	}

} /* namespace widgets */
} /* namespace spine */
