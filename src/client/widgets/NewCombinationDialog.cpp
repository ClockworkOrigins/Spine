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

#include "widgets/NewCombinationDialog.h"

#include "Config.h"
#include "Database.h"
#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	NewCombinationDialog::NewCombinationDialog(QString title, QString text, QSettings * iniParser, QWidget * par) : QDialog(par), _dontShowAgainBox(nullptr), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QLabel * infoLabel = new QLabel(text, this);
		infoLabel->setWordWrap(true);
		l->addWidget(infoLabel);

		_dontShowAgainBox = new QCheckBox(QApplication::tr("DontShowAgain"), this);
		l->addWidget(_dontShowAgainBox);

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * btn = dbb->button(QDialogButtonBox::StandardButton::Ok);
		btn->setText(QApplication::tr("Ok"));

		connect(btn, SIGNAL(clicked()), this, SIGNAL(accepted()));
		connect(btn, SIGNAL(clicked()), this, SLOT(accept()));
		connect(btn, SIGNAL(clicked()), this, SLOT(hide()));

		btn = dbb->button(QDialogButtonBox::StandardButton::Cancel);
		btn->setText(QApplication::tr("Cancel"));

		connect(btn, SIGNAL(clicked()), this, SIGNAL(rejected()));
		connect(btn, SIGNAL(clicked()), this, SLOT(reject()));
		connect(btn, SIGNAL(clicked()), this, SLOT(hide()));

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(title);

		const bool b = _iniParser->value("NEWCOMBINATIONDIALOG/DontShowAgain", false).toBool();
		_dontShowAgainBox->setChecked(b);
	}

	NewCombinationDialog::~NewCombinationDialog() {
		_iniParser->setValue("NEWCOMBINATIONDIALOG/DontShowAgain", _dontShowAgainBox->isChecked());
	}

	bool NewCombinationDialog::canShow() const {
		return !_dontShowAgainBox->isChecked();
	}

} /* namespace widgets */
} /* namespace spine */
