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

class QLineEdit;
class QTextEdit;

namespace spine {
namespace widgets {

	class FeedbackDialog : public QDialog {
		Q_OBJECT

	public:
		enum class Type {
			Spine,
			Project
		};

		FeedbackDialog(int32_t projectID, Type type, uint8_t majorVersion, uint8_t minorVersion, uint8_t patchVersion);

	public slots:
		void loginChanged();

	private slots:
		void accept() override;
		void reject() override;

	private:
		QTextEdit * _textEdit;
		QLineEdit * _usernameEdit;

		int32_t _projectID;

		Type _type;

		uint8_t _majorVersion;
		uint8_t _minorVersion;
		uint8_t _patchVersion;

		void closeEvent(QCloseEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
