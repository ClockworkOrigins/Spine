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

#include "widgets/MainWindow.h"

#include "FileDownloader.h"
#include "LibraryFilterModel.h"
#include "MultiFileDownloader.h"
#include "ReportGenerator.h"
#include "SpineConfig.h"
#include "Uninstaller.h"

#include "common/MessageStructs.h"

#include "models/SpineEditorModel.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/Hashing.h"
#include "utils/WindowsExtensions.h"

#include "widgets/AboutDialog.h"
#include "widgets/AutoUpdateDialog.h"
#include "widgets/ChangelogDialog.h"
#include "widgets/DeveloperSettingsWidget.h"
#include "widgets/DownloadProgressDialog.h"
#include "widgets/ExportDialog.h"
#include "widgets/FAQDialog.h"
#include "widgets/FeedbackDialog.h"
#include "widgets/FriendsView.h"
#include "widgets/GameSettingsWidget.h"
#include "widgets/GeneralSettingsWidget.h"
#include "widgets/ImportDialog.h"
#include "widgets/IniConfigurator.h"

#ifdef Q_OS_WIN
	#include "widgets/InstallGothic2FromCDDialog.h"
#endif

#include "widgets/IntegrityCheckDialog.h"
#include "widgets/LibraryListView.h"
#include "widgets/LocationSettingsWidget.h"
#include "widgets/LoginDialog.h"
#include "widgets/ManagementDialog.h"
#include "widgets/ModDatabaseView.h"
#include "widgets/ModInfoPage.h"
#include "widgets/ModInfoView.h"
#include "widgets/ModUpdateDialog.h"
#include "widgets/ProfileView.h"
#include "widgets/SavegameDialog.h"
#include "widgets/SettingsDialog.h"
#include "widgets/SpineEditor.h"
#include "widgets/StartPageWidget.h"
#include "widgets/SubmitCompatibilityDialog.h"
#include "widgets/UpdateLanguage.h"

#ifdef WITH_TRANSLATOR
	#include "translator/TranslationRequestDialog.h"
	#include "translator/TranslatorDialog.h"
#endif

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStorageInfo>
#include <QtConcurrentRun>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <QtWinExtras/qwinfunctions.h>
	#include <shellapi.h>
#endif

using namespace spine;
using namespace spine::translator;
using namespace spine::utils;
using namespace spine::widgets;

enum MainTabsOnline {
	StartOnline,
	Info,
	Database,
	LibraryOnline,
	Profile,
	Friends
};

enum MainTabsOffline {
	StartOffline,
	LibraryOffline
};

MainWindow * MainWindow::instance = nullptr;

MainWindow::MainWindow(bool showChangelog, QMainWindow * par) : QMainWindow(par), _modListView(nullptr), _modInfoView(nullptr), _profileView(nullptr), _friendsView(nullptr), _gothicDirectory(), _gothic2Directory(), _settingsDialog(nullptr), _autoUpdateDialog(), _changelogDialog(nullptr), _modListModel(nullptr), _loginDialog(nullptr), _modUpdateDialog(nullptr), _installGothic2FromCDDialog(nullptr), _feedbackDialog(nullptr), _developerModeActive(false), _devModeAction(nullptr), _modDatabaseView(nullptr), _parsedInis(), _tabWidget(nullptr), _spineEditorAction(nullptr), _spineEditor(nullptr), _modInfoPage(nullptr) {
	instance = this;

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #1: " << getPRAMValue());
#endif

	if (!Config::IniParser->contains("INSTALLATION/DirectX")) {
		// so far Spine automatically install DirectX during installation, but to enforce reinstall on download of e.g. renderer, this can manually set to false in the ini
		Config::IniParser->setValue("INSTALLATION/DirectX", true);
	}

	setWindowIcon(QIcon(":/Spine.ico"));
	_settingsDialog = new SettingsDialog(this); // create at first

#ifdef Q_OS_WIN
	_installGothic2FromCDDialog = new InstallGothic2FromCDDialog();
#endif

	_feedbackDialog = new FeedbackDialog();

	restoreSettings();

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #2: " << getPRAMValue());
#endif

	qRegisterMetaType<int32_t>("int32_t");

	QString version = Config::IniParser->value("MISC/Version", "").toString();
	Config::OnlineMode = Config::IniParser->value("MISC/OnlineMode", true).toBool();
	if (version != QString::fromStdString(VERSION_STRING)) {
		QStringList versionSplit = version.split(".");
		if (versionSplit.size() == 3) {
			if (versionSplit[0].toInt() == 1 && versionSplit[1].toInt() < 5) {
				QFile(QString::fromStdString(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE)).remove();
			}
		}
		showChangelog = true;
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #3: " << getPRAMValue());
#endif

	if (Config::OnlineMode) {
		{
			clockUtils::sockets::TcpSocket sock;
			const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000);
			const bool cuTest = cErr == clockUtils::ClockError::SUCCESS;

			Config::OnlineMode = cuTest;
		}
		if (Config::OnlineMode) {
			QtConcurrent::run([]() {
				clockUtils::sockets::TcpSocket sock;
				if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
					common::UpdateStartTimeMessage ustm;
					ustm.dayOfTheWeek = QDate::currentDate().dayOfWeek();
					ustm.hour = QTime::currentTime().hour();
					const std::string serialized = ustm.SerializePublic();
					sock.writePacket(serialized);
				}
			});
		}
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #4: " << getPRAMValue());
#endif

	// remove High Vegetation Mod from patches
	{
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE PatchID = 35;", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM patches WHERE ModID = 35;", err);
	}

	if (showChangelog) {
		QSettings versionSettings(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Spine)", QSettings::NativeFormat);
		versionSettings.setValue("VersionMajor", VERSION_MAJOR);
		versionSettings.setValue("VersionMinor", VERSION_MINOR);
		versionSettings.setValue("VersionPatch", VERSION_PATCH);

		Config::IniParser->setValue("MISC/Version", QString::fromStdString(VERSION_STRING));
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #5: " << getPRAMValue());
#endif

	_tabWidget = new QTabWidget(this);
	_tabWidget->setProperty("library", true);

	StartPageWidget * startPage = new StartPageWidget(this, _settingsDialog->getGeneralSettingsWidget(), _tabWidget);
	connect(_settingsDialog->getGeneralSettingsWidget(), &GeneralSettingsWidget::languageChanged, startPage, &StartPageWidget::setLanguage);

	_tabWidget->addTab(startPage, QApplication::tr("Startpage"));
	UPDATELANGUAGESETTABTEXT(_tabWidget, Config::OnlineMode ? int(MainTabsOnline::StartOnline) : int(MainTabsOffline::StartOffline), "Startpage");

	if (Config::OnlineMode) {
		_modInfoPage = new ModInfoPage(this, _settingsDialog->getGeneralSettingsWidget(), this);
		_tabWidget->addTab(_modInfoPage, QApplication::tr("InfoPage"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Info, "InfoPage");
	}

	if (Config::OnlineMode) {
		_modDatabaseView = new ModDatabaseView(this, _settingsDialog->getGeneralSettingsWidget(), _tabWidget);
		_tabWidget->addTab(_modDatabaseView, QApplication::tr("Database"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Database, "Database");
	}

	if (Config::OnlineMode) {
		connect(startPage, &StartPageWidget::tryInstallMod, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int)>(&ModDatabaseView::updateModList));
		connect(_modInfoPage, &ModInfoPage::tryInstallMod, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int)>(&ModDatabaseView::updateModList));
		connect(_modInfoPage, &ModInfoPage::tryInstallPackage, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int)>(&ModDatabaseView::updateModList));
		connect(_modDatabaseView, &ModDatabaseView::loadPage, _modInfoPage, &ModInfoPage::loadPage);
		connect(_modDatabaseView, &ModDatabaseView::loadPage, this, &MainWindow::changeToInfoTab);

		connect(_modDatabaseView, &ModDatabaseView::finishedInstallation, startPage, &StartPageWidget::finishedInstallation);
		connect(_modDatabaseView, &ModDatabaseView::finishedInstallation, _modInfoPage, &ModInfoPage::finishedInstallation);

		connect(_modInfoPage, &ModInfoPage::triggerModStart, this, &MainWindow::triggerModStart);
	}
	connect(startPage, &StartPageWidget::triggerModStart, this, &MainWindow::triggerModStart);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #6: " << getPRAMValue());
