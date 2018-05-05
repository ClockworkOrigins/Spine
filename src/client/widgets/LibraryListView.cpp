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

#include "widgets/LibraryListView.h"

#include "LibraryFilterModel.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>

namespace spine {
namespace widgets {

	LibraryListView::LibraryListView(QWidget * par) : QListView(par) {
	}

	LibraryListView::~LibraryListView() {
	}

	void LibraryListView::contextMenuEvent(QContextMenuEvent * evt) {
		if (!selectedIndexes().empty()) {
			QModelIndex idx = selectedIndexes().constFirst();
			if (idx.data(LibraryFilterModel::InstalledRole).toBool()) {
				QMenu * menu = new QMenu(this);
				if (idx.data(LibraryFilterModel::HiddenRole).toBool()) {
					QAction * showModAction = menu->addAction(QApplication::tr("Show"));
					connect(showModAction, &QAction::triggered, this, &LibraryListView::showModTriggered);
				} else {
					QAction * hideModAction = menu->addAction(QApplication::tr("Hide"));
					connect(hideModAction, &QAction::triggered, this, &LibraryListView::hideModTriggered);
				}
				QAction * uninstallModAction = menu->addAction(QApplication::tr("Uninstall"));
				connect(uninstallModAction, &QAction::triggered, this, &LibraryListView::uninstallModTriggered);
				menu->popup(viewport()->mapToGlobal(evt->pos()));
				menu->exec();
				delete menu;
			}
		}
		evt->accept();
	}

} /* namespace widgets */
} /* namespace spine */
