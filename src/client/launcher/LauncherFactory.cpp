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
// Copyright 2019 Clockwork Origins

#include "launcher/LauncherFactory.h"

#include "launcher/GameLauncher.h"
#include "launcher/Gothic1Launcher.h"
#include "launcher/Gothic2Launcher.h"

using namespace spine::common;
using namespace spine::launcher;

LauncherFactory * LauncherFactory::getInstance() {
	static LauncherFactory factory;
	return &factory;
}

LauncherFactory::LauncherFactory() {
	_launchers.append(ILauncherPtr(new Gothic1Launcher));
	_launchers.append(ILauncherPtr(new Gothic2Launcher));
	_launchers.append(ILauncherPtr(new GameLauncher));

	for (auto & launcher : _launchers) {
		initLauncher(launcher);
	}
}

ILauncherPtr LauncherFactory::getLauncher(GameType gothic) const {
	for (const auto & l : _launchers) {
		if (!l->supportsGame(gothic)) continue;

		return l;
	}

	return ILauncherPtr();
}

ILauncherPtr LauncherFactory::getLauncher(int32_t modID, const QString & iniFile) const {
	for (const auto & l : _launchers) {
		if (!l->supportsModAndIni(modID, iniFile)) continue;

		return l;
	}

	return ILauncherPtr();
}

void LauncherFactory::loginChanged() {
	for (const auto & l : _launchers) {
		l->loginChanged();
	}
}

void LauncherFactory::setDeveloperMode(bool enabled) {		
	for (const auto & l : _launchers) {
		l->setDeveloperMode(enabled);
	}
}

void LauncherFactory::setShowAchievements(bool enabled) {		
	for (const auto & l : _launchers) {
		l->setShowAchievements(enabled);
	}
}

void LauncherFactory::setZSpyActivated(bool enabled) {		
	for (const auto & l : _launchers) {
		l->setZSpyActivated(enabled);
	}
}

void LauncherFactory::restoreSettings() {
	for (const auto & l : _launchers) {
		l->restoreSettings();
	}
}

void LauncherFactory::saveSettings() {
	for (const auto & l : _launchers) {
		l->saveSettings();
	}
}

void LauncherFactory::updateModel(QStandardItemModel * model) {
	for (const auto & l : _launchers) {
		l->updateModel(model);
	}
}

void LauncherFactory::initLauncher(ILauncherPtr launcher) const {
	if (!launcher) return;
	
	launcher->init();
	
	connect(launcher.data(), &ILauncher::restartAsAdmin, this, &LauncherFactory::restartAsAdmin);
	connect(launcher.data(), &ILauncher::errorMessage, this, &LauncherFactory::errorMessage);
	connect(launcher.data(), &ILauncher::openAchievementView, this, &LauncherFactory::openAchievementView);
	connect(launcher.data(), &ILauncher::openScoreView, this, &LauncherFactory::openScoreView);
	connect(launcher.data(), &ILauncher::loadedSurvey, this, &LauncherFactory::showSurvey);
	connect(this, &LauncherFactory::finishedInstallation, launcher.data(), &ILauncher::finishedInstallation);
	connect(this, &LauncherFactory::updateStarted, launcher.data(), &ILauncher::updateStarted);
	connect(this, &LauncherFactory::updateFinished, launcher.data(), &ILauncher::updateFinished);
}
