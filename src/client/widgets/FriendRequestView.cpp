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

#include "widgets/FriendRequestView.h"

#include <thread>

#include "IconCache.h"
#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::client;
using namespace spine::utils;
using namespace spine::widgets;

FriendRequestView::FriendRequestView(QString friendname, uint32_t level, QWidget * par) : QWidget(par), _friendname(friendname), _level(level) {
	QHBoxLayout * l = new QHBoxLayout();
	l->setAlignment(Qt::AlignCenter);

	QLabel * nameLabel = new QLabel(friendname + " (" + QApplication::tr("Level") + " " + i2s(_level) + ")", this);
	nameLabel->setProperty("requestName", true);
	nameLabel->setAlignment(Qt::AlignCenter);
	l->addWidget(nameLabel, 1, Qt::AlignLeft);

	{
		QVBoxLayout * vl = new QVBoxLayout();

		QPushButton * acceptButton = new QPushButton(IconCache::getInstance()->getOrLoadIcon(":/svg/check.svg"), "", this);
		vl->addWidget(acceptButton);

		QPushButton * declineButton = new QPushButton(IconCache::getInstance()->getOrLoadIcon(":/svg/close.svg"), "", this);
		vl->addWidget(declineButton);

		l->addLayout(vl);

		connect(acceptButton, &QPushButton::released, this, &FriendRequestView::accept);
		connect(declineButton, &QPushButton::released, this, &FriendRequestView::decline);
	}

	setFixedWidth(500);

	setLayout(l);
}

void FriendRequestView::accept() {
	common::AcceptFriendRequestMessage afrm;
	afrm.username = Config::Username.toStdString();
	afrm.password = Config::Password.toStdString();
	afrm.friendname = _friendname.toStdString();
	const std::string serialized = afrm.SerializePublic();
	clockUtils::sockets::TcpSocket sock;
	const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
	if (clockUtils::ClockError::SUCCESS == cErr) {
		sock.writePacket(serialized);
	}
	hide();
	emit accepted();
}

void FriendRequestView::decline() {
	common::DeclineFriendRequestMessage dfrm;
	dfrm.username = Config::Username.toStdString();
	dfrm.password = Config::Password.toStdString();
	dfrm.friendname = _friendname.toStdString();
	const std::string serialized = dfrm.SerializePublic();
	clockUtils::sockets::TcpSocket sock;
	const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
	if (clockUtils::ClockError::SUCCESS == cErr) {
		sock.writePacket(serialized);
	}
	hide();
}
