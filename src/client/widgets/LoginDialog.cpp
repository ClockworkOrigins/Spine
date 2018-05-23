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

#include "widgets/LoginDialog.h"

#include <sstream>
#include <thread>

#include "Config.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "common/MessageStructs.h"

#include "security/Hash.h"

#include "widgets/GeneralSettingsWidget.h"

#include "clockUtils/errors.h"
#include "clockUtils/compression/Compression.h"
#include "clockUtils/compression/algorithm/HuffmanGeneric.h"
#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QNetworkReply>

#include "simple-web-server/client_https.hpp"

using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

namespace clockwork {
namespace login {

	enum class Response {
		// Both
		SUCCESS,							//!< registration/login worked
		CONNECTION_FAILURE,					//!< connection to login server failed or some other error occured
		// Registration
		USER_EXISTS,						//!< user with given name already exists
		MAIL_EXISTS,						//!< another username uses given mail already
		// Login
		INVALID_MAIL,						//!< invalid mail entered
		KEY_NOTFOUND,						//!< key for encryption was not found
		WRONG_PASSWORD_OR_USER_NOTFOUND		//!< those two are merged now for security reasons
	};
}
}

namespace spine {
namespace widgets {
namespace {
		
	 clockwork::login::Response statusCodeToResponse(SimpleWeb::StatusCode code) {
		switch (code) {
		case SimpleWeb::StatusCode::success_ok: {
			return clockwork::login::Response::SUCCESS;
		}
		case SimpleWeb::StatusCode::client_error_not_found: {
			return clockwork::login::Response::CONNECTION_FAILURE;
		}
		case SimpleWeb::StatusCode::server_error_variant_also_negotiates: {
			return clockwork::login::Response::USER_EXISTS;
		}
		case SimpleWeb::StatusCode::server_error_loop_detected: {
			return clockwork::login::Response::MAIL_EXISTS;
		}
		case SimpleWeb::StatusCode::server_error_not_implemented: {
			return clockwork::login::Response::INVALID_MAIL;
		}
		case SimpleWeb::StatusCode::server_error_insufficient_storage: {
			return clockwork::login::Response::KEY_NOTFOUND;
		}
		case SimpleWeb::StatusCode::server_error_network_authentication_required: {
			return clockwork::login::Response::WRONG_PASSWORD_OR_USER_NOTFOUND;
		}
		default: {
			return clockwork::login::Response::CONNECTION_FAILURE;
		}
		}
	}

}

	LoginDialog::LoginDialog(bool onlineMode, QSettings * iniParser, GeneralSettingsWidget * generalSettingsWidget, QWidget *) : QDialog(nullptr), _iniParser(iniParser), _connected(false), _dontShow(false), _language(), _onlineMode(onlineMode) {
		QDir dir(Config::BASEDIR + "/databases");
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b);
		}
		std::string username;
		std::string password;
		QFile f(Config::BASEDIR + "/databases/Login.bin");
		if (f.open(QIODevice::ReadOnly)) {
			char buf[4098];
			QByteArray arr = f.readAll();
			const quint64 bytesRead = arr.length();
			memcpy(buf, arr.data(), std::min(bytesRead, quint64(4098)));

			if (bytesRead) {
				int pathLength;
				int pathLengthCompressed;
				memcpy(&pathLength, buf, sizeof(int));
				memcpy(&pathLengthCompressed, buf + sizeof(int), sizeof(int));
				const std::string compressedPath = std::string(buf + 2 * sizeof(int), pathLengthCompressed);
				std::string d = std::string(buf + 2 * sizeof(int) + pathLengthCompressed, bytesRead - 2 * sizeof(int) - pathLengthCompressed);

				if (!d.empty()) {
					QDir homeDir = QDir::home();
					if (homeDir.absolutePath().length() == pathLength) {
						clockUtils::compression::Compression<clockUtils::compression::algorithm::HuffmanGeneric> dec;
						std::string path;
						if (clockUtils::ClockError::SUCCESS == dec.decompress(compressedPath, path)) {
							if (homeDir.absolutePath() == QString::fromStdString(path)) {
								std::string decompressed;
								if (clockUtils::ClockError::SUCCESS == dec.decompress(d, decompressed)) {
									size_t usernameLength;
									memcpy(&usernameLength, decompressed.c_str(), sizeof(size_t));
									username.append(decompressed, sizeof(size_t), usernameLength);
									size_t passwordLength;
									memcpy(&passwordLength, decompressed.c_str() + sizeof(size_t) + usernameLength, sizeof(size_t));
									password.append(decompressed, sizeof(size_t) * 2 + usernameLength, passwordLength);
								}
							}
						}
					}
				}
			}
		}

