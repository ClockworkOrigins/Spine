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
#include <QProcess>

#ifdef Q_OS_WIN

class QLineEdit;

namespace spine {
namespace widgets {

	class InstallGothic2FromCDDialog : public QDialog {
		Q_OBJECT

	public:
		InstallGothic2FromCDDialog();

	signals:
		void updateGothic2Directory(QString);
		void updateProgressLog(QString);

	public slots:
		int exec() override;

	private slots:
		void openGothicFileDialog();
		void startInstallation();
		void startAddonInstallation(int exitCode, QProcess::ExitStatus exitStatus);
		void finishedInstallation(int exitCode, QProcess::ExitStatus exitStatus);
		void updateLog();

	private:
		QLineEdit * _gothicPathLineEdit;
		QString _installationLog;
	};

} /* namespace widgets */
} /* namespace spine */

#endif
