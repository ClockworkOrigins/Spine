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

#ifndef __SPINE_WIDGETS_MANAGEMENT_USERMANAGEMENTWIDGET_H__
#define __SPINE_WIDGETS_MANAGEMENT_USERMANAGEMENTWIDGET_H__

#include "ManagementCommon.h"

#include "common/MessageStructs.h"

#include <QModelIndex>
#include <QWidget>

class QListView;
class QPushButton;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace spine {
namespace widgets {

	class UserManagementWidget : public QWidget {
		Q_OBJECT

	public:
		UserManagementWidget(QString username, QString language, QWidget * par);
		~UserManagementWidget();

		void updateModList(QList<client::ManagementMod> modList);
		void selectedMod(int index);

	public slots:
		void updateUserList(std::vector<std::string> userList);

	private slots:
		void addUser();
		void removeUser();
		void selectedUser(QModelIndex index);
		void selectedUnlockedUser(QModelIndex index);
		void changedNameFilter(QString filter);

	private:
		QSortFilterProxyModel * _sortModel;
		QStandardItemModel * _userListModel;
		QStandardItemModel * _unlockedListModel;
		QString _username;
		QString _language;
		QList<client::ManagementMod> _mods;
		int _modIndex;
		QStringList _userList;
		QListView * _userListView;
		QListView * _unlockedListView;
		QModelIndex _selectedUser;
		QModelIndex _selectedUnlockedUser;
		QPushButton * _addUserButton;
		QPushButton * _removeUserButton;

		void changeAccessRight(QString username, bool enabled);
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MANAGEMENT_USERMANAGEMENTWIDGET_H__ */
