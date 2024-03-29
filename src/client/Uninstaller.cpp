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

#include "Uninstaller.h"

#include "SpineConfig.h"

#include "utils/Config.h"
#include "utils/Database.h"
#include "utils/Hashing.h"

#include "widgets/UninstallDialog.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDirIterator>
#include <QMessageBox>

using namespace spine::client;
using namespace spine::utils;
using namespace spine::widgets;

bool Uninstaller::uninstall(uint32_t modID, QString modName, QString directory) {
	UninstallDialog dlg(QApplication::tr("ReallyWantToUninstall"), QApplication::tr("ReallyWantToUninstallText").arg(modName), Config::DOWNLOADDIR + "/mods/" + QString::number(modID));
	
	if (QDialog::DialogCode::Accepted != dlg.exec()) return false;
	
	if (!dlg.keepSavegame()) {
		QDirIterator it(Config::DOWNLOADDIR + "/mods/" + QString::number(modID), QStringList() << "*ini", QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			QDir dir(QString("%1/saves_%2/").arg(directory).arg(it.fileInfo().baseName()));
			dir.removeRecursively();
		}
	}

	Database::DBError err;

	// get all modfiles for the mod
	const auto files = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File, Hash FROM modfiles WHERE ModID = " + std::to_string(modID) + ";", err);

	// check if found file is a mod file and in case it is, remove it
	for (const auto & p : files) {
		QFile checkFile(directory + "/" + QString::fromStdString(p.first));
		const bool b = Hashing::checkHash(directory + "/" + QString::fromStdString(p.first), QString::fromStdString(p.second));
		if (b) {
			checkFile.close();
			checkFile.remove();
		}
	}

	const bool b = uninstall(modID);
	
	if (b) {
		QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("UninstallationSuccessful"), QApplication::tr("UninstallationSuccessfulText").arg(modName), QMessageBox::StandardButton::Ok);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		resultMsg.exec();
		return true;
	}
	
	QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("UninstallationUnsuccessful"), QApplication::tr("UninstallationUnsuccessfulText").arg(modName), QMessageBox::StandardButton::Ok);
	resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
	resultMsg.exec();
	
	return false;
}

bool Uninstaller::uninstall(uint32_t modID) {
	QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(modID));
	if (dir.removeRecursively()) {
		Database::DBError err;

		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM modfiles WHERE ModID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM mods WHERE ModID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM patches WHERE ModID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM packages WHERE ModID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM installDates WHERE ModID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM languages WHERE ProjectID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE PatchID = " + std::to_string(modID) + ";", err);
		
		return true;
	}
	
	return false;
}
