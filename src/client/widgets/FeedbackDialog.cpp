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

#include "widgets/FeedbackDialog.h"

#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "common/MessageStructs.h"

#include "widgets/GeneralSettingsWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	FeedbackDialog::FeedbackDialog(GeneralSettingsWidget * generalSettingsWidget) : QDialog(), _textEdit(nullptr), _usernameEdit(nullptr), _username() {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QLabel * infoLabel = new QLabel(QApplication::tr("FeedbackText"), this);
		l->addWidget(infoLabel);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, infoLabel, "FeedbackText");

		_textEdit = new QTextEdit(this);
		l->addWidget(_textEdit);

		_usernameEdit = new QLineEdit(this);
		l->addWidget(_usernameEdit);
		_usernameEdit->setDisabled(true);

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
		b->setText(QApplication::tr("Submit"));
		UPDATELANGUAGESETTEXT(generalSettingsWidget, b, "Submit");

		connect(b, SIGNAL(clicked()), this, SLOT(accept()));

		b = dbb->button(QDialogButtonBox::StandardButton::Discard); 
		b->setText(QApplication::tr("Discard"));
		UPDATELANGUAGESETTEXT(generalSettingsWidget, b, "Discard");

		connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
		connect(b, SIGNAL(clicked()), this, SLOT(reject()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		setWindowTitle(QApplication::tr("Feedback"));
		UPDATELANGUAGESETWINDOWTITLE(generalSettingsWidget, this, "Feedback");
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	}

	FeedbackDialog::~FeedbackDialog() {
	}

	void FeedbackDialog::setUsername(QString username) {
		_username = username;
		_usernameEdit->setText(username);
		_usernameEdit->setEnabled(!_username.isEmpty());
	}

	void FeedbackDialog::accept() {
		common::FeedbackMessage fm;
		fm.text = _textEdit->toPlainText().trimmed().toStdString();
		fm.majorVersion = VERSION_MAJOR;
		fm.minorVersion = VERSION_MINOR;
		fm.patchVersion = VERSION_PATCH;
		fm.username = _usernameEdit->text().toStdString();
		if (fm.text.empty()) {
			return;
		}
		const std::string serialized = fm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == err) {
			if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
				QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("FeedbackSuccessful"), QApplication::tr("FeedbackSuccessfulText"), QMessageBox::StandardButton::Ok);
				resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				resultMsg.exec();
				_textEdit->clear();
				QDialog::accept();
				return;
			}
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

} /* namespace widgets */
} /* namespace spine */
