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

#ifndef __SPINE_WIDGETS_FAQDIALOG_H__
#define __SPINE_WIDGETS_FAQDIALOG_H__

#include <QDialog>

class QSettings;

namespace spine {
namespace widgets {

	class FAQDialog : public QDialog {
	public:
		FAQDialog(QSettings * iniParser, QWidget * par);
		~FAQDialog();

	private:
		QSettings * _iniParser;

		void initEntries(QWidget * par, QLayout * l);
		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_FAQDIALOG_H__ */
