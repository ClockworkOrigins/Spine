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

#include "common/MessageStructs.h"

#include <QWidget>

class QListView;
class QPushButton;
class QScrollArea;
class QStandardItemModel;
class QVBoxLayout;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace widgets {

	class FriendRequestView;

	class FriendsView : public QWidget {
		Q_OBJECT

	public:
		FriendsView(QWidget * par);

		void updateFriendList();

	signals:
		void receivedFriends(std::vector<common::Friend>, std::vector<common::Friend>);

	public slots:
		void loginChanged();

	private slots:
		void updateFriendsList(std::vector<common::Friend> friends, std::vector<common::Friend> friendRequests);
		void openAddFriendDialog();
		void acceptedFriend();

	private:
		QListView * _friendsList;
		gui::WaitSpinner * _waitSpinner;
		QStandardItemModel * _model;
		QPushButton * _sendRequestButton;
		QStringList _users;
		QList<FriendRequestView *> _friendRequests;
		QScrollArea * _scrollArea;
		QWidget * _mainWidget;
		QVBoxLayout * _scrollLayout;
	};

} /* namespace widgets */
} /* namespace spine */
