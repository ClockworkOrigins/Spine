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

#include "widgets/LibraryListView.h"

#include "SpineConfig.h"
#include "IconCache.h"
#include "LibraryFilterModel.h"

#include "common/Language.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QStandardPaths>

using namespace spine::client;
using namespace spine::common;
using namespace spine::utils;
using namespace spine::widgets;

LibraryListView::LibraryListView(QWidget * par) : QListView(par) {}

void LibraryListView::contextMenuEvent(QContextMenuEvent * evt) {
	if (selectedIndexes().empty()) {
		evt->accept();
		return;
	}
	
	const QModelIndex idx = selectedIndexes().constFirst();
	if (!idx.data(LibraryFilterModel::InstalledRole).toBool()) {
		evt->accept();
		return;
	}
	
	auto * menu = new QMenu(this);
	if (idx.data(LibraryFilterModel::HiddenRole).toBool()) {
		QAction * showModAction = menu->addAction(QApplication::tr("Show"));
		connect(showModAction, &QAction::triggered, this, &LibraryListView::showModTriggered);
	} else {
		QAction * hideModAction = menu->addAction(QApplication::tr("Hide"));
		connect(hideModAction, &QAction::triggered, this, &LibraryListView::hideModTriggered);
	}

	const int projectID = idx.data(LibraryFilterModel::ModIDRole).toInt();
	const QString projectName = idx.data(Qt::DisplayRole).toString();
	QAction * checkIntegrityAction = menu->addAction(QApplication::tr("CheckIntegrity"));
	connect(checkIntegrityAction, &QAction::triggered, this, [this, projectID]() {
		emit checkIntegrity(projectID);
	});

	QAction * createShortcut = menu->addAction(QApplication::tr("CreateShortcut"));
	connect(createShortcut, &QAction::triggered, this, [this, projectID, projectName]() {
		createDesktopShortcut(projectID, projectName);
	});

	menu->addSeparator();
	
	QAction * uninstallModAction = menu->addAction(QApplication::tr("Uninstall"));
	connect(uninstallModAction, &QAction::triggered, this, &LibraryListView::uninstallModTriggered);

	Database::DBError err;
	auto l = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Languages FROM supportedLanguages WHERE ProjectID = " + std::to_string(projectID) + " LIMIT 1;", err);
	auto cl = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Language FROM languages WHERE ProjectID = " + std::to_string(projectID) + " LIMIT 1;", err);

	const int currentLanguage = cl.empty() ? English : cl[0];

	if (!l.empty()) {
		const int languages = l[0];
		
		menu->addSeparator();

		QMenu * languageMenu = menu->addMenu(QApplication::tr("Language"));

		if (languages & English) {
			QAction * a = languageMenu->addAction(IconCache::getInstance()->getOrLoadIcon(":/languages/en-US.png"), QApplication::tr("English"));

			if (currentLanguage != English) {
				connect(a, &QAction::triggered, [this, projectID]() {
					changeLanguage(projectID, English);
				});
			} else {
				a->setChecked(true);
			}
		}

		if (languages & German) {
			QAction * a = languageMenu->addAction(IconCache::getInstance()->getOrLoadIcon(":/languages/de-DE.png"), QApplication::tr("German"));

			if (currentLanguage != German) {
				connect(a, &QAction::triggered, [this, projectID]() {
					changeLanguage(projectID, German);
				});
			} else {
				a->setChecked(true);
				languageMenu->setActiveAction(a);
			}
		}

		if (languages & Polish) {
			QAction * a = languageMenu->addAction(IconCache::getInstance()->getOrLoadIcon(":/languages/pl-PL.png"), QApplication::tr("Polish"));

			if (currentLanguage != Polish) {
				connect(a, &QAction::triggered, [this, projectID]() {
					changeLanguage(projectID, Polish);
				});
			} else {
				a->setChecked(true);
			}
		}

		if (languages & Russian) {
			QAction * a = languageMenu->addAction(IconCache::getInstance()->getOrLoadIcon(":/languages/ru-RU.png"), QApplication::tr("Russian"));

			if (currentLanguage != Russian) {
				connect(a, &QAction::triggered, [this, projectID]() {
					changeLanguage(projectID, Russian);
				});
			} else {
				a->setChecked(true);
			}
		}
	}
	
	menu->popup(viewport()->mapToGlobal(evt->pos()));
	menu->exec();
	delete menu;
	
	evt->accept();
}

void LibraryListView::changeLanguage(int projectID, int language) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE languages SET Language = " + std::to_string(language) + " WHERE ProjectID = " + std::to_string(projectID) + ";", err);

	emit forceUpdate(projectID, true);
}

void LibraryListView::createDesktopShortcut(int projectID, const QString & projectName) {
	const auto cmd = "spine://start/" + QString::number(projectID);
	QFile f(cmd);

	const auto linkPath = QStandardPaths::locate(QStandardPaths::DesktopLocation, "", QStandardPaths::LocateDirectory) + projectName + ".url";

	const QString linkCommand1 = QString("echo [InternetShortcut] > \"%1\"").arg(linkPath);
	const QString linkCommand2 = QString("echo URL=%2 >> \"%1\"").arg(linkPath).arg(cmd);
	const QString linkCommand3 = QString("echo IconFile=%2 >> \"%1\"").arg(linkPath).arg(QDir::toNativeSeparators(Config::DOWNLOADDIR + "/icons/" + QString::number(projectID) + ".ico"));
	const QString linkCommand4 = QString("echo IconIndex=0 >> \"%1\"").arg(linkPath);
	const QString linkCommand5 = QString("echo HotKey=0 >> \"%1\"").arg(linkPath);
	const QString linkCommand6 = QString("echo IDList= >> \"%1\"").arg(linkPath);
	
	system(linkCommand1.toStdString().c_str());
	system(linkCommand2.toStdString().c_str());
	system(linkCommand3.toStdString().c_str());
	system(linkCommand4.toStdString().c_str());
	system(linkCommand5.toStdString().c_str());
	system(linkCommand6.toStdString().c_str());
}
