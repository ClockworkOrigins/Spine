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

#include "widgets/ModInfoView.h"

#include "launcher/Gothic1And2Launcher.h"
#include "launcher/LauncherFactory.h"

#include "utils/Config.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QAbstractButton>
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <Windows.h>
	#include <shellapi.h>
#endif

using namespace spine;
using namespace spine::launcher;
using namespace spine::utils;
using namespace spine::widgets;

ModInfoView::ModInfoView(GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _layout(nullptr) {
	connect(this, &ModInfoView::errorMessage, this, &ModInfoView::showErrorMessage);

	{
		auto * const factory = LauncherFactory::getInstance();
		connect(factory, &LauncherFactory::restartAsAdmin, this, &ModInfoView::restartSpineAsAdmin);
		connect(factory, &LauncherFactory::errorMessage, this, &ModInfoView::errorMessage);
		
		{
			const auto launcher = factory->getLauncher(common::GameType::Gothic);
			const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();

			connect(gothicLauncher.data(), &Gothic1And2Launcher::installMod, this, &ModInfoView::installMod);
		}
		{
			const auto launcher = factory->getLauncher(common::GameType::Gothic2);
			const auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();

			connect(gothicLauncher.data(), &Gothic1And2Launcher::installMod, this, &ModInfoView::installMod);
		}
	}

	connect(LauncherFactory::getInstance(), &LauncherFactory::openAchievementView, this, &ModInfoView::openAchievementView);
	connect(LauncherFactory::getInstance(), &LauncherFactory::openScoreView, this, &ModInfoView::openScoreView);
	connect(LauncherFactory::getInstance(), &LauncherFactory::showSurvey, this, &ModInfoView::showSurvey);

	setHideIncompatible(generalSettingsWidget->getHideIncompatible());
	connect(generalSettingsWidget, &GeneralSettingsWidget::changedHideIncompatible, this, &ModInfoView::setHideIncompatible);

	restoreSettings();

	_layout = new QVBoxLayout();
	setLayout(_layout);
	_lastWidget = nullptr;
}

ModInfoView::~ModInfoView() {
	saveSettings();
}

void ModInfoView::selectMod(const QString & modID, const QString & iniFile) {
	_currentLauncher = LauncherFactory::getInstance()->getLauncher(modID.toInt(), iniFile);

	if (_lastWidget) {
		_layout->removeWidget(_lastWidget);
		_lastWidget->hide();
		_lastWidget = nullptr;
	}
	
	if (!_currentLauncher) return;
	
	_currentLauncher->updateView(modID.toInt(), iniFile);
	_lastWidget = _currentLauncher->getLibraryWidget();
	_layout->addWidget(_lastWidget);
	_lastWidget->show();
}

void ModInfoView::setGothicDirectory(QString directory) {
	auto * const factory = LauncherFactory::getInstance();
	const auto launcher = factory->getLauncher(common::GameType::Gothic);
	auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
	gothicLauncher->setDirectory(directory);
}

void ModInfoView::setGothic2Directory(QString directory) {
	auto * const factory = LauncherFactory::getInstance();
	const auto launcher = factory->getLauncher(common::GameType::Gothic2);
	auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
	gothicLauncher->setDirectory(directory);
}

void ModInfoView::start() {
	if (!_currentLauncher) return;

	_currentLauncher->start();
}

void ModInfoView::loginChanged() {
	if (Config::OnlineMode) {
		LauncherFactory::getInstance()->loginChanged();
	}
}

void ModInfoView::setDeveloperMode(bool active) {
	LauncherFactory::getInstance()->setDeveloperMode(active);
}

void ModInfoView::setZSpyActivated(bool active) {
	LauncherFactory::getInstance()->setZSpyActivated(active);
}

void ModInfoView::setShowAchievements(bool showAchievements) {
	LauncherFactory::getInstance()->setShowAchievements(showAchievements);
}

void ModInfoView::setHideIncompatible(bool enabled) {
	auto * const factory = LauncherFactory::getInstance();
	{
		const auto launcher = factory->getLauncher(common::GameType::Gothic);
		auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->setHideIncompatible(enabled);
	}
	{
		const auto launcher = factory->getLauncher(common::GameType::Gothic2);
		auto gothicLauncher = launcher.dynamicCast<Gothic1And2Launcher>();
		gothicLauncher->setHideIncompatible(enabled);
	}
}

void ModInfoView::updatedMod(int modID) {
	if (!_currentLauncher) return;
	
	_currentLauncher->refresh(modID);
}

void ModInfoView::restartSpineAsAdmin() {
#ifdef Q_OS_WIN
	const QString exeFileName = qApp->applicationDirPath() + "/" + qApp->applicationName();
	const int result = int(::ShellExecuteA(nullptr, "runas", exeFileName.toUtf8().constData(), nullptr, nullptr, SW_SHOWNORMAL));
	if (result > 32) { // no error
		qApp->quit();
	}
#endif
}

void ModInfoView::showErrorMessage(QString msg) {
	QMessageBox msgBox(QMessageBox::Icon::Information, QApplication::tr("ErrorOccurred"), msg, QMessageBox::StandardButton::Ok);
	msgBox.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	msgBox.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
	msgBox.exec();
}

void ModInfoView::restoreSettings() {
	LauncherFactory::getInstance()->restoreSettings();
}

void ModInfoView::saveSettings() {
	LauncherFactory::getInstance()->saveSettings();
}

void ModInfoView::showSurvey(widgets::Survey survey, int versionMajor, int versionMinor, int versionPatch) {
	SurveyDialog sd(survey, versionMajor, versionMinor, versionPatch, this);
	sd.exec();
}
