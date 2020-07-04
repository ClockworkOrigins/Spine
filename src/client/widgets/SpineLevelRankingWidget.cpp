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

#include "widgets/SpineLevelRankingWidget.h"

#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

using namespace spine::gui;
using namespace spine::utils;
using namespace spine::widgets;

SpineLevelRankingWidget::SpineLevelRankingWidget(QWidget * par) : QWidget(par), _waitSpinner(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();

	_tableView = new QTableView(this);
	_model = new QStandardItemModel(_tableView);
	_model->setHorizontalHeaderLabels({ QApplication::tr("Rank"), QApplication::tr("Name"), QApplication::tr("Level"), QApplication::tr("Experience") });

	_tableView->setModel(_model);
	_tableView->verticalHeader()->hide();
	_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	_tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
	_tableView->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
	_tableView->setProperty("score", true);

	l->addWidget(_tableView);

	setLayout(l);

	qRegisterMetaType<QList<RankingEntry>>("QList<RankingEntry>");
	
	connect(this, &SpineLevelRankingWidget::receivedRankings, this, &SpineLevelRankingWidget::updateView);
}

void SpineLevelRankingWidget::requestUpdate() {
	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("LoadingScores"), this);

	https::Https::postAsync(DATABASESERVER_PORT, "getSpineLevelRanking", "", [this](const QJsonObject & json, int status) {
		const auto it = json.find("Names");
		
		QList<RankingEntry> rankingEntries;
		
		if (it != json.end()) {
			const auto jsonArray = it->toArray();
			for (const auto & i : jsonArray) {
				const auto jsonEntry = i.toObject();

				RankingEntry re;
				re.username = jsonEntry["Name"].toString();
				re.rank = jsonEntry["Rank"].toString().toInt();
				re.level = jsonEntry["Level"].toString().toInt();
				re.xp = jsonEntry["XP"].toString().toInt();

				rankingEntries.push_back(re);
			}
		}
		
		emit receivedRankings(rankingEntries);
	});
}

void SpineLevelRankingWidget::updateView(QList<RankingEntry> rankingEntries) {
	delete _waitSpinner;
	_waitSpinner = nullptr;

	_model->clear();
	_model->setHorizontalHeaderLabels({ QApplication::tr("Rank"), QApplication::tr("Name"), QApplication::tr("Level"), QApplication::tr("Experience") });

	int row = 0;
	
	for (const auto & re : rankingEntries) {
		_model->appendRow({ new QStandardItem(QString::number(re.rank)), new QStandardItem(re.username), new QStandardItem(QString::number(re.level)), new QStandardItem(QString::number(re.xp)) });
		_model->item(row, 0)->setEnabled(re.username == Config::Username);
		_model->item(row, 0)->setTextAlignment(Qt::AlignCenter);
		_model->item(row, 1)->setEnabled(re.username == Config::Username);
		_model->item(row, 1)->setTextAlignment(Qt::AlignCenter);
		_model->item(row, 2)->setEnabled(re.username == Config::Username);
		_model->item(row, 2)->setTextAlignment(Qt::AlignCenter);
		row++;
	}
}
