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

#include "widgets/AddFriendDialog.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QComboBox>
#include <QCompleter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QVBoxLayout>

using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

AddFriendDialog::AddFriendDialog(QStringList users, QWidget * par) : QDialog(par), _comboBox(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);


	_comboBox = new QComboBox(this);
	_comboBox->setEditable(true);
	_comboBox->setDuplicatesEnabled(false);

	_sourceModel = new QStandardItemModel(_comboBox);
	auto * sortModel = new QSortFilterProxyModel(_comboBox);
	sortModel->setSourceModel(_sourceModel);

	for (const QString & s : users) {
		_sourceModel->appendRow(new QStandardItem(s));
	}

	auto * completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(sortModel);
	completer->setCompletionColumn(0);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	_comboBox->setCompleter(completer);

	l->addWidget(_comboBox);

	auto * requestButton = new QPushButton(QApplication::tr("SendFriendRequest"), this);

	l->addWidget(requestButton);

	setLayout(l);

	connect(requestButton, &QPushButton::released, this, &AddFriendDialog::sendRequest);

	setWindowTitle(QApplication::tr("SendFriendRequest"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void AddFriendDialog::sendRequest() {
	if (Config::Username.isEmpty()) return;

	const QString friendname = _comboBox->currentText();
	
	if (friendname.isEmpty()) return;
	
	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Friend"] = friendname;

	Https::postAsync(DATABASESERVER_PORT, "friendRequest", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});
	
	close();
}
