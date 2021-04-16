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

#include "widgets/management/StatisticsWidget.h"

#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include <QApplication>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;

StatisticsWidget::StatisticsWidget(QWidget * par) : QWidget(par), _modIndex(-1), _waitSpinner(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * sa = new QScrollArea(this);
		auto * cw = new QWidget(sa);
		auto * vl = new QVBoxLayout();
		vl->setAlignment(Qt::AlignTop);
		cw->setLayout(vl);
		sa->setWidget(cw);
		sa->setWidgetResizable(true);
		sa->setProperty("default", true);
		cw->setProperty("default", true);

		l->addWidget(sa, 1);

		{
			auto * downloadsBox = new QGroupBox(QApplication::tr("Downloads"), cw);
			_downloadsLayout = new QVBoxLayout();
			downloadsBox->setLayout(_downloadsLayout);

			vl->addWidget(downloadsBox);
		}

		{
			auto * playersBox = new QGroupBox(QApplication::tr("Players"), cw);
			_playersLayout = new QVBoxLayout();
			playersBox->setLayout(_playersLayout);

			vl->addWidget(playersBox);
		}

		{
			auto * playtimesBox = new QGroupBox(QApplication::tr("Playtimes"), cw);
			_playtimesLayout = new QVBoxLayout();
			playtimesBox->setLayout(_playtimesLayout);

			vl->addWidget(playtimesBox);
		}

		{
			auto * achievementsBox = new QGroupBox(QApplication::tr("Achievements"), cw);
			_achievementsLayout = new QVBoxLayout();
			achievementsBox->setLayout(_achievementsLayout);

			vl->addWidget(achievementsBox);
		}
	}

	setLayout(l);

	qRegisterMetaType<ManagementStatistics>("ManagementStatistics");

	connect(this, &StatisticsWidget::removeSpinner, [this]() {
		if (!_waitSpinner) return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});

	connect(this, &StatisticsWidget::loadedData, this, &StatisticsWidget::updateData);
}

StatisticsWidget::~StatisticsWidget() {
	_futureWatcher.waitForFinished();
}

void StatisticsWidget::updateModList(QList<ManagementMod> modList) {
	_mods = modList;
}

void StatisticsWidget::selectedMod(int index) {
	_modIndex = index;
}

void StatisticsWidget::updateView() {
	if (_modIndex == -1 || _modIndex >= _mods.size()) return;
	
	qDeleteAll(_labelList);
	_labelList.clear();
	
	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["Language"] = Config::Language;
	requestData["ModID"] = _mods[_modIndex].id;
	
	const auto f = https::Https::postAsync(MANAGEMENTSERVER_PORT, "getStatistics", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		ManagementStatistics ms;
		ms.read(json);
		
		emit loadedData(ms);
		emit removeSpinner();
	});
	_futureWatcher.setFuture(f);
}