#endif

	_spineEditor = new SpineEditor(this);

	QWidget * w = new QWidget(_tabWidget);
	w->setProperty("library", true);

	QVBoxLayout * l = new QVBoxLayout();

	QWidget * topWidget = new QWidget(w);
	topWidget->setProperty("library", true);
	QHBoxLayout * topLayout = new QHBoxLayout();

	_modListView = new LibraryListView(topWidget);
	_modListView->setProperty("library", true);
	connect(_modListView, &LibraryListView::hideModTriggered, this, &MainWindow::hideMod);
	connect(_modListView, &LibraryListView::showModTriggered, this, &MainWindow::showMod);
	connect(_modListView, &LibraryListView::uninstallModTriggered, this, &MainWindow::uninstallMod);
	_modListModel = new QStandardItemModel(_modListView);
	_sortModel = new LibraryFilterModel(_modListView);
	_sortModel->setSourceModel(_modListModel);
	_sortModel->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
	_sortModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
	_modListView->setModel(_sortModel);
	{
		QFont f = _modListView->font();
		f.setPointSize(12);
		_modListView->setFont(f);
	}
	_modListView->setIconSize(QSize(200, 200));

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #7: " << getPRAMValue());
#endif

	{
		QWidget * filterWidget = new QWidget(this);
		filterWidget->setProperty("library", true);
		QHBoxLayout * hl = new QHBoxLayout();
		QLineEdit * le = new QLineEdit(filterWidget);
		le->setProperty("library", true);
		le->setPlaceholderText(QApplication::tr("SearchPlaceholder"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(le, "SearchPlaceholder");

		connect(le, &QLineEdit::textChanged, _sortModel, static_cast<void(QSortFilterProxyModel::*)(const QString &)>(&QSortFilterProxyModel::setFilterRegExp));

		hl->addWidget(le);
		{
			QGroupBox * gb = new QGroupBox(QApplication::tr("Game"), filterWidget);
			gb->setProperty("library", true);
			UPDATELANGUAGESETTITLE(gb, "Game");

			QVBoxLayout * vbl = new QVBoxLayout();

			QCheckBox * cb1 = new QCheckBox(QApplication::tr("Gothic"), filterWidget);
			cb1->setProperty("library", true);
			cb1->setChecked(_sortModel->isGothicActive());
			UPDATELANGUAGESETTEXT(cb1, "Gothic");
			connect(cb1, SIGNAL(stateChanged(int)), _sortModel, SLOT(gothicChanged(int)));

			QCheckBox * cb2 = new QCheckBox(QApplication::tr("Gothic2"), filterWidget);
			cb2->setProperty("library", true);
			cb2->setChecked(_sortModel->isGothic2Active());
			UPDATELANGUAGESETTEXT(cb2, "Gothic2");
			connect(cb2, SIGNAL(stateChanged(int)), _sortModel, SLOT(gothic2Changed(int)));

			QCheckBox * cb3 = new QCheckBox(QApplication::tr("GothicAndGothic2"), filterWidget);
			cb3->setProperty("library", true);
			cb3->setChecked(_sortModel->isGothicAndGothic2Active());
			UPDATELANGUAGESETTEXT(cb3, "GothicAndGothic2");
			connect(cb3, SIGNAL(stateChanged(int)), _sortModel, SLOT(gothicAndGothic2Changed(int)));
			cb3->hide();

			vbl->addWidget(cb1);
			vbl->addWidget(cb2);
			vbl->addWidget(cb3);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}
		{
			QGroupBox * gb = new QGroupBox(QApplication::tr("General"), filterWidget);
			gb->setProperty("library", true);
			UPDATELANGUAGESETTITLE(gb, "General");

			QVBoxLayout * vbl = new QVBoxLayout();

			QCheckBox * cb1 = new QCheckBox(QApplication::tr("ShowHidden"), filterWidget);
			cb1->setProperty("library", true);
			cb1->setChecked(_sortModel->isShowHiddenActive());
			UPDATELANGUAGESETTEXT(cb1, "ShowHidden");
			connect(cb1, SIGNAL(stateChanged(int)), _sortModel, SLOT(showHidden(int)));

			vbl->addWidget(cb1);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}
		filterWidget->setLayout(hl);
		l->addWidget(filterWidget);
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #8: " << getPRAMValue());
#endif

	connect(_modListView, &LibraryListView::clicked, this, &MainWindow::selectedMod);
	connect(_modListView, &LibraryListView::doubleClicked, this, &MainWindow::selectedMod);

	_modInfoView = new ModInfoView(_settingsDialog->getGeneralSettingsWidget(), topWidget);
	_modInfoView->setProperty("library", true);

	_modInfoView->setDeveloperMode(_settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive());
	_modInfoView->setZSpyActivated(_settingsDialog->getDeveloperSettingsWidget()->isZSpyActive());
	_modInfoView->setShowAchievements(_settingsDialog->getGameSettingsWidget()->getShowAchievements());
	_developerModeActive = _settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive();

	connect(_modListView, &LibraryListView::doubleClicked, _modInfoView, &ModInfoView::start);
	connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::developerModeChanged, _modInfoView, &ModInfoView::setDeveloperMode);
	connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::zSpyChanged, _modInfoView, &ModInfoView::setZSpyActivated);
	connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::developerModeChanged, this, &MainWindow::setDeveloperMode);
	connect(_settingsDialog->getGameSettingsWidget(), &GameSettingsWidget::showAchievementsChanged, _modInfoView, &ModInfoView::setShowAchievements);

	_modListView->setFixedWidth(400);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #9: " << getPRAMValue());
#endif

	topLayout->addWidget(_modListView);

	QVBoxLayout * vbl = new QVBoxLayout();
	vbl->addWidget(_modInfoView);

	topLayout->addLayout(vbl);

	topWidget->setLayout(topLayout);
	l->addWidget(topWidget);

	w->setLayout(l);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #10: " << getPRAMValue());
