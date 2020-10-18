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

#include "widgets/Gothic1IniWidget.h"

#include "widgets/gothicIniPages/ControlsPage.h"
#include "widgets/gothicIniPages/EnginePage.h"
#include "widgets/gothicIniPages/GamePage.h"
#include "widgets/gothicIniPages/PerformancePage.h"
#include "widgets/gothicIniPages/SoundPage.h"
#include "widgets/gothicIniPages/VideoPage.h"
#include "widgets/gothicIniPages/VisualizationPage.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

using namespace spine::widgets;

Gothic1IniWidget::Gothic1IniWidget(QString directory, QWidget * par) : QWidget(par), _iniParser(new QSettings(directory + "/System/Gothic.ini", QSettings::IniFormat)), _directory(directory), _tabWidget(nullptr), _gamePage(nullptr), _performancePage(nullptr), _enginePage(nullptr), _visualizationPage(nullptr), _videoPage(nullptr), _soundPage(nullptr), _controlsPage(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();

	_tabWidget = new QTabWidget(this);

	_gamePage = new g1::GamePage(_iniParser, _tabWidget);
	_performancePage = new g1::PerformancePage(_iniParser, _tabWidget);
	_enginePage = new g1::EnginePage(_iniParser, _tabWidget);
	_visualizationPage = new g1::VisualizationPage(_iniParser, _tabWidget);
	_videoPage = new g1::VideoPage(_iniParser, _tabWidget);
	_soundPage = new g1::SoundPage(_iniParser, _tabWidget);
	_controlsPage = new g1::ControlsPage(_iniParser, _tabWidget);

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

		connect(backupButton, &QPushButton::released, this, &Gothic1IniWidget::backup);
		connect(restoreButton, &QPushButton::released, this, &Gothic1IniWidget::restore);
	}

	setLayout(l);
}

Gothic1IniWidget::~Gothic1IniWidget() {
	delete _iniParser;
}

void Gothic1IniWidget::accept() {
	_gamePage->accept();
	_performancePage->accept();
	_enginePage->accept();
	_visualizationPage->accept();
	_videoPage->accept();
	_soundPage->accept();
	_controlsPage->accept();
}

void Gothic1IniWidget::reject() {
	_gamePage->reject();
	_performancePage->reject();
	_enginePage->reject();
	_visualizationPage->reject();
	_videoPage->reject();
	_soundPage->reject();
	_controlsPage->reject();
}

void Gothic1IniWidget::backup() {
	const QString folder = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectBackupFolder"));
	if (!folder.isEmpty()) {
		QFile::copy(_directory + "/System/Gothic.ini", folder + "/Gothic.ini");
	}
}

void Gothic1IniWidget::restore() {
	const QString backupFile = QFileDialog::getOpenFileName(this, QApplication::tr("SelectBackupFile"), QString(), "Gothic.ini");
	
	if (backupFile.isEmpty()) return;
	
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