void StatisticsWidget::updateData(ManagementStatistics content) {
	{
		auto * overallDownloadsLabel = new QLabel(QApplication::tr("OverallDownloads"), this);
		auto * overallDownloadsCountLabel = new QLabel(i2s(content.overallDownloads), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(overallDownloadsLabel, 0, Qt::AlignLeft);
		hl->addWidget(overallDownloadsCountLabel, 0, Qt::AlignRight);
		_downloadsLayout->addLayout(hl);

		_labelList.append(overallDownloadsLabel);
		_labelList.append(overallDownloadsCountLabel);
	}
	{
		const auto m = content.downloadsPerVersion;
		for (const auto & it : m) {
			auto * downloadsLabel = new QLabel(QApplication::tr("DownloadsForVersion").arg(it.version), this);
			auto * downloadsCountLabel = new QLabel(i2s(it.downloads), this);
			auto * hl = new QHBoxLayout();
			hl->addWidget(downloadsLabel, 0, Qt::AlignLeft);
			hl->addWidget(downloadsCountLabel, 0, Qt::AlignRight);
			_downloadsLayout->addLayout(hl);

			_labelList.append(downloadsLabel);
			_labelList.append(downloadsCountLabel);
		}
	}
	{
		auto * playersLabel = new QLabel(QApplication::tr("PlayersOverall"), this);
		auto * playersCountLabel = new QLabel(i2s(content.overallPlayerCount), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(playersLabel, 0, Qt::AlignLeft);
		hl->addWidget(playersCountLabel, 0, Qt::AlignRight);
		_playersLayout->addLayout(hl);

		_labelList.append(playersLabel);
		_labelList.append(playersCountLabel);
	}
	{
		auto * playersLabel = new QLabel(QApplication::tr("Players24Hours"), this);
		auto * playersCountLabel = new QLabel(i2s(content.last24HoursPlayerCount), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(playersLabel, 0, Qt::AlignLeft);
		hl->addWidget(playersCountLabel, 0, Qt::AlignRight);
		_playersLayout->addLayout(hl);

		_labelList.append(playersLabel);
		_labelList.append(playersCountLabel);
	}
	{
		auto * playersLabel = new QLabel(QApplication::tr("Players7Days"), this);
		auto * playersCountLabel = new QLabel(i2s(content.last7DaysPlayerCount), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(playersLabel, 0, Qt::AlignLeft);
		hl->addWidget(playersCountLabel, 0, Qt::AlignRight);
		_playersLayout->addLayout(hl);

		_labelList.append(playersLabel);
		_labelList.append(playersCountLabel);
	}
	{
		auto * maxPlaytimeLabel = new QLabel(QApplication::tr("MaxPlaytime"), this);
		auto * maxPlaytimeCountLabel = new QLabel(timeToString(content.playTime.maximum), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(maxPlaytimeLabel, 0, Qt::AlignLeft);
		hl->addWidget(maxPlaytimeCountLabel, 0, Qt::AlignRight);
		_playtimesLayout->addLayout(hl);

		_labelList.append(maxPlaytimeLabel);
		_labelList.append(maxPlaytimeCountLabel);
	}
	{
		auto * medianPlaytimeLabel = new QLabel(QApplication::tr("MedianPlaytime"), this);
		auto * medianPlaytimeCountLabel = new QLabel(timeToString(content.playTime.median), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(medianPlaytimeLabel, 0, Qt::AlignLeft);
		hl->addWidget(medianPlaytimeCountLabel, 0, Qt::AlignRight);
		_playtimesLayout->addLayout(hl);

		_labelList.append(medianPlaytimeLabel);
		_labelList.append(medianPlaytimeCountLabel);
	}
	{
		auto * avgPlaytimeLabel = new QLabel(QApplication::tr("AvgPlaytime"), this);
		auto * avgPlaytimeCountLabel = new QLabel(timeToString(content.playTime.average), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(avgPlaytimeLabel, 0, Qt::AlignLeft);
		hl->addWidget(avgPlaytimeCountLabel, 0, Qt::AlignRight);
		_playtimesLayout->addLayout(hl);

		_labelList.append(avgPlaytimeLabel);
		_labelList.append(avgPlaytimeCountLabel);
	}
	{
		auto * maxSessiontimeLabel = new QLabel(QApplication::tr("MaxSessiontime"), this);
		auto * maxSessiontimeCountLabel = new QLabel(timeToString(content.sessionTime.maximum), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(maxSessiontimeLabel, 0, Qt::AlignLeft);
		hl->addWidget(maxSessiontimeCountLabel, 0, Qt::AlignRight);
		_playtimesLayout->addLayout(hl);

		_labelList.append(maxSessiontimeLabel);
		_labelList.append(maxSessiontimeCountLabel);
	}
	{
		auto * medianSessiontimeLabel = new QLabel(QApplication::tr("MedianSessiontime"), this);
		auto * medianSessiontimeCountLabel = new QLabel(timeToString(content.sessionTime.median), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(medianSessiontimeLabel, 0, Qt::AlignLeft);
		hl->addWidget(medianSessiontimeCountLabel, 0, Qt::AlignRight);
		_playtimesLayout->addLayout(hl);

		_labelList.append(medianSessiontimeLabel);
		_labelList.append(medianSessiontimeCountLabel);
	}
	{
		auto * avgSessiontimeLabel = new QLabel(QApplication::tr("AvgSessiontime"), this);
		auto * avgSessiontimeCountLabel = new QLabel(timeToString(content.sessionTime.average), this);
		auto * hl = new QHBoxLayout();
		hl->addWidget(avgSessiontimeLabel, 0, Qt::AlignLeft);
		hl->addWidget(avgSessiontimeCountLabel, 0, Qt::AlignRight);
		_playtimesLayout->addLayout(hl);

		_labelList.append(avgSessiontimeLabel);
		_labelList.append(avgSessiontimeCountLabel);
	}
	{
		const auto m = content.achievementStatistics;
		for (const auto & it : m) {
			{
				auto * achievementLabel = new QLabel(QApplication::tr("MaxAchievementTime").arg(it.name), this);
				auto * achievementCountLabel = new QLabel(timeToString(it.statistic.maximum), this);
				auto * hl = new QHBoxLayout();
				hl->addWidget(achievementLabel, 0, Qt::AlignLeft);
				hl->addWidget(achievementCountLabel, 0, Qt::AlignRight);
				_achievementsLayout->addLayout(hl);

				_labelList.append(achievementLabel);
				_labelList.append(achievementCountLabel);
			}
			{
				auto * achievementLabel = new QLabel(QApplication::tr("MedianAchievementTime").arg(it.name), this);
				auto * achievementCountLabel = new QLabel(timeToString(it.statistic.median), this);
				auto * hl = new QHBoxLayout();
				hl->addWidget(achievementLabel, 0, Qt::AlignLeft);
				hl->addWidget(achievementCountLabel, 0, Qt::AlignRight);
				_achievementsLayout->addLayout(hl);

				_labelList.append(achievementLabel);
				_labelList.append(achievementCountLabel);
			}
			{
				auto * achievementLabel = new QLabel(QApplication::tr("AvgAchievementTime").arg(it.name), this);
				auto * achievementCountLabel = new QLabel(timeToString(it.statistic.average), this);
				auto * hl = new QHBoxLayout();
				hl->addWidget(achievementLabel, 0, Qt::AlignLeft);
				hl->addWidget(achievementCountLabel, 0, Qt::AlignRight);
				_achievementsLayout->addLayout(hl);

				_labelList.append(achievementLabel);
				_labelList.append(achievementCountLabel);
			}
		}
	}
}
