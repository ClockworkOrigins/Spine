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

#include "widgets/management/UserManagementWidget.h"

#include <thread>

#include "SpineConfig.h"

#include "utils/Conversion.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	UserManagementWidget::UserManagementWidget(QString username, QString language, QWidget * par) : QWidget(par), _username(username), _language(language), _mods(), _modIndex(-1) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QLineEdit * le = new QLineEdit(this);
			connect(le, &QLineEdit::textChanged, this, &UserManagementWidget::changedNameFilter);

			l->addWidget(le);
		}

		{
			QHBoxLayout * hl = new QHBoxLayout();

			_userListView = new QListView(this);
			_userListView->setToolTip(QApplication::tr("UserlistTooltip"));
			_userListModel = new QStandardItemModel(_userListView);
			_sortModel = new QSortFilterProxyModel(_userListView);
			_sortModel->setSourceModel(_userListModel);
			_sortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
			_userListView->setModel(_sortModel);
			connect(_userListView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedUser(QModelIndex)));
			hl->addWidget(_userListView);

			{
				QVBoxLayout * vl = new QVBoxLayout();
				_addUserButton = new QPushButton("->", this);
				_addUserButton->setToolTip(QApplication::tr("UnlockUserTooltip"));
				_removeUserButton = new QPushButton("<-", this);
				_removeUserButton->setToolTip(QApplication::tr("LockUserTooltip"));
				vl->addStretch(1);
				vl->addWidget(_addUserButton);
				vl->addWidget(_removeUserButton);
				vl->addStretch(1);

				_addUserButton->setDisabled(true);
				_removeUserButton->setDisabled(true);

				connect(_addUserButton, &QPushButton::released, this, &UserManagementWidget::addUser);
				connect(_removeUserButton, &QPushButton::released, this, &UserManagementWidget::removeUser);

				hl->addLayout(vl);
			}

			_unlockedListView = new QListView(this);
			_unlockedListView->setToolTip(QApplication::tr("UnlockedUserlistTooltip"));
			_unlockedListModel = new QStandardItemModel(_unlockedListView);
			_unlockedListView->setModel(_unlockedListModel);
			connect(_unlockedListView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedUnlockedUser(QModelIndex)));
			hl->addWidget(_unlockedListView);

			l->addLayout(hl);
		}

		setLayout(l);
	}

	UserManagementWidget::~UserManagementWidget() {
	}

	void UserManagementWidget::updateModList(QList<client::ManagementMod> modList) {
		_mods = modList;
	}

	void UserManagementWidget::selectedMod(int index) {
		// TODO
		/*_modIndex = index;
		_userListModel->clear();
		_unlockedListModel->clear();
		QSet<QString> unlockedUsers;
		for (const std::string & s : _mods[index].userList) {
			unlockedUsers.insert(s2q(s));
			QStandardItem * itm = new QStandardItem(s2q(s));
			itm->setEditable(false);
			_unlockedListModel->appendRow(itm);
		}
		for (const QString & s : _userList) {
			if (!unlockedUsers.contains(s)) {
				QStandardItem * itm = new QStandardItem(s);
				itm->setEditable(false);
				_userListModel->appendRow(itm);
			}
		}*/
	}

	void UserManagementWidget::updateUserList(std::vector<std::string> userList) {
		_userList.clear();
		for (const std::string & s : userList) {
			_userList.append(s2q(s));
		}
	}

	void UserManagementWidget::addUser() {
		const QString username = _selectedUser.data(Qt::DisplayRole).toString();
		QStandardItem * itm = new QStandardItem(username);
		itm->setEditable(false);
		_unlockedListModel->appendRow(itm);
		_userListModel->removeRow(_selectedUser.row());

		_addUserButton->setDisabled(true);

		changeAccessRight(username, true);
	}

	void UserManagementWidget::removeUser() {
		const QString username = _selectedUnlockedUser.data(Qt::DisplayRole).toString();
		QStandardItem * itm = new QStandardItem(username);
		itm->setEditable(false);
		_userListModel->appendRow(itm);
		_unlockedListModel->removeRow(_selectedUnlockedUser.row());

		_removeUserButton->setDisabled(true);

		changeAccessRight(username, false);
	}

	void UserManagementWidget::selectedUser(QModelIndex index) {
		_selectedUser = _sortModel->mapToSource(index);
		_addUserButton->setEnabled(_selectedUser.isValid());
	}

	void UserManagementWidget::selectedUnlockedUser(QModelIndex index) {
		_selectedUnlockedUser = index;
		_removeUserButton->setEnabled(_selectedUnlockedUser.isValid());
	}

	void UserManagementWidget::changedNameFilter(QString filter) {
		_sortModel->setFilterRegExp(filter);
	}

	void UserManagementWidget::changeAccessRight(QString username, bool enabled) {
		// TODO
		/*if (enabled) {
			_mods[_modIndex].userList.push_back(q2s(username));
		} else {
			for (auto it = _mods[_modIndex].userList.begin(); it != _mods[_modIndex].userList.end(); ++it) {
				if (*it == q2s(username)) {
					_mods[_modIndex].userList.erase(it);
					break;
				}
			}
		}
		common::UpdateEarlyAccessStateMessage ueasm;
		ueasm.modID = _mods[_modIndex].modID;
		ueasm.username = q2s(username);
		ueasm.enabled = enabled;
		const std::string serialized = ueasm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			sock.writePacket(serialized);
		}*/
	}

} /* namespace widgets */
} /* namespace spine */