#endif

	_tabWidget->addTab(w, QApplication::tr("Library"));
	UPDATELANGUAGESETTABTEXT(_tabWidget, (Config::OnlineMode) ? int(MainTabsOnline::LibraryOnline) : int(MainTabsOffline::LibraryOffline), "Library");

	if (Config::OnlineMode) {
		_profileView = new ProfileView(this, _settingsDialog->getGeneralSettingsWidget(), _tabWidget);

		_tabWidget->addTab(_profileView, QApplication::tr("Profile"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Profile, "Profile");

		_friendsView = new FriendsView(_tabWidget);

		_tabWidget->addTab(_friendsView, QApplication::tr("Friends"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Friends, "Friends");

		connect(_modInfoView, SIGNAL(installMod(int)), _modDatabaseView, SLOT(updateModList(int)));
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #11: " << getPRAMValue());
#endif

	_tabWidget->setCurrentWidget(startPage);

	setCentralWidget(_tabWidget);

	if (Config::OnlineMode) {
		connect(_modInfoView, SIGNAL(openAchievementView(int32_t, QString)), _profileView, SLOT(openAchievementView(int32_t, QString)));
		connect(_modInfoView, SIGNAL(openScoreView(int32_t, QString)), _profileView, SLOT(openScoreView(int32_t, QString)));
		connect(_modInfoView, SIGNAL(openAchievementView(int32_t, QString)), this, SLOT(openSpecialProfileView()));
		connect(_modInfoView, SIGNAL(openScoreView(int32_t, QString)), this, SLOT(openSpecialProfileView()));

		connect(_modInfoPage, SIGNAL(openAchievementView(int32_t, QString)), _profileView, SLOT(openAchievementView(int32_t, QString)));
		connect(_modInfoPage, SIGNAL(openScoreView(int32_t, QString)), _profileView, SLOT(openScoreView(int32_t, QString)));
		connect(_modInfoPage, SIGNAL(openAchievementView(int32_t, QString)), this, SLOT(openSpecialProfileView()));
		connect(_modInfoPage, SIGNAL(openScoreView(int32_t, QString)), this, SLOT(openSpecialProfileView()));
	}

	bool firstStartup = Config::IniParser->value("MISC/firstStartup", true).toBool();
	QString path = Config::IniParser->value("PATH/Gothic", "").toString();
	if (!path.isEmpty()) {
		_gothicDirectory = path;
	}
	path = Config::IniParser->value("PATH/Gothic2", "").toString();
	if (!path.isEmpty()) {
		_gothic2Directory = path;
	}
	if (firstStartup) {
		findGothic();
	}
	firstStartup = false;
	Config::IniParser->setValue("MISC/firstStartup", firstStartup);
	if (_settingsDialog->getLocationSettingsWidget()->isGothicValid(true)) {
		_modInfoView->setGothicDirectory(_gothicDirectory);
		if (Config::OnlineMode) {
			_modDatabaseView->setGothicDirectory(_gothicDirectory);
			_profileView->setGothicDirectory(_gothicDirectory);
		}
	}
	if (_settingsDialog->getLocationSettingsWidget()->isGothic2Valid(true)) {
		_modInfoView->setGothic2Directory(_gothic2Directory);
		if (Config::OnlineMode) {
			_modDatabaseView->setGothic2Directory(_gothic2Directory);
			_profileView->setGothic2Directory(_gothic2Directory);
			_spineEditor->getModel()->setPath(_gothic2Directory);
		}
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #12: " << getPRAMValue());
#endif

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS hiddenMods (ModID INT PRIMARY KEY);", err);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #13: " << getPRAMValue());
#endif

	parseMods();

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #14: " << getPRAMValue());
#endif

	_loginDialog = new LoginDialog(this);

	QMenu * fileMenu = new QMenu(QApplication::tr("File"), this);
	UPDATELANGUAGESETTITLE(fileMenu, "File");
	QMenu * toolsMenu = new QMenu(QApplication::tr("Tools"), this);
	UPDATELANGUAGESETTITLE(toolsMenu, "Tools");
	QMenu * developerMenu = new QMenu(QApplication::tr("Developer"), this);
	UPDATELANGUAGESETTITLE(developerMenu, "Developer");
	QMenu * helpMenu = new QMenu(QApplication::tr("Help"), this);
	UPDATELANGUAGESETTITLE(helpMenu, "Help");

	QAction * exportAction = fileMenu->addAction(QApplication::tr("Export"));
	UPDATELANGUAGESETTEXT(exportAction, "Export");
	connect(exportAction, SIGNAL(triggered()), this, SLOT(execExport()));

	QAction * importAction = fileMenu->addAction(QApplication::tr("Import"));
	UPDATELANGUAGESETTEXT(importAction, "Import");
	connect(importAction, SIGNAL(triggered()), this, SLOT(execImport()));

	fileMenu->addSeparator();

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #15: " << getPRAMValue());
#endif

	if (!Config::OnlineMode) {
		QAction * onlineAction = fileMenu->addAction(QApplication::tr("SwitchToOnline"));
		UPDATELANGUAGESETTEXT(onlineAction, "SwitchToOnline");
		connect(onlineAction, SIGNAL(triggered()), this, SLOT(switchToOnline()));
	} else {
		QAction * offlineAction = fileMenu->addAction(QApplication::tr("SwitchToOffline"));
		UPDATELANGUAGESETTEXT(offlineAction, "SwitchToOffline");
		connect(offlineAction, SIGNAL(triggered()), this, SLOT(switchToOffline()));
	}

	fileMenu->addSeparator();

	QAction * quitAction = fileMenu->addAction(QApplication::tr("Quit"));
	UPDATELANGUAGESETTEXT(quitAction, "Quit");
	connect(quitAction, SIGNAL(triggered()), this, SLOT(onQuit()));

#ifdef Q_OS_WIN
	QAction * installG2FromCDAction = toolsMenu->addAction(QApplication::tr("InstallGothic2FromCD"));
	UPDATELANGUAGESETTEXT(installG2FromCDAction, "InstallGothic2FromCD");
#endif
	QAction * openIniConfiguratorAction = toolsMenu->addAction(QApplication::tr("IniConfigurator"));
	UPDATELANGUAGESETTEXT(openIniConfiguratorAction, "IniConfigurator");
	if (Config::OnlineMode) {
		QAction * submitCompatibilityAction = toolsMenu->addAction(QApplication::tr("SubmitCompatibility"));
		UPDATELANGUAGESETTEXT(submitCompatibilityAction, "SubmitCompatibility");
		connect(submitCompatibilityAction, SIGNAL(triggered()), this, SLOT(submitCompatibility()));
		connect(_loginDialog, &LoginDialog::loggedIn, [submitCompatibilityAction]() {
			submitCompatibilityAction->setEnabled(!Config::Username.isEmpty());
		});
	}
	QAction * savegameEditorAction = toolsMenu->addAction(QApplication::tr("SavegameEditor"));
	UPDATELANGUAGESETTEXT(savegameEditorAction, "SavegameEditor");
	toolsMenu->addSeparator();
	QAction * settingsAction = toolsMenu->addAction(QApplication::tr("Settings"));
	settingsAction->setShortcut(QKeySequence(Qt::Key::Key_O));
	UPDATELANGUAGESETTEXT(settingsAction, "Settings");

	QAction * translationRequestAction = developerMenu->addAction(QApplication::tr("RequestTranslation"));
	UPDATELANGUAGESETTEXT(translationRequestAction, "RequestTranslation");
	translationRequestAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
	connect(translationRequestAction, SIGNAL(triggered()), this, SLOT(openTranslationRequest()));

	QAction * translatorAction = developerMenu->addAction(QApplication::tr("Translator"));
	UPDATELANGUAGESETTEXT(translatorAction, "Translator");
	translatorAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
	connect(translatorAction, SIGNAL(triggered()), this, SLOT(openTranslator()));

	_devModeAction = developerMenu->addAction(QApplication::tr("ActivateDeveloperMode"));
	UPDATELANGUAGESETTEXT(_devModeAction, "ActivateDeveloperMode");
	_devModeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	connect(_devModeAction, SIGNAL(triggered()), _settingsDialog->getDeveloperSettingsWidget(), SLOT(changedDeveloperMode()));
	_devModeAction->setCheckable(true);
	_devModeAction->setChecked(_settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive());

	_spineEditorAction = developerMenu->addAction(QApplication::tr("SpineEditor"));
	UPDATELANGUAGESETTEXT(_spineEditorAction, "SpineEditor");
	_spineEditorAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	connect(_spineEditorAction, SIGNAL(triggered()), _spineEditor, SLOT(exec()));
	_spineEditorAction->setEnabled(_settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive());
	connect(_settingsDialog->getDeveloperSettingsWidget(), SIGNAL(developerModeChanged(bool)), _spineEditorAction, SLOT(setEnabled(bool)));

	if (Config::OnlineMode) {
		QAction * managementAction = developerMenu->addAction(QApplication::tr("Management"));
		UPDATELANGUAGESETTEXT(managementAction, "Management");
		managementAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
		connect(managementAction, SIGNAL(triggered()), this, SLOT(execManagement()));
	}

	developerMenu->addSeparator();

	for (int i = 0; i < 10; i++) {
		QAction * devPathAction = developerMenu->addAction(QApplication::tr("DevPath").arg(i));
		UPDATELANGUAGESETTEXTARG(devPathAction, "DevPath", i);
		devPathAction->setShortcut(QKeySequence(Qt::CTRL + (Qt::Key_0 + i)));
		devPathAction->setProperty("id", i);
		connect(devPathAction, SIGNAL(triggered()), this, SLOT(setDevPath()));
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #16: " << getPRAMValue());
#endif

	_autoUpdateDialog = new AutoUpdateDialog(this);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #17: " << getPRAMValue());
#endif

	QAction * faqAction = helpMenu->addAction(QApplication::tr("FAQ"));
	faqAction->setToolTip(QApplication::tr("FAQTooltip"));
	UPDATELANGUAGESETTEXT(faqAction, "FAQ");
	UPDATELANGUAGESETTOOLTIP(faqAction, "FAQTooltip");

	if (Config::OnlineMode) {
		QAction * checkIntegrityAction = helpMenu->addAction(QApplication::tr("CheckIntegrity"));
		checkIntegrityAction->setToolTip(QApplication::tr("CheckIntegrityTooltip"));
		UPDATELANGUAGESETTEXT(checkIntegrityAction, "CheckIntegrity");
		UPDATELANGUAGESETTOOLTIP(checkIntegrityAction, "CheckIntegrityTooltip");
		connect(checkIntegrityAction, SIGNAL(triggered()), this, SLOT(checkIntegrity()));
	}
	QAction * generateReportAction = helpMenu->addAction(QApplication::tr("GenerateReport"));
	UPDATELANGUAGESETTEXT(generateReportAction, "GenerateReport");

	QAction * showChangelogAction = helpMenu->addAction(QApplication::tr("ShowChangelog"));
	UPDATELANGUAGESETTEXT(showChangelogAction, "ShowChangelog");

	if (Config::OnlineMode) {
		QAction * updateAction = helpMenu->addAction(QApplication::tr("CheckForUpdates"));
		UPDATELANGUAGESETTEXT(updateAction, "CheckForUpdates");
		connect(updateAction, SIGNAL(triggered()), _autoUpdateDialog, SLOT(exec()));
	}

	helpMenu->addSeparator();
	
	QAction * discordAction = helpMenu->addAction(QApplication::tr("Discord"));
	UPDATELANGUAGESETTEXT(discordAction, "Discord");
	if (Config::OnlineMode) {
		QAction * feedbackAction = helpMenu->addAction(QApplication::tr("Feedback"));
		UPDATELANGUAGESETTEXT(feedbackAction, "Feedback");
		connect(feedbackAction, SIGNAL(triggered()), _feedbackDialog, SLOT(exec()));
	}
	QAction * aboutAction = helpMenu->addAction(QApplication::tr("About"));
	UPDATELANGUAGESETTEXT(aboutAction, "About");

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #18: " << getPRAMValue());
#endif

	AboutDialog * aboutDialog = new AboutDialog(this);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #19: " << getPRAMValue());
#endif

	_changelogDialog = new ChangelogDialog(this);

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #20: " << getPRAMValue());
#endif

#ifdef Q_OS_WIN
	connect(installG2FromCDAction, SIGNAL(triggered()), _installGothic2FromCDDialog, SLOT(exec()));
#endif

	connect(openIniConfiguratorAction, SIGNAL(triggered()), this, SLOT(openIniConfigurator()));
	connect(savegameEditorAction, SIGNAL(triggered()), this, SLOT(openSavegameEditor()));
	connect(settingsAction, SIGNAL(triggered()), _settingsDialog, SLOT(exec()));

	connect(faqAction, SIGNAL(triggered()), this, SLOT(execFAQ()));
	connect(generateReportAction, &QAction::triggered, this, &MainWindow::generateReports);
	connect(showChangelogAction, SIGNAL(triggered()), _changelogDialog, SLOT(exec()));
	connect(discordAction, &QAction::triggered, []() {
		QDesktopServices::openUrl(QUrl("https://discord.gg/x4x5Mc8"));
	});
	connect(aboutAction, SIGNAL(triggered()), aboutDialog, SLOT(exec()));

	connect(_settingsDialog->getLocationSettingsWidget(), SIGNAL(pathChanged()), this, SLOT(pathChanged()));

	menuBar()->addMenu(fileMenu);
	menuBar()->addMenu(toolsMenu);
	menuBar()->addMenu(developerMenu);
	menuBar()->addMenu(helpMenu);

	if (Config::OnlineMode) {
		QMenu * loginMenu = new QMenu(QApplication::tr("Login"), this);
		UPDATELANGUAGESETTITLE(loginMenu, "Login");
		menuBar()->addMenu(loginMenu);
		connect(loginMenu, SIGNAL(aboutToShow()), _loginDialog, SLOT(execute()));
		connect(_loginDialog, &LoginDialog::loggedIn, [loginMenu]() {
			loginMenu->menuAction()->setVisible(false);
		});
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #21: " << getPRAMValue());
#endif

	if (showChangelog && !version.isEmpty()) {
		QTimer::singleShot(0, _changelogDialog, &ChangelogDialog::execStartup);
	}

	if (Config::OnlineMode) {
		_modDatabaseView->gothicValidationChanged(_settingsDialog->getLocationSettingsWidget()->isGothicValid(true));
		_modDatabaseView->gothic2ValidationChanged(_settingsDialog->getLocationSettingsWidget()->isGothic2Valid(true));
	}
#ifdef Q_OS_WIN
	installG2FromCDAction->setVisible(!_settingsDialog->getLocationSettingsWidget()->isGothic2Valid(true));
#endif

	connect(_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
	if (Config::OnlineMode) {
		connect(_settingsDialog->getLocationSettingsWidget(), SIGNAL(validGothic(bool)), _modDatabaseView, SLOT(gothicValidationChanged(bool)));
		connect(_settingsDialog->getLocationSettingsWidget(), SIGNAL(validGothic2(bool)), _modDatabaseView, SLOT(gothic2ValidationChanged(bool)));
	}
#ifdef Q_OS_WIN
	connect(_settingsDialog->getLocationSettingsWidget(), &LocationSettingsWidget::validGothic2, [=](bool b) {
		installG2FromCDAction->setVisible(!b);
	});
	connect(_installGothic2FromCDDialog, SIGNAL(updateGothic2Directory(QString)), _settingsDialog->getLocationSettingsWidget(), SLOT(setGothic2Directory(QString)));
#endif

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #22: " << getPRAMValue());
#endif

	if (Config::OnlineMode) {
		QTimer::singleShot(0, _loginDialog, &LoginDialog::exec);
	}

	connect(_loginDialog, &LoginDialog::loggedIn, _modInfoView, &ModInfoView::loginChanged);
	if (Config::OnlineMode) {
		connect(_loginDialog, &LoginDialog::loggedIn, _modDatabaseView, &ModDatabaseView::loginChanged);
		connect(_loginDialog, &LoginDialog::loggedIn, _profileView, &ProfileView::loginChanged);
		connect(_loginDialog, &LoginDialog::loggedIn, startPage, &StartPageWidget::loginChanged);
		connect(_loginDialog, &LoginDialog::loggedIn, _feedbackDialog, &FeedbackDialog::loginChanged);
		connect(_loginDialog, &LoginDialog::loggedIn, _modInfoPage, &ModInfoPage::loginChanged);
		connect(_loginDialog, &LoginDialog::loggedIn, _friendsView, &FriendsView::loginChanged);
	}

	if (Config::OnlineMode) {
		connect(_profileView, SIGNAL(triggerLogout()), _loginDialog, SLOT(logout()));
	}

	if (Config::OnlineMode) {
		const bool checkForUpdate = Config::IniParser->value("MISC/checkForUpdate", true).toBool();
		if (checkForUpdate) {
			QTimer::singleShot(0, _autoUpdateDialog, &AutoUpdateDialog::checkForUpdate);
		}
	}

	if (Config::OnlineMode) {
		_modUpdateDialog = new ModUpdateDialog(this);
		connect(_loginDialog, &LoginDialog::loggedIn, _modUpdateDialog, &ModUpdateDialog::loginChanged);

		_modUpdateDialog->loginChanged();

		connect(_settingsDialog->getGeneralSettingsWidget(), SIGNAL(resetModUpdates()), _modUpdateDialog, SLOT(checkForUpdate()));

		connect(_modUpdateDialog, &ModUpdateDialog::updatedMod, _modInfoView, &ModInfoView::updatedMod);
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage MainWindow c'tor #23: " << getPRAMValue());
#endif
}

MainWindow::~MainWindow() {
	saveSettings();
	delete _autoUpdateDialog;
	delete _changelogDialog;
	delete _modUpdateDialog;
#ifdef Q_OS_WIN
	delete _installGothic2FromCDDialog;
#endif
	delete _feedbackDialog;
}

void MainWindow::selectedMod(const QModelIndex & index) {
	const auto idx = _sortModel->mapToSource(index);
	const QString modID = _modListModel->data(idx, LibraryFilterModel::ModIDRole).toString();
	const QString iniFile = _modListModel->data(idx, LibraryFilterModel::IniFileRole).toString();
	_modInfoView->selectMod(modID, iniFile);
}

void MainWindow::pathChanged() {
	if (!_developerModeActive) {
		LocationSettingsWidget * locationSettingsWidget = _settingsDialog->getLocationSettingsWidget();
		_gothicDirectory = locationSettingsWidget->getGothicDirectory();
		_gothic2Directory = locationSettingsWidget->getGothic2Directory();
		if (locationSettingsWidget->isGothicValid(false)) {
			if (_modInfoView) {
				_modInfoView->setGothicDirectory(_gothicDirectory);
			}
			if (_modDatabaseView) {
				_modDatabaseView->setGothicDirectory(_gothicDirectory);
			}
			if (_profileView) {
				_profileView->setGothicDirectory(_gothicDirectory);
			}
		}
		if (locationSettingsWidget->isGothic2Valid(false)) {
			if (_modInfoView) {
				_modInfoView->setGothic2Directory(_gothic2Directory);
			}
			if (_modDatabaseView) {
				_modDatabaseView->setGothic2Directory(_gothic2Directory);
			}
			if (_profileView) {
				_profileView->setGothic2Directory(_gothic2Directory);
			}
		}
	}

	_modListModel->clear();
	parseMods();
}

void MainWindow::tabChanged(int index) {
	if (Config::OnlineMode) {
		if (index == MainTabsOnline::Database) {
			_modDatabaseView->updateModList(-1);
			_profileView->reset();
		} else if (index == MainTabsOnline::Profile) {
			_profileView->updateList();
		} else if (index == MainTabsOnline::LibraryOnline) {
			_modListModel->clear();
			parseMods();
			_profileView->reset();
		} else if (index == MainTabsOnline::Friends) {
			_friendsView->updateFriendList();
		} else {
			_profileView->reset();
		}
	} else {
		if (index == MainTabsOffline::LibraryOffline) {
			_modListModel->clear();
			parseMods();
		}
	}
}

void MainWindow::setDeveloperMode(bool devMode) {
	_developerModeActive = devMode;
	_devModeAction->setChecked(devMode);
	pathChanged();
}

void MainWindow::checkIntegrity() {
	IntegrityCheckDialog dlg(this, this);
	if (_settingsDialog->getLocationSettingsWidget()->isGothicValid(true)) {
		dlg.setGothicDirectory(_gothicDirectory);
	}
	if (_settingsDialog->getLocationSettingsWidget()->isGothic2Valid(true)) {
		dlg.setGothic2Directory(_gothic2Directory);
	}
	if (dlg.exec() == QDialog::Accepted) {
		QList<IntegrityCheckDialog::ModFile> corruptFiles = dlg.getCorruptFiles();
		QList<IntegrityCheckDialog::ModFile> corruptGothicFiles = dlg.getCorruptGothicFiles();
		QList<IntegrityCheckDialog::ModFile> corruptGothic2Files = dlg.getCorruptGothic2Files();
		if (corruptFiles.empty() && corruptGothicFiles.empty() && corruptGothic2Files.empty()) {
			QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("CheckIntegrity"), QApplication::tr("CheckIntegritySuccess"), QMessageBox::StandardButton::Ok);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			if (QMessageBox::StandardButton::Ok == msg.exec()) {
			}
		} else {
			QString text = QApplication::tr("CheckIntegrityFailed") + "\n" + QApplication::tr("CorruptedFiles") + ":\n\n";
			for (const auto & mf : corruptFiles) {
				text += QFileInfo(mf.file).fileName() + "\n";
			}
			for (const auto & mf : corruptGothicFiles) {
				text += QFileInfo(mf.file).fileName() + "\n";
			}
			for (const auto & mf : corruptGothic2Files) {
				text += QFileInfo(mf.file).fileName() + "\n";
			}
			text.resize(text.size() - 1);
			QMessageBox msg(QMessageBox::Icon::Critical, QApplication::tr("CheckIntegrity"), text, QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
			msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			if (QMessageBox::StandardButton::Ok == msg.exec()) {
				MultiFileDownloader * mfd = new MultiFileDownloader(this);
				connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
				connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
				common::RequestOriginalFilesMessage rofm;
				for (const IntegrityCheckDialog::ModFile & file : corruptFiles) {
					rofm.files.emplace_back(file.modID, file.file.toStdString());
				}
				std::string serialized = rofm.SerializePublic();
				clockUtils::sockets::TcpSocket sock;
				if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
					if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
						if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
							try {
								common::Message * m = common::Message::DeserializePublic(serialized);
								if (m) {
									common::SendOriginalFilesMessage * sofm = dynamic_cast<common::SendOriginalFilesMessage *>(m);
									corruptFiles.clear();
									for (const auto & p : sofm->files) {
										corruptFiles.append(IntegrityCheckDialog::ModFile(std::to_string(p.first), p.second.first, p.second.second));
									}
								}
								delete m;
							} catch (...) {
								return;
							}
						}
					}
				}
				for (const IntegrityCheckDialog::ModFile & file : corruptFiles) {
					QFileInfo fi(file.file);
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/mods/" + QString::number(file.modID) + "/" + file.file), Config::MODDIR + "/mods/" + QString::number(file.modID) + "/" + fi.path(), fi.fileName(), file.hash, mfd);
					mfd->addFileDownloader(fd);
				}
				for (const IntegrityCheckDialog::ModFile & file : corruptGothicFiles) {
					QFileInfo fi(file.file);
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g1/" + file.file), _gothicDirectory + "/" + fi.path(), fi.fileName(), file.hash, mfd);
					mfd->addFileDownloader(fd);
				}
				for (const IntegrityCheckDialog::ModFile & file : corruptGothic2Files) {
					QFileInfo fi(file.file);
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/g2/" + file.file), _gothic2Directory + "/" + fi.path(), fi.fileName(), file.hash, mfd);
					mfd->addFileDownloader(fd);
				}
				DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, this);
				progressDlg.setCancelButton(nullptr);
				progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				progressDlg.exec();
				if (progressDlg.hasDownloadSucceeded()) {
					Database::DBError err;
					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
					for (IntegrityCheckDialog::ModFile file : corruptFiles) {
						QFileInfo fi(file.file);
						if (fi.suffix() == "z") {
							file.file = file.file.mid(0, file.file.size() - 2);
						}
						Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE modfiles SET Hash = '" + file.hash.toStdString() + "' WHERE ModID = " + std::to_string(file.modID) + " AND File = '" + file.file.toStdString() + "';", err);
					}
					Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
					QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("CheckIntegrity"), QApplication::tr("IntegrityRepairSuccess"), QMessageBox::StandardButton::Ok);
					resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
					resultMsg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
					resultMsg.exec();
				} else {
					QMessageBox resultMsg(QMessageBox::Icon::Critical, QApplication::tr("CheckIntegrity"), QApplication::tr("IntegrityRepairFailure"), QMessageBox::StandardButton::Ok);
					resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
					resultMsg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
					resultMsg.exec();
				}
			}
		}
	}
}

void MainWindow::openSpecialProfileView() {
	_tabWidget->setCurrentWidget(_profileView);
}

void MainWindow::changeToInfoTab() {
	_tabWidget->setCurrentWidget(_modInfoPage);
}

void MainWindow::openIniConfigurator() {
	IniConfigurator cfg(_gothicDirectory, _gothic2Directory, Config::IniParser, this);
	cfg.exec();
}

void MainWindow::setDevPath() {
	QAction * action = qobject_cast<QAction *>(sender());
	const int id = action->property("id").toInt();
	const QString path = _settingsDialog->getDeveloperSettingsWidget()->getPath(id);
	const common::GameType gv = _settingsDialog->getDeveloperSettingsWidget()->getGothicVersion(id);
	if (path.isEmpty()) {
		return;
	}
	if (!_developerModeActive) {
		_devModeAction->trigger();
	}
	_gothicDirectory = (gv == common::GameType::Gothic) ? path : "";
	_gothic2Directory = (gv == common::GameType::Gothic2) ? path : "";
	_modInfoView->setGothicDirectory(_gothicDirectory);
	_modInfoView->setGothic2Directory(_gothic2Directory);
	if (Config::OnlineMode) {
		_modDatabaseView->setGothicDirectory(_gothicDirectory);
		_profileView->setGothicDirectory(_gothicDirectory);
		_modDatabaseView->setGothic2Directory(_gothic2Directory);
		_profileView->setGothic2Directory(_gothic2Directory);
	}

	_spineEditor->getModel()->setPath(!_gothicDirectory.isEmpty() ? _gothicDirectory : _gothic2Directory);
	_spineEditor->getModel()->setGothicVersion(gv);

	_modListModel->clear();
	parseMods();
}

void MainWindow::submitCompatibility() {
	if (!Config::Username.isEmpty()) {
		SubmitCompatibilityDialog dlg;
		dlg.exec();
	}
}

void MainWindow::triggerModStart(int modID, QString iniFile) {
	_modInfoView->selectMod(QString::number(modID), iniFile);
	_modInfoView->start();
}

void MainWindow::execFAQ() {
	FAQDialog dlg(this);
	dlg.exec();
}

void MainWindow::execExport() {
	ExportDialog dlg(this);
	dlg.exec();
}

void MainWindow::execImport() {
	ImportDialog dlg(this);
	dlg.exec();
}

void MainWindow::openSavegameEditor() {
	SavegameDialog dlg(_settingsDialog->getLocationSettingsWidget(), this);
	dlg.exec();
}

void MainWindow::execManagement() {
	client::widgets::ManagementDialog dlg(this);
	connect(&dlg, &client::widgets::ManagementDialog::triggerInfoPage, _modInfoPage, &ModInfoPage::loadPage);
	connect(&dlg, &client::widgets::ManagementDialog::triggerInfoPage, this, &MainWindow::changeToInfoTab);
	connect(&dlg, &client::widgets::ManagementDialog::triggerInfoPage, _modInfoPage, &ModInfoPage::forceEditPage);
	connect(&dlg, SIGNAL(checkForUpdate(int32_t)), _modUpdateDialog, SLOT(checkForUpdate(int32_t)));
	dlg.exec();
}

void MainWindow::switchToOnline() {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "DELETE FROM sync;", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + OFFLINE_DATABASE, "INSERT INTO sync (Enabled) VALUES (1);", err);
	Config::OnlineMode = true;
	changedOnlineMode();
}

