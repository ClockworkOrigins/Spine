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

#ifndef __SPINE_WIDGETS_MAINWINDOW_H__
#define __SPINE_WIDGETS_MAINWINDOW_H__

#include <cstdint>

#include <QMainWindow>
#include <QMap>

class QListView;
class QSettings;
class QStandardItemModel;
class QTabWidget;
class QTextEdit;

namespace spine {
	class AutoUpdate;
	class LibraryFilterModel;
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

	class MainWindow : public QMainWindow {
		Q_OBJECT

	public:
		MainWindow(bool showChangelog, QSettings * iniParser, QMainWindow * par = nullptr);
		~MainWindow();

	private slots:
		void selectedMod(const QModelIndex & index);
		void pathChanged();
		void tabChanged(int index);
		void setDeveloperMode(bool devMode);
		void checkIntegrity();
		void openSpecialProfileView();
		void changeToInfoTab();
		void openIniConfigurator();
		void setDevPath();
		void submitCompatibility();
		void setUsername(QString username, QString password);
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

	private:
		LibraryListView * _modListView;
		ModInfoView * _modInfoView;
		ProfileView * _profileView;
		FriendsView * _friendsView;
		QTextEdit * _descriptionView;
		QString _gothicDirectory;
		QString _gothic2Directory;
		QSettings * _iniParser;
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
		QMap<QString, std::tuple<QString, int32_t>> _parsedInis;
		QTabWidget * _tabWidget;
		QAction * _spineEditorAction;
		SpineEditor * _spineEditor;
		ModInfoPage * _modInfoPage;
		QString _username;
		QString _password;
		bool _onlineMode;

		void findGothic();
		void parseMods();
		void parseMods(QString baseDir);
		void parseInstalledMods();
		void restoreSettings();
		void saveSettings();

		void changedOnlineMode();

		void closeEvent(QCloseEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MAINWINDOW_H__ */
