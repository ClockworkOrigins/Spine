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

#include "ScreenshotManager.h"

#include <thread>

#include "utils/WindowsExtensions.h"

#include "widgets/LocationSettingsWidget.h"

#include <QDirIterator>
#include <QGuiApplication>
#include <QScreen>
#include <QtConcurrentRun>

#ifdef Q_OS_WIN
	#include <Windows.h>
#endif

using namespace spine;

ScreenshotManager::ScreenshotManager(QObject * par) : QObject(par), _running(false), _modID() {
	_screenshotDirectory = widgets::LocationSettingsWidget::getInstance()->getScreenshotDirectory();
	connect(widgets::LocationSettingsWidget::getInstance(), &widgets::LocationSettingsWidget::screenshotDirectoryChanged, this, &ScreenshotManager::setScreenshotDirectory);
}

ScreenshotManager::~ScreenshotManager() {
	if (!_workerThread.isFinished()) {
		_workerThread.waitForFinished();
	}
}

void ScreenshotManager::start(int32_t modID) {
	_modID = modID;

	const QString path = _screenshotDirectory + "/" + QString::number(_modID) + "/";
	if (!QDir(path).exists()) {
		const bool b = QDir(path).mkpath(path);
		Q_UNUSED(b)
	}

	_running = true;
	_workerThread = QtConcurrent::run(this, &ScreenshotManager::execute);
}

void ScreenshotManager::stop() {
	_running = false;
	_workerThread.waitForFinished();
}

void ScreenshotManager::setScreenshotDirectory(QString screenshotDirectory) {
	_screenshotDirectory = screenshotDirectory;
}

void ScreenshotManager::execute() {
#ifdef Q_OS_WIN
	bool toggled = false;
	while (_running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		const SHORT tabKeyState = GetAsyncKeyState(VK_F12);

		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if ((1 << 15) & tabKeyState && !toggled) {
			toggled = true;
			takeScreenshot();
		} else if (!((1 << 15) & tabKeyState) && toggled) {
			toggled = false;
		}
	}
#endif
}

void ScreenshotManager::takeScreenshot() {
	QScreen * screen = QGuiApplication::primaryScreen();

	if (!screen)
		return;

	const auto pixmap = screen->grabWindow(0);
	pixmap.save(_screenshotDirectory + "/" + QString::number(_modID) + "/screen_" + QString::number(time(nullptr)) + ".jpg");
}
