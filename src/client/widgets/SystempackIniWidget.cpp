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

#include "widgets/SystempackIniWidget.h"

#include "widgets/systempackIniPages/GamePage.h"
#include "widgets/systempackIniPages/SystemPage.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	SystempackIniWidget::SystempackIniWidget(QString directory, QWidget * par) : QWidget(par), _iniParser(new QSettings(directory + "/System/Systempack.ini", QSettings::IniFormat)), _directory(directory), _tabWidget(nullptr), _gamePage(nullptr), _systemPage(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();

		_tabWidget = new QTabWidget(this);

		_gamePage = new sp::GamePage(_iniParser, _tabWidget);
		_systemPage = new sp::SystemPage(_iniParser, _tabWidget);

		_tabWidget->addTab(_gamePage, QApplication::tr("Game"));
		_tabWidget->addTab(_systemPage, QApplication::tr("System"));

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

	SystempackIniWidget::~SystempackIniWidget() {
		delete _iniParser;
	}

	void SystempackIniWidget::accept() {
		_gamePage->accept();
		_systemPage->accept();
	}

	void SystempackIniWidget::reject() {
		_gamePage->reject();
		_systemPage->reject();
	}

	void SystempackIniWidget::backup() {
		const QString folder = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectBackupFolder"));
		if (!folder.isEmpty()) {
			QFile::copy(_directory + "/System/Systempack.ini", folder + "/Systempack.ini");
		}
	}

	void SystempackIniWidget::restore() {
		const QString backupFile = QFileDialog::getOpenFileName(this, QApplication::tr("SelectBackupFile"), QString(), "Systempack.ini");
		if (!backupFile.isEmpty()) {
			QFile::copy(backupFile, _directory + "/System/Systempack.ini");
		}
	}

} /* namespace widgets */
} /* namespace spine */
