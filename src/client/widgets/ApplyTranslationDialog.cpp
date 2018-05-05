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

#include "widgets/ApplyTranslationDialog.h"

#include "Conversion.h"

#include "translator/TranslationApplier.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	ApplyTranslationDialog::ApplyTranslationDialog(uint32_t requestID, QString title, QSettings * iniParser, QWidget * par) : QDialog(par), _iniParser(iniParser), _requestID(requestID) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();

			QLabel * pathLabel = new QLabel(QApplication::tr("ParsePath"), this);
			_pathEdit = new QLineEdit(this);
			_pathEdit->setReadOnly(true);
			_pathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
			QPushButton * pathPushButton = new QPushButton("...", this);
			hl->addWidget(pathLabel);
			hl->addWidget(_pathEdit);
			hl->addWidget(pathPushButton);
			connect(pathPushButton, &QPushButton::released, this, &ApplyTranslationDialog::openFileDialog);

			hl->setAlignment(Qt::AlignLeft);

			l->addLayout(hl);
		}

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
		b->setText(QApplication::tr("Apply"));
		_applyButton = b;
		b->setDisabled(true);

		connect(b, &QPushButton::released, this, &ApplyTranslationDialog::applyTranslation);

		b = dbb->button(QDialogButtonBox::StandardButton::Cancel);
		b->setText(QApplication::tr("Cancel"));

		connect(b, &QPushButton::released, this, &ApplyTranslationDialog::reject);

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(title);

		restoreSettings();
	}

	ApplyTranslationDialog::~ApplyTranslationDialog() {
		saveSettings();
	}

	void ApplyTranslationDialog::openFileDialog() {
		QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothicDir"), _pathEdit->text());
		if (!path.isEmpty()) {
			if (_pathEdit->text() != path) {
				_pathEdit->setText(path);
			}
		}
		_applyButton->setEnabled(!path.isEmpty());
	}

	void ApplyTranslationDialog::applyTranslation() {
		QProgressDialog progressDialog(QApplication::tr("ApplyingTranslation"), "", 0, 100, this);
		progressDialog.setCancelButton(nullptr);
		progressDialog.setWindowFlags(progressDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		translation::TranslationApplier ta(_requestID, this);
		connect(&ta, &translation::TranslationApplier::updatedCurrentProgress, &progressDialog, &QProgressDialog::setValue);
		connect(&ta, &translation::TranslationApplier::updateMaxProgress, &progressDialog, &QProgressDialog::setMaximum);
		ta.parseTexts(_pathEdit->text());
		accept();
	}

	void ApplyTranslationDialog::restoreSettings() {
		const QByteArray arr = _iniParser->value("WINDOWGEOMETRY/ApplyTranslationDialogGeometry", QByteArray()).toByteArray();
		if (!restoreGeometry(arr)) {
			_iniParser->remove("WINDOWGEOMETRY/ApplyTranslationDialogGeometry");
		}
	}

	void ApplyTranslationDialog::saveSettings() {
		_iniParser->setValue("WINDOWGEOMETRY/ApplyTranslationDialogGeometry", saveGeometry());
	}

} /* namespace widgets */
} /* namespace spine */
