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

#include "widgets/FriendsView.h"

#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include "widgets/AddFriendDialog.h"
#include "widgets/FriendRequestView.h"
#include "widgets/FriendsListView.h"
#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QPushButton>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QVBoxLayout>

using namespace spine::common;
using namespace spine::gui;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

FriendsView::FriendsView(QWidget * par) : QWidget(par), _friendsList(nullptr), _waitSpinner(nullptr), _model(nullptr), _sendRequestButton(nullptr), _scrollArea(nullptr), _mainWidget(nullptr), _scrollLayout(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * hl = new QHBoxLayout();
		hl->addStretch(1);

		_sendRequestButton = new QPushButton("+", this);
		_sendRequestButton->setToolTip(QApplication::tr("SendFriendRequest"));
		UPDATELANGUAGESETTOOLTIP(_sendRequestButton, "SendFriendRequest");
		hl->addWidget(_sendRequestButton);

		l->addLayout(hl);

		connect(_sendRequestButton, &QPushButton::released, this, &FriendsView::openAddFriendDialog);

		_sendRequestButton->setVisible(false);
	}

	_scrollArea = new QScrollArea(this);
	_mainWidget = new QWidget(this);
	_scrollLayout = new QVBoxLayout();
	_scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	_mainWidget->setLayout(_scrollLayout);
	_scrollArea->setWidget(_mainWidget);
	_scrollArea->setWidgetResizable(true);
	_mainWidget->setProperty("default", true);
	_scrollArea->setFixedWidth(550);
	_scrollArea->hide();

	l->addWidget(_scrollArea, 0, Qt::AlignHCenter);

	_friendsList = new FriendsListView(this);
	_friendsList->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
	_model = new QStandardItemModel(this);
	auto * filterModel = new QSortFilterProxyModel(this);
	filterModel->setSourceModel(_model);
	_friendsList->setModel(filterModel);

	l->addWidget(_friendsList, 1);

	setLayout(l);

	qRegisterMetaType<std::vector<std::string>>("std::vector<common::Friend::string>");
	qRegisterMetaType<std::vector<Friend>>("std::vector<common::Friend>");
	connect(this, &FriendsView::receivedFriends, this, &FriendsView::updateFriendsList);

	connect(_friendsList, &FriendsListView::removeFriend, this, &FriendsView::removeFriend);
}

void FriendsView::updateFriendList() {
	if (Config::Username.isEmpty()) return;

	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("LoadingFriends"), this);
	_users.clear();

	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;

	Https::postAsync(DATABASESERVER_PORT, "requestAllFriends", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & data, int statusCode) {
		if (statusCode != 200) return;

		std::vector<Friend> friends;
		std::vector<Friend> friendRequests;
		
		if (data.contains("Friends")) {
			const auto arr = data["Friends"].toArray();
			for (const auto jsonRef : arr) {
				const auto json2 = jsonRef.toObject();

				const Friend f(q2s(json2["Name"].toString()), json2["Level"].toString().toInt());

				friends.push_back(f);
			}
		}

		if (data.contains("FriendRequests")) {
			const auto arr = data["FriendRequests"].toArray();
			for (const auto jsonRef : arr) {
				const auto json2 = jsonRef.toObject();

				const Friend f(q2s(json2["Name"].toString()), json2["Level"].toString().toInt());

				friendRequests.push_back(f);
			}
		}

		if (data.contains("Users")) {
			const auto arr = data["Users"].toArray();
			for (const auto jsonRef : arr) {
				const auto name = jsonRef.toString();

				if (name == Config::Username) continue;

				_users << name;
			}
		}

		emit receivedFriends(friends, friendRequests);
	});
}

void FriendsView::loginChanged() {
	_sendRequestButton->setVisible(!Config::Username.isEmpty());

	updateFriendList();
}

void FriendsView::updateFriendsList(std::vector<Friend> friends, std::vector<Friend> friendRequests) {
	_model->clear();
	for (const Friend & s : friends) {
		_model->appendRow(new QStandardItem(s2q(s.name) + " (" + QApplication::tr("Level") + " " + i2s(s.level) + ")"));
	}

	for (FriendRequestView * frv : _friendRequests) {
		frv->deleteLater();
	}
	_friendRequests.clear();

	for (const Friend & f : friendRequests) {
		auto * frv = new FriendRequestView(s2q(f.name), f.level, this);
		_scrollLayout->addWidget(frv);
		_friendRequests.append(frv);
		connect(frv, &FriendRequestView::accepted, this, &FriendsView::acceptedFriend);
	}
	_scrollArea->setVisible(!friendRequests.empty());

	delete _waitSpinner;
	_waitSpinner = nullptr;
}

void FriendsView::openAddFriendDialog() {
	AddFriendDialog dlg(_users, this);
	dlg.exec();
}

void FriendsView::acceptedFriend() {
	updateFriendList();
}

void FriendsView::removeFriend(const QString & friendName) {
	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Friend"] = friendName;

	Https::postAsync(DATABASESERVER_PORT, "removeFriend", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & data, int statusCode) {});
}