		QVBoxLayout * l = new QVBoxLayout();

		QTabWidget * tw = new QTabWidget(this);

		{
			// register tab
			QWidget * w = new QWidget(tw);
			QGridLayout * gl = new QGridLayout();

			QLabel * l1 = new QLabel(QApplication::tr("Username") + " *", w);
			QLabel * l2 = new QLabel(QApplication::tr("Mail") + " *", w);
			QLabel * l3 = new QLabel(QApplication::tr("Password") + " *", w);
			QLabel * l4 = new QLabel(QApplication::tr("PasswordRepeat") + " *", w);

			_registerUsernameEdit = new QLineEdit(w);
			_registerMailEdit = new QLineEdit(w);
			_registerPasswordEdit = new QLineEdit(w);
			_registerPasswordRepeatEdit = new QLineEdit(w);
			
			_registerAcceptPrivacyPolicy = new QCheckBox(QApplication::tr("AcceptPrivacy"), w);
			QLabel * privacyLink = new QLabel("<a href=\"https://clockwork-origins.com/privacy\">" + QApplication::tr("Privacy") + "</a>", this);
			privacyLink->setOpenExternalLinks(true);
			_registerStayBox = new QCheckBox(QApplication::tr("StayLoggedIn"), w);
			_registerButton = new QPushButton(QApplication::tr("Register"), w);
			_registerButton->setEnabled(false);

			_registerPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);
			_registerPasswordRepeatEdit->setEchoMode(QLineEdit::EchoMode::Password);

			QPushButton * resetPasswordButton = new QPushButton(QApplication::tr("ForgotPassword"), this);

			gl->addWidget(l1, 0, 0);
			gl->addWidget(_registerUsernameEdit, 0, 1);
			gl->addWidget(l2, 1, 0);
			gl->addWidget(_registerMailEdit, 1, 1);
			gl->addWidget(l3, 2, 0);
			gl->addWidget(_registerPasswordEdit, 2, 1);
			gl->addWidget(l4, 3, 0);
			gl->addWidget(_registerPasswordRepeatEdit, 3, 1);
			gl->addWidget(_registerAcceptPrivacyPolicy, 4, 1);
			gl->addWidget(privacyLink, 5, 1);
			gl->addWidget(_registerStayBox, 6, 1);
			gl->addWidget(resetPasswordButton, 7, 0);
			gl->addWidget(_registerButton, 7, 1);

			w->setLayout(gl);

