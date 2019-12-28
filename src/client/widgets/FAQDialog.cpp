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

#include "widgets/FAQDialog.h"

#include "widgets/FAQEntry.h"

#include <QApplication>
#include <QScrollArea>
#include <QSettings>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	FAQDialog::FAQDialog(QSettings * iniParser, QWidget * par) : QDialog(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QScrollArea * scrollArea = new QScrollArea(this);
		QWidget * mainWidget = new QWidget(this);
		QVBoxLayout * scrollLayout = new QVBoxLayout();
		scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		mainWidget->setLayout(scrollLayout);
		scrollArea->setWidget(mainWidget);
		scrollArea->setWidgetResizable(true);
		mainWidget->setProperty("FAQ", true);

		l->addWidget(scrollArea);

		setLayout(l);

		initEntries(mainWidget, scrollLayout);

		setWindowTitle(QApplication::tr("FAQ"));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		restoreSettings();
	}

	FAQDialog::~FAQDialog() {
		saveSettings();
	}

	void FAQDialog::initEntries(QWidget * par, QLayout * l) {
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion1"), QApplication::tr("FAQAnswer1"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion2"), QApplication::tr("FAQAnswer2"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion3"), QApplication::tr("FAQAnswer3"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion4"), QApplication::tr("FAQAnswer4"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion5"), QApplication::tr("FAQAnswer5"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion6"), QApplication::tr("FAQAnswer6"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion7"), QApplication::tr("FAQAnswer7"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion8"), QApplication::tr("FAQAnswer8"), par));
		l->addWidget(new FAQEntry(QApplication::tr("FAQQuestion9"), QApplication::tr("FAQAnswer9"), par));
	}

	void FAQDialog::restoreSettings() {
		const QByteArray arr = _iniParser->value("WINDOWGEOMETRY/FAQDialogGeometry", QByteArray()).toByteArray();
		if (!restoreGeometry(arr)) {
			_iniParser->remove("WINDOWGEOMETRY/FAQDialogGeometry");
		}
	}

	void FAQDialog::saveSettings() {
		_iniParser->setValue("WINDOWGEOMETRY/FAQDialogGeometry", saveGeometry());
	}

} /* namespace widgets */
} /* namespace spine */
