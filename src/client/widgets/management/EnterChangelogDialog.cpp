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
// Copyright 2020 Clockwork Origins

#include "widgets/management/EnterChangelogDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::client::widgets;

EnterChangelogDialog::EnterChangelogDialog(QWidget * par) : QDialog(par) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QHBoxLayout * hl = new QHBoxLayout();

		QLabel * lbl = new QLabel(QApplication::tr("Language"), this);

		_languageBox = new QComboBox(this);
		_languageBox->addItems({ "Deutsch", "English", "Polish", "Russian" });
		_languageBox->setCurrentIndex(1); // always default to English
		_languageBox->setEditable(false);

		_currentLanguage = _languageBox->currentText();

		connect(_languageBox, &QComboBox::currentTextChanged, [this]() {
			_changelogs[_currentLanguage] = _textEdit->toHtml();

			_currentLanguage = _languageBox->currentText();

			_textEdit->setText(_changelogs[_currentLanguage]);
		});
		
		_savegameCompatibleBox = new QCheckBox(QString("%1?").arg(QApplication::tr("SavegameCompatible")), this);

		hl->addWidget(lbl);
		hl->addWidget(_languageBox);
		hl->addStretch(1);
		hl->addWidget(_savegameCompatibleBox);
		
		l->addLayout(hl);
	}

	_textEdit = new QTextEdit(this);
	_textEdit->setAutoFormatting(QTextEdit::AutoAll);
	connect(_textEdit, &QTextEdit::textChanged, [this]() {
		_changelogs[_languageBox->currentText()] = _textEdit->toHtml();
	});

	l->addWidget(_textEdit, 1);

	QDialogButtonBox * dlgButtons = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);

	auto * okButton = dlgButtons->button(QDialogButtonBox::Ok);
	auto * cancelButton = dlgButtons->button(QDialogButtonBox::Cancel);

	okButton->setText(QApplication::tr("Submit"));
	cancelButton->setText(QApplication::tr("Cancel"));

	connect(okButton, &QPushButton::released, this, &QDialog::accept);
	connect(cancelButton, &QPushButton::released, this, &QDialog::reject);

	l->addWidget(dlgButtons);

	setLayout(l);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

bool EnterChangelogDialog::isSavegameCompatible() const {
	return _savegameCompatibleBox->isChecked();
}

QMap<QString, QString> EnterChangelogDialog::getChangelogs() const {
	QMap<QString, QString> changelogs;

	for (auto it = _changelogs.begin(); it != _changelogs.end(); ++it) {
		const QString & language = it.key();
		QString changelog = it.value();
		changelog.replace("'", "&apos;");
		changelog.replace("\n", "");
		changelog.replace("\"", "&quot;");
		changelog = changelog.trimmed();

		if (changelog.isEmpty()) continue;

		changelogs.insert(language, changelog);
	}

	return changelogs;
}
