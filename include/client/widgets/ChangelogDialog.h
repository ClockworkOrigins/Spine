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

#ifndef __SPINE_WIDGETS_CHANGELOGDIALOG_H__
#define __SPINE_WIDGETS_CHANGELOGDIALOG_H__

#include <QDialog>

class QSettings;
class QTextBrowser;

namespace spine {
namespace widgets {

	class ChangelogDialog : public QDialog {
		Q_OBJECT

	public:
		ChangelogDialog(QSettings * iniParser, QWidget * par);
		~ChangelogDialog();

	public slots:
		int execStartup();
		int exec() override;

	private slots:
		void changedShowState(int state);

	private:
		QTextBrowser * _changelogBrowser;
		QSettings * _iniParser;

		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_CHANGELOGDIALOG_H__ */
