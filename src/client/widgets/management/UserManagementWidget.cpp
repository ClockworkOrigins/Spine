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

#include "widgets/management/UserManagementWidget.h"

#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;

UserManagementWidget::UserManagementWidget(QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _waitSpinner(nullptr) {
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
		connect(_userListView, &QListView::clicked, this, &UserManagementWidget::selectedUser);
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
		connect(_unlockedListView, &QListView::clicked, this, &UserManagementWidget::selectedUnlockedUser);
		hl->addWidget(_unlockedListView);

		l->addLayout(hl);
	}

	setLayout(l);

	connect(this, &UserManagementWidget::removeSpinner, [this]() {
		if (!_waitSpinner) return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});

	connect(this, &UserManagementWidget::loadedData, this, &UserManagementWidget::updateData);
	connect(this, &UserManagementWidget::loadedUsers, this, &UserManagementWidget::updateUserList);
}

UserManagementWidget::~UserManagementWidget() {
	_futureWatcher.waitForFinished();
}

void UserManagementWidget::updateModList(QList<ManagementMod> modList) {
	_mods = modList;
}

void UserManagementWidget::selectedMod(int index) {
	_modIndex = index;
	_userListModel->clear();
	_unlockedListModel->clear();
}

void UserManagementWidget::updateView() {
	if (_modIndex == -1 || _modIndex >= _mods.size()) return;
	
	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ModID"] = _mods[_modIndex].id;
	
	const auto f = https::Https::postAsync(MANAGEMENTSERVER_PORT, "getUsers", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		QList<QString> userList;
		QList<QString> unlockedUserList;

		const auto it = json.find("Users");
		if (it != json.end()) {
			const auto usersArr = it->toArray();
			for (auto user : usersArr) {
				userList.append(user.toObject()["Name"].toString());
			}
		}

		const auto it2 = json.find("UnlockedUsers");
		if (it2 != json.end()) {
			const auto usersArr = it2->toArray();
			for (auto user : usersArr) {
				unlockedUserList.append(user.toObject()["Name"].toString());
			}
		}
		
		emit loadedUsers(userList);
		emit loadedData(unlockedUserList);
		emit removeSpinner();
	});
	_futureWatcher.setFuture(f);
}

void UserManagementWidget::updateData(QStringList users) {
	_unlockedListModel->clear();
	_userListModel->clear();
	
	QSet<QString> unlockedUsers;
	for (const auto & s : users) {
		unlockedUsers.insert(s);
		QStandardItem * itm = new QStandardItem(s);
		itm->setEditable(false);
		_unlockedListModel->appendRow(itm);
	}
	for (const QString & s : _userList) {
		if (!unlockedUsers.contains(s)) {
			QStandardItem * itm = new QStandardItem(s);
			itm->setEditable(false);
			_userListModel->appendRow(itm);
		}
	}
}

void UserManagementWidget::updateUserList(QStringList userList) {
	_userList.clear();
	for (const auto & s : userList) {
		_userList.append(s);
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
	if (_modIndex == -1) return;

	QJsonObject json;
	json["ModID"] = _mods[_modIndex].id;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	
	json["User"] = username;
	json["Enabled"] = enabled;
	
	const QJsonDocument doc(json);
	const QString content = doc.toJson(QJsonDocument::Compact);

	https::Https::postAsync(MANAGEMENTSERVER_PORT, "changeUserAccess", content, [](const QJsonObject &, int) {
		// we could do some error handling here
	});
}