			tw->addTab(w, QApplication::tr("Register"));
			UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, l1, "Username", " *");
			UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, l2, "Mail", " *");
			UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, l3, "Password", " *");
			UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, l4, "PasswordRepeat", " *");
			UPDATELANGUAGESETTABTEXT(generalSettingsWidget, tw, 0, "Register");
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _registerAcceptPrivacyPolicy, "AcceptPrivacy");
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _registerStayBox, "StayLoggedIn");
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _registerButton, "Register");

			connect(_registerButton, SIGNAL(released()), this, SLOT(registerUser()));
			connect(_registerUsernameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changedRegisterInput()));
			connect(_registerMailEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changedRegisterInput()));
			connect(_registerPasswordEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changedRegisterInput()));
			connect(_registerPasswordRepeatEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changedRegisterInput()));
			connect(resetPasswordButton, &QPushButton::released, this, &LoginDialog::resetPassword);
			connect(_registerAcceptPrivacyPolicy, &QCheckBox::toggled, this, &LoginDialog::onPrivacyAcceptChanged);
		}
		{
			// login tab
			QWidget * w = new QWidget(tw);
			QGridLayout * gl = new QGridLayout();

			QLabel * l1 = new QLabel(QApplication::tr("Username") + " *", w);
			QLabel * l2 = new QLabel(QApplication::tr("Password") + " *", w);

			_loginUsernameEdit = new QLineEdit(w);
			_loginPasswordEdit = new QLineEdit(w);

			QPushButton * resetPasswordButton = new QPushButton(QApplication::tr("ForgotPassword"), this);

			_loginStayBox = new QCheckBox(QApplication::tr("StayLoggedIn"), w);
			_loginButton = new QPushButton(QApplication::tr("Login"), w);

			_loginPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);

			gl->addWidget(l1, 0, 0);
			gl->addWidget(_loginUsernameEdit, 0, 1);
			gl->addWidget(l2, 1, 0);
			gl->addWidget(_loginPasswordEdit, 1, 1);
			gl->addWidget(_loginStayBox, 2, 1);
			gl->addWidget(_loginButton, 3, 1);;
			gl->addWidget(resetPasswordButton, 3, 0);

			w->setLayout(gl);

			tw->addTab(w, QApplication::tr("Login"));
			UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, l1, "Username", " *");
			UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, l2, "Password", " *");
			UPDATELANGUAGESETTABTEXT(generalSettingsWidget, tw, 1, "Login");
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _loginStayBox, "StayLoggedIn");
			UPDATELANGUAGESETTEXT(generalSettingsWidget, _loginButton, "Login");

			connect(_loginButton, &QPushButton::released, this, &LoginDialog::loginUser);
			connect(_loginUsernameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changedLoginInput()));
			connect(_loginPasswordEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changedLoginInput()));
			connect(resetPasswordButton, &QPushButton::released, this, &LoginDialog::resetPassword);
		}

		l->addWidget(tw);

		QLabel * infoLabel = new QLabel(QApplication::tr("RequiredField"), this);
		QFont fnt = infoLabel->font();
		fnt.setPointSize(7);
		infoLabel->setFont(fnt);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, infoLabel, "RequiredField");

		l->addWidget(infoLabel);

		QLabel * accountInfoLabel = new QLabel(QApplication::tr("AccountInformation"), this);
		accountInfoLabel->setWordWrap(true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, accountInfoLabel, "AccountInformation");

		l->addWidget(accountInfoLabel);

		_dontShow = _iniParser->value("LOGIN/DontShowAgain", false).toBool();

		{
			QHBoxLayout * hbl = new QHBoxLayout();

			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("DontShowAgain"), this);
				cb->setProperty("noLogin", true);
				cb->setChecked(_dontShow);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, cb, "DontShowAgain");

				hbl->addWidget(cb);

				connect(cb, SIGNAL(stateChanged(int)), this, SLOT(dontShowAgainChanged(int)));
			}
			{
				QPushButton * pb = new QPushButton(QApplication::tr("ContinueWithoutLogin"), this);
				pb->setProperty("noLogin", true);
				UPDATELANGUAGESETTEXT(generalSettingsWidget, pb, "ContinueWithoutLogin");

				hbl->addWidget(pb);

				connect(pb, SIGNAL(clicked()), this, SLOT(accept()));
				connect(pb, SIGNAL(clicked()), this, SLOT(hide()));
			}

			l->addLayout(hbl);
		}

		connect(this, SIGNAL(loggedIn(QString, QString)), this, SLOT(loginSuccess()));
		connect(this, SIGNAL(showErrorMessage(QString)), this, SLOT(showErrorMessageBox(QString)));
		connect(this, SIGNAL(showInfoMessage(QString)), this, SLOT(showInfoMessageBox(QString)));

		if (!username.empty() && !password.empty()) {
			_loginUsernameEdit->setText(QString::fromStdString(username));
			_loginPasswordEdit->setText(QString::fromStdString(password));
		}

		_language = generalSettingsWidget->getLanguage();
		connect(generalSettingsWidget, SIGNAL(languageChanged(QString)), this, SLOT(setLanguage(QString)));

		changedLoginInput();
		changedRegisterInput();

		setLayout(l);

		setWindowTitle(QApplication::tr("Login"));
		UPDATELANGUAGESETWINDOWTITLE(generalSettingsWidget, this, "Login");
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	}

	QString LoginDialog::getUsername() const {
		return _loginUsernameEdit->text();
	}

	QString LoginDialog::getPassword() const {
		return _loginPasswordEdit->text();
	}

	int LoginDialog::exec() {
		if (!_loginUsernameEdit->text().isEmpty() && !_loginPasswordEdit->text().isEmpty()) {
			loginUser();
			return QDialog::Accepted;
		}
		if (!_dontShow) {
			return QDialog::exec();
		} else {
			return QDialog::Accepted;
		}
	}

	int LoginDialog::execute() {
		return QDialog::exec();
	}

	void LoginDialog::logout() {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("LogoutTitle"), QApplication::tr("LogoutText"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == msg.exec()) {
			QFile(Config::BASEDIR + "/databases/Login.bin").remove();
			emit loggedIn(QString(), QString());
		}
	}

	void LoginDialog::loginUser() {
		const QString passwd = _loginPasswordEdit->text();
		_loginPasswordEdit->setText("");
		const QString username = _loginUsernameEdit->text();
		bool stayLoggedIn = _loginStayBox->isChecked();

		if (_onlineMode) {
			HttpsClient client("clockwork-origins.com:19101", false);

			QString content = R"({"type": 1,"username": "%1","password": "%2"})";
			content = content.arg(username, passwd);

			// Synchronous request examples
			client.request("POST", "/json", content.toStdString(), [this, username, passwd, stayLoggedIn](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code & ec) {
				Q_ASSERT(response);
				if (!response) {
					emit showErrorMessage(QApplication::tr("ConnectionFailed"));
					return;
				}
				QString code = QString::fromStdString(response->status_code).split(" ")[0];
				clockwork::login::Response rp = statusCodeToResponse(SimpleWeb::StatusCode(code.toInt()));

				switch (rp) {
				case clockwork::login::Response::SUCCESS: {
					break;
				}
				case clockwork::login::Response::CONNECTION_FAILURE: {
					emit showErrorMessage(QApplication::tr("ConnectionFailed"));
					return;
				}
				case clockwork::login::Response::WRONG_PASSWORD_OR_USER_NOTFOUND: {
					emit showErrorMessage(QApplication::tr("WrongLoginData"));
					return;
				}
				default: {
					emit showErrorMessage(QApplication::tr("UnknownError"));
					return;
				}
				}

				handleLogin(username, passwd);

				if (stayLoggedIn) {
					QFile f(Config::BASEDIR + "/databases/Login.bin");
					if (f.open(QIODevice::WriteOnly)) {
						QDir homeDir = QDir::home();
						int pathLength = homeDir.absolutePath().length();
						char pl[sizeof(int)];
						memcpy(pl, &pathLength, sizeof(int));
						f.write(pl, sizeof(int));
						clockUtils::compression::Compression<clockUtils::compression::algorithm::HuffmanGeneric> c;
						std::string compressedPath;
						clockUtils::ClockError cErr = c.compress(homeDir.absolutePath().toStdString(), compressedPath);
						Q_UNUSED(cErr);
						pathLength = int(compressedPath.length());
						memcpy(pl, &pathLength, sizeof(int));
						f.write(pl, sizeof(int));
						f.write(compressedPath.c_str(), pathLength);
						std::string compressed;
						std::string uncompressed;
						char usernameLength[sizeof(size_t)];
						char passwordLength[sizeof(size_t)];
						size_t l = username.toStdString().length();
						memcpy(usernameLength, &l, sizeof(size_t));
						l = passwd.toStdString().length();
						memcpy(passwordLength, &l, sizeof(size_t));
						uncompressed.append(usernameLength, sizeof(size_t));
						uncompressed.append(username.toStdString());
						uncompressed.append(passwordLength, sizeof(size_t));
						uncompressed.append(passwd.toStdString());
						if (clockUtils::ClockError::SUCCESS == c.compress(uncompressed, compressed)) {
							f.write(compressed.c_str(), compressed.length());
						}
					}
				}

				emit loggedIn(username, passwd);
			});
			client.io_service->run();
		}
	}

	void LoginDialog::registerUser() {
		if (_registerPasswordEdit->text() != _registerPasswordRepeatEdit->text()) {
			_registerPasswordEdit->setText("");
			_registerPasswordRepeatEdit->setText("");
			emit showErrorMessage(QApplication::tr("PasswordsDontMatch"));
			return;
		}

		QString passwd = _registerPasswordEdit->text();
		_registerPasswordEdit->setText("");
		_registerPasswordRepeatEdit->setText("");

		QString username = _registerUsernameEdit->text();
		bool stayLoggedIn = _registerStayBox->isChecked();

		if (_onlineMode) {
			HttpsClient client("clockwork-origins.com:19101", false);

			QString content = R"({"type": 0,"username": "%1","password": "%2", "mail": "%3"})";
			content = content.arg(username, passwd);

			// Synchronous request examples
			client.request("POST", "/json", content.toStdString(), [this, username, passwd, stayLoggedIn](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code & ec) {
				clockwork::login::Response rp = statusCodeToResponse(SimpleWeb::StatusCode(std::stoi(response->status_code)));

				switch (rp) {
				case clockwork::login::Response::SUCCESS: {
					break;
				}
				case clockwork::login::Response::CONNECTION_FAILURE: {
					emit showErrorMessage(QApplication::tr("ConnectionFailed"));
					return;
				}
				case clockwork::login::Response::USER_EXISTS: {
					emit showErrorMessage(QApplication::tr("UserExists"));
					return;
				}
				case clockwork::login::Response::MAIL_EXISTS: {
					emit showErrorMessage(QApplication::tr("MailExists"));
					return;
				}
				default: {
					emit showErrorMessage(QApplication::tr("UnknownError"));
					return;
				}
				}

				handleLogin(username, passwd);

				if (stayLoggedIn) {
					QFile f(Config::BASEDIR + "/databases/Login.bin");
					if (f.open(QIODevice::WriteOnly)) {
						QDir homeDir = QDir::home();
						int pathLength = homeDir.absolutePath().length();
						char pl[sizeof(int)];
						memcpy(pl, &pathLength, sizeof(int));
						f.write(pl, sizeof(int));
						clockUtils::compression::Compression<clockUtils::compression::algorithm::HuffmanGeneric> c;
						std::string compressedPath;
						clockUtils::ClockError cErr = c.compress(homeDir.absolutePath().toStdString(), compressedPath);
						Q_UNUSED(cErr);
						pathLength = int(compressedPath.length());
						memcpy(pl, &pathLength, sizeof(int));
						f.write(pl, sizeof(int));
						f.write(compressedPath.c_str(), pathLength);
						std::string compressed;
						std::string uncompressed;
						char usernameLength[sizeof(size_t)];
						char passwordLength[sizeof(size_t)];
						size_t l = username.toStdString().length();
						memcpy(usernameLength, &l, sizeof(size_t));
						l = passwd.toStdString().length();
						memcpy(passwordLength, &l, sizeof(size_t));
						uncompressed.append(usernameLength, sizeof(size_t));
						uncompressed.append(username.toStdString());
						uncompressed.append(passwordLength, sizeof(size_t));
						uncompressed.append(passwd.toStdString());
						if (clockUtils::ClockError::SUCCESS == c.compress(uncompressed, compressed)) {
							f.write(compressed.c_str(), compressed.length());
						}
					}
				}

				emit loggedIn(username, passwd);
			});
			client.io_service->run();
		}
	}

	void LoginDialog::loginSuccess() {
		_connected = true;
		accept();
		hide();
	}

	void LoginDialog::changedLoginInput() {
		_loginButton->setEnabled(_loginUsernameEdit->text().size() > 0 && _loginPasswordEdit->text().size() > 0);
	}

	void LoginDialog::changedRegisterInput() {
		_registerButton->setEnabled(_registerUsernameEdit->text().size() > 0 && _registerPasswordEdit->text().size() > 0 && _registerPasswordRepeatEdit->text().size() > 0 && _registerMailEdit->text().size() > 0);
	}

	void LoginDialog::showErrorMessageBox(QString message) {
		QMessageBox msg(QMessageBox::Icon::Critical, QApplication::tr("ErrorOccurred"), message, QMessageBox::StandardButton::Ok);
		msg.exec();
	}

	void LoginDialog::showInfoMessageBox(QString message) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ForgotPassword"), message, QMessageBox::StandardButton::Ok);
		msg.exec();
	}

	void LoginDialog::dontShowAgainChanged(int state) {
		_dontShow = state == Qt::CheckState::Checked;
		_iniParser->setValue("LOGIN/DontShowAgain", _dontShow);
	}

	void LoginDialog::setLanguage(QString language) {
		_language = language;
	}

	void LoginDialog::resetPassword() {
		QString text = QInputDialog::getText(this, QApplication::tr("ForgotPassword"), QApplication::tr("ForgotPasswordDescription"));
		if (text.isEmpty()) {
			return;
		}
		HttpsClient client("clockwork-origins.com:19101", false);

		QString content = R"({"type": 2,"username": "%1"})";
		content = content.arg(text);

		// Synchronous request examples
		client.request("POST", "/json", content.toStdString(), [this](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code & ec) {
			clockwork::login::Response rp = statusCodeToResponse(SimpleWeb::StatusCode(std::stoi(response->status_code)));

			switch (rp) {
			case clockwork::login::Response::SUCCESS: {
				emit showInfoMessage(QApplication::tr("ForgotPasswordWaitForMail"));
				break;
			}
			case clockwork::login::Response::CONNECTION_FAILURE: {
				emit showErrorMessage(QApplication::tr("ConnectionFailed"));
				return;
			}
			case clockwork::login::Response::WRONG_PASSWORD_OR_USER_NOTFOUND: {
				emit showErrorMessage(QApplication::tr("ForgotPasswordNoUser"));
				return;
			}
			default: {
				emit showErrorMessage(QApplication::tr("UnknownError"));
				return;
			}
			}
		});
		client.io_service->run();
	}

	void LoginDialog::onPrivacyAcceptChanged(bool enabled) {
		_registerButton->setEnabled(enabled);
	}

	void LoginDialog::handleLogin(QString username, QString password) {
		std::thread([this, username, password]() {
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
				{
					common::UpdateLoginTimeMessage ultm;
					ultm.username = username.toStdString();
					std::string serialized = ultm.SerializePublic();
					sock.writePacket(serialized);
				}
				{
					common::SendUserInfosMessage suim;
					suim.username = username.toStdString();
					suim.password = password.toStdString();
					suim.hash = security::Hash::calculateSystemHash().toStdString();
					suim.mac = security::Hash::getMAC().toStdString();
					suim.language = _language.toStdString();
					std::string serialized = suim.SerializePublic();
					sock.writePacket(serialized);
				}
			}
		}).detach();
	}

} /* namespace widgets */
} /* namespace spine */
