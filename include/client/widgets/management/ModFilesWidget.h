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

#ifndef __SPINE_WIDGETS_MANAGEMENT_MODFILESWIDGET_H__
#define __SPINE_WIDGETS_MANAGEMENT_MODFILESWIDGET_H__

#include "ManagementCommon.h"

#include <QMap>
#include <QWidget>

class QSpinBox;
class QStandardItem;
class QStandardItemModel;
class QTreeView;
class QWinTaskbarProgress;

namespace clockUtils {
namespace iniParser {
	class IniParser;
} /* namespace iniParser */
} /* namespace clockUtils */

namespace spine {
namespace widgets {

	class WaitSpinner;

	class ModFilesWidget : public QWidget {
		Q_OBJECT

	public:
		ModFilesWidget(QString username, QString language, QWidget * par);

		void updateModList(QList<client::ManagementMod> modList);
		void selectedMod(int index);

	signals:
		void finishedUpload(bool, int);
		void updateUploadText(QString);
		void checkForUpdate(int32_t);
		void updateProgress(int);
		void updateProgressMax(int);

	private slots:
		void addFile();
		void deleteFile();
		void uploadCurrentMod();
		void changedLanguage(QStandardItem * itm);
		void updateVersion();
		void finishUpload(bool success, int updatedCount);
		void testUpdate();

	private:
		QStandardItemModel * _fileList;
		QString _username;
		QString _language;
		QList<client::ManagementMod> _mods;
		QTreeView * _fileTreeView;
		int _modIndex;
		QMap<QString, QStandardItem *> _directory;
		QMap<QString, QString> _fileMap;
		QSpinBox * _majorVersionBox;
		QSpinBox * _minorVersionBox;
		QSpinBox * _patchVersionBox;
		WaitSpinner * _waitSpinner;
		QWinTaskbarProgress * _taskbarProgress;

		void addFile(QStandardItem * itm, QString file, QString language);
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MANAGEMENT_MODFILESWIDGET_H__ */
