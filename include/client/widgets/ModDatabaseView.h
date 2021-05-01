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

#include <QModelIndex>
#include <QSet>
#include <QWidget>

#include "common/MessageStructs.h"

class QMainWindow;
class QResizeEvent;
class QStandardItemModel;
class QTableView;
class QTreeView;

namespace spine {
	class DatabaseFilterModel;
namespace client {
	enum class InstallMode;
}
namespace gui {
	class WaitSpinner;
}
namespace widgets {

	class GeneralSettingsWidget;
	class TextItem;

	class ModDatabaseView : public QWidget {
		Q_OBJECT

	public:
		ModDatabaseView(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par);

		static bool isInstalled(int modID);

	signals:
		void receivedModList(QList<common::Mod>);
		void receivedModSizes(QList<QPair<int32_t, uint64_t>>);
		void receivedModFilesList(common::Mod, QSharedPointer<QList<QPair<QString, QString>>>, QString);
		void receivedPackageList(QList<common::UpdatePackageListMessage::Package>);
		void receivedPackageFilesList(common::Mod, common::UpdatePackageListMessage::Package, QSharedPointer<QList<QPair<QString, QString>>>, QString fileserver);
		void triggerInstallMod(int);
		void triggerInstallPackage(int, int);
		void loadPage(int32_t);
		void finishedInstallation(int, int, bool);
		void receivedPlayedProjects(const QSet<int32_t> & playedProjects);

	public slots:
		void changeLanguage(QString language);
		void updateModList(int modID, int packageID, client::InstallMode mode);
		void gothicValidationChanged(bool valid);
		void gothic2ValidationChanged(bool valid);
		void gothic3ValidationChanged(bool valid);
		void loginChanged();
		void setGothicDirectory(QString dir);
		void setGothic2Directory(QString dir);

	private slots:
		void updateModList(QList<common::Mod> mods);
		void selectedIndex(const QModelIndex & index);
		void doubleClickedIndex(const QModelIndex & index);
		void downloadModFiles(common::Mod mod, QSharedPointer<QList<QPair<QString, QString>>> fileList, QString fileserver);
		void sortByColumn(int column);
		void changedFilterExpression(const QString & expression);
		void updatePackageList(QList<common::UpdatePackageListMessage::Package> packages);
		void downloadPackageFiles(common::Mod mod, common::UpdatePackageListMessage::Package package, QSharedPointer<QList<QPair<QString, QString>>> fileList, QString fileserver);
		void installMod(int modID);
		void installPackage(int modID, int packageID);

	private:
		QMainWindow * _mainWindow;
		QTreeView * _treeView;
		QStandardItemModel * _sourceModel;
		DatabaseFilterModel * _sortModel;
		QList<common::Mod> _mods;
		bool _gothicValid;
		bool _gothic2Valid;
		bool _gothic3Valid;
		QMap<int32_t, QModelIndex> _parentMods;
		QMap<int32_t, std::vector<common::UpdatePackageListMessage::Package>> _packages;
		QString _gothicDirectory;
		QString _gothic2Directory;
		QMap<int32_t, TextItem *> _packageIDIconMapping;
		gui::WaitSpinner * _waitSpinner;
		bool _allowRenderer;
		QList<int32_t> _downloadingList;
		QList<int32_t> _downloadingPackageList;
		QSet<int32_t> _installSilently;

		bool _cached;

		void resizeEvent(QResizeEvent * evt) override;
		qint64 getDownloadSize(common::Mod mod) const;
		void selectedModIndex(const QModelIndex & index);
		void selectedPackageIndex(const QModelIndex & index);

		void updateDatabaseEntries();
	};

} /* namespace widgets */
} /* namespace spine */
