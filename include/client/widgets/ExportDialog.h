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

#ifndef __SPINE_WIDGETS_EXPORTDIALOG_H__
#define __SPINE_WIDGETS_EXPORTDIALOG_H__

#include <QDialog>

class QLineEdit;
class QPushButton;

namespace spine {
namespace widgets {

	class ExportDialog : public QDialog {
		Q_OBJECT

	public:
		ExportDialog(QWidget * par);
		~ExportDialog();

	signals:
		void updateProgress(int);
		void updateFile(QString);

	private slots:
		void openExportPathDialog();
		void exportMods();

	private:
		QLineEdit * _exportPathLineEdit;
		QPushButton * _exportPushButton;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_EXPORTDIALOG_H__ */
