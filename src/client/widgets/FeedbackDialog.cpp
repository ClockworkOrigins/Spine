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

#include "widgets/FeedbackDialog.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"

#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

FeedbackDialog::FeedbackDialog(int32_t projectID, Type type, uint8_t majorVersion, uint8_t minorVersion, uint8_t patchVersion) : QDialog(), _textEdit(nullptr), _usernameEdit(nullptr), _projectID(projectID), _type(type), _majorVersion(majorVersion), _minorVersion(minorVersion), _patchVersion(patchVersion) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	const char * feedbackText = type == Type::Spine ? "FeedbackText" : "FeedbackTextProject";
	
	auto * infoLabel = new QLabel(QApplication::tr(feedbackText), this);
	l->addWidget(infoLabel);
	UPDATELANGUAGESETTEXT(infoLabel, feedbackText);

	_textEdit = new QTextEdit(this);
	l->addWidget(_textEdit);

	_usernameEdit = new QLineEdit(this);
	l->addWidget(_usernameEdit);
	_usernameEdit->setDisabled(true);

	auto * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
	b->setText(QApplication::tr("Submit"));
	UPDATELANGUAGESETTEXT(b, "Submit");

	connect(b, &QPushButton::released, this, &FeedbackDialog::accept);

	b = dbb->button(QDialogButtonBox::StandardButton::Discard); 
	b->setText(QApplication::tr("Discard"));
	UPDATELANGUAGESETTEXT(b, "Discard");

	connect(b, &QPushButton::released, this, &FeedbackDialog::rejected);
	connect(b, &QPushButton::released, this, &FeedbackDialog::reject);
	connect(b, &QPushButton::released, this, &FeedbackDialog::hide);

	setWindowTitle(QApplication::tr("Feedback"));
	UPDATELANGUAGESETWINDOWTITLE(this, "Feedback");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void FeedbackDialog::loginChanged() {
	_usernameEdit->setText(Config::Username);
	_usernameEdit->setEnabled(!Config::Username.isEmpty());
}

void FeedbackDialog::accept() {
	const auto text = _textEdit->toPlainText().trimmed().replace("'", "&apos;");
	
	if (text.isEmpty()) return;

	QJsonObject json;
	json["Username"] = Config::Username;
	json["ProjectID"] = _projectID;
	json["Text"] = text;
	json["MajorVersion"] = static_cast<int>(_majorVersion);
	json["MinorVersion"] = static_cast<int>(_minorVersion);
	json["PatchVersion"] = static_cast<int>(_patchVersion);

	bool success = false;

	Https::post(DATABASESERVER_PORT, "feedback", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, &success](const QJsonObject &, int statusCode) {
		success = statusCode == 200;
	});

	if (success) {
		QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("FeedbackSuccessful"), QApplication::tr("FeedbackSuccessfulText"), QMessageBox::StandardButton::Ok);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.exec();
		_textEdit->clear();
		QDialog::accept();
		return;
	}

	QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("FeedbackUnsuccessful"), QApplication::tr("FeedbackUnsuccessfulText"), QMessageBox::StandardButton::Ok);
	resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	resultMsg.exec();
	QDialog::accept();
}

void FeedbackDialog::reject() {
	QDialog::reject();
}

void FeedbackDialog::closeEvent(QCloseEvent * evt) {
	QDialog::closeEvent(evt);
	evt->accept();
	reject();
}
