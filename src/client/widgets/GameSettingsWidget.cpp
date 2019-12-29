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

#include "widgets/GameSettingsWidget.h"

#include "utils/Config.h"

#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCheckBox>
#include <QSettings>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

GameSettingsWidget::GameSettingsWidget(QWidget * par) : QWidget(par), _showAchievementsCheckBox(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	_showAchievementsCheckBox = new QCheckBox(QApplication::tr("ShowAchievements"), this);

	const bool b = Config::IniParser->value("GAME/showAchievements", true).toBool();

	_showAchievementsCheckBox->setChecked(b);
	UPDATELANGUAGESETTEXT(_showAchievementsCheckBox, "ShowAchievements");

	l->addWidget(_showAchievementsCheckBox);

	setLayout(l);
}

GameSettingsWidget::~GameSettingsWidget() {
}

void GameSettingsWidget::saveSettings() {
	const bool b = Config::IniParser->value("GAME/showAchievements", true).toBool();
	if (b != _showAchievementsCheckBox->isChecked()) {
		Config::IniParser->setValue("GAME/showAchievements", _showAchievementsCheckBox->isChecked());
		emit showAchievementsChanged(_showAchievementsCheckBox->isChecked());
	}
}

void GameSettingsWidget::rejectSettings() {
	const bool b = Config::IniParser->value("GAME/showAchievements", true).toBool();
	_showAchievementsCheckBox->setChecked(b);
}

bool GameSettingsWidget::getShowAchievements() const {
	return _showAchievementsCheckBox->isChecked();
}
