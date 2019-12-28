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

#include "Config.h"

#include "launcher/Gothic1And2Launcher.h"
#include "launcher/LauncherFactory.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QAbstractButton>
#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <Windows.h>
	#include <shellapi.h>
#endif

namespace spine {
namespace widgets {

	ModInfoView::ModInfoView(GeneralSettingsWidget * generalSettingsWidget, QSettings * iniParser, QWidget * par) : QWidget(par), _layout(nullptr), _iniParser(iniParser) {
		connect(this, &ModInfoView::errorMessage, this, &ModInfoView::showErrorMessage);

		{
			const auto factory = launcher::LauncherFactory::getInstance();
			connect(factory, &launcher::LauncherFactory::restartAsAdmin, this, &ModInfoView::restartSpineAsAdmin);
			
			{
				const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC);
				const auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();

				connect(gothicLauncher.data(), &launcher::Gothic1And2Launcher::installMod, this, &ModInfoView::installMod);
			}
			{
				const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC2);
				const auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();

				connect(gothicLauncher.data(), &launcher::Gothic1And2Launcher::installMod, this, &ModInfoView::installMod);
			}
		}

		setHideIncompatible(generalSettingsWidget->getHideIncompatible());
		connect(generalSettingsWidget, &GeneralSettingsWidget::changedHideIncompatible, this, &ModInfoView::setHideIncompatible);

		restoreSettings();

		_lastWidget = nullptr;
	}

	ModInfoView::~ModInfoView() {
		saveSettings();
	}

	void ModInfoView::selectMod(const QString & modID, const QString & iniFile) {
		_currentLauncher = launcher::LauncherFactory::getInstance()->getLauncher(modID.toInt(), iniFile);

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
		const auto factory = launcher::LauncherFactory::getInstance();
		const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC);
		auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();
		gothicLauncher->setDirectory(directory);
	}

	void ModInfoView::setGothic2Directory(QString directory) {
		const auto factory = launcher::LauncherFactory::getInstance();
		const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC2);
		auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();
		gothicLauncher->setDirectory(directory);
	}

	void ModInfoView::start() {
		if (!_currentLauncher) return;

		_currentLauncher->start();
	}

	void ModInfoView::loginChanged() {
		if (Config::OnlineMode) {
			launcher::LauncherFactory::getInstance()->loginChanged();
		}
	}

	void ModInfoView::setDeveloperMode(bool active) {
		launcher::LauncherFactory::getInstance()->setDeveloperMode(active);
	}

	void ModInfoView::setZSpyActivated(bool active) {
		launcher::LauncherFactory::getInstance()->setZSpyActivated(active);
	}

	void ModInfoView::setShowAchievements(bool showAchievements) {
		launcher::LauncherFactory::getInstance()->setShowAchievements(showAchievements);
	}

	void ModInfoView::setHideIncompatible(bool enabled) {
		const auto factory = launcher::LauncherFactory::getInstance();
		{
			const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC);
			auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();
			gothicLauncher->setHideIncompatible(enabled);
		}
		{
			const auto launcher = factory->getLauncher(common::GothicVersion::GOTHIC2);
			auto gothicLauncher = launcher.dynamicCast<launcher::Gothic1And2Launcher>();
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
		launcher::LauncherFactory::getInstance()->restoreSettings(_iniParser);
	}

	void ModInfoView::saveSettings() {
		launcher::LauncherFactory::getInstance()->saveSettings(_iniParser);
	}

} /* namespace widgets */
} /* namespace spine */
