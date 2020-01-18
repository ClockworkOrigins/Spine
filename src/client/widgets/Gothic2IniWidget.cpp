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

#include "widgets/Gothic2IniWidget.h"

#include "widgets/gothic2IniPages/ControlsPage.h"
#include "widgets/gothic2IniPages/EnginePage.h"
#include "widgets/gothic2IniPages/GamePage.h"
#include "widgets/gothic2IniPages/PerformancePage.h"
#include "widgets/gothic2IniPages/SoundPage.h"
#include "widgets/gothic2IniPages/VideoPage.h"
#include "widgets/gothic2IniPages/VisualizationPage.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

using namespace spine::widgets;

Gothic2IniWidget::Gothic2IniWidget(QString directory, QWidget * par) : QWidget(par), _iniParser(new QSettings(directory + "/System/Gothic.ini", QSettings::IniFormat)), _directory(directory), _tabWidget(nullptr), _gamePage(nullptr), _performancePage(nullptr), _enginePage(nullptr), _visualizationPage(nullptr), _videoPage(nullptr), _soundPage(nullptr), _controlsPage(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();

	_tabWidget = new QTabWidget(this);

	_gamePage = new g2::GamePage(_iniParser, _tabWidget);
	_performancePage = new g2::PerformancePage(_iniParser, _tabWidget);
	_enginePage = new g2::EnginePage(_iniParser, _tabWidget);
	_visualizationPage = new g2::VisualizationPage(_iniParser, _tabWidget);
	_videoPage = new g2::VideoPage(_iniParser, _tabWidget);
	_soundPage = new g2::SoundPage(_iniParser, _tabWidget);
	_controlsPage = new g2::ControlsPage(_iniParser, _tabWidget);

	_tabWidget->addTab(_gamePage, QApplication::tr("Game"));
	_tabWidget->addTab(_performancePage, QApplication::tr("Performance"));
	_tabWidget->addTab(_enginePage, QApplication::tr("Engine"));
	_tabWidget->addTab(_visualizationPage, QApplication::tr("Visualization"));
	_tabWidget->addTab(_videoPage, QApplication::tr("Video"));
	_tabWidget->addTab(_soundPage, QApplication::tr("Sound"));
	_tabWidget->addTab(_controlsPage, QApplication::tr("Controls"));

	l->addWidget(_tabWidget);

	{
		QHBoxLayout * hl = new QHBoxLayout();
		QPushButton * backupButton = new QPushButton(QApplication::tr("Backup"), this);
		QPushButton * restoreButton = new QPushButton(QApplication::tr("Restore"), this);
		hl->addWidget(backupButton);
		hl->addWidget(restoreButton);
		hl->addStretch(1);
		l->addLayout(hl);

		connect(backupButton, SIGNAL(released()), this, SLOT(backup()));
		connect(restoreButton, SIGNAL(released()), this, SLOT(restore()));
	}

	setLayout(l);
}

Gothic2IniWidget::~Gothic2IniWidget() {
	delete _iniParser;
}

void Gothic2IniWidget::accept() {
	_gamePage->accept();
	_performancePage->accept();
	_enginePage->accept();
	_visualizationPage->accept();
	_videoPage->accept();
	_soundPage->accept();
	_controlsPage->accept();
}

void Gothic2IniWidget::reject() {
	_gamePage->reject();
	_performancePage->reject();
	_enginePage->reject();
	_visualizationPage->reject();
	_videoPage->reject();
	_soundPage->reject();
	_controlsPage->reject();
}

void Gothic2IniWidget::backup() {
	const QString folder = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectBackupFolder"));
	if (!folder.isEmpty()) {
		QFile::copy(_directory + "/System/Gothic.ini", folder + "/Gothic.ini");
	}
}

void Gothic2IniWidget::restore() {
	const QString backupFile = QFileDialog::getOpenFileName(this, QApplication::tr("SelectBackupFile"), QString(), "Gothic.ini");
	if (!backupFile.isEmpty()) {
		delete _iniParser;

		if (QFileInfo::exists(_directory + "/System/Gothic.ini")) {
			QFile::remove(_directory + "/System/Gothic.ini");
		}
		
		QFile::copy(backupFile, _directory + "/System/Gothic.ini");
		
		_iniParser = new QSettings(_directory + "/System/Gothic.ini", QSettings::IniFormat);
		
		_gamePage->updateSettings(_iniParser);
		_performancePage->updateSettings(_iniParser);
		_enginePage->updateSettings(_iniParser);
		_visualizationPage->updateSettings(_iniParser);
		_videoPage->updateSettings(_iniParser);
		_soundPage->updateSettings(_iniParser);
		_controlsPage->updateSettings(_iniParser);
	}
}
