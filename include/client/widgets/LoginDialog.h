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

#ifndef __SPINE_WIDGETS_LOGINDIALOG_H__
#define __SPINE_WIDGETS_LOGINDIALOG_H__

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QSettings;

namespace spine {
namespace widgets {

	class GeneralSettingsWidget;

	class LoginDialog : public QDialog {
		Q_OBJECT

	public:
		LoginDialog(bool onlineMode, QSettings * iniParser, GeneralSettingsWidget * generalSettingsWidget, QWidget * par);

		QString getUsername() const;
		QString getPassword() const;

	signals:
		void loggedIn(QString, QString);
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
		void setLanguage(QString language);
		void resetPassword();
		void onPrivacyAcceptChanged(bool enabled);

	private:
		QSettings * _iniParser;

		bool _connected;

		QLineEdit * _registerUsernameEdit;
		QLineEdit * _registerMailEdit;
		QLineEdit * _registerPasswordEdit;
		QLineEdit * _registerPasswordRepeatEdit;
		
		QCheckBox * _registerAcceptPrivacyPolicy;
		QCheckBox * _registerStayBox;
		QPushButton * _registerButton;

		QLineEdit * _loginUsernameEdit;
		QLineEdit * _loginPasswordEdit;

		QCheckBox * _loginStayBox;
		QPushButton * _loginButton;

		bool _dontShow;

		QString _language;

		bool _onlineMode;

		void handleLogin(QString username, QString password);
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_LOGINDIALOG_H__ */
