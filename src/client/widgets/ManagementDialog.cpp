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

#include "Conversion.h"
#include "SpineConfig.h"

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

		qRegisterMetaType<std::vector<common::SendModManagementMessage::ModManagement>>("std::vector<common::SendModManagementMessage::ModManagement>");
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

	void ManagementDialog::updateModList(std::vector<common::SendModManagementMessage::ModManagement> modList) {
		std::sort(modList.begin(), modList.end(), [](const common::SendModManagementMessage::ModManagement & a, const common::SendModManagementMessage::ModManagement & b) {
			return a.name < b.name;
		});
		_mods = modList;
		for (const auto & m : modList) {
			QStandardItem * itm = new QStandardItem(s2q(m.name));
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
		std::thread([this]() {
			common::RequestModManagementMessage rmmm;
			rmmm.username = _username.toStdString();
			rmmm.password = _password.toStdString();
			rmmm.language = _language.toStdString();
			std::string serialized = rmmm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendModManagementMessage * smmm = dynamic_cast<common::SendModManagementMessage *>(m);
							emit receivedMods(smmm->modList);
							emit receivedUsers(smmm->userList);
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << int(cErr);
				}
			}
		}).detach();
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
