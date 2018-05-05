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

#include "widgets/management/AchievementsWidget.h"

#include "SpineConfig.h"

#include "widgets/management/AchievementWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	AchievementsWidget::AchievementsWidget(QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _achievementEdits() {
		QVBoxLayout * vl = new QVBoxLayout();

		{
			QScrollArea * sa = new QScrollArea(this);
			QWidget * cw = new QWidget(sa);
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
			QHBoxLayout * hl = new QHBoxLayout();
			hl->addStretch(1);

			QPushButton * pb = new QPushButton("+", this);
			hl->addWidget(pb);
			connect(pb, &QPushButton::released, this, &AchievementsWidget::addAchievement);

			vl->addLayout(hl);
		}

		QDialogButtonBox * dbb = new QDialogButtonBox(this);
		QPushButton * submitButton = new QPushButton(QApplication::tr("Submit"), this);
		dbb->addButton(submitButton, QDialogButtonBox::ButtonRole::AcceptRole);
		connect(submitButton, &QPushButton::released, this, &AchievementsWidget::updateAchievements);

		vl->addWidget(dbb);

		setLayout(vl);
	}

	AchievementsWidget::~AchievementsWidget() {
	}

	void AchievementsWidget::updateModList(std::vector<common::SendModManagementMessage::ModManagement> modList) {
		_mods = modList;
	}

	void AchievementsWidget::selectedMod(int index) {
		Q_ASSERT(index < _mods.size());
		_modIndex = index;
		
		for (AchievementWidget * aw : _achievementEdits) {
			aw->deleteLater();
		}
		_achievementEdits.clear();

		for (const auto & achievement : _mods[_modIndex].achievements) {
			AchievementWidget * achievementWidget = new AchievementWidget(this);
			achievementWidget->setAchievement(_mods[_modIndex].modID, achievement);

			_layout->addWidget(achievementWidget);

			_achievementEdits.push_back(achievementWidget);
		}
	}

	void AchievementsWidget::updateAchievements() {
		if (_modIndex == -1) {
			return;
		}
		common::UpdateAchievementsMessage uam;
		uam.modID = _mods[_modIndex].modID;

		for (auto & achievementEdit : _achievementEdits) {
			common::UpdateAchievementsMessage::Achievement achievement;

			achievement = achievementEdit->getAchievement();

			if (achievement.isValid()) {
				uam.achievements.push_back(achievement);
			}
		}

		const std::string serialized = uam.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			sock.writePacket(serialized);
		}
	}

	void AchievementsWidget::addAchievement() {
		AchievementWidget * achievementWidget = new AchievementWidget(this);

		_layout->addWidget(achievementWidget);

		_achievementEdits.push_back(achievementWidget);
	}

} /* namespace widgets */
} /* namespace spine */
