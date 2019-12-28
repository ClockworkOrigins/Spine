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

#ifndef __SPINE_WIDGETS_MODDATABASEVIEW_H__
#define __SPINE_WIDGETS_MODDATABASEVIEW_H__

#include <QMap>
#include <QModelIndex>
#include <QWidget>

#include "common/MessageStructs.h"

class QMainWindow;
class QResizeEvent;
class QSettings;
class QStandardItemModel;
class QTableView;
class QTreeView;

namespace spine {
	class DatabaseFilterModel;
namespace widgets {

	class GeneralSettingsWidget;
	class TextItem;
	class WaitSpinner;

	class ModDatabaseView : public QWidget {
		Q_OBJECT

	public:
		ModDatabaseView(QMainWindow * mainWindow, QSettings * iniParser, GeneralSettingsWidget * generalSettingsWidget, QWidget * par);

		static bool isInstalled(int modID);

	signals:
		void receivedModList(std::vector<common::Mod>);
		void receivedModSizes(std::vector<std::pair<int32_t, uint64_t>>);
		void receivedModFilesList(common::Mod, std::vector<std::pair<std::string, std::string>>, QString);
		void receivedPackageList(std::vector<common::UpdatePackageListMessage::Package>);
		void receivedPackageFilesList(common::Mod, common::UpdatePackageListMessage::Package, std::vector<std::pair<std::string, std::string>>, QString fileserver);
		void triggerInstallMod(int);
		void triggerInstallPackage(int, int);
		void loadPage(int32_t);
		void finishedInstallation(int, int, bool);

	public slots:
		void changeLanguage(QString language);
		void updateModList(int modID, int packageID = -1);
		void gothicValidationChanged(bool valid);
		void gothic2ValidationChanged(bool valid);
		void loginChanged();
		void setGothicDirectory(QString dir);
		void setGothic2Directory(QString dir);

	private slots:
		void updateModList(std::vector<common::Mod> mods);
		void selectedIndex(const QModelIndex & index);
		void doubleClickedIndex(const QModelIndex & index);
		void downloadModFiles(common::Mod mod, std::vector<std::pair<std::string, std::string>> fileList, QString fileserver);
		void sortByColumn(int column);
		void changedFilterExpression(const QString & expression);
		void updatePackageList(std::vector<common::UpdatePackageListMessage::Package> packages);
		void downloadPackageFiles(common::Mod mod, common::UpdatePackageListMessage::Package package, std::vector<std::pair<std::string, std::string>> fileList, QString fileserver);
		void installMod(int modID);
		void installPackage(int modID, int packageID);

	private:
		QMainWindow * _mainWindow;
		QSettings * _iniParser;
		QTreeView * _treeView;
		QStandardItemModel * _sourceModel;
		DatabaseFilterModel * _sortModel;
		std::vector<common::Mod> _mods;
		bool _gothicValid;
		bool _gothic2Valid;
		QMap<int32_t, QModelIndex> _parentMods;
		QMap<int32_t, std::vector<common::UpdatePackageListMessage::Package>> _packages;
		QString _gothicDirectory;
		QString _gothic2Directory;
		QMap<int32_t, TextItem *> _packageIDIconMapping;
		WaitSpinner * _waitSpinner;
		bool _allowRenderer;

		void resizeEvent(QResizeEvent * evt) override;
		qint64 getDownloadSize(common::Mod mod) const;
		void selectedModIndex(const QModelIndex & index);
		void selectedPackageIndex(const QModelIndex & index);

		void removeInvalidDatabaseEntries();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MODDATABASEVIEW_H__ */
