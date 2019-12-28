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

#include "Config.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char ** argv) {
	QApplication app(argc, argv);
	const int i = spine::Config::Init();
	if (i != 0) {
		return i;
	}
	QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("RemoveAllMods"), QApplication::tr("RemoveAllModsDescription"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
	msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Yes"));
	msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("No"));
	QTimer::singleShot(0, &msg, &QMessageBox::exec);
	const int result = QApplication::exec();

	if (QMessageBox::StandardButton::Ok == msg.result()) {
		if (!spine::Config::BASEDIR.isEmpty() && spine::Config::BASEDIR.endsWith("Clockwork Origins/Spine")) {
			QDir(spine::Config::BASEDIR + "/").removeRecursively();
		}
		if (!spine::Config::MODDIR.isEmpty()) {
			QDir(spine::Config::MODDIR + "/mods/").removeRecursively();
		}
	}

	return result;
}
