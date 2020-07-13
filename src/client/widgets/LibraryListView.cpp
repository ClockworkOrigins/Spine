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
#include <QMenu>

using namespace spine::client;
using namespace spine::common;
using namespace spine::utils;
using namespace spine::widgets;

LibraryListView::LibraryListView(QWidget * par) : QListView(par) {}

void LibraryListView::contextMenuEvent(QContextMenuEvent * evt) {
	if (!selectedIndexes().empty()) {
		const QModelIndex idx = selectedIndexes().constFirst();
		if (idx.data(LibraryFilterModel::InstalledRole).toBool()) {
			QMenu * menu = new QMenu(this);
			if (idx.data(LibraryFilterModel::HiddenRole).toBool()) {
				QAction * showModAction = menu->addAction(QApplication::tr("Show"));
				connect(showModAction, &QAction::triggered, this, &LibraryListView::showModTriggered);
			} else {
				QAction * hideModAction = menu->addAction(QApplication::tr("Hide"));
				connect(hideModAction, &QAction::triggered, this, &LibraryListView::hideModTriggered);
			}
			QAction * uninstallModAction = menu->addAction(QApplication::tr("Uninstall"));
			connect(uninstallModAction, &QAction::triggered, this, &LibraryListView::uninstallModTriggered);

			const int projectID = idx.data(LibraryFilterModel::ModIDRole).toInt();

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
		}
	}
	evt->accept();
}

void LibraryListView::changeLanguage(int projectID, int language) {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE languages SET Language = " + std::to_string(language) + " WHERE ProjectID = " + std::to_string(projectID) + ";", err);

	emit forceUpdate(projectID, true);
}
