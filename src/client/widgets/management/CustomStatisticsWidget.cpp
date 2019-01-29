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

#include "widgets/management/CustomStatisticsWidget.h"

#include <set>

#include "Conversion.h"

#include "models/CustomStatisticsModel.h"

#include <QApplication>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	CustomStatisticsWidget::CustomStatisticsWidget(QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _sourceModel(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		_sourceModel = new QStandardItemModel(this);
		models::CustomStatisticsModel * sortModel = new models::CustomStatisticsModel(_sourceModel);
		sortModel->setSourceModel(_sourceModel);

		{
			QHBoxLayout * hl = new QHBoxLayout();
			QLabel * identifierLabel = new QLabel(QApplication::tr("Identifier"), this);
			_identifierBox = new QComboBox(this);
			QLabel * guildLabel = new QLabel(QApplication::tr("Guild"), this);
			_guildBox = new QComboBox(this);
			QLabel * nameLabel = new QLabel(QApplication::tr("Name"), this);
			_nameBox = new QComboBox(this);

			hl->addWidget(identifierLabel);
			hl->addWidget(_identifierBox);
			hl->addWidget(guildLabel);
			hl->addWidget(_guildBox);
			hl->addWidget(nameLabel);
			hl->addWidget(_nameBox, 1);

			l->addLayout(hl);

			connect(_identifierBox, &QComboBox::currentTextChanged, [sortModel](QString text) {
				sortModel->identifierChanged(text.toInt());
			});
			connect(_guildBox, &QComboBox::currentTextChanged, [sortModel](QString text) {
				sortModel->guildChanged(text.toInt());
			});
			connect(_nameBox, &QComboBox::currentTextChanged, [sortModel](QString text) {
				sortModel->nameChanged(text);
			});
		}

		QTreeView * treeView = new QTreeView(this);
		l->addWidget(treeView, 1);
		treeView->header()->setSortIndicatorShown(true);
		treeView->header()->setStretchLastSection(true);
		treeView->header()->setDefaultAlignment(Qt::AlignHCenter);
		treeView->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
		treeView->setAlternatingRowColors(true);
		_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("Identifier") << QApplication::tr("Guild") << QApplication::tr("Name") << QApplication::tr("Value") << QApplication::tr("Amount"));

		treeView->setModel(sortModel);

		setLayout(l);
	}

	CustomStatisticsWidget::~CustomStatisticsWidget() {
	}

	void CustomStatisticsWidget::updateModList(std::vector<common::SendModManagementMessage::ModManagement> modList) {
		_mods = modList;
	}

	void CustomStatisticsWidget::selectedMod(int index) {
		_modIndex = index;
		_sourceModel->clear();
		_identifierBox->clear();
		_guildBox->clear();
		_stats.clear();
		_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("Identifier") << QApplication::tr("Guild") << QApplication::tr("Name") << QApplication::tr("Value") << QApplication::tr("Amount"));

		if (_modIndex >= int(_mods.size())) {
			return;
		}
		std::set<int32_t> identifierSet;
		std::set<int32_t> guildSet;
		std::set<QString> nameSet;
		common::SendModManagementMessage::ModManagement mm = _mods[_modIndex];
		for (auto it = mm.customStatistics.cbegin(); it != mm.customStatistics.cend(); ++it) {
			StatTuple st;
			st.identifier = it->first.first;
			st.guild = it->first.second;
			for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
				st.name = s2q(it2->name);
				st.value = it2->value;
				_stats[st]++;
				nameSet.insert(s2q(it2->name));
			}
			identifierSet.insert(it->first.first);
			guildSet.insert(it->first.second);
		}
		for (auto it = _stats.constBegin(); it != _stats.constEnd(); ++it) {
			QStandardItem * identifierItem = new QStandardItem(QString::number(it.key().identifier));
			QStandardItem * guildItem = new QStandardItem(QString::number(it.key().guild));
			QStandardItem * nameItem = new QStandardItem(it.key().name);
			QStandardItem * valueItem = new QStandardItem(QString::number(it.key().value));
			QStandardItem * amountItem = new QStandardItem(QString::number(it.value()));

			identifierItem->setData(it.key().identifier, models::CustomStatisticsRole::FilterRole);
			identifierItem->setData(it.key().identifier, models::CustomStatisticsRole::SortRole);
			guildItem->setData(it.key().guild, models::CustomStatisticsRole::FilterRole);
			guildItem->setData(it.key().guild, models::CustomStatisticsRole::SortRole);
			nameItem->setData(it.key().name, models::CustomStatisticsRole::FilterRole);
			nameItem->setData(it.key().name, models::CustomStatisticsRole::SortRole);
			valueItem->setData(it.key().value, models::CustomStatisticsRole::FilterRole);
			valueItem->setData(it.key().value, models::CustomStatisticsRole::SortRole);

			identifierItem->setEditable(false);
			guildItem->setEditable(false);
			nameItem->setEditable(false);
			valueItem->setEditable(false);
			amountItem->setEditable(false);

			_sourceModel->appendRow(QList<QStandardItem *>() << identifierItem << guildItem << nameItem << valueItem << amountItem);
		}
		/*for (auto it = mm.customStatistics.cbegin(); it != mm.customStatistics.cend(); ++it) {
			int counter = 0;
			for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); ++it2) {
				QStandardItem * identifierItem = new QStandardItem(QString::number(it->first.first));
				QStandardItem * guildItem = new QStandardItem(QString::number(it->first.second));
				QStandardItem * nameItem = new QStandardItem(s2q(it2->name));
				QStandardItem * valueItem = new QStandardItem(QString::number(it2->value));

				identifierItem->setData(it->first.first, models::CustomStatisticsRole::FilterRole);
				identifierItem->setData(it->first.first, models::CustomStatisticsRole::SortRole);
				guildItem->setData(it->first.second, models::CustomStatisticsRole::FilterRole);
				guildItem->setData(it->first.second, models::CustomStatisticsRole::SortRole);
				nameItem->setData(s2q(it2->name), models::CustomStatisticsRole::FilterRole);
				nameItem->setData(s2q(it2->name), models::CustomStatisticsRole::SortRole);
				valueItem->setData(it2->value, models::CustomStatisticsRole::FilterRole);
				valueItem->setData(it2->value, models::CustomStatisticsRole::SortRole);

				identifierItem->setEditable(false);
				guildItem->setEditable(false);
				nameItem->setEditable(false);
				valueItem->setEditable(false);

				_sourceModel->appendRow(QList<QStandardItem *>() << identifierItem << guildItem << nameItem << valueItem);

				if (counter++ == int(it->second.size()) / 2) {
					identifierItem->setBackground(QBrush(QColor("#800000")));
					guildItem->setBackground(QBrush(QColor("#800000")));
					nameItem->setBackground(QBrush(QColor("#800000")));
					valueItem->setBackground(QBrush(QColor("#800000")));
				}
				nameSet.insert(s2q(it2->name));
			}
			identifierSet.insert(it->first.first);
			guildSet.insert(it->first.second);
		}*/

		for (int32_t identifier : identifierSet) {
			_identifierBox->addItem(QString::number(identifier));
		}
		for (int32_t guild : guildSet) {
			_guildBox->addItem(QString::number(guild));
		}
		for (QString name : nameSet) {
			_nameBox->addItem(name);
		}
	}

} /* namespace widgets */
} /* namespace spine */
