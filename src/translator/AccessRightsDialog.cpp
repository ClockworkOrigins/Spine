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

#include "translator/AccessRightsDialog.h"

#include "common/MessageStructs.h"

#include "translator/TranslatorAPI.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include <QApplication>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QVBoxLayout>

using namespace spine::common;
using namespace spine::translation;
using namespace spine::utils;

AccessRightsDialog::AccessRightsDialog(uint32_t requestID, QString title, QWidget * par) : QDialog(par), _requestID(requestID) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * le = new QLineEdit(this);
		connect(le, &QLineEdit::textChanged, this, &AccessRightsDialog::changedNameFilter);

		l->addWidget(le);
	}

	{
		auto * hl = new QHBoxLayout();

		_userListView = new QListView(this);
		_userListView->setToolTip(QApplication::tr("LockedTranslatorsTooltip"));
		_userListModel = new QStandardItemModel(_userListView);
		_sortModel = new QSortFilterProxyModel(_userListView);
		_sortModel->setSourceModel(_userListModel);
		_sortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
		_userListView->setModel(_sortModel);
		connect(_userListView, &QListView::clicked, this, &AccessRightsDialog::selectedUser);
		hl->addWidget(_userListView);

		{
			auto * vl = new QVBoxLayout();
			_addUserButton = new QPushButton("->", this);
			_addUserButton->setToolTip(QApplication::tr("UnlockTranslatorTooltip"));
			_removeUserButton = new QPushButton("<-", this);
			_removeUserButton->setToolTip(QApplication::tr("LockTranslatorTooltip"));
			vl->addStretch(1);
			vl->addWidget(_addUserButton);
			vl->addWidget(_removeUserButton);
			vl->addStretch(1);

			_addUserButton->setDisabled(true);
			_removeUserButton->setDisabled(true);

			connect(_addUserButton, &QPushButton::released, this, &AccessRightsDialog::addUser);
			connect(_removeUserButton, &QPushButton::released, this, &AccessRightsDialog::removeUser);

			hl->addLayout(vl);
		}

		_unlockedListView = new QListView(this);
		_unlockedListView->setToolTip(QApplication::tr("UnlockedTranslatorsTooltip"));
		_unlockedListModel = new QStandardItemModel(_unlockedListView);
		_unlockedListView->setModel(_unlockedListModel);
		connect(_unlockedListView, &QListView::clicked, this, &AccessRightsDialog::selectedUnlockedUser);
		hl->addWidget(_unlockedListView);

		l->addLayout(hl);
	}

	setLayout(l);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(title);

	qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
	connect(this, &AccessRightsDialog::receivedUserList, this, &AccessRightsDialog::updateUserList);

	requestUsers();
}

void AccessRightsDialog::updateUserList(std::vector<std::string> userList, std::vector<std::string> translators) {
	QSet<QString> unlockedUsers;
	for (const std::string s : translators) {
		unlockedUsers.insert(s2q(s));
		auto * itm = new QStandardItem(s2q(s));
		itm->setEditable(false);
		_unlockedListModel->appendRow(itm);
	}
	for (const std::string s : userList) {
		auto * itm = new QStandardItem(s2q(s));
		itm->setEditable(false);
		_userListModel->appendRow(itm);
	}
}

void AccessRightsDialog::addUser() {
	const QString username = _selectedUser.data(Qt::DisplayRole).toString();
	auto * itm = new QStandardItem(username);
	itm->setEditable(false);
	_unlockedListModel->appendRow(itm);
	_userListModel->removeRow(_selectedUser.row());

	_addUserButton->setDisabled(true);

	changeAccessRight(username, true);
}

void AccessRightsDialog::removeUser() {
	const QString username = _selectedUnlockedUser.data(Qt::DisplayRole).toString();
	auto * itm = new QStandardItem(username);
	itm->setEditable(false);
	_userListModel->appendRow(itm);
	_unlockedListModel->removeRow(_selectedUnlockedUser.row());

	_removeUserButton->setDisabled(true);

	changeAccessRight(username, false);
}

void AccessRightsDialog::selectedUser(QModelIndex index) {
	_selectedUser = _sortModel->mapToSource(index);
	_addUserButton->setEnabled(_selectedUser.isValid());
}

void AccessRightsDialog::selectedUnlockedUser(QModelIndex index) {
	_selectedUnlockedUser = index;
	_removeUserButton->setEnabled(_selectedUnlockedUser.isValid());
}

void AccessRightsDialog::changedNameFilter(QString filter) {
	_sortModel->setFilterRegExp(filter);
}

void AccessRightsDialog::requestUsers() {
	QtConcurrent::run([this]() {
		SendTranslatorsMessage * stm = TranslatorAPI::requestTranslators(_requestID);
		if (stm) {
			emit receivedUserList(stm->lockedUsers, stm->translators);
			delete stm;
		}
	});
}

void AccessRightsDialog::changeAccessRight(QString username, bool enabled) {
	TranslatorAPI::changeTranslatorRights(_requestID, q2s(username), enabled);
}

void AccessRightsDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/AccessRightsDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/AccessRightsDialogGeometry");
	}
}

void AccessRightsDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/AccessRightsDialogGeometry", saveGeometry());
}
