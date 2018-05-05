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

#ifndef __SPINE_WIDGETS_APPLYTRANSLATIONDIALOG_H__
#define __SPINE_WIDGETS_APPLYTRANSLATIONDIALOG_H__

#include "common/MessageStructs.h"

#include <QDialog>

class QLineEdit;
class QPushButton;
class QSettings;

namespace spine {
namespace widgets {

	class ApplyTranslationDialog : public QDialog {
		Q_OBJECT

	public:
		ApplyTranslationDialog(uint32_t requestID, QString title, QSettings * iniParser, QWidget * par);
		~ApplyTranslationDialog();

	private slots :
		void openFileDialog();
		void applyTranslation();

	private:
		QSettings * _iniParser;
		uint32_t _requestID;
		QLineEdit * _pathEdit;
		QPushButton * _applyButton;

		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_APPLYTRANSLATIONDIALOG_H__ */
