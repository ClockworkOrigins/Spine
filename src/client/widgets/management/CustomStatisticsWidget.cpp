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

#include "SpineConfig.h"

#include "https/Https.h"

#include "models/CustomStatisticsModel.h"

#include "utils/Conversion.h"

#include "widgets/WaitSpinner.h"

#include <QApplication>
#include <QComboBox>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace spine {
namespace client {
namespace widgets {

	CustomStatisticsWidget::CustomStatisticsWidget(const QString & username, const QString & password, QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _sourceModel(nullptr), _waitSpinner(nullptr), _username(username), _password(password) {
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

		qRegisterMetaType<ManagementCustomStatistics>("ManagementCustomStatistics");

		connect(this, &CustomStatisticsWidget::removeSpinner, [this]() {
			if (!_waitSpinner) return;
			
			_waitSpinner->deleteLater();
			_waitSpinner = nullptr;
		});

		connect(this, &CustomStatisticsWidget::loadedData, this, &CustomStatisticsWidget::updateData);

		treeView->setModel(sortModel);

		setLayout(l);
	}

	void CustomStatisticsWidget::updateModList(QList<client::ManagementMod> modList) {
		_mods = modList;
	}

	void CustomStatisticsWidget::selectedMod(int index) {
		_modIndex = index;
	}

	void CustomStatisticsWidget::updateView() {
		if (_modIndex == -1 || _modIndex >= _mods.size()) return;
		
		_sourceModel->clear();
		_identifierBox->clear();
		_guildBox->clear();
		_stats.clear();
		_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("Identifier") << QApplication::tr("Guild") << QApplication::tr("Name") << QApplication::tr("Value") << QApplication::tr("Amount"));
		
		delete _waitSpinner;
		_waitSpinner = new spine::widgets::WaitSpinner(QApplication::tr("Updating"), this);

		QJsonObject json;
		json["Username"] = _username;
		json["Password"] = _password;
		json["ModID"] = _mods[_modIndex].id;
		
		https::Https::postAsync(MANAGEMENTSERVER_PORT, "getCustomStatistics", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) {
				emit removeSpinner();
				return;
			}
			ManagementCustomStatistics mcs;
			mcs.read(json);

			emit loadedData(mcs);
			emit removeSpinner();
		});
	}

	void CustomStatisticsWidget::updateData(ManagementCustomStatistics content) {
		std::set<int32_t> identifierSet;
		std::set<int32_t> guildSet;
		std::set<QString> nameSet;
		for (auto it = content.stats.cbegin(); it != content.stats.cend(); ++it) {
			StatTuple st;
			st.identifier = it.key().first;
			st.guild = it.key().second;
			for (const auto & entry : it.value()) {
				st.name = entry.name;
				st.value = entry.value;
				_stats[st]++;
				nameSet.insert(entry.name);
			}
			identifierSet.insert(it.key().first);
			guildSet.insert(it.key().second);
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

		for (int32_t identifier : identifierSet) {
			_identifierBox->addItem(QString::number(identifier));
		}
		for (int32_t guild : guildSet) {
			_guildBox->addItem(QString::number(guild));
		}
		for (const QString & name : nameSet) {
			_nameBox->addItem(name);
		}
	}

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
