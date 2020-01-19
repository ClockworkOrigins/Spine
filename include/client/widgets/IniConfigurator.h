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

class QSettings;
class QTabWidget;

namespace spine {
namespace widgets {

	class Gothic1IniWidget;
	class Gothic2IniWidget;
	class SystempackIniWidget;

	class IniConfigurator : public QDialog {
		Q_OBJECT

	public:
		IniConfigurator(QString gothicDirectory, QString gothic2Directory, QSettings * iniParser, QWidget * par);
		~IniConfigurator();

	public slots:
		void accept();
		void reject();

	private:
		QSettings * _iniParser;
		QTabWidget * _tabWidget;
		Gothic1IniWidget * _gothic1IniWidget;
		Gothic2IniWidget * _gothic2IniWidget;
		SystempackIniWidget * _systempackGothic1IniWidget;
		SystempackIniWidget * _systempackGothic2IniWidget;

		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */
