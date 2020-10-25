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

#include "InstallMode.h"
#include "LibraryFilterModel.h"
#include "ReportGenerator.h"
#include "SpineConfig.h"
#include "Uninstaller.h"

#include "common/MessageStructs.h"

#include "discord/DiscordManager.h"

#include "gui/DownloadQueueWidget.h"

#include "launcher/LauncherFactory.h"

#include "models/SpineEditorModel.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/DownloadQueue.h"
#include "utils/FileDownloader.h"
#include "utils/MultiFileDownloader.h"

#include "widgets/AboutDialog.h"
#include "widgets/AutoUpdateDialog.h"
#include "widgets/ChangelogDialog.h"
#include "widgets/DeveloperSettingsWidget.h"
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
#include "widgets/SpineLevelRankingWidget.h"
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
using namespace spine::client;
using namespace spine::discord;
using namespace spine::gui;
using namespace spine::launcher;
using namespace spine::translator;
using namespace spine::utils;
using namespace spine::widgets;

enum MainTabsOnline {
	StartOnline,
	Info,
	Database,
	LibraryOnline,
	Downloads,
	Profile,
	Friends,
	SpineLevelRanking
};

enum MainTabsOffline {
	StartOffline,
	LibraryOffline
};

MainWindow * MainWindow::instance = nullptr;

