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

#include "widgets/ManagementDialog.h"

#include <thread>

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Conversion.h"

#include "widgets/management/AchievementsWidget.h"
#include "widgets/management/CustomStatisticsWidget.h"
#include "widgets/management/GeneralConfigurationWidget.h"
#include "widgets/management/ModFilesWidget.h"
#include "widgets/management/ScoresWidget.h"
#include "widgets/management/StatisticsWidget.h"
#include "widgets/management/UserManagementWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QListView>
#include <QSettings>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	ManagementDialog::ManagementDialog(QString username, QString password, QString language, QSettings * iniParser, QWidget * par) : QDialog(par), _iniParser(iniParser), _modList(nullptr), _username(username), _password(password), _language(language), _mods(), _modIndex(-1), _generalConfigurationWidget(nullptr), _modFilesWidget(nullptr), _userManagementWidget(nullptr), _statisticsWidget(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();

			QListView * modList = new QListView(this);
			_modList = new QStandardItemModel(modList);
			modList->setModel(_modList);
			connect(modList, SIGNAL(clicked(QModelIndex)), this, SLOT(selectedMod(QModelIndex)));
			hl->addWidget(modList);

			_tabWidget = new QTabWidget(this);
			_generalConfigurationWidget = new GeneralConfigurationWidget(this);
			_modFilesWidget = new ModFilesWidget(username, language, this);
			_userManagementWidget = new UserManagementWidget(username, language, this);
			_statisticsWidget = new StatisticsWidget(this);
			_achievementsWidget = new AchievementsWidget(this);
			_scoresWidget = new ScoresWidget(this);
			_customStatisticsWidget = new CustomStatisticsWidget(this);
			_tabWidget->addTab(_generalConfigurationWidget, QApplication::tr("General"));
			_tabWidget->addTab(_modFilesWidget, QApplication::tr("ModFiles"));
			_tabWidget->addTab(_userManagementWidget, QApplication::tr("UserManagement"));
			_tabWidget->addTab(_statisticsWidget, QApplication::tr("Statistics"));
			_tabWidget->addTab(_achievementsWidget, QApplication::tr("Achievements"));
			_tabWidget->addTab(_scoresWidget, QApplication::tr("Scores"));
			_tabWidget->addTab(_customStatisticsWidget, QApplication::tr("CustomStatistics"));
			hl->addWidget(_tabWidget);

			l->addLayout(hl);
		}

		setLayout(l);

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(QApplication::tr("Management"));

		qRegisterMetaType<QList<QPair<QString, int>>>("QList<QPair<QString, int>>");
		qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");

		connect(this, &ManagementDialog::receivedMods, this, &ManagementDialog::updateModList);
		connect(this, &ManagementDialog::receivedUsers, _userManagementWidget, &UserManagementWidget::updateUserList);
		connect(_generalConfigurationWidget, &GeneralConfigurationWidget::triggerInfoPage, this, &ManagementDialog::triggerInfoPage);
		connect(_generalConfigurationWidget, &GeneralConfigurationWidget::triggerInfoPage, this, &ManagementDialog::reject);
		connect(_modFilesWidget, &ModFilesWidget::checkForUpdate, this, &ManagementDialog::checkForUpdate);

		_tabWidget->setEnabled(false);

		loadModList();

		restoreSettings();
	}

	ManagementDialog::~ManagementDialog() {
		saveSettings();
	}

	void ManagementDialog::updateModList(QList<client::ManagementMod> modList) {
		std::sort(modList.begin(), modList.end(), [](const client::ManagementMod & a, const client::ManagementMod & b) {
			return a.name < b.name;
		});
		_mods = modList;
		for (const auto & m : modList) {
			QStandardItem * itm = new QStandardItem(m.name);
			itm->setEditable(false);
			_modList->appendRow(itm);
		}
		_generalConfigurationWidget->updateModList(modList);
		_modFilesWidget->updateModList(modList);
		_userManagementWidget->updateModList(modList);
		_statisticsWidget->updateModList(modList);
		_achievementsWidget->updateModList(modList);
		_scoresWidget->updateModList(modList);
		_customStatisticsWidget->updateModList(modList);

		_tabWidget->setEnabled(!modList.empty());
	}

	void ManagementDialog::selectedMod(const QModelIndex & index) {
		_modIndex = index.row();
		_generalConfigurationWidget->selectedMod(_modIndex);
		_modFilesWidget->selectedMod(_modIndex);
		_userManagementWidget->selectedMod(_modIndex);
		_statisticsWidget->selectedMod(_modIndex);
		_achievementsWidget->selectedMod(_modIndex);
		_scoresWidget->selectedMod(_modIndex);
		_customStatisticsWidget->selectedMod(_modIndex);
	}

	void ManagementDialog::loadModList() {
		https::Https::postAsync(MANAGEMENTSERVER_PORT, "getMods", QString("{ \"username\": \"%1\", \"password\": \"%2\", \"language\": \"%3\" }").arg(_username, _password, _language), [this](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) return;
			
			const auto it = json.find("Mods");
			
			if (it == json.end()) return;

			const auto mods = it->toArray();

			QList<client::ManagementMod> modList;
			
			for (auto mod : mods) {
				const auto jo = mod.toObject();

				if (jo.isEmpty()) continue;

				if (!jo.contains("Name")) continue;
				
				if (!jo.contains("ID")) continue;

				client::ManagementMod entry {
					jo["Name"].toString(),
					jo["ID"].toInt()
				};
				modList.append(entry);
			}

			emit receivedMods(modList);
		});
	}

	void ManagementDialog::restoreSettings() {
		const QByteArray arr = _iniParser->value("WINDOWGEOMETRY/ManagementDialogGeometry", QByteArray()).toByteArray();
		if (!restoreGeometry(arr)) {
			_iniParser->remove("WINDOWGEOMETRY/ManagementDialogGeometry");
		}
	}

	void ManagementDialog::saveSettings() {
		_iniParser->setValue("WINDOWGEOMETRY/ManagementDialogGeometry", saveGeometry());
	}

} /* namespace widgets */
} /* namespace spine */
