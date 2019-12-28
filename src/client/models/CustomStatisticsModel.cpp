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

#include "models/CustomStatisticsModel.h"

#include <QSettings>
#include <QStandardItemModel>

namespace spine {
namespace models {

	CustomStatisticsModel::CustomStatisticsModel(QObject * par) : QSortFilterProxyModel(par), _identifier(-1), _guild(-1), _name() {
	}

	void CustomStatisticsModel::identifierChanged(int index) {
		_identifier = index;
		invalidateFilter();
	}

	void CustomStatisticsModel::guildChanged(int index) {
		_guild = index;
		invalidateFilter();
	}

	void CustomStatisticsModel::nameChanged(QString name) {
		_name = name;
		invalidateFilter();
	}

	bool CustomStatisticsModel::filterAcceptsRow(int source_row, const QModelIndex &) const {
		bool result = true;
		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(sourceModel());
		result = result && (_identifier == -1 || model->item(source_row, CustomStatisticsColumn::Identifier)->data(CustomStatisticsRole::FilterRole).toInt() == _identifier);
		result = result && (_guild == -1 || model->item(source_row, CustomStatisticsColumn::Guild)->data(CustomStatisticsRole::FilterRole).toInt() == _guild);
		result = result && (_name.isEmpty() || model->item(source_row, CustomStatisticsColumn::Name)->data(CustomStatisticsRole::FilterRole).toString() == _name);
		return result;
	}

	bool CustomStatisticsModel::lessThan(const QModelIndex & left, const QModelIndex & right) const {
		bool result = true;
		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(sourceModel());
		const int32_t leftIdentifier = model->item(left.row(), CustomStatisticsColumn::Identifier)->data(CustomStatisticsRole::SortRole).toInt();
		const int32_t rightIdentifier = model->item(right.row(), CustomStatisticsColumn::Identifier)->data(CustomStatisticsRole::SortRole).toInt();

		const int32_t leftGuild = model->item(left.row(), CustomStatisticsColumn::Guild)->data(CustomStatisticsRole::SortRole).toInt();
		const int32_t rightGuild = model->item(right.row(), CustomStatisticsColumn::Guild)->data(CustomStatisticsRole::SortRole).toInt();

		const QString leftName = model->item(left.row(), CustomStatisticsColumn::Name)->data(CustomStatisticsRole::SortRole).toString();
		const QString rightName = model->item(right.row(), CustomStatisticsColumn::Name)->data(CustomStatisticsRole::SortRole).toString();

		const int32_t leftValue = model->item(left.row(), CustomStatisticsColumn::Value)->data(CustomStatisticsRole::SortRole).toInt();
		const int32_t rightValue = model->item(right.row(), CustomStatisticsColumn::Value)->data(CustomStatisticsRole::SortRole).toInt();

		if (leftIdentifier > rightIdentifier) {
			result = false;
		} else if (leftIdentifier == rightIdentifier) {
			if (leftGuild > rightGuild) {
				result = false;
			} else if (leftGuild == rightGuild) {
				if (leftName > rightName) {
					result = false;
				} else if (leftName == rightName) {
					result = leftValue < rightValue;
				}
			}
		}
		return result;
	}

} /* namespace models */
} /* namespace spine */