MainWindow::MainWindow(bool showChangelog, QMainWindow * par) : QMainWindow(par), _modListView(nullptr), _modInfoView(nullptr), _profileView(nullptr), _friendsView(nullptr), _spineLevelRankingWidget(nullptr), _settingsDialog(nullptr), _autoUpdateDialog(), _changelogDialog(nullptr), _modListModel(nullptr), _loginDialog(nullptr), _modUpdateDialog(nullptr), _installGothic2FromCDDialog(nullptr), _feedbackDialog(nullptr), _developerModeActive(false), _devModeAction(nullptr), _modDatabaseView(nullptr), _tabWidget(nullptr), _spineEditorAction(nullptr), _spineEditor(nullptr), _modInfoPage(nullptr) {
	instance = this;

	LOGINFO(QSslSocket::supportsSsl() << q2s(QSslSocket::sslLibraryBuildVersionString()) << q2s(QSslSocket::sslLibraryVersionString()))

	connect(DiscordManager::instance(), &DiscordManager::connected, []() {
		DiscordManager::instance()->updatePresence(QApplication::tr("Browsing"), "");
	});

	_downloadQueue = new DownloadQueue();

	if (!Config::IniParser->contains("INSTALLATION/DirectX")) {
		// so far Spine automatically install DirectX during installation, but to enforce reinstall on download of e.g. renderer, this can manually set to false in the ini
		Config::IniParser->setValue("INSTALLATION/DirectX", false);
	}

	setWindowIcon(QIcon(":/Spine.ico"));
	_settingsDialog = new SettingsDialog(this); // create at first

#ifdef Q_OS_WIN
	_installGothic2FromCDDialog = new InstallGothic2FromCDDialog();
#endif

	_feedbackDialog = new FeedbackDialog(-1, FeedbackDialog::Type::Spine, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	restoreSettings();

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
					ustm.dayOfTheWeek = static_cast<int16_t>(QDate::currentDate().dayOfWeek());
					ustm.hour = static_cast<int16_t>(QTime::currentTime().hour());
					const std::string serialized = ustm.SerializePublic();
					sock.writePacket(serialized);
				}
			});
		}
	}

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
	
	setProperty("default", true);

	_tabWidget = new QTabWidget(this);
	_tabWidget->setProperty("library", true);	
	_tabWidget->setProperty("default", true);

	StartPageWidget * startPage = new StartPageWidget(_tabWidget);
	connect(_settingsDialog->getGeneralSettingsWidget(), &GeneralSettingsWidget::languageChanged, startPage, &StartPageWidget::setLanguage);

	_tabWidget->addTab(startPage, QApplication::tr("Startpage"));
	UPDATELANGUAGESETTABTEXT(_tabWidget, Config::OnlineMode ? static_cast<int>(MainTabsOnline::StartOnline) : static_cast<int>(MainTabsOffline::StartOffline), "Startpage");

	if (Config::OnlineMode) {
		_modInfoPage = new ModInfoPage(this, this);
		_tabWidget->addTab(_modInfoPage, QApplication::tr("InfoPage"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Info, "InfoPage");
	}

	if (Config::OnlineMode) {
		_modDatabaseView = new ModDatabaseView(this, _settingsDialog->getGeneralSettingsWidget(), _tabWidget);
		_tabWidget->addTab(_modDatabaseView, QApplication::tr("Database"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Database, "Database");
	}

	if (Config::OnlineMode) {
		connect(startPage, &StartPageWidget::tryInstallMod, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int, InstallMode)>(&ModDatabaseView::updateModList));
		connect(_modInfoPage, &ModInfoPage::tryInstallMod, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int, InstallMode)>(&ModDatabaseView::updateModList));
		connect(_modInfoPage, &ModInfoPage::tryInstallPackage, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int, InstallMode)>(&ModDatabaseView::updateModList));
		connect(_modDatabaseView, &ModDatabaseView::loadPage, _modInfoPage, &ModInfoPage::loadPage);
		connect(_modDatabaseView, &ModDatabaseView::loadPage, this, &MainWindow::changeToInfoTab);

		connect(_modDatabaseView, &ModDatabaseView::finishedInstallation, startPage, &StartPageWidget::finishedInstallation);
		connect(_modDatabaseView, &ModDatabaseView::finishedInstallation, _modInfoPage, &ModInfoPage::finishedInstallation);
		connect(_modDatabaseView, &ModDatabaseView::finishedInstallation, LauncherFactory::getInstance(), &LauncherFactory::finishedInstallation);

		connect(_modInfoPage, &ModInfoPage::triggerModStart, this, &MainWindow::triggerModStart);

		connect(startPage, &StartPageWidget::showInfoPage, _modInfoPage, &ModInfoPage::loadPage);
		connect(startPage, &StartPageWidget::showInfoPage, [this]() {
			_tabWidget->setCurrentIndex(MainTabsOnline::Info);
		});
	}
	connect(startPage, &StartPageWidget::triggerModStart, this, &MainWindow::triggerModStart);

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
			connect(cb1, &QCheckBox::stateChanged, _sortModel, &LibraryFilterModel::gothicChanged);

			QCheckBox * cb2 = new QCheckBox(QApplication::tr("Gothic2"), filterWidget);
			cb2->setProperty("library", true);
			cb2->setChecked(_sortModel->isGothic2Active());
			UPDATELANGUAGESETTEXT(cb2, "Gothic2");
			connect(cb2, &QCheckBox::stateChanged, _sortModel, &LibraryFilterModel::gothic2Changed);

			QCheckBox * cb3 = new QCheckBox(QApplication::tr("GothicAndGothic2"), filterWidget);
			cb3->setProperty("library", true);
			cb3->setChecked(_sortModel->isGothicAndGothic2Active());
			UPDATELANGUAGESETTEXT(cb3, "GothicAndGothic2");
			connect(cb3, &QCheckBox::stateChanged, _sortModel, &LibraryFilterModel::gothicAndGothic2Changed);
			cb3->hide();

			QCheckBox * cb4 = new QCheckBox(QApplication::tr("Game"), filterWidget);
			cb4->setProperty("library", true);
			cb4->setChecked(_sortModel->isGameActive());
			UPDATELANGUAGESETTEXT(cb4, "Game");
			connect(cb4, &QCheckBox::stateChanged, _sortModel, &LibraryFilterModel::gameChanged);

			vbl->addWidget(cb1);
			vbl->addWidget(cb2);
			vbl->addWidget(cb3);
			vbl->addWidget(cb4);

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
			connect(cb1, &QCheckBox::stateChanged, _sortModel, &LibraryFilterModel::showHidden);

			vbl->addWidget(cb1);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}
		filterWidget->setLayout(hl);
		l->addWidget(filterWidget);
	}

	connect(_modListView, &LibraryListView::clicked, this, &MainWindow::selectedMod);
	connect(_modListView, &LibraryListView::doubleClicked, this, &MainWindow::selectedMod);

	_modInfoView = new ModInfoView(_settingsDialog->getGeneralSettingsWidget(), topWidget);
	_modInfoView->setProperty("library", true);

	const auto devEnabled = Config::IniParser->value("DEVELOPER/Enabled", false).toBool();

	if (devEnabled) {
		_modInfoView->setDeveloperMode(_settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive());
		_modInfoView->setZSpyActivated(_settingsDialog->getDeveloperSettingsWidget()->isZSpyActive());
		_modInfoView->setShowAchievements(_settingsDialog->getGameSettingsWidget()->getShowAchievements());
		
		_developerModeActive = _settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive();
		
		connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::developerModeChanged, _modInfoView, &ModInfoView::setDeveloperMode);
		connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::zSpyChanged, _modInfoView, &ModInfoView::setZSpyActivated);
		connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::developerModeChanged, this, &MainWindow::setDeveloperMode);
	}

	connect(_modListView, &LibraryListView::doubleClicked, _modInfoView, &ModInfoView::start);
	connect(_settingsDialog->getGameSettingsWidget(), &GameSettingsWidget::showAchievementsChanged, _modInfoView, &ModInfoView::setShowAchievements);

	_modListView->setFixedWidth(400);

	topLayout->addWidget(_modListView);

	QVBoxLayout * vbl = new QVBoxLayout();
	vbl->addWidget(_modInfoView);

	topLayout->addLayout(vbl);

	topWidget->setLayout(topLayout);
	l->addWidget(topWidget);

	w->setLayout(l);

	_tabWidget->addTab(w, QApplication::tr("Library"));
	UPDATELANGUAGESETTABTEXT(_tabWidget, Config::OnlineMode ? static_cast<int>(MainTabsOnline::LibraryOnline) : static_cast<int>(MainTabsOffline::LibraryOffline), "Library");

	if (Config::OnlineMode) {
		auto * dqw = new DownloadQueueWidget(_tabWidget);

		_tabWidget->addTab(dqw, QApplication::tr("Downloads"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Downloads, "Downloads");
		
		_profileView = new ProfileView(this, _settingsDialog->getGeneralSettingsWidget(), _tabWidget);

		_tabWidget->addTab(_profileView, QApplication::tr("Profile"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Profile, "Profile");

		_friendsView = new FriendsView(_tabWidget);

		_tabWidget->addTab(_friendsView, QApplication::tr("Friends"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::Friends, "Friends");

		connect(_friendsView, &FriendsView::receivedFriends, [this](std::vector<common::Friend>, std::vector<common::Friend> friendRequests) {
			if (friendRequests.empty()) {
				_tabWidget->setTabText(MainTabsOnline::Friends, QApplication::tr("Friends"));
			} else {
				_tabWidget->setTabText(MainTabsOnline::Friends, QString("%1 (%2 %3)").arg(QApplication::tr("Friends")).arg(friendRequests.size()).arg(QApplication::tr(friendRequests.size() == 1 ? "Request" : "Requests")));
			}
		});

		_spineLevelRankingWidget = new SpineLevelRankingWidget(_tabWidget);

		_tabWidget->addTab(_spineLevelRankingWidget, QApplication::tr("Ranking"));
		UPDATELANGUAGESETTABTEXT(_tabWidget, MainTabsOnline::SpineLevelRanking, "Ranking");

		connect(_modInfoView, &ModInfoView::installMod, _modDatabaseView, static_cast<void(ModDatabaseView::*)(int, int, InstallMode)>(&ModDatabaseView::updateModList));

		connect(GeneralSettingsWidget::getInstance(), &GeneralSettingsWidget::languageChanged, dqw, [dqw]() {
			dqw->setTitle(QApplication::tr("Downloads"));
		});
	}

	_tabWidget->setCurrentWidget(startPage);

	QWidget * cw = new QWidget(this);

	QVBoxLayout * vl = new QVBoxLayout();

	{
		QHBoxLayout * hl = new QHBoxLayout();

		QPushButton * donateButton = new QPushButton(cw);
		QPixmap pixmap(":/donate.png");
		donateButton->setIconSize(pixmap.rect().size());
		donateButton->setFixedSize(pixmap.rect().size());
		donateButton->setProperty("donateButton", true);

		hl->addStretch(1);

		QLabel * donateLabel = new QLabel(QApplication::tr("SupportUsText"), cw);
		UPDATELANGUAGESETTEXT(donateLabel, "SupportUsText");

		donateLabel->setProperty("donateText", true);
		
		hl->addWidget(donateLabel);
		hl->addWidget(donateButton);
		
		vl->addLayout(hl);

		connect(donateButton, &QPushButton::released, []() {
			QDesktopServices::openUrl(QUrl("https://paypal.me/ClockworkOrigins"));
		});
	}	

	vl->addWidget(_tabWidget);

	cw->setLayout(vl);

	setCentralWidget(cw);

	if (Config::OnlineMode) {
		connect(_modInfoView, &ModInfoView::openAchievementView, _profileView, &ProfileView::openAchievementView);
		connect(_modInfoView, &ModInfoView::openScoreView, _profileView, &ProfileView::openScoreView);
		connect(_modInfoView, &ModInfoView::openAchievementView, this, &MainWindow::openSpecialProfileView);
		connect(_modInfoView, &ModInfoView::openScoreView, this, &MainWindow::openSpecialProfileView);

		connect(_modInfoPage, &ModInfoPage::openAchievementView, _profileView, &ProfileView::openAchievementView);
		connect(_modInfoPage, &ModInfoPage::openScoreView, _profileView, &ProfileView::openScoreView);
		connect(_modInfoPage, &ModInfoPage::openAchievementView, this, &MainWindow::openSpecialProfileView);
		connect(_modInfoPage, &ModInfoPage::openScoreView, this, &MainWindow::openSpecialProfileView);
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

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS hiddenMods (ModID INT PRIMARY KEY);", err);

	LauncherFactory::getInstance()->updateModel(_modListModel);
	
	_sortModel->sort(0);

	const auto vec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "SELECT ModID, Ini FROM lastPlayed LIMIT 1;", err);
	
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

	_loginDialog = new LoginDialog(this);

	QMenu * fileMenu = new QMenu(QApplication::tr("File"), this);
	UPDATELANGUAGESETTITLE(fileMenu, "File");
	QMenu * toolsMenu = new QMenu(QApplication::tr("Tools"), this);
	UPDATELANGUAGESETTITLE(toolsMenu, "Tools");

	QMenu * developerMenu = nullptr;
	if (devEnabled) {
		developerMenu = new QMenu(QApplication::tr("Developer"), this);
		UPDATELANGUAGESETTITLE(developerMenu, "Developer");

		QAction * translationRequestAction = developerMenu->addAction(QApplication::tr("RequestTranslation"));
		UPDATELANGUAGESETTEXT(translationRequestAction, "RequestTranslation");
		translationRequestAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
		connect(translationRequestAction, &QAction::triggered, this, &MainWindow::openTranslationRequest);

		QAction * translatorAction = developerMenu->addAction(QApplication::tr("Translator"));
		UPDATELANGUAGESETTEXT(translatorAction, "Translator");
		translatorAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
		connect(translatorAction, &QAction::triggered, this, &MainWindow::openTranslator);

		_devModeAction = developerMenu->addAction(QApplication::tr("ActivateDeveloperMode"));
		UPDATELANGUAGESETTEXT(_devModeAction, "ActivateDeveloperMode");
		_devModeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
		connect(_devModeAction, &QAction::triggered, _settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::changedDeveloperMode);
		_devModeAction->setCheckable(true);
		_devModeAction->setChecked(_settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive());

		_spineEditorAction = developerMenu->addAction(QApplication::tr("SpineEditor"));
		UPDATELANGUAGESETTEXT(_spineEditorAction, "SpineEditor");
		_spineEditorAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
		connect(_spineEditorAction, &QAction::triggered, _spineEditor, &SpineEditor::exec);
		_spineEditorAction->setEnabled(_settingsDialog->getDeveloperSettingsWidget()->isDeveloperModeActive());
		connect(_settingsDialog->getDeveloperSettingsWidget(), &DeveloperSettingsWidget::developerModeChanged, _spineEditorAction, &QAction::setEnabled);

		if (Config::OnlineMode) {
			QAction * managementAction = developerMenu->addAction(QApplication::tr("Management"));
			UPDATELANGUAGESETTEXT(managementAction, "Management");
			managementAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
			connect(managementAction, &QAction::triggered, this, &MainWindow::execManagement);
		}

		developerMenu->addSeparator();

		for (int i = 0; i < 10; i++) {
			QAction * devPathAction = developerMenu->addAction(QApplication::tr("DevPath").arg(i));
			UPDATELANGUAGESETTEXTARG(devPathAction, "DevPath", i);
			devPathAction->setShortcut(QKeySequence(Qt::CTRL + (Qt::Key_0 + i)));
			devPathAction->setProperty("id", i);
			connect(devPathAction, &QAction::triggered, this, &MainWindow::setDevPath);
		}
	}
	
	QMenu * helpMenu = new QMenu(QApplication::tr("Help"), this);
	UPDATELANGUAGESETTITLE(helpMenu, "Help");

	QAction * exportAction = fileMenu->addAction(QApplication::tr("Export"));
	UPDATELANGUAGESETTEXT(exportAction, "Export");
	connect(exportAction, &QAction::triggered, this, &MainWindow::execExport);

	QAction * importAction = fileMenu->addAction(QApplication::tr("Import"));
	UPDATELANGUAGESETTEXT(importAction, "Import");
	connect(importAction, &QAction::triggered, this, &MainWindow::execImport);

	fileMenu->addSeparator();

	if (!Config::OnlineMode) {
		QAction * onlineAction = fileMenu->addAction(QApplication::tr("SwitchToOnline"));
		UPDATELANGUAGESETTEXT(onlineAction, "SwitchToOnline");
		connect(onlineAction, &QAction::triggered, this, &MainWindow::switchToOnline);
	} else {
		QAction * offlineAction = fileMenu->addAction(QApplication::tr("SwitchToOffline"));
		UPDATELANGUAGESETTEXT(offlineAction, "SwitchToOffline");
		connect(offlineAction, &QAction::triggered, this, &MainWindow::switchToOffline);
	}

	fileMenu->addSeparator();

	QAction * quitAction = fileMenu->addAction(QApplication::tr("Quit"));
	UPDATELANGUAGESETTEXT(quitAction, "Quit");
	connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);

#ifdef Q_OS_WIN
	QAction * installG2FromCDAction = toolsMenu->addAction(QApplication::tr("InstallGothic2FromCD"));
	UPDATELANGUAGESETTEXT(installG2FromCDAction, "InstallGothic2FromCD");
#endif
	QAction * openIniConfiguratorAction = toolsMenu->addAction(QApplication::tr("IniConfigurator"));
	UPDATELANGUAGESETTEXT(openIniConfiguratorAction, "IniConfigurator");
	if (Config::OnlineMode) {
		QAction * submitCompatibilityAction = toolsMenu->addAction(QApplication::tr("SubmitCompatibility"));
		UPDATELANGUAGESETTEXT(submitCompatibilityAction, "SubmitCompatibility");
		connect(submitCompatibilityAction, &QAction::triggered, this, &MainWindow::submitCompatibility);
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

	_autoUpdateDialog = new AutoUpdateDialog(this);

	QAction * faqAction = helpMenu->addAction(QApplication::tr("FAQ"));
	faqAction->setToolTip(QApplication::tr("FAQTooltip"));
	UPDATELANGUAGESETTEXT(faqAction, "FAQ");
	UPDATELANGUAGESETTOOLTIP(faqAction, "FAQTooltip");

	if (Config::OnlineMode) {
		QAction * checkIntegrityAction = helpMenu->addAction(QApplication::tr("CheckIntegrity"));
		checkIntegrityAction->setToolTip(QApplication::tr("CheckIntegrityTooltip"));
		UPDATELANGUAGESETTEXT(checkIntegrityAction, "CheckIntegrity");
		UPDATELANGUAGESETTOOLTIP(checkIntegrityAction, "CheckIntegrityTooltip");
		connect(checkIntegrityAction, &QAction::triggered, this, [this]() {
			checkIntegrity(-1);
		});
	}
	QAction * generateReportAction = helpMenu->addAction(QApplication::tr("GenerateReport"));
	UPDATELANGUAGESETTEXT(generateReportAction, "GenerateReport");

	QAction * showChangelogAction = helpMenu->addAction(QApplication::tr("ShowChangelog"));
	UPDATELANGUAGESETTEXT(showChangelogAction, "ShowChangelog");

	if (Config::OnlineMode) {
		QAction * updateAction = helpMenu->addAction(QApplication::tr("CheckForUpdates"));
		UPDATELANGUAGESETTEXT(updateAction, "CheckForUpdates");
		connect(updateAction, &QAction::triggered, _autoUpdateDialog, &AutoUpdateDialog::exec);
	}

	helpMenu->addSeparator();

	QAction * tutorialsAction = helpMenu->addAction(QApplication::tr("Tutorials"));
	UPDATELANGUAGESETTEXT(tutorialsAction, "Tutorials");
	connect(tutorialsAction, &QAction::triggered, this, &MainWindow::openTutorials);
	
	QAction * publishingAction = helpMenu->addAction(QApplication::tr("PublishingOnSpine"));
	UPDATELANGUAGESETTEXT(publishingAction, "PublishingOnSpine");
	connect(publishingAction, &QAction::triggered, this, &MainWindow::openPublishingTutorial);

	helpMenu->addSeparator();
	
	QAction * discordAction = helpMenu->addAction(QApplication::tr("Discord"));
	UPDATELANGUAGESETTEXT(discordAction, "Discord");
	if (Config::OnlineMode) {
		QAction * feedbackAction = helpMenu->addAction(QApplication::tr("Feedback"));
		UPDATELANGUAGESETTEXT(feedbackAction, "Feedback");
		connect(feedbackAction, &QAction::triggered, _feedbackDialog, &FeedbackDialog::exec);
	}
	QAction * aboutAction = helpMenu->addAction(QApplication::tr("About"));
	UPDATELANGUAGESETTEXT(aboutAction, "About");

	AboutDialog * aboutDialog = new AboutDialog(this);

	_changelogDialog = new ChangelogDialog(this);

#ifdef Q_OS_WIN
	connect(installG2FromCDAction, &QAction::triggered, _installGothic2FromCDDialog, &InstallGothic2FromCDDialog::exec);
#endif

	connect(openIniConfiguratorAction, &QAction::triggered, this, &MainWindow::openIniConfigurator);
	connect(savegameEditorAction, &QAction::triggered, this, &MainWindow::openSavegameEditor);
	connect(settingsAction, &QAction::triggered, _settingsDialog, &SettingsDialog::exec);

	connect(faqAction, &QAction::triggered, this, &MainWindow::execFAQ);
	connect(generateReportAction, &QAction::triggered, this, &MainWindow::generateReports);
	connect(showChangelogAction, &QAction::triggered, _changelogDialog, &ChangelogDialog::exec);
	connect(discordAction, &QAction::triggered, []() {
		QDesktopServices::openUrl(QUrl("https://discord.gg/x4x5Mc8"));
	});
	connect(aboutAction, &QAction::triggered, aboutDialog, &AboutDialog::exec);

	connect(_settingsDialog->getLocationSettingsWidget(), &LocationSettingsWidget::pathChanged, this, &MainWindow::pathChanged);

	menuBar()->addMenu(fileMenu);
	menuBar()->addMenu(toolsMenu);

	if (developerMenu) {
		menuBar()->addMenu(developerMenu);
	}
	menuBar()->addMenu(helpMenu);

	if (Config::OnlineMode) {
		QMenu * loginMenu = new QMenu(QApplication::tr("Login"), this);
		UPDATELANGUAGESETTITLE(loginMenu, "Login");
		menuBar()->addMenu(loginMenu);
		connect(loginMenu, &QMenu::aboutToShow, _loginDialog, &LoginDialog::execute);
		connect(_loginDialog, &LoginDialog::loggedIn, [loginMenu]() {
			loginMenu->menuAction()->setVisible(false);
		});
	}

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

	connect(_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
	if (Config::OnlineMode) {
		connect(_settingsDialog->getLocationSettingsWidget(), &LocationSettingsWidget::validGothic, _modDatabaseView, &ModDatabaseView::gothicValidationChanged);
		connect(_settingsDialog->getLocationSettingsWidget(), &LocationSettingsWidget::validGothic2, _modDatabaseView, &ModDatabaseView::gothic2ValidationChanged);
	}
#ifdef Q_OS_WIN
	connect(_settingsDialog->getLocationSettingsWidget(), &LocationSettingsWidget::validGothic2, [=](bool b) {
		installG2FromCDAction->setVisible(!b);
	});
	connect(_installGothic2FromCDDialog, &InstallGothic2FromCDDialog::updateGothic2Directory, _settingsDialog->getLocationSettingsWidget(), &LocationSettingsWidget::setGothic2Directory);
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
		connect(_profileView, &ProfileView::triggerLogout, _loginDialog, &LoginDialog::logout);
	}

	if (Config::OnlineMode) {
		QTimer::singleShot(0, _autoUpdateDialog, &AutoUpdateDialog::checkForUpdate);
	}

	if (Config::OnlineMode) {
		_modUpdateDialog = new ModUpdateDialog(this);
		connect(_loginDialog, &LoginDialog::loggedIn, _modUpdateDialog, &ModUpdateDialog::loginChanged);
		connect(_loginDialog, &LoginDialog::notLoggedIn, _modUpdateDialog, &ModUpdateDialog::loginChanged);

		connect(_settingsDialog->getGeneralSettingsWidget(), &GeneralSettingsWidget::resetModUpdates, _modUpdateDialog, QOverload<>::of(&ModUpdateDialog::checkForUpdate));

		connect(_modUpdateDialog, &ModUpdateDialog::updatedMod, _modInfoView, &ModInfoView::updatedMod);

		connect(_modUpdateDialog, &ModUpdateDialog::updateStarted, LauncherFactory::getInstance(), &LauncherFactory::updateStarted);
		connect(_modUpdateDialog, &ModUpdateDialog::updateStarted, _modInfoPage, &ModInfoPage::updateStarted);

		connect(_modUpdateDialog, &ModUpdateDialog::updatedMod, LauncherFactory::getInstance(), &LauncherFactory::updateFinished);
		connect(_modUpdateDialog, &ModUpdateDialog::updatedMod, _modInfoPage, &ModInfoPage::updateFinished);

		connect(_modListView, &LibraryListView::forceUpdate, _modUpdateDialog, QOverload<int32_t, bool>::of(&ModUpdateDialog::checkForUpdate));
		connect(_modListView, &LibraryListView::checkIntegrity, this, &MainWindow::checkIntegrity);

		connect(_autoUpdateDialog, &AutoUpdateDialog::upToDate, _modUpdateDialog, &ModUpdateDialog::spineUpToDate);
	}
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
	delete _downloadQueue;
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
	LauncherFactory::getInstance()->updateModel(_modListModel);
}

void MainWindow::tabChanged(int index) {
	if (Config::OnlineMode) {
		if (index == MainTabsOnline::Database) {
			_modDatabaseView->updateModList(-1, -1, InstallMode::None);
			_profileView->reset();
		} else if (index == MainTabsOnline::Profile) {
			_profileView->updateList();
		} else if (index == MainTabsOnline::LibraryOnline) {
			_profileView->reset();
		} else if (index == MainTabsOnline::Friends) {
			_friendsView->updateFriendList();
		} else if (index == MainTabsOnline::SpineLevelRanking) {
			_spineLevelRankingWidget->requestUpdate();
		} else {
			_profileView->reset();
		}
	}
}

void MainWindow::setDeveloperMode(bool devMode) {
	_developerModeActive = devMode;
	_devModeAction->setChecked(devMode);
	pathChanged();
}

void MainWindow::checkIntegrity(int projectID) {
	IntegrityCheckDialog dlg(this, this);
	if (_settingsDialog->getLocationSettingsWidget()->isGothicValid(true)) {
		dlg.setGothicDirectory(_gothicDirectory);
	}
	if (_settingsDialog->getLocationSettingsWidget()->isGothic2Valid(true)) {
		dlg.setGothic2Directory(_gothic2Directory);
	}
	if (dlg.exec(projectID) == QDialog::Accepted) {
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
			QString text = QApplication::tr("CheckIntegrityFailed") + "\n" + QApplication::tr("CorruptedFiles") + ": " + QString::number(corruptFiles.count() + corruptGothicFiles.count() + corruptGothic2Files.count());
			
			QMessageBox msg(QMessageBox::Icon::Critical, QApplication::tr("CheckIntegrity"), text, QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
			msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			if (QMessageBox::StandardButton::Ok == msg.exec()) {
				MultiFileDownloader * mfd = new MultiFileDownloader(this);
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
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/mods/" + QString::number(file.modID) + "/" + file.file), Config::DOWNLOADDIR + "/mods/" + QString::number(file.modID) + "/" + fi.path(), fi.fileName(), file.hash, mfd);
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

				connect(mfd, &MultiFileDownloader::downloadSucceeded, [corruptFiles]() {
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
					resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
					resultMsg.exec();
				});

				connect(mfd, &MultiFileDownloader::downloadFailed, []() {
					QMessageBox resultMsg(QMessageBox::Icon::Critical, QApplication::tr("CheckIntegrity"), QApplication::tr("IntegrityRepairFailure"), QMessageBox::StandardButton::Ok);
					resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
					resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
					resultMsg.exec();
				});
				
				_downloadQueue->add(mfd);
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

	if (!_settingsDialog->getDeveloperSettingsWidget()) return;
	
	const QString path = _settingsDialog->getDeveloperSettingsWidget()->getPath(id);
	const common::GameType gv = _settingsDialog->getDeveloperSettingsWidget()->getGothicVersion(id);
	
	if (path.isEmpty()) return;

	if (!_developerModeActive) {
		_devModeAction->trigger();
	}
	_gothicDirectory = gv == common::GameType::Gothic ? path : "";
	_gothic2Directory = gv == common::GameType::Gothic2 ? path : "";
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
	LauncherFactory::getInstance()->updateModel(_modListModel);
}

void MainWindow::submitCompatibility() {
	if (Config::Username.isEmpty()) return;
	
	SubmitCompatibilityDialog dlg;
	dlg.exec();
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
	connect(&dlg, &client::widgets::ManagementDialog::checkForUpdate, _modUpdateDialog, QOverload<int32_t, bool>::of(&ModUpdateDialog::checkForUpdate));
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
	
	if (idxList.empty()) return;

	const QModelIndex idx = idxList.constFirst();
	const int modId = idx.data(LibraryFilterModel::ModIDRole).toInt();
	const common::GameType gothicVersion = static_cast<common::GameType>(idx.data(LibraryFilterModel::GameRole).toInt());
	const QString directory = gothicVersion == common::GameType::Gothic ? _gothicDirectory : _gothic2Directory;
	_modListView->clearSelection();

	const bool uninstalled = client::Uninstaller::uninstall(modId, idx.data(Qt::DisplayRole).toString(), directory);
	
	if (!uninstalled) return;
	
	const auto match = _modListModel->match(_modListModel->index(0, 0), LibraryFilterModel::ModIDRole, modId);
	
	if (!match.isEmpty()) {
		_modListModel->removeRows(match[0].row(), match.count());
	}

	_modDatabaseView->updateModList(-1, -1, InstallMode::None);
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

void MainWindow::openTutorials() {
	if (GeneralSettingsWidget::getInstance()->getLanguage() == "Deutsch") {
		QDesktopServices::openUrl(QUrl("https://clockwork-origins.com/de/spine-tutorials/"));
	} else {
		QDesktopServices::openUrl(QUrl("https://clockwork-origins.com/spine-tutorials/"));		
	}
}

void MainWindow::openPublishingTutorial() {
	if (GeneralSettingsWidget::getInstance()->getLanguage() == "Deutsch") {
		QDesktopServices::openUrl(QUrl("https://clockwork-origins.com/de/spine-tutorial-publishing-on-spine/"));
	} else {
		QDesktopServices::openUrl(QUrl("https://clockwork-origins.com/spine-tutorial-publishing-on-spine/"));		
	}
}

void MainWindow::findGothic() {
	QDialog dlg;
	dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	QVBoxLayout * l = new QVBoxLayout();
	QLabel * descriptionLabel = new QLabel(QApplication::tr("SetGothicPathText"), &dlg);
	descriptionLabel->setWordWrap(true);
	LocationSettingsWidget * lsw = new LocationSettingsWidget(true, &dlg);
	QDialogButtonBox * db = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok, &dlg);
	QPushButton * pb = db->button(QDialogButtonBox::StandardButton::Ok);
	connect(pb, &QPushButton::released, &dlg, &QDialog::accept);
	connect(pb, &QPushButton::released, &dlg, &QDialog::hide);
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

	lsw->saveSettings();

	auto* mainLsw = _settingsDialog->getLocationSettingsWidget();
	
	mainLsw->rejectSettings();
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
	const int result = reinterpret_cast<int>(::ShellExecuteA(0, "runas", exeFileName.toUtf8().constData(), 0, 0, SW_SHOWNORMAL));
	if (result > 32) { // no error
		qApp->quit();
	}
#endif
}

void MainWindow::showEvent(QShowEvent * event) {
	QMainWindow::showEvent(event);
	
	_downloadQueue->setWindow(this);
}

void MainWindow::closeEvent(QCloseEvent * evt) {
	if (onQuit()) {
		QMainWindow::closeEvent(evt);
	} else {
		evt->ignore();
	}
}
