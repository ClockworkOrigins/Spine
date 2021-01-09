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
// Copyright 2020 Clockwork Origins

#pragma once

#include <QModelIndex>
#include <QWidget>

class QLineEdit;
class QSortFilterProxyModel;
class QStandardItemModel;
class QTableView;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace widgets {

	class SpineLevelRankingWidget : public QWidget {
		Q_OBJECT

		typedef struct {
			QString username;
			int level;
			int xp;
			int rank;
		} RankingEntry;

	public:
		explicit SpineLevelRankingWidget(QWidget * par);

		void requestUpdate();

	signals:
		void receivedRankings(QList<RankingEntry> rankingEntries);

	private slots:
		void updateView(QList<RankingEntry> rankingEntries);
		void scrollToOwnPosition();
		void updateFilter();

	private:
		QTableView * _tableView;
		QStandardItemModel * _model;
		gui::WaitSpinner * _waitSpinner;
		QSortFilterProxyModel * _sortModel;
		QLineEdit * _filterEdit;

		QModelIndex _ownIndex;
	};

} /* namespace widgets */
} /* namespace spine */