void MainWindow::switchToOffline() {
	Config::OnlineMode = false;
	changedOnlineMode();
}

void MainWindow::hideMod() {
	const QModelIndexList idxList = _modListView->selectionModel()->selectedIndexes();
	if (idxList.empty()) {
		return;
	}
	const QModelIndex idx = idxList.constFirst();
	const int modId = idx.data(LibraryFilterModel::ModIDRole).toInt();
	_modListView->model()->setData(idx, true, LibraryFilterModel::HiddenRole);
	_modListView->clearSelection();
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO hiddenMods (ModID) VALUES (" + std::to_string(modId) + ");", err);
}

void MainWindow::showMod() {
	const QModelIndexList idxList = _modListView->selectionModel()->selectedIndexes();
	if (idxList.empty()) {
		return;
	}
	const QModelIndex idx = idxList.constFirst();
	const int modId = idx.data(LibraryFilterModel::ModIDRole).toInt();
	_modListView->model()->setData(idx, false, LibraryFilterModel::HiddenRole);
	_modListView->clearSelection();
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM hiddenMods WHERE ModID = " + std::to_string(modId) + ";", err);
}

void MainWindow::uninstallMod() {
	const QModelIndexList idxList = _modListView->selectionModel()->selectedIndexes();
	if (idxList.empty()) {
		return;
	}
	const QModelIndex idx = idxList.constFirst();
	const int modId = idx.data(LibraryFilterModel::ModIDRole).toInt();
	const common::GameType gothicVersion = common::GameType(idx.data(LibraryFilterModel::GothicRole).toInt());
	const QString directory = gothicVersion == common::GameType::Gothic ? _gothicDirectory : _gothic2Directory;
	_modListView->clearSelection();

	const bool uninstalled = client::Uninstaller::uninstall(modId, idx.data(Qt::DisplayRole).toString(), directory);
	if (uninstalled) {
		_modListModel->clear();
		parseMods();
	}
}

