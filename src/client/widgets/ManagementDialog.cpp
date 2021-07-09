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

#include "widgets/ManagementDialog.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"

#include "widgets/management/AchievementsWidget.h"
#include "widgets/management/CustomStatisticsWidget.h"
#include "widgets/management/GeneralConfigurationWidget.h"
#include "widgets/management/ModFilesWidget.h"
#include "widgets/management/ScoresWidget.h"
#include "widgets/management/StatisticsWidget.h"
#include "widgets/management/SurveyWidget.h"
#include "widgets/management/UserManagementWidget.h"

#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QSettings>
#include <QStandardItemModel>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::utils;

ManagementDialog::ManagementDialog(QWidget * par) : QDialog(par), _modList(nullptr), _modIndex(-1), _generalConfigurationWidget(nullptr), _modFilesWidget(nullptr), _userManagementWidget(nullptr), _statisticsWidget(nullptr), _surveyWidget(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * hl = new QHBoxLayout();

		auto * modList = new QListView(this);
		_modList = new QStandardItemModel(modList);
		modList->setModel(_modList);
		connect(modList, &QListView::clicked, this, &ManagementDialog::selectedMod);
		hl->addWidget(modList);

		_tabWidget = new QTabWidget(this);
		_generalConfigurationWidget = new GeneralConfigurationWidget(this);
		_modFilesWidget = new ModFilesWidget(this);
		_userManagementWidget = new UserManagementWidget(this);
		_statisticsWidget = new StatisticsWidget(this);
		_achievementsWidget = new AchievementsWidget(this);
		_scoresWidget = new ScoresWidget(this);
		_customStatisticsWidget = new CustomStatisticsWidget(this);
		_surveyWidget = new SurveyWidget(this);
		_tabWidget->addTab(_generalConfigurationWidget, QApplication::tr("General"));
		_tabWidget->addTab(_modFilesWidget, QApplication::tr("ModFiles"));
		_tabWidget->addTab(_userManagementWidget, QApplication::tr("UserManagement"));
		_tabWidget->addTab(_statisticsWidget, QApplication::tr("Statistics"));
		_tabWidget->addTab(_achievementsWidget, QApplication::tr("Achievements"));
		_tabWidget->addTab(_scoresWidget, QApplication::tr("Scores"));
		_tabWidget->addTab(_customStatisticsWidget, QApplication::tr("CustomStatistics"));
		_tabWidget->addTab(_surveyWidget, QApplication::tr("Surveys"));
		hl->addWidget(_tabWidget);

		connect(_tabWidget, &QTabWidget::currentChanged, this, &ManagementDialog::changedTab);

		l->addLayout(hl);
	}

	setLayout(l);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(QApplication::tr("Management"));

	qRegisterMetaType<QList<ManagementMod>>("QList<ManagementMod>");

	connect(this, &ManagementDialog::receivedMods, this, &ManagementDialog::updateModList);
	connect(_generalConfigurationWidget, &GeneralConfigurationWidget::triggerInfoPage, this, &ManagementDialog::triggerInfoPage);
	connect(_generalConfigurationWidget, &GeneralConfigurationWidget::triggerInfoPage, this, &ManagementDialog::reject);
	connect(_modFilesWidget, &ModFilesWidget::checkForUpdate, this, &ManagementDialog::checkForUpdate);

	_tabWidget->setEnabled(false);

	loadModList();

	restoreSettings();
}

ManagementDialog::~ManagementDialog() {
	saveSettings();

	_futureWatcher.waitForFinished();
}

void ManagementDialog::updateModList(QList<ManagementMod> modList) {
	std::sort(modList.begin(), modList.end(), [](const ManagementMod & a, const ManagementMod & b) {
		return a.name < b.name;
	});
	_mods = modList;
	for (const auto & m : modList) {
		auto * itm = new QStandardItem(m.name + QStringLiteral(" (ID: %1)").arg(m.id));
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
	_surveyWidget->updateModList(modList);

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
	_surveyWidget->selectedMod(_modIndex);

	changedTab();
}

void ManagementDialog::loadModList() {
	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["Language"] = Config::Language;
	
	const auto f = https::Https::postAsync(MANAGEMENTSERVER_PORT, "getMods", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) return;
		
		const auto it = json.find("Mods");
		
		if (it == json.end()) return;

		const auto mods = it->toArray();

		QList<ManagementMod> modList;
		
		for (auto mod : mods) {
			const auto jo = mod.toObject();

			ManagementMod entry;
			entry.read(jo);
			
			modList.append(entry);
		}

		emit receivedMods(modList);
	});
	_futureWatcher.setFuture(f);
}

void ManagementDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/ManagementDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/ManagementDialogGeometry");
	}
}

void ManagementDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/ManagementDialogGeometry", saveGeometry());
}

void ManagementDialog::changedTab() {
	auto * const tab = _tabWidget->currentWidget();
	auto * managementWidget = dynamic_cast<IManagementWidget *>(tab);

	managementWidget->updateView();
}
