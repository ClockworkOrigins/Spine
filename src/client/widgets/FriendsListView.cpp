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
// Copyright 2021 Clockwork Origins

#include "widgets/FriendsListView.h"

#include "SpineConfig.h"
#include "IconCache.h"
#include "LibraryFilterModel.h"

#include "common/Language.h"

#include "utils/Config.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

using namespace spine::client;
using namespace spine::common;
using namespace spine::utils;
using namespace spine::widgets;

FriendsListView::FriendsListView(QWidget * par) : QListView(par) {}

void FriendsListView::contextMenuEvent(QContextMenuEvent * evt) {
	if (!selectedIndexes().empty()) {
		const QModelIndex idx = selectedIndexes().constFirst();
		
		auto * menu = new QMenu(this);

		const QString friendName = idx.data().toString();
		
		QAction * removeFriendAction = menu->addAction(QApplication::tr("Uninstall"));
		connect(removeFriendAction, &QAction::triggered, this, [this, friendName]() {
			QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("RemoveFriend"), QApplication::tr("RemoveFriendDescription"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::No);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.button(QMessageBox::StandardButton::No)->setText(QApplication::tr("No"));
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			
			if (resultMsg.exec() != QMessageBox::StandardButton::Ok) return;
			
			emit removeFriend(friendName);
		});
		
		menu->popup(viewport()->mapToGlobal(evt->pos()));
		menu->exec();
		delete menu;
	}
	evt->accept();
}
