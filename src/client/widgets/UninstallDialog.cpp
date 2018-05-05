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

#include "widgets/UninstallDialog.h"

#include "Config.h"
#include "Database.h"
#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	UninstallDialog::UninstallDialog(QString title, QString text, QString path) : QDialog(nullptr), _savegameCheckbox(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QLabel * infoLabel = new QLabel(text, this);
		infoLabel->setWordWrap(true);
		l->addWidget(infoLabel);

		QDirIterator it(path, QStringList() << "*ini", QDir::Files, QDirIterator::Subdirectories);
		_savegameCheckbox = new QCheckBox(QApplication::tr("LeaveSavegame"), this);
		l->addWidget(_savegameCheckbox);
		_savegameCheckbox->setChecked(true);
		_savegameCheckbox->setVisible(it.hasNext());

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Ok);
		b->setText(QApplication::tr("Ok"));

		connect(b, SIGNAL(clicked()), this, SIGNAL(accepted()));
		connect(b, SIGNAL(clicked()), this, SLOT(accept()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		b = dbb->button(QDialogButtonBox::StandardButton::Cancel);
		b->setText(QApplication::tr("Cancel"));

		connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
		connect(b, SIGNAL(clicked()), this, SLOT(reject()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(title);
	}

	UninstallDialog::~UninstallDialog() {
	}

	bool UninstallDialog::keepSavegame() const {
		return _savegameCheckbox->isChecked();
	}

} /* namespace widgets */
} /* namespace spine */
