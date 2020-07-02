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

#include "widgets/LoginDialog.h"

#include <regex>
#include <sstream>

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "https/Https.h"

#include "security/Hash.h"

#include "utils/Config.h"

#include "widgets/UpdateLanguage.h"

#include "clockUtils/errors.h"
#include "clockUtils/compression/Compression.h"
#include "clockUtils/compression/algorithm/HuffmanGeneric.h"
#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QTabWidget>
#include <QtConcurrentRun>
#include <QVBoxLayout>

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

using namespace spine;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

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

LoginDialog::LoginDialog(QWidget *) : QDialog(nullptr), _connected(false), _dontShow(false) {
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
	
	const QRegularExpression nameRegex("[a-zA-Z0-9 _.-]+");
	const QRegularExpression passwordRegex(R"([a-zA-Z0-9 _.,@;:\+#!?\(\)\[\]$&{}-]+)");
	const QRegularExpression mailRegex("[a-zA-Z0-9 _.@-]+");

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

		QValidator * nameValidator = new QRegularExpressionValidator(nameRegex, this);
		_registerUsernameEdit->setValidator(nameValidator);

		QValidator * passwordValidator = new QRegularExpressionValidator(passwordRegex, this);
		_registerPasswordEdit->setValidator(passwordValidator);
		_registerPasswordRepeatEdit->setValidator(passwordValidator);

		QValidator * mailValidator = new QRegularExpressionValidator(mailRegex, this);
		_registerMailEdit->setValidator(mailValidator);
		
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
		_registerButton->setShortcut(QKeySequence(Qt::Key_Enter));

		w->setLayout(gl);

		tw->addTab(w, QApplication::tr("Register"));
		UPDATELANGUAGESETTEXTEXT(l1, "Username", " *");
		UPDATELANGUAGESETTEXTEXT(l2, "Mail", " *");
		UPDATELANGUAGESETTEXTEXT(l3, "Password", " *");
		UPDATELANGUAGESETTEXTEXT(l4, "PasswordRepeat", " *");
		UPDATELANGUAGESETTABTEXT(tw, 0, "Register");
		UPDATELANGUAGESETTEXT(_registerAcceptPrivacyPolicy, "AcceptPrivacy");
		UPDATELANGUAGESETTEXT(_registerStayBox, "StayLoggedIn");
		UPDATELANGUAGESETTEXT(_registerButton, "Register");

		connect(_registerButton, &QPushButton::released, this, &LoginDialog::registerUser);
		connect(_registerUsernameEdit, &QLineEdit::textChanged, this, &LoginDialog::changedRegisterInput);
		connect(_registerMailEdit, &QLineEdit::textChanged, this, &LoginDialog::changedRegisterInput);
		connect(_registerPasswordEdit, &QLineEdit::textChanged, this, &LoginDialog::changedRegisterInput);
		connect(_registerPasswordRepeatEdit, &QLineEdit::textChanged, this, &LoginDialog::changedRegisterInput);
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

		QValidator * nameValidator = new QRegularExpressionValidator(nameRegex, this);
		_loginUsernameEdit->setValidator(nameValidator);

		QValidator * passwordValidator = new QRegularExpressionValidator(passwordRegex, this);
		_loginPasswordEdit->setValidator(passwordValidator);

		QPushButton * resetPasswordButton = new QPushButton(QApplication::tr("ForgotPassword"), this);

		_loginStayBox = new QCheckBox(QApplication::tr("StayLoggedIn"), w);
		_loginButton = new QPushButton(QApplication::tr("Login"), w);
		_loginButton->setShortcut(QKeySequence(Qt::Key_Enter));

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
		UPDATELANGUAGESETTEXTEXT(l1, "Username", " *");
		UPDATELANGUAGESETTEXTEXT(l2, "Password", " *");
		UPDATELANGUAGESETTABTEXT(tw, 1, "Login");
		UPDATELANGUAGESETTEXT(_loginStayBox, "StayLoggedIn");
		UPDATELANGUAGESETTEXT(_loginButton, "Login");

		connect(_loginButton, &QPushButton::released, this, &LoginDialog::loginUser);
		connect(_loginUsernameEdit, &QLineEdit::textChanged, this, &LoginDialog::changedLoginInput);
		connect(_loginPasswordEdit, &QLineEdit::textChanged, this, &LoginDialog::changedLoginInput);
		connect(resetPasswordButton, &QPushButton::released, this, &LoginDialog::resetPassword);
	}

	l->addWidget(tw);

	QLabel * infoLabel = new QLabel(QApplication::tr("RequiredField"), this);
	QFont fnt = infoLabel->font();
	fnt.setPointSize(7);
	infoLabel->setFont(fnt);
	UPDATELANGUAGESETTEXT(infoLabel, "RequiredField");

	l->addWidget(infoLabel);

	QLabel * accountInfoLabel = new QLabel(QApplication::tr("AccountInformation"), this);
	accountInfoLabel->setWordWrap(true);
	UPDATELANGUAGESETTEXT(accountInfoLabel, "AccountInformation");

	l->addWidget(accountInfoLabel);

	_dontShow = Config::IniParser->value("LOGIN/DontShowAgain", false).toBool();

	{
		QHBoxLayout * hbl = new QHBoxLayout();

		{
			QCheckBox * cb = new QCheckBox(QApplication::tr("DontShowAgain"), this);
			cb->setProperty("noLogin", true);
			cb->setChecked(_dontShow);
			UPDATELANGUAGESETTEXT(cb, "DontShowAgain");

			hbl->addWidget(cb);

			connect(cb, &QCheckBox::stateChanged, this, &LoginDialog::dontShowAgainChanged);
		}
		{
			QPushButton * pb = new QPushButton(QApplication::tr("ContinueWithoutLogin"), this);
			pb->setProperty("noLogin", true);
			UPDATELANGUAGESETTEXT(pb, "ContinueWithoutLogin");

			hbl->addWidget(pb);

			connect(pb, &QPushButton::clicked, this, &LoginDialog::accept);
			connect(pb, &QPushButton::clicked, this, &LoginDialog::hide);
		}

		l->addLayout(hbl);
	}

	connect(this, &LoginDialog::loggedIn, this, &LoginDialog::loginSuccess);
	connect(this, &LoginDialog::showErrorMessage, this, &LoginDialog::showErrorMessageBox);
	connect(this, &LoginDialog::showInfoMessage, this, &LoginDialog::showInfoMessageBox);

	if (!username.empty() && !password.empty()) {
		_loginUsernameEdit->setText(QString::fromStdString(username));
		_loginPasswordEdit->setText(QString::fromStdString(password));
	}

	changedLoginInput();
	changedRegisterInput();

	tw->setCurrentIndex(1);

	setLayout(l);

	setWindowTitle(QApplication::tr("Login"));
	UPDATELANGUAGESETWINDOWTITLE(this, "Login");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

