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

#include <QListView>

namespace spine {
namespace widgets {

	class LibraryListView : public QListView {
		Q_OBJECT

	public:
		LibraryListView(QWidget * par);

	signals:
		void hideModTriggered();
		void showModTriggered();
		void uninstallModTriggered();
		void forceUpdate(int projectID, bool forceAccept);

	private slots:
		void changeLanguage(int projectID, int language);

	private:
		void contextMenuEvent(QContextMenuEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