void MainWindow::openTranslator() {
#ifdef WITH_TRANSLATOR
	TranslatorDialog dlg(this);
	dlg.exec();
#endif
}

void MainWindow::openTranslationRequest() {
#ifdef WITH_TRANSLATOR
	TranslationRequestDialog dlg(this);
	dlg.exec();
#endif
}

void MainWindow::generateReports() {
	if (!_gothicDirectory.isEmpty()) {
		ReportGenerator rg;
		rg.generateReport(_gothicDirectory, "Report_G1");
	}
	if (!_gothic2Directory.isEmpty()) {
		ReportGenerator rg;
		rg.generateReport(_gothic2Directory, "Report_G2");
	}
}

bool MainWindow::onQuit() {
	QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyQuit"), QApplication::tr("ReallyQuitDescription"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
	msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
	msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
	if (GeneralSettingsWidget::skipExitCheckbox || QMessageBox::StandardButton::Ok == msg.exec()) {
		qApp->quit();
		return true;
	}
	return false;
}

void MainWindow::findGothic() {
	{
		QDialog dlg;
		dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		QVBoxLayout * l = new QVBoxLayout();
		QLabel * descriptionLabel = new QLabel(QApplication::tr("SetGothicPathText"), &dlg);
		descriptionLabel->setWordWrap(true);
		LocationSettingsWidget * lsw = new LocationSettingsWidget(true, &dlg);
		QDialogButtonBox * db = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok, &dlg);
		QPushButton * pb = db->button(QDialogButtonBox::StandardButton::Ok);
		connect(pb, SIGNAL(clicked()), &dlg, SLOT(accept()));
		connect(pb, SIGNAL(clicked()), &dlg, SLOT(hide()));
		l->addWidget(descriptionLabel);
		l->addWidget(lsw);
		l->addWidget(db);
		dlg.setLayout(l);

		const QSettings steamSettings(R"(HKEY_CURRENT_USER\SOFTWARE\Valve\Steam)", QSettings::NativeFormat);
		if (steamSettings.value("SteamPath").isValid()) {
			const QString steamDir = steamSettings.value("SteamPath").toString();
			const QString gothicDir = steamDir + "/SteamApps/common/Gothic";
			if (QFileInfo::exists(gothicDir + "/System/Gothic.exe")) {
				lsw->setGothicDirectory(gothicDir);
			}
			const QString gothic2Dir = steamDir + "/SteamApps/common/Gothic II";
			if (QFileInfo::exists(gothic2Dir + "/System/Gothic2.exe")) {
				lsw->setGothic2Directory(gothic2Dir);
			}
		}

		dlg.exec();
		lsw->saveSettings();
		_gothicDirectory = lsw->getGothicDirectory();
		_gothic2Directory = lsw->getGothic2Directory();
		_settingsDialog->getLocationSettingsWidget()->setGothicDirectory(_gothicDirectory);
		_settingsDialog->getLocationSettingsWidget()->setGothic2Directory(_gothic2Directory);
		return;
	}
}

