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

#pragma once

#include <cstdint>

#include <QMainWindow>

class QListView;
class QStandardItemModel;
class QTabWidget;
class QTextEdit;

namespace spine {
	class AutoUpdate;
	class LibraryFilterModel;
namespace utils {
	class DownloadQueue;
}
namespace widgets {

	class AutoUpdateDialog;
	class ChangelogDialog;
	class FeedbackDialog;
	class FriendsView;
	class InstallGothic2FromCDDialog;
	class LibraryListView;
	class LoginDialog;
	class ModDatabaseView;
	class ModInfoPage;
	class ModInfoView;
	class ModUpdateDialog;
	class ProfileView;
	class SettingsDialog;
	class SpineEditor;
	class SpineLevelRankingWidget;

	class MainWindow : public QMainWindow {
		Q_OBJECT

	public:
		MainWindow(bool showChangelog, QMainWindow * par = nullptr);
		~MainWindow() override;

		static MainWindow * getInstance() { return instance; }

	public slots:
		void startProject(int projectID);
		void installProject(int projectID);

	private slots:
		void selectedMod(const QModelIndex & index);
		void pathChanged();
		void tabChanged(int index);
		void setDeveloperMode(bool devMode);
		void checkIntegrity(int projectID);
		void openSpecialProfileView();
		void changeToInfoTab();
		void openIniConfigurator();
		void setDevPath();
		void submitCompatibility();
		void triggerModStart(int modID, QString iniFile);
		void execFAQ();
		void execExport();
		void execImport();
		void openSavegameEditor();
		void execManagement();
		void switchToOnline();
		void switchToOffline();
		void hideMod();
		void showMod();
		void uninstallMod();

		void openTranslator();
		void openTranslationRequest();

		void generateReports();
		bool onQuit();

		void openTutorials();
		void openPublishingTutorial();

	private:
		LibraryListView * _modListView;
		ModInfoView * _modInfoView;
		ProfileView * _profileView;
		FriendsView * _friendsView;
		SpineLevelRankingWidget * _spineLevelRankingWidget;
		QString _gothicDirectory;
		QString _gothic2Directory;
		QString _gothic3Directory;
		SettingsDialog * _settingsDialog;
		AutoUpdateDialog * _autoUpdateDialog;
		ChangelogDialog * _changelogDialog;
		QStandardItemModel * _modListModel;
		LibraryFilterModel * _sortModel;
		LoginDialog * _loginDialog;
		ModUpdateDialog * _modUpdateDialog;
		InstallGothic2FromCDDialog * _installGothic2FromCDDialog;
		FeedbackDialog * _feedbackDialog;
		bool _developerModeActive;
		QAction * _devModeAction;
		ModDatabaseView * _modDatabaseView;
		QTabWidget * _tabWidget;
		QAction * _spineEditorAction;
		SpineEditor * _spineEditor;
		ModInfoPage * _modInfoPage;
		utils::DownloadQueue * _downloadQueue;
		QTimer * _updateCheckTimer = nullptr;

		static MainWindow * instance;

		void findGothic();
		void restoreSettings();
		void saveSettings();

		void changedOnlineMode();

		void showEvent(QShowEvent * event) override;
		void closeEvent(QCloseEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
