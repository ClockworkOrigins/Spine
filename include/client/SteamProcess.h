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
// Copyright 2020 Clockwork Origins

#pragma once

#include <cstdint>

#include <QObject>
#include <QProcess>
#include <QString>

#ifdef Q_OS_WIN

namespace spine {
namespace client {

	class SteamProcess : public QObject {
		Q_OBJECT
		
	public:
		SteamProcess(int32_t steamID, QString executable, QStringList arguments);

		void start(int timeoutInSecs);

	signals:
		void finished(int, QProcess::ExitStatus);

	private:
		int32_t _steamID;
		QString _executable;
		QStringList _arguments;

		void checkIfProcessRunning(int timeoutInSecs);
	};

} /* namespace client */
} /* namespace spine */

#endif
