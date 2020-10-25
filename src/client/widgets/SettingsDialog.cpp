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

#include "widgets/SettingsDialog.h"

#include "utils/Config.h"

#include "widgets/DeveloperSettingsWidget.h"

#include "widgets/GameSettingsWidget.h"
#include "widgets/LocationSettingsWidget.h"
#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

SettingsDialog::SettingsDialog(QWidget * par) : QDialog(par), _developerSettingsWidget(nullptr), _gameSettingsWidget(nullptr), _generalSettingsWidget(nullptr), _locationSettingsWidget(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	QTabWidget * tabWidget = new QTabWidget(this);

	int tabCounter = 0;
	_generalSettingsWidget = new GeneralSettingsWidget(tabWidget);
	tabWidget->addTab(_generalSettingsWidget, QApplication::tr("General"));
	UPDATELANGUAGESETTABTEXT(tabWidget, tabCounter, "General");
	++tabCounter;

	_gameSettingsWidget = new GameSettingsWidget(tabWidget);
	tabWidget->addTab(_gameSettingsWidget, QApplication::tr("Game"));
	UPDATELANGUAGESETTABTEXT(tabWidget, tabCounter, "Game");
	++tabCounter;

	_locationSettingsWidget = new LocationSettingsWidget(false, tabWidget);
	tabWidget->addTab(_locationSettingsWidget, QApplication::tr("Locations"));
	UPDATELANGUAGESETTABTEXT(tabWidget, tabCounter, "Locations");
	++tabCounter;
	
	const auto devEnabled = Config::IniParser->value("DEVELOPER/Enabled", false).toBool();

	if (devEnabled) {
		_developerSettingsWidget = new DeveloperSettingsWidget(tabWidget);
		tabWidget->addTab(_developerSettingsWidget, QApplication::tr("Developer"));
		UPDATELANGUAGESETTABTEXT(tabWidget, tabCounter, "Developer");
		++tabCounter;
	}
	
	l->addWidget(tabWidget);

	QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
	b->setText(QApplication::tr("Apply"));
	UPDATELANGUAGESETTEXT(b, "Apply");

	connect(b, &QPushButton::released, this, &SettingsDialog::accepted);
	connect(b, &QPushButton::released, this, &SettingsDialog::accept);
	connect(b, &QPushButton::released, this, &SettingsDialog::hide);

	b = dbb->button(QDialogButtonBox::StandardButton::Discard);
	b->setText(QApplication::tr("Discard"));
	UPDATELANGUAGESETTEXT(b, "Discard");

	connect(b, &QPushButton::released, this, &SettingsDialog::rejected);
	connect(b, &QPushButton::released, this, &SettingsDialog::reject);
	connect(b, &QPushButton::released, this, &SettingsDialog::hide);

	setWindowTitle(QApplication::tr("Settings"));
	UPDATELANGUAGESETWINDOWTITLE(this, "Settings");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	restoreSettings();
}

SettingsDialog::~SettingsDialog() {
	saveSettings();
}

void SettingsDialog::accept() {
	if (_developerSettingsWidget) {
		_developerSettingsWidget->saveSettings();
	}
	
	_gameSettingsWidget->saveSettings();
	_generalSettingsWidget->saveSettings();
	_locationSettingsWidget->saveSettings();
}

void SettingsDialog::reject() {
	if (_developerSettingsWidget) {
		_developerSettingsWidget->rejectSettings();
	}
	
	_gameSettingsWidget->rejectSettings();
	_generalSettingsWidget->rejectSettings();
	_locationSettingsWidget->rejectSettings();
}

void SettingsDialog::closeEvent(QCloseEvent * evt) {
	QDialog::closeEvent(evt);
	evt->accept();
	reject();
}

void SettingsDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/SettingsDialogGeometry", QByteArray()).toByteArray();
	
	if (restoreGeometry(arr)) return;
	
	Config::IniParser->remove("WINDOWGEOMETRY/SettingsDialogGeometry");
}

void SettingsDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/SettingsDialogGeometry", saveGeometry());
}
