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

#include "widgets/IniConfigurator.h"

#include "utils/Conversion.h"

#include "widgets/Gothic1IniWidget.h"
#include "widgets/Gothic2IniWidget.h"
#include "widgets/SystempackIniWidget.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	IniConfigurator::IniConfigurator(QString gothicDirectory, QString gothic2Directory, QSettings * iniParser, QWidget * par) : QDialog(par), _iniParser(iniParser), _tabWidget(nullptr), _gothic1IniWidget(nullptr), _gothic2IniWidget(nullptr), _systempackGothic1IniWidget(nullptr), _systempackGothic2IniWidget(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		_tabWidget = new QTabWidget(this);

		_gothic1IniWidget = new Gothic1IniWidget(gothicDirectory, _tabWidget);
		_gothic2IniWidget = new Gothic2IniWidget(gothic2Directory, _tabWidget);
		_systempackGothic1IniWidget = new SystempackIniWidget(gothicDirectory, _tabWidget);
		_systempackGothic2IniWidget = new SystempackIniWidget(gothic2Directory, _tabWidget);

		_tabWidget->addTab(_gothic1IniWidget, QApplication::tr("Gothic1Settings"));
		_tabWidget->addTab(_gothic2IniWidget, QApplication::tr("Gothic2Settings"));
		_tabWidget->addTab(_systempackGothic1IniWidget, QApplication::tr("SystempackGothic1Settings"));
		_tabWidget->addTab(_systempackGothic2IniWidget, QApplication::tr("SystempackGothic2Settings"));

		_tabWidget->setTabEnabled(0, !gothicDirectory.isEmpty());
		_tabWidget->setTabEnabled(1, !gothic2Directory.isEmpty());
		_tabWidget->setTabEnabled(2, !gothicDirectory.isEmpty());
		_tabWidget->setTabEnabled(3, !gothic2Directory.isEmpty());

		l->addWidget(_tabWidget);

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
		b->setText(QApplication::tr("Apply"));

		connect(b, SIGNAL(clicked()), this, SIGNAL(accepted()));
		connect(b, SIGNAL(clicked()), this, SLOT(accept()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		b = dbb->button(QDialogButtonBox::StandardButton::Discard);
		b->setText(QApplication::tr("Discard"));

		connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
		connect(b, SIGNAL(clicked()), this, SLOT(reject()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(QApplication::tr("IniConfigurator"));

		setLayout(l);

		restoreSettings();
	}

	IniConfigurator::~IniConfigurator() {
		saveSettings();
	}

	void IniConfigurator::accept() {
		_gothic1IniWidget->accept();
		_gothic2IniWidget->accept();
		_systempackGothic1IniWidget->accept();
		_systempackGothic2IniWidget->accept();
		QDialog::accept();
	}

	void IniConfigurator::reject() {
		_gothic1IniWidget->reject();
		_gothic2IniWidget->reject();
		_systempackGothic1IniWidget->reject();
		_systempackGothic2IniWidget->reject();
		QDialog::reject();
	}

	void IniConfigurator::restoreSettings() {
		const QByteArray arr = _iniParser->value("WINDOWGEOMETRY/IniConfiguratorGeometry", QByteArray()).toByteArray();
		if (!restoreGeometry(arr)) {
			_iniParser->remove("WINDOWGEOMETRY/IniConfiguratorGeometry");
		}
	}

	void IniConfigurator::saveSettings() {
		_iniParser->setValue("WINDOWGEOMETRY/IniConfiguratorGeometry", saveGeometry());
	}

} /* namespace widgets */
} /* namespace spine */
