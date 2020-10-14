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

#include "gui/ReportContentDialog.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

using namespace spine::gui;
using namespace spine::utils;

ReportContentDialog::ReportContentDialog(QString context, QWidget * par) : QDialog(par), _context(context) {
	QVBoxLayout* vl = new QVBoxLayout();

	const QString description = QApplication::tr("ReportContentDescription");
	
	QLabel * descriptionLabel = new QLabel(description, this);
	descriptionLabel->setWordWrap(true);
	vl->addWidget(descriptionLabel);

	_textEdit = new QTextEdit(this);
	vl->addWidget(_textEdit);

	vl->addStretch(1);

	QDialogButtonBox* dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Orientation::Horizontal, this);
	vl->addWidget(dbb);

	connect(dbb, &QDialogButtonBox::accepted, this, &ReportContentDialog::submit);
	connect(dbb, &QDialogButtonBox::rejected, this, &ReportContentDialog::reject);

	dbb->button(QDialogButtonBox::Ok)->setText(QApplication::tr("Submit"));
	dbb->button(QDialogButtonBox::Cancel)->setText(QApplication::tr("Cancel"));

	connect(_textEdit, &QTextEdit::textChanged, [this, dbb]() {
		auto* btn = dbb->button(QDialogButtonBox::Ok);
		btn->setEnabled(!_textEdit->toPlainText().isEmpty());
	});

	connect(this, &ReportContentDialog::error, this, &ReportContentDialog::showError);
	connect(this, &ReportContentDialog::success, this, &ReportContentDialog::showSuccess);

	setLayout(vl);
	
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ReportContentDialog::submit() {
	QString errorMessage = _textEdit->toPlainText();
    errorMessage = errorMessage.replace("\r\n", "<br>");
    errorMessage = errorMessage.replace("\n", "<br>");
    errorMessage = errorMessage.replace("\"", "&quot;");
    errorMessage = errorMessage.replace("\'", "&apos;");
	
	QJsonObject json;
	json["project"] = Config::ProjectID;
	json["username"] = Config::Username;
	json["password"] = Config::Password;
	json["context"] = _context;
	json["message"] = errorMessage;

	https::Https::postAsync(19101, "reportContent", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int response) {
		if (response == 200) {
			emit success();
		} else {
			emit error();
		}
	});
}

void ReportContentDialog::showError() {
	QMessageBox msg(QMessageBox::Critical, QApplication::tr("ReportContent"), QApplication::tr("ConnectionFailed"), QMessageBox::Ok, this);
	msg.show();
}

void ReportContentDialog::showSuccess() {
	QString message = QApplication::tr("ReportedContent");

	if (!Config::Username.isEmpty() && !Config::Password.isEmpty()) {
		message += "\n" + QApplication::tr("ReportedContentLoggedIn");
	}
	
	QMessageBox msg(QMessageBox::NoIcon, QApplication::tr("ReportContent"), message, QMessageBox::Ok, this);
	msg.exec();

	accept();
}
