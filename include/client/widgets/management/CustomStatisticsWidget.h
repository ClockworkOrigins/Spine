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

#include "ManagementCommon.h"

#include "widgets/management/IManagementWidget.h"

#include <QFutureWatcher>
#include <QModelIndex>
#include <QWidget>

class QComboBox;
class QStandardItemModel;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class CustomStatisticsWidget : public QWidget, public IManagementWidget {
		Q_OBJECT

	public:
		CustomStatisticsWidget(QWidget * par);
		~CustomStatisticsWidget();

		void updateModList(QList<ManagementMod> modList);
		void selectedMod(int index);
		void updateView() override;

	signals:
		void removeSpinner();
		void loadedData(ManagementCustomStatistics);

	private slots:
		void updateData(ManagementCustomStatistics content);

	private:
		typedef struct StatTuple {
			int32_t identifier;
			int32_t guild;
			QString name;
			int32_t value;

			bool operator<(const StatTuple & other) const {
				bool ret = false;
				if (identifier < other.identifier) {
					ret = true;
				} else if (identifier == other.identifier) {
					if (guild < other.guild) {
						ret = true;
					} else if (guild == other.guild) {
						if (name < other.name) {
							ret = true;
						} else if (name == other.name) {
							ret = value < other.value;
						}
					}
				}
				return ret;
			}

			bool operator==(const StatTuple & other) const {
				return identifier == other.identifier && guild == other.guild && name == other.name && value == other.value;
			}
		} StatTuple;

		QList<ManagementMod> _mods;
		int _modIndex;
		QStandardItemModel * _sourceModel;
		QComboBox * _identifierBox;
		QComboBox * _guildBox;
		QComboBox * _nameBox;
		QMap<StatTuple, int> _stats;
		gui::WaitSpinner * _waitSpinner;

		QFutureWatcher<void> _futureWatcher;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
