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

#pragma once

#include "ManagementCommon.h"

#include "widgets/management/IManagementWidget.h"

#include <QModelIndex>
#include <QWidget>

class QListView;
class QPushButton;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class UserManagementWidget : public QWidget, public IManagementWidget {
		Q_OBJECT

	public:
		UserManagementWidget(QWidget * par);
		~UserManagementWidget();

		void updateModList(QList<ManagementMod> modList);
		void selectedMod(int index);
		void updateView() override;

	signals:
		void removeSpinner();
		void loadedUsers(QStringList);
		void loadedData(QStringList);

	private slots:
		void updateUserList(QStringList users);
		void updateData(QStringList users);
		void addUser();
		void removeUser();
		void selectedUser(QModelIndex index);
		void selectedUnlockedUser(QModelIndex index);
		void changedNameFilter(QString filter);

	private:
		QSortFilterProxyModel * _sortModel;
		QStandardItemModel * _userListModel;
		QStandardItemModel * _unlockedListModel;
		QList<ManagementMod> _mods;
		int _modIndex;
		QStringList _userList;
		QListView * _userListView;
		QListView * _unlockedListView;
		QModelIndex _selectedUser;
		QModelIndex _selectedUnlockedUser;
		QPushButton * _addUserButton;
		QPushButton * _removeUserButton;
		gui::WaitSpinner * _waitSpinner;

		void changeAccessRight(QString username, bool enabled);
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
