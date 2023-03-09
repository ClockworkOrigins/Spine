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

#include "widgets/management/AchievementsWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include "widgets/management/AchievementWidget.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;

AchievementsWidget::AchievementsWidget(QWidget * par) : QWidget(par), _modIndex(-1), _waitSpinner(nullptr) {
	auto * vl = new QVBoxLayout();

	{
		auto * sa = new QScrollArea(this);
		auto * cw = new QWidget(sa);
		_layout = new QVBoxLayout();
		_layout->setAlignment(Qt::AlignTop);
		cw->setLayout(_layout);
		sa->setWidget(cw);
		sa->setWidgetResizable(true);
		sa->setProperty("default", true);
		cw->setProperty("default", true);

		vl->addWidget(sa, 1);
	}

	{
		auto * hl = new QHBoxLayout();
		hl->addStretch(1);

		auto * pb = new QPushButton("+", this);
		hl->addWidget(pb);
		connect(pb, &QPushButton::released, this, &AchievementsWidget::addAchievement);

		vl->addLayout(hl);
	}

	{
		auto * hl = new QHBoxLayout();
		hl->addStretch(1);

		_clearAchievementsButton = new QPushButton(QApplication::tr("ResetAchievementProgress"), this);
		_clearAchievementsButton->setToolTip(QApplication::tr("ResetAchievementProgressTooltip"));
		hl->addWidget(_clearAchievementsButton);
		connect(_clearAchievementsButton, &QPushButton::released, this, &AchievementsWidget::clearAchievementProgress);

		vl->addLayout(hl);
	}

	auto * dbb = new QDialogButtonBox(this);
	auto * submitButton = new QPushButton(QApplication::tr("Submit"), this);
	dbb->addButton(submitButton, QDialogButtonBox::ButtonRole::AcceptRole);
	connect(submitButton, &QPushButton::released, this, &AchievementsWidget::updateAchievements);

	qRegisterMetaType<QList<ManagementAchievement>>("QList<ManagementAchievement>");

	connect(this, &AchievementsWidget::removeSpinner, this, [this]() {
		if (!_waitSpinner) return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});

	connect(this, &AchievementsWidget::loadedAchievements, this, &AchievementsWidget::updateAchievementViews);

	vl->addWidget(dbb);

	setLayout(vl);
}

AchievementsWidget::~AchievementsWidget() {
	_futureWatcher.waitForFinished();
}

void AchievementsWidget::updateModList(QList<ManagementMod> modList) {
	_mods = modList;
}

void AchievementsWidget::selectedMod(int index) {
	Q_ASSERT(index < static_cast<int>(_mods.size()));
	_modIndex = index;
	
	for (AchievementWidget * aw : _achievementEdits) {
		aw->deleteLater();
	}
	_achievementEdits.clear();
}

void AchievementsWidget::updateView() {
	if (_modIndex == -1 || _modIndex >= _mods.size())
		return;

	selectedMod(_modIndex);

	setDisabled(_mods[_modIndex].packageID >= 0);

	if (_mods[_modIndex].packageID >= 0)
		return;
	
	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	for (AchievementWidget * aw : _achievementEdits) {
		aw->deleteLater();
	}
	_achievementEdits.clear();

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ModID"] = _mods[_modIndex].id;
	
	const auto f = https::Https::postAsync(MANAGEMENTSERVER_PORT, "getAchievements", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		QList<ManagementAchievement> achievementList;

		const auto it = json.find("Achievements");
		if (it != json.end()) {
			const auto achievementArr = it->toArray();
			for (auto achV : achievementArr) {
				const auto ach = achV.toObject();
				
				if (ach.isEmpty()) continue;

				ManagementAchievement ma;
				ma.read(ach);

				achievementList.append(ma);
			}
		}

		const bool canClear = json.contains("Clearable");
		
		emit loadedAchievements(achievementList, canClear);
		emit removeSpinner();
	});
	_futureWatcher.setFuture(f);
}

void AchievementsWidget::updateAchievements() {
	if (_modIndex == -1) return;

	QJsonObject json;
	json["ModID"] = _mods[_modIndex].id;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;

	common::UploadAchievementIconsMessage uaim;
	uaim.modID = _mods[_modIndex].id;

	QJsonArray achievementArray;
	for (auto* achievementEdit : _achievementEdits) {
		const auto achievement = achievementEdit->getAchievement();

		if (achievement.isValid()) {
			QJsonObject ach;
			achievement.write(ach);
			achievementArray.append(ach);

			if (!achievement.lockedImageData.empty()) {
				common::UploadAchievementIconsMessage::Icon icon;
				icon.name = q2s(achievement.lockedImageName);
				icon.data = achievement.lockedImageData;
				uaim.icons.push_back(icon);
			}

			if (!achievement.unlockedImageData.empty()) {
				common::UploadAchievementIconsMessage::Icon icon;
				icon.name = q2s(achievement.unlockedImageName);
				icon.data = achievement.unlockedImageData;
				uaim.icons.push_back(icon);
			}
		}
	}

	if (achievementArray.isEmpty()) return; // no need to synchronize something if there are no achievements
	
	json["Achievements"] = achievementArray;

	const QJsonDocument doc(json);
	const QString content = doc.toJson(QJsonDocument::Compact);

	https::Https::postAsync(MANAGEMENTSERVER_PORT, "updateAchievements", content, [](const QJsonObject &, int) {
		// we could do some error handling here
	});

	if (uaim.icons.empty()) return;

	const std::string serialized = uaim.SerializeBlank();
	clockUtils::sockets::TcpSocket sock;
	const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
	if (clockUtils::ClockError::SUCCESS == cErr) {
		sock.writePacket(serialized);
	}
}

void AchievementsWidget::addAchievement() {
	auto * achievementWidget = new AchievementWidget(this);

	_layout->addWidget(achievementWidget);

	_achievementEdits.push_back(achievementWidget);
}

void AchievementsWidget::updateAchievementViews(QList<ManagementAchievement> achievementList, bool canClear) {
	for (const auto & achievement : achievementList) {
		auto * achievementWidget = new AchievementWidget(this);
		achievementWidget->setAchievement(_mods[_modIndex].id, achievement);

		_layout->addWidget(achievementWidget);

		_achievementEdits.push_back(achievementWidget);
	}

	_clearAchievementsButton->setVisible(canClear);
}

void AchievementsWidget::clearAchievementProgress() {
	QJsonObject json;
	json["ProjectID"] = _mods[_modIndex].id;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;

	const QJsonDocument doc(json);
	const QString content = doc.toJson(QJsonDocument::Compact);

	https::Https::postAsync(MANAGEMENTSERVER_PORT, "clearAchievementProgress", content, [](const QJsonObject &, int) {});
}
