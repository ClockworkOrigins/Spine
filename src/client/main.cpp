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

#include "widgets/MainWindow.h"

#include <thread>

#include "SpineConfig.h"
#include "UrlProtocolHandler.h"

#include "utils/Config.h"
#include "utils/FileLogger.h"
#include "utils/WindowsExtensions.h"

#include "clockUtils/compression/Compression.h"
#include "clockUtils/compression/algorithm/HuffmanFixed.h"
#include "clockUtils/log/Log.h"

#include <QApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>

#ifdef Q_OS_WIN
	#include <Windows.h>
#endif

using namespace spine;
using namespace spine::utils;

bool CheckOneInstance() {
#ifdef Q_OS_WIN
	HANDLE  startEvent = CreateEventW(nullptr, FALSE, FALSE, L"Global\\CSAPP");

	if (startEvent == nullptr) {
		CloseHandle(startEvent);
		return false;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(startEvent);
		startEvent = nullptr;
		// already exist
		// send message from here to existing copy of the application
		return false;
	}
#endif
	// the only instance, start in a usual way
	return true;
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	if (msg.contains("QFSFileEngine") || msg.contains("QPixmap::scaled")) {
		return;
	}
	const QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtInfoMsg:
		fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		abort();
	default:
		break;
	}
}

int main(int argc, char ** argv) {
	QString urlCommand;
	
	for (int i = 0; i < argc; i++) {
		QString command = argv[i];

		if (command.startsWith("spine://")) {
			urlCommand = command;
			break;
		}
	}
	
	{
		clockUtils::compression::Compression<clockUtils::compression::algorithm::HuffmanFixed> c;
		std::string s;
		clockUtils::ClockError err = c.compress("Foobar", s);
		Q_UNUSED(err);
		err = c.decompress("Foobar", s);
		Q_UNUSED(c);
		Q_UNUSED(err);
	}

	int counter = 5;
	if (!CheckOneInstance()) {
		bool pass = false;
		while (counter--) {
			pass = CheckOneInstance();
			if (pass) {
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		if (!pass && !urlCommand.isEmpty()) {
			client::UrlProtocolHandler urlProtocolHandler;
			urlProtocolHandler.send(urlCommand);
			return 1;
		}
	}

	QDate date(2000, 1, 1);
	qDebug() << date.daysTo(QDate(2017, 9, 24));
	qInstallMessageHandler(myMessageOutput);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication app(argc, argv);
	const int configSucceeded = Config::Init();
	if (configSucceeded != 0) {
		return -1;
	}

	QFileInfoList fil = QDir(Config::BASEDIR + "/logs/").entryInfoList(QDir::Filter::Files, QDir::SortFlag::Time | QDir::SortFlag::Reversed);
	while (fil.size() > 9) {
		QFile(fil[0].absoluteFilePath()).remove();
		fil.pop_front();
	}
	FileLogger logFile(Config::BASEDIR.toStdString() + "/logs/log_" + std::to_string(time(nullptr)) + ".log");
	clockUtils::log::Logger::addSink(&logFile);

	LOGINFO("Start logging");

	int ret;
	{
		widgets::MainWindow wnd(false);

		wnd.show();

		client::UrlProtocolHandler urlProtocolHandler;

		QObject::connect(&urlProtocolHandler, &client::UrlProtocolHandler::start, &wnd, &widgets::MainWindow::startProject);
		QObject::connect(&urlProtocolHandler, &client::UrlProtocolHandler::install, &wnd, &widgets::MainWindow::installProject);
		
		urlProtocolHandler.listen();
		urlProtocolHandler.handle(urlCommand);

		ret = QApplication::exec();
	}
	delete Config::IniParser;
	return ret;
}
