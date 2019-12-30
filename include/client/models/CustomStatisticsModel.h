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

#ifndef __SPINE_MODELS_CUSTOMSTATISTICSMODEL_H__
#define __SPINE_MODELS_CUSTOMSTATISTICSMODEL_H__

#include <cstdint>

#include <QSortFilterProxyModel>

namespace spine {
namespace models {

	enum CustomStatisticsColumn {
		Identifier,
		Guild,
		Name,
		Value,
		Amount
	};

	enum CustomStatisticsRole {
		SortRole = Qt::UserRole,
		FilterRole
	};

	class CustomStatisticsModel : public QSortFilterProxyModel {
		Q_OBJECT

	public:
		CustomStatisticsModel(QObject * par);

	public slots:
		void identifierChanged(int state);
		void guildChanged(int state);
		void nameChanged(QString name);

	private:
		int32_t _identifier;
		int32_t _guild;
		QString _name;

		bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;
		bool lessThan(const QModelIndex & left, const QModelIndex & right) const override;
	};

} /* namespace models */
} /* namespace spine */

#endif /* __SPINE_MODELS_CUSTOMSTATISTICSMODEL_H__ */