void MainWindow::parseMods() {
	if (Config::extendedLogging) {
		LOGINFO("Parsing Mods");
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage parseMods #1: " << getPRAMValue());
#endif
	_parsedInis.clear();
	if (!_developerModeActive) {
		if (Config::extendedLogging) {
			LOGINFO("Parsing Installed Mods");
		}
		parseInstalledMods();
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage parseMods #2: " << getPRAMValue());
#endif
	if (!_gothicDirectory.isEmpty()) {
		if (Config::extendedLogging) {
			LOGINFO("Parsing Mods in Gothic 1 Folder");
		}
		parseMods(_gothicDirectory);
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage parseMods #3: " << getPRAMValue());
#endif
	if (!_gothic2Directory.isEmpty()) {
		if (Config::extendedLogging) {
			LOGINFO("Parsing Mods in Gothic 2 Folder");
		}
		parseMods(_gothic2Directory);
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage parseMods #4: " << getPRAMValue());
#endif
	_sortModel->sort(0);

	Database::DBError err;
	std::vector<std::vector<std::string>> vec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "SELECT ModID, Ini FROM lastPlayed LIMIT 1;", err);
	
	if (!vec.empty()) {
		for (int i = 0; i < _sortModel->rowCount(); i++) {
			QModelIndex idx = _sortModel->index(i, 0);
			const QString modID = QString::fromStdString(vec[0][0]);
			const QString iniFile = QString::fromStdString(vec[0][1]);
			if (idx.data(LibraryFilterModel::ModIDRole).toInt() == std::stoi(vec[0][0]) && idx.data(LibraryFilterModel::IniFileRole).toString().contains(iniFile)) {
				_modListView->selectionModel()->select(idx, QItemSelectionModel::SelectionFlag::SelectCurrent);
				_modInfoView->selectMod(modID, iniFile);
				_modListView->scrollTo(idx);
				break;
			}
		}
	}

#ifdef Q_OS_WIN
	LOGINFO("Memory Usage parseMods #5: " << getPRAMValue());
#endif
}

void MainWindow::parseMods(QString baseDir) {
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
				QFile(baseDir + "/System/SpineAPI.dll").remove();
				QFile(baseDir + "/System/m2etis.dll").remove();
				QFile(baseDir + "/Data/Spine.vdf").remove();
				continue;
			}
		}

		QSettings iniParser(s, QSettings::IniFormat);
		QString title = iniParser.value("INFO/Title", "").toString();

		if (title.isEmpty()) continue;

		const QString icon = iniParser.value("INFO/Icon", "").toString();
		QPixmap pixmap(baseDir + "/System/" + icon);
		if (pixmap.isNull()) {
			if (QFileInfo::exists(baseDir + "/System/Gothic.exe")) {
				static QPixmap p1 = QPixmap::fromImage(QImage(":Gothic.ico"));
				pixmap = p1;
			} else if (QFileInfo::exists(baseDir + "/System/Gothic2.exe")) {					
				static QPixmap p2 = QPixmap::fromImage(QImage(":Gothic2.ico"));
				pixmap = p2;
			}
		}
		pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
		while (title.startsWith(' ') || title.startsWith('\t')) {
			title = title.remove(0, 1);
		}
		QStandardItem * item = new QStandardItem(QIcon(pixmap), title);
		item->setData(s, LibraryFilterModel::IniFileRole);
		item->setData(false, LibraryFilterModel::InstalledRole);
		item->setData(false, LibraryFilterModel::HiddenRole);
		item->setEditable(false);
		if (QFileInfo::exists(baseDir + "/System/Gothic.exe")) {
			item->setData(int(common::GameType::Gothic), LibraryFilterModel::GothicRole);
		} else if (QFileInfo::exists(baseDir + "/System/Gothic2.exe")) {
			item->setData(int(common::GameType::Gothic2), LibraryFilterModel::GothicRole);
		}
		_modListModel->appendRow(item);
	}
}

