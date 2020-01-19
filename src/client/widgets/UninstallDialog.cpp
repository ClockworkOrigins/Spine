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

#include "widgets/UninstallDialog.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDirIterator>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

UninstallDialog::UninstallDialog(QString title, QString text, QString path) : QDialog(nullptr), _savegameCheckbox(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	QLabel * infoLabel = new QLabel(text, this);
	infoLabel->setWordWrap(true);
	l->addWidget(infoLabel);

	const QDirIterator it(path, QStringList() << "*ini", QDir::Files, QDirIterator::Subdirectories);
	_savegameCheckbox = new QCheckBox(QApplication::tr("LeaveSavegame"), this);
	l->addWidget(_savegameCheckbox);
	_savegameCheckbox->setChecked(true);
	_savegameCheckbox->setVisible(it.hasNext());

	QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Ok);
	b->setText(QApplication::tr("Ok"));

	connect(b, &QPushButton::clicked, this, &UninstallDialog::accepted);
	connect(b, &QPushButton::clicked, this, &UninstallDialog::accept);
	connect(b, &QPushButton::clicked, this, &UninstallDialog::hide);

	b = dbb->button(QDialogButtonBox::StandardButton::Cancel);
	b->setText(QApplication::tr("Cancel"));

	connect(b, &QPushButton::clicked, this, &UninstallDialog::rejected);
	connect(b, &QPushButton::clicked, this, &UninstallDialog::reject);
	connect(b, &QPushButton::clicked, this, &UninstallDialog::hide);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(title);
}

UninstallDialog::~UninstallDialog() {
}

bool UninstallDialog::keepSavegame() const {
	return _savegameCheckbox->isChecked();
}
