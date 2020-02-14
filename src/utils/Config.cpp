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

#include <iostream>
#include <thread>

#include <QApplication>
#include <QDirIterator>
#include <QFontDatabase>
#include <QLocale>
#include <QProcessEnvironment>
#include <QSettings>
#include <QtConcurrentRun>
#include <QTranslator>

#ifdef Q_OS_WIN
	#include <Windows.h>
#endif

using namespace spine;
using namespace spine::utils;

QString Config::BASEDIR;
QString Config::MODDIR;
QString Config::NEWSIMAGEDIR;
QString Config::STYLESDIR;
QSettings * Config::IniParser = nullptr;
bool Config::OnlineMode = true;

QString Config::Username = "";
QString Config::Password = "";
QString Config::Language = "";

int Config::downloadRate = 5120;
bool Config::extendedLogging = false;

int Config::Init() {
	struct TamperCheckWrapper {
		TamperCheckWrapper() {
#ifdef SPINE_RELEASE
			t = std::thread([this]() {
				while (running || false) {
					if (IsDebuggerPresent()) {
						// do some weird stuff here
						char * buf = new char[1024 * 1024 * 1024];
						std::cout << buf;
					}
					std::this_thread::sleep_for(std::chrono::seconds(5));
				}
			});
#endif
		}

		~TamperCheckWrapper() {
			running = false;
#ifdef SPINE_RELEASE
			t.join();
#endif
		}
		bool running = true;
		std::thread t;
	};
	static TamperCheckWrapper tcw;

	BASEDIR = QProcessEnvironment::systemEnvironment().value("APPDATA");
	QRegExp regEx("[A-Za-z0-9:\\\\/_ ]+");
	if (!regEx.exactMatch(BASEDIR)) {
		BASEDIR = BASEDIR[0] + ":/Users/Public";
	}
	if (BASEDIR.isEmpty()) {
		return -1;
	}
	BASEDIR = BASEDIR.replace("\\", "/");
	BASEDIR += "/Clockwork Origins/Spine";
	{
		QDir baseDir(BASEDIR);
		if (!baseDir.exists()) {
			bool b = baseDir.mkpath(baseDir.absolutePath());
			Q_UNUSED(b);
		}
		Q_ASSERT(baseDir.exists());
	}
	STYLESDIR = BASEDIR + "/styles/";
	{
		QDir stylesDir(STYLESDIR);
		if (!stylesDir.exists()) {
			bool b = stylesDir.mkpath(stylesDir.absolutePath());
			Q_UNUSED(b);
		}
	}
	NEWSIMAGEDIR = BASEDIR + "/news/images/";
	{
		QDir newsDir(NEWSIMAGEDIR);
		if (!newsDir.exists()) {
			bool b = newsDir.mkpath(NEWSIMAGEDIR);
			Q_UNUSED(b);
		}
	}
	const QString logDir = BASEDIR + "/logs/";
	{
		QDir ld(logDir);
		if (!ld.exists()) {
			bool b = ld.mkpath(logDir);
			Q_UNUSED(b);
		}
	}
	QFile oldIni(qApp->applicationDirPath() + "/Spine.ini");
	if (oldIni.exists()) {
		if (!QFileInfo::exists(BASEDIR + "/Spine.ini")) {
			oldIni.rename(BASEDIR + "/Spine.ini");
		}
	}
	IniParser = new QSettings(BASEDIR + "/Spine.ini", QSettings::IniFormat);
	{
		const QString path = IniParser->value("PATH/Downloads", "").toString();
		MODDIR = path;
	}
	QString language = IniParser->value("MISC/language", "").toString();
	if (language.isEmpty()) {
		QLocale locale = QLocale::system();
		if (locale.language() == QLocale::Language::German) {
			language = "Deutsch";
		} else if (locale.language() == QLocale::Language::Polish) {
			language = "Polish";
		} else if (locale.language() == QLocale::Language::Russian) {
			language = "Russian";
		} else if (locale.language() == QLocale::Language::Spanish) {
			language = "Spanish";
		} else {
			language = "English";
		}
		IniParser->setValue("MISC/language", language);
	}

	QString style = IniParser->value("MISC/style", "Default").toString();
	if (style != "Default" && style != "Dark Theme By Elgcahlxukuth" && style != "Dark Theme By Milky-Way") {
		style = STYLESDIR + "/" + style + ".css";
		if (!QFileInfo::exists(style)) {
			style = ":styles.css";
		}
	} else if (style == "Dark Theme By Elgcahlxukuth") {
		style = ":dark_theme_for_spine_app.css";
	} else if (style == "Dark Theme By Milky-Way") {
		style = ":monokai.css";
	} else {
		style = ":styles.css";
	}
	QFile stylesFile(style);
	if (stylesFile.open(QIODevice::ReadOnly)) {
		const QString s(stylesFile.readAll());
		qApp->setStyleSheet(s);
	} else {
		return -1;
	}
	QTranslator * translator = new QTranslator(qApp);
	if (language == "Deutsch") {
		QLocale::setDefault(QLocale("de_DE"));
		translator->load(qApp->applicationDirPath() + "/de_DE");
	} else if (language == "Polish") {
		QLocale::setDefault(QLocale(QLocale::Language::Polish, QLocale::Country::Poland));
		translator->load(qApp->applicationDirPath() + "/po_PO");
	} else if (language == "Russian") {
		QLocale::setDefault(QLocale(QLocale::Language::Russian, QLocale::Country::Russia));
		translator->load(qApp->applicationDirPath() + "/ru_RU");
	} else if (language == "Spanish") {
		QLocale::setDefault(QLocale(QLocale::Language::Spanish, QLocale::Country::Spain));
		translator->load(qApp->applicationDirPath() + "/es_ES");
	} else {
		QLocale::setDefault(QLocale("en_US"));
		translator->load(qApp->applicationDirPath() + "/en_US");
	}
	qApp->installTranslator(translator);
	QFontDatabase::addApplicationFont(":/fontawesome-webfont.ttf");
	QFontDatabase::addApplicationFont(":/Lato.ttf");
	{
		QDir oldDir(qApp->applicationDirPath() + "/../databases");
		QDir dir = BASEDIR + "/databases";
		if (oldDir.exists() && !dir.exists()) {
			QDirIterator it(oldDir.absolutePath(), QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
			QStringList files;
			while (it.hasNext()) {
				QString fileName = it.filePath();
				if (!fileName.isEmpty()) {
					files.append(fileName);
				}
				it.next();
			}
			QString fileName = it.filePath();
			for (QString file : files) {
				QFileInfo fi(file);
				QDir newDir(dir.absolutePath() + fi.absolutePath().replace(oldDir.absolutePath(), ""));
				if (!newDir.exists()) {
					bool b = newDir.mkpath(newDir.absolutePath());
					Q_UNUSED(b);
				}
				QFile copyFile(file);
				copyFile.rename(dir.absolutePath() + file.replace(oldDir.absolutePath(), ""));
			}
			oldDir.removeRecursively();
		}
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b);
		}
	}
	{
		QDir oldDir(qApp->applicationDirPath() + "/../mods");
		QDir dir = BASEDIR + "/mods";
		if (oldDir.exists() && !dir.exists()) {
			QDirIterator it(oldDir.absolutePath(), QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
			QStringList files;
			while (it.hasNext()) {
				QString fileName = it.filePath();
				if (!fileName.isEmpty()) {
					files.append(fileName);
				}
				it.next();
			}
			QString fileName = it.filePath();
			for (QString file : files) {
				QFileInfo fi(file);
				QDir newDir(dir.absolutePath() + fi.absolutePath().replace(oldDir.absolutePath(), ""));
				if (!newDir.exists()) {
					bool b = newDir.mkpath(newDir.absolutePath());
					Q_UNUSED(b);
				}
				QFile copyFile(file);
				copyFile.rename(dir.absolutePath() + file.replace(oldDir.absolutePath(), ""));
			}
			oldDir.removeRecursively();
		}
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b);
		}
	}
	return 0;
}