void MainWindow::parseInstalledMods() {
	if (Config::extendedLogging) {
		LOGINFO("Checking Files in " << Config::MODDIR.toStdString());
	}
	QDirIterator it(Config::MODDIR + "/mods", QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
	QStringList files;
	while (it.hasNext()) {
		it.next();
		QString fileName = it.filePath();
		if (!fileName.isEmpty()) {
			files.append(fileName);
		}
	}
	for (const QString & s : files) {
		QSettings iniParser(s, QSettings::IniFormat);
		QString title = iniParser.value("INFO/Title", "").toString();
		if (title.isEmpty()) {
			if (Config::extendedLogging) {
				LOGINFO("No title set in " << s.toStdString());
			}
			continue;
		}
		const QString icon = iniParser.value("INFO/Icon", "").toString();
		QFileInfo fi(s);
		const QString iconPath = fi.absolutePath() + "/" + icon;
		QPixmap pixmap(iconPath);
		QString modID = fi.absolutePath();
		QDir md(Config::MODDIR + "/mods");
		modID.replace(md.absolutePath(), "");
		modID = modID.split("/", QString::SplitBehavior::SkipEmptyParts).front();
		Database::DBError err;
		common::GameType mid;
		bool found;
		if (!Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + modID.toStdString() + " LIMIT 1;", err).empty()) {
			mid = common::GameType(Database::queryNth<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + modID.toStdString() + " LIMIT 1;", err, 0).front());
			found = true;
		} else {
			if (Config::extendedLogging) {
				LOGINFO("Mod installed, but not in database");
			}
			continue;
		}
		if (pixmap.isNull()) {
			if (found) {
				if (mid == common::GameType::Gothic && QFileInfo::exists(_gothicDirectory + "/System/Gothic.exe")) {
					static QPixmap p1 = QPixmap::fromImage(QImage(":Gothic.ico"));
					pixmap = p1;
				} else if (mid == common::GameType::Gothic2 && QFileInfo::exists(_gothic2Directory + "/System/Gothic2.exe")) {
					static QPixmap p2 = QPixmap::fromImage(QImage(":Gothic2.ico"));
					pixmap = p2;
				}
			}
		}
		pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
		while (title.startsWith(' ') || title.startsWith('\t')) {
			title = title.remove(0, 1);
		}
		QStandardItem * item = new QStandardItem(QIcon(pixmap), title);
		item->setData(s, LibraryFilterModel::IniFileRole);
		item->setData(true, LibraryFilterModel::InstalledRole);
		item->setData(modID.toInt(), LibraryFilterModel::ModIDRole);
		item->setData(int(mid), LibraryFilterModel::GothicRole);
		if (!Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM hiddenMods WHERE ModID = " + modID.toStdString() + " LIMIT 1;", err).empty()) {
			item->setData(true, LibraryFilterModel::HiddenRole);
		} else {
			item->setData(false, LibraryFilterModel::HiddenRole);
		}
		item->setEditable(false);
		_modListModel->appendRow(item);

		QString hashSum;
		const bool b = utils::Hashing::hash(s, hashSum);
		if (b) {
			_parsedInis.insert(QFileInfo(s).fileName(), std::make_tuple(hashSum, modID.toInt()));
		}
		if (Config::extendedLogging) {
			LOGINFO("Listing Mod: " << title.toStdString());
		}
	}
}

