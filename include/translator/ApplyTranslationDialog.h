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

#include "common/MessageStructs.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QSettings;

namespace spine {
namespace translation {

	class ApplyTranslationDialog : public QDialog {
		Q_OBJECT

	public:
		ApplyTranslationDialog(uint32_t requestID, QString title, QWidget * par);
		~ApplyTranslationDialog();

	private slots :
		void openFileDialog();
		void applyTranslation();

	private:
		uint32_t _requestID;
		QLineEdit * _pathEdit;
		QPushButton * _applyButton;

		void restoreSettings();
		void saveSettings();
	};

} /* namespace translation */
} /* namespace spine */
