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
// Copyright 2022 Clockwork Origins

#include "DontShowAgainDialog.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

using namespace spine::gui;
using namespace spine::utils;

DontShowAgainDialog::DontShowAgainDialog(QString title, QString text, const QString & settingsKey, QWidget * par) : QDialog(par), _settingsKey(settingsKey), _dontShowAgainBox(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	auto * infoLabel = new QLabel(text, this);
	infoLabel->setWordWrap(true);
	l->addWidget(infoLabel);

	_dontShowAgainBox = new QCheckBox(QApplication::tr("DontShowAgain"), this);
	l->addWidget(_dontShowAgainBox);

	auto * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * btn = dbb->button(QDialogButtonBox::StandardButton::Ok);
	btn->setText(QApplication::tr("Ok"));

	connect(btn, &QPushButton::released, this, &DontShowAgainDialog::accepted);
	connect(btn, &QPushButton::released, this, &DontShowAgainDialog::accept);
	connect(btn, &QPushButton::released, this, &DontShowAgainDialog::hide);

	btn = dbb->button(QDialogButtonBox::StandardButton::Cancel);
	btn->setText(QApplication::tr("Cancel"));

	connect(btn, &QPushButton::released, this, &DontShowAgainDialog::rejected);
	connect(btn, &QPushButton::released, this, &DontShowAgainDialog::reject);
	connect(btn, &QPushButton::released, this, &DontShowAgainDialog::hide);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(title);

	const bool b = Config::IniParser->value(_settingsKey, false).toBool();
	_dontShowAgainBox->setChecked(b);
}

DontShowAgainDialog::~DontShowAgainDialog() {
	Config::IniParser->setValue(_settingsKey, _dontShowAgainBox->isChecked());
}

bool DontShowAgainDialog::canShow() const {
	return !_dontShowAgainBox->isChecked();
}