int LoginDialog::exec() {
	disconnect(_connection);
	
	if (!_loginUsernameEdit->text().isEmpty() && !_loginPasswordEdit->text().isEmpty()) {
		_connection = connect(this, &LoginDialog::showErrorMessage, this, &LoginDialog::exec);
		
		loginUser();		
		return QDialog::Accepted;
	}
	if (_dontShow) {
		emit notLoggedIn();
		
		return QDialog::Accepted;
	}
	
	return QDialog::exec();
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

		Config::Username.clear();
		Config::Password.clear();
		
		emit loggedIn();
	}
}

void LoginDialog::loginUser() {
	const QString passwd = _loginPasswordEdit->text();
	_loginPasswordEdit->setText("");
	const QString username = _loginUsernameEdit->text();
	bool stayLoggedIn = _loginStayBox->isChecked();

	if (Config::OnlineMode) {
		HttpsClient client("clockwork-origins.com:19101", false);

		QString content = R"({"type": 1,"username": "%1","password": "%2"})";
		content = content.arg(username, passwd);

		// Synchronous request examples
		client.request("POST", "/json", content.toStdString(), [this, username, passwd, stayLoggedIn](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &) {
			Q_ASSERT(response);
			if (!response) {
				emit showErrorMessage(QApplication::tr("ConnectionFailed"));
				return;
			}
			const QString code = QString::fromStdString(response->status_code).split(" ")[0];
			const clockwork::login::Response rp = statusCodeToResponse(SimpleWeb::StatusCode(code.toInt()));

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

			handleLogin();

			if (stayLoggedIn) {
				QFile f(Config::BASEDIR + "/databases/Login.bin");
				if (f.open(QIODevice::WriteOnly)) {
					const QDir homeDir = QDir::home();
					int pathLength = homeDir.absolutePath().length();
					char pl[sizeof(int)];
					memcpy(pl, &pathLength, sizeof(int));
					f.write(pl, sizeof(int));
					clockUtils::compression::Compression<clockUtils::compression::algorithm::HuffmanGeneric> c;
					std::string compressedPath;
					clockUtils::ClockError cErr = c.compress(homeDir.absolutePath().toStdString(), compressedPath);
					Q_UNUSED(cErr);
					pathLength = static_cast<int>(compressedPath.length());
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

			Config::Username = username;
			Config::Password = passwd;

			emit loggedIn();
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

	const QString passwd = _registerPasswordEdit->text();
	_registerPasswordEdit->setText("");
	_registerPasswordRepeatEdit->setText("");

	const QString username = _registerUsernameEdit->text();
	const bool stayLoggedIn = _registerStayBox->isChecked();

	const QString mail = _registerMailEdit->text();

	if (Config::OnlineMode) {
		HttpsClient client("clockwork-origins.com:19101", false);

		QString content = R"({"type": 0,"username": "%1","password": "%2", "mail": "%3"})";
		content = content.arg(username, passwd, mail);

		// Synchronous request examples
		client.request("POST", "/json", content.toStdString(), [this, username, passwd, stayLoggedIn](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &) {
			const clockwork::login::Response rp = statusCodeToResponse(SimpleWeb::StatusCode(std::stoi(response->status_code)));

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
			case clockwork::login::Response::INVALID_MAIL: {
				emit showErrorMessage(QApplication::tr("InvalidMail"));
				return;
			}
			default: {
				emit showErrorMessage(QApplication::tr("UnknownError"));
				return;
			}
			}

			handleLogin();

			if (stayLoggedIn) {
				QFile f(Config::BASEDIR + "/databases/Login.bin");
				if (f.open(QIODevice::WriteOnly)) {
					const QDir homeDir = QDir::home();
					int pathLength = homeDir.absolutePath().length();
					char pl[sizeof(int)];
					memcpy(pl, &pathLength, sizeof(int));
					f.write(pl, sizeof(int));
					const clockUtils::compression::Compression<clockUtils::compression::algorithm::HuffmanGeneric> c;
					std::string compressedPath;
					clockUtils::ClockError cErr = c.compress(homeDir.absolutePath().toStdString(), compressedPath);
					Q_UNUSED(cErr);
					pathLength = static_cast<int>(compressedPath.length());
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

			Config::Username = username;
			Config::Password = passwd;

			emit loggedIn();
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
	_loginButton->setEnabled(!_loginUsernameEdit->text().isEmpty() && !_loginPasswordEdit->text().isEmpty());
}

void LoginDialog::changedRegisterInput() {
	_registerButton->setEnabled(!_registerUsernameEdit->text().isEmpty() && !_registerPasswordEdit->text().isEmpty() && !_registerPasswordRepeatEdit->text().isEmpty() && !_registerMailEdit->text().isEmpty() && _registerAcceptPrivacyPolicy->isChecked());
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
	Config::IniParser->setValue("LOGIN/DontShowAgain", _dontShow);
}

void LoginDialog::resetPassword() {
	const QString text = QInputDialog::getText(this, QApplication::tr("ForgotPassword"), QApplication::tr("ForgotPasswordDescription"));
	if (text.isEmpty()) {
		return;
	}
	HttpsClient client("clockwork-origins.com:19101", false);

	QString content = R"({"type": 2,"username": "%1"})";
	content = content.arg(text);

	// Synchronous request examples
	client.request("POST", "/json", content.toStdString(), [this](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &) {
		const clockwork::login::Response rp = statusCodeToResponse(SimpleWeb::StatusCode(std::stoi(response->status_code)));

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

void LoginDialog::onPrivacyAcceptChanged(bool) {
	changedRegisterInput();
}

void LoginDialog::handleLogin() {
	QtConcurrent::run([]() {
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
			{
				common::UpdateLoginTimeMessage ultm;
				ultm.username = Config::Username.toStdString();
				ultm.password = Config::Password.toStdString();
				const std::string serialized = ultm.SerializePublic();
				sock.writePacket(serialized);
			}
			{
				common::SendUserInfosMessage suim;
				suim.username = Config::Username.toStdString();
				suim.password = Config::Password.toStdString();
				suim.hash = security::Hash::calculateSystemHash().toStdString();
				suim.mac = security::Hash::getMAC().toStdString();
				suim.language = Config::Language.toStdString();
				const std::string serialized = suim.SerializePublic();
				sock.writePacket(serialized);
			}
			Https::postAsync(DATABASESERVER_PORT, "getUserID", QString("{ \"Username\": \"%1\", \"Password\": \"%2\" }").arg(Config::Username).arg(Config::Password), [](const QJsonObject & json, int) {
				if (!json.contains("ID")) return;

				Config::UserID = json["ID"].toString().toInt();
			});
		}
	});
}

void LoginDialog::closeEvent(QCloseEvent *) {
	if (_connected) return;

	emit notLoggedIn();
}
