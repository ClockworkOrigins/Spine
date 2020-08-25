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

#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;

namespace spine {
namespace widgets {

	class LoginDialog : public QDialog {
		Q_OBJECT

	public:
		LoginDialog(QWidget * par);

	signals:
		void loggedIn();
		void notLoggedIn();
		void showErrorMessage(QString);
		void showInfoMessage(QString);

	public slots:
		int exec() override;
		int execute();
		void logout();

	private slots:
		void loginUser();
		void registerUser();
		void loginSuccess();
		void changedLoginInput();
		void changedRegisterInput();
		void showErrorMessageBox(QString message);
		void showInfoMessageBox(QString message);
		void dontShowAgainChanged(int state);
		void resetPassword();
		void onPrivacyAcceptChanged(bool enabled);

	private:
		bool _connected;

		QLineEdit * _registerUsernameEdit;
		QLineEdit * _registerMailEdit;
		QLineEdit * _registerPasswordEdit;
		QLineEdit * _registerPasswordRepeatEdit;
		
		QCheckBox * _registerAcceptPrivacyPolicy;
		QCheckBox * _registerSubscribeNewsletterBox;
		QCheckBox * _registerStayBox;
		QPushButton * _registerButton;

		QLineEdit * _loginUsernameEdit;
		QLineEdit * _loginPasswordEdit;

		QCheckBox * _loginStayBox;
		QPushButton * _loginButton;

		bool _dontShow;

		QString _language;

		QMetaObject::Connection _connection;

		void handleLogin();
		void closeEvent(QCloseEvent * evt) override;
		void saveCredentials(const QString & username, const QString & passwd);
	};

} /* namespace widgets */
} /* namespace spine */
