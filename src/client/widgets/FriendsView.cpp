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

#include "widgets/FriendsView.h"

#include <thread>

#include "Config.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "common/MessageStructs.h"

#include "utils/Conversion.h"

#include "widgets/AddFriendDialog.h"
#include "widgets/FriendRequestView.h"
#include "widgets/GeneralSettingsWidget.h"
#include "widgets/WaitSpinner.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDebug>
#include <QListView>
#include <QPushButton>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	FriendsView::FriendsView(GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _friendsList(nullptr), _waitSpinner(nullptr), _model(nullptr), _username(), _sendRequestButton(nullptr), _users(), _friendRequests(), _scrollArea(nullptr), _mainWidget(nullptr), _scrollLayout(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();
			hl->addStretch(1);

			_sendRequestButton = new QPushButton("+", this);
			_sendRequestButton->setToolTip(QApplication::tr("SendFriendRequest"));
			UPDATELANGUAGESETTOOLTIP(generalSettingsWidget, _sendRequestButton, "SendFriendRequest");
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

		_friendsList = new QListView(this);
		_friendsList->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
		_model = new QStandardItemModel(this);
		QSortFilterProxyModel * filterModel = new QSortFilterProxyModel(this);
		filterModel->setSourceModel(_model);
		_friendsList->setModel(filterModel);

		l->addWidget(_friendsList, 1);

		setLayout(l);

		qRegisterMetaType<std::vector<std::string>>("std::vector<common::Friend::string>");
		qRegisterMetaType<std::vector<common::Friend>>("std::vector<common::Friend>");
		connect(this, &FriendsView::receivedFriends, this, &FriendsView::updateFriendsList);
	}

	FriendsView::~FriendsView() {
	}

	void FriendsView::updateFriendList() {
		if (_username.isEmpty()) {
			return;
		}
		delete _waitSpinner;
		_waitSpinner = new WaitSpinner(QApplication::tr("LoadingFriends"), this);
		_users.clear();
		std::thread([this]() {
			common::RequestAllFriendsMessage rafm;
			rafm.username = _username.toStdString();
			rafm.password = _password.toStdString();
			std::string serialized = rafm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendAllFriendsMessage * safm = dynamic_cast<common::SendAllFriendsMessage *>(m);
							emit receivedFriends(safm->friends, safm->friendRequests);
							for (const std::string & n : safm->nonFriends) {
								QString qn = s2q(n);
								if (qn != _username) {
									_users << qn;
								}
							}
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << int(cErr);
				}
			}
		}).detach();
	}

	void FriendsView::setUsername(QString username, QString password) {
		_username = username;
		_password = password;
		_sendRequestButton->setVisible(!username.isEmpty());
	}

	void FriendsView::updateFriendsList(std::vector<common::Friend> friends, std::vector<common::Friend> friendRequests) {
		_model->clear();
		for (const common::Friend & s : friends) {
			_model->appendRow(new QStandardItem(s2q(s.name) + " (" + QApplication::tr("Level") + " " + QString::number(s.level) + ")"));
		}

		for (FriendRequestView * frv : _friendRequests) {
			frv->deleteLater();
		}
		_friendRequests.clear();

		for (const common::Friend & f : friendRequests) {
			FriendRequestView * frv = new FriendRequestView(_username, _password, s2q(f.name), f.level, this);
			_scrollLayout->addWidget(frv);
			_friendRequests.append(frv);
			connect(frv, &FriendRequestView::accepted, this, &FriendsView::acceptedFriend);
		}
		_scrollArea->setVisible(!friendRequests.empty());

		delete _waitSpinner;
		_waitSpinner = nullptr;
	}

	void FriendsView::openAddFriendDialog() {
		AddFriendDialog dlg(_users, _username, _password, this);
		dlg.exec();
	}

	void FriendsView::acceptedFriend() {
		updateFriendList();
	}

} /* namespace widgets */
} /* namespace spine */