void MainWindow::restoreSettings() {
	Config::IniParser->beginGroup("WINDOWGEOMETRY");
	QByteArray arr = Config::IniParser->value("MainWindowGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("MainWindowGeometry");
	}
	arr = Config::IniParser->value("MainWindowState", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("MainWindowState");
	}
	Config::IniParser->endGroup();
}

void MainWindow::saveSettings() {
	Config::IniParser->beginGroup("WINDOWGEOMETRY");
	Config::IniParser->setValue("MainWindowGeometry", saveGeometry());
	Config::IniParser->setValue("MainWindowState", saveState());
	Config::IniParser->endGroup();
}

void MainWindow::changedOnlineMode() {
	Config::IniParser->setValue("MISC/OnlineMode", Config::OnlineMode);
	const QString exeFileName = qApp->applicationDirPath() + "/" + qApp->applicationName();
#ifdef Q_OS_WIN
	const int result = int(::ShellExecuteA(0, "runas", exeFileName.toUtf8().constData(), 0, 0, SW_SHOWNORMAL));
	if (result > 32) { // no error
		qApp->quit();
	}
#endif
}

void MainWindow::closeEvent(QCloseEvent * evt) {
	if (onQuit()) {
		QMainWindow::closeEvent(evt);
	} else {
		evt->ignore();
	}
}
