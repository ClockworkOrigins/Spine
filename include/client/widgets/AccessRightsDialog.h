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

#ifndef __SPINE_WIDGETS_ACCESSRIGHTSDIALOG_H__
#define __SPINE_WIDGETS_ACCESSRIGHTSDIALOG_H__

#include "common/MessageStructs.h"

#include <QModelIndex>
#include <QDialog>

class QListView;
class QPushButton;
class QSettings;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace spine {
namespace widgets {

	class AccessRightsDialog : public QDialog {
		Q_OBJECT

	public:
		AccessRightsDialog(uint32_t requestID, QString title, QSettings * iniParser, QWidget * par);
		~AccessRightsDialog();

	signals:
		void receivedUserList(std::vector<std::string>, std::vector<std::string>);

	private slots :
		void updateUserList(std::vector<std::string> userList, std::vector<std::string> translators);
		void addUser();
		void removeUser();
		void selectedUser(QModelIndex index);
		void selectedUnlockedUser(QModelIndex index);
		void changedNameFilter(QString filter);

	private:
		QSettings * _iniParser;
		QSortFilterProxyModel * _sortModel;
		QStandardItemModel * _userListModel;
		QStandardItemModel * _unlockedListModel;
		uint32_t _requestID;
		QListView * _userListView;
		QListView * _unlockedListView;
		QModelIndex _selectedUser;
		QModelIndex _selectedUnlockedUser;
		QPushButton * _addUserButton;
		QPushButton * _removeUserButton;

		void requestUsers();
		void changeAccessRight(QString username, bool enabled);
		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_ACCESSRIGHTSDIALOG_H__ */
