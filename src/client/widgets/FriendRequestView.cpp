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

#include "IconCache.h"
#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

FriendRequestView::FriendRequestView(QString friendname, uint32_t level, QWidget * par) : QWidget(par), _friendname(friendname), _level(level) {
	auto * l = new QHBoxLayout();
	l->setAlignment(Qt::AlignCenter);

	auto * nameLabel = new QLabel(friendname + " (" + QApplication::tr("Level") + " " + i2s(_level) + ")", this);
	nameLabel->setProperty("requestName", true);
	nameLabel->setAlignment(Qt::AlignCenter);
	l->addWidget(nameLabel, 1, Qt::AlignLeft);

	{
		auto * vl = new QVBoxLayout();

		auto * acceptButton = new QPushButton(IconCache::getInstance()->getOrLoadIcon(":/svg/check.svg"), "", this);
		vl->addWidget(acceptButton);

		auto * declineButton = new QPushButton(IconCache::getInstance()->getOrLoadIcon(":/svg/close.svg"), "", this);
		vl->addWidget(declineButton);

		l->addLayout(vl);

		connect(acceptButton, &QPushButton::released, this, &FriendRequestView::accept);
		connect(declineButton, &QPushButton::released, this, &FriendRequestView::decline);
	}

	setFixedWidth(500);

	setLayout(l);
}

void FriendRequestView::accept() {
	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Friend"] = _friendname;

	Https::postAsync(DATABASESERVER_PORT, "acceptFriend", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});
	
	hide();
	emit accepted();
}

void FriendRequestView::decline() {
	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Friend"] = _friendname;

	Https::postAsync(DATABASESERVER_PORT, "declineFriend", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});

	hide();
}
