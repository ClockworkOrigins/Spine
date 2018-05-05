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

#ifndef __SPINE_WIDGETS_SAVEGAMEDIALOG_H__
#define __SPINE_WIDGETS_SAVEGAMEDIALOG_H__

#include <QDialog>

class QSettings;
class QSortFilterProxyModel;
class QStandardItem;
class QStandardItemModel;

namespace clockUtils {
namespace iniParser {
	class IniParser;
} /* namespace iniParser */
} /* namespace clockUtils */

namespace spine {
	class SavegameManager;
	struct Variable;
namespace widgets {

	class LocationSettingsWidget;

	class SavegameDialog : public QDialog {
		Q_OBJECT

	public:
		SavegameDialog(LocationSettingsWidget * locationSettingsWidget, QSettings * iniParser, QWidget * par);
		~SavegameDialog();

	private slots:
		void openG1Save();
		void openG2Save();
		void save();
		void itemChanged(QStandardItem * itm);

	private:
		QSortFilterProxyModel * _filterModel;
		QStandardItemModel * _model;
		SavegameManager * _savegameManager;
		QString _gothicDirectory;
		QString _gothic2Directory;
		QString _openedFile;
		QList<Variable> _variables;
		QSettings * _iniParser;

		void updateView();
		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_SAVEGAMEDIALOG_H__ */
