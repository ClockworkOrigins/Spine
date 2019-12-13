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

#pragma once

#include "ManagementCommon.h"

#include "common/MessageStructs.h"

#include <QDialog>

class QSettings;
class QStandardItemModel;
class QTabWidget;

namespace spine {
namespace client {
namespace widgets {

	class AchievementsWidget;
	class CustomStatisticsWidget;
	class GeneralConfigurationWidget;
	class ModFilesWidget;
	class ScoresWidget;
	class StatisticsWidget;
	class UserManagementWidget;

	class ManagementDialog : public QDialog {
		Q_OBJECT

	public:
		ManagementDialog(QString username, QString password, QString language, QSettings * iniParser, QWidget * par);
		~ManagementDialog();

	signals:
		void receivedMods(QList<ManagementMod>);
		void triggerInfoPage(int32_t);
		void checkForUpdate(int32_t);

	private slots:
		void updateModList(QList<client::ManagementMod> modList);
		void selectedMod(const QModelIndex & index);
		void changedTab();

	private:
		QSettings * _iniParser;
		QStandardItemModel * _modList;
		QString _username;
		QString _password;
		QString _language;
		QList<ManagementMod> _mods;
		int _modIndex;
		GeneralConfigurationWidget * _generalConfigurationWidget;
		ModFilesWidget * _modFilesWidget;
		UserManagementWidget * _userManagementWidget;
		StatisticsWidget * _statisticsWidget;
		AchievementsWidget * _achievementsWidget;
		ScoresWidget * _scoresWidget;
		CustomStatisticsWidget * _customStatisticsWidget;
		QTabWidget * _tabWidget;

		void loadModList();
		void restoreSettings();
		void saveSettings();
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
