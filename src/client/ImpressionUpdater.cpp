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

#include "ImpressionUpdater.h"

#include <thread>

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QDate>
#include <QtConcurrentRun>

namespace spine {

	QSet<int> ImpressionUpdater::ids;
	QMutex ImpressionUpdater::lock;
	static int id = 0;

	void ImpressionUpdater::update() {
		return;
		QtConcurrent::run([]() {
			int currentID = id++;
			{
				QMutexLocker lg(&lock);
				ids.clear();
				ids.insert(currentID);
			}
			std::this_thread::sleep_for(std::chrono::seconds(30));
			{
				QMutexLocker lg(&lock);
				if (!ids.contains(currentID)) { // if impression was removed because view was changed before 30 seconds timer
					return;
				}
			}
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 5000)) {
				common::UpdateImpressionMessage uim;

				uim.year = QDate::currentDate().year();
				uim.month = QDate::currentDate().month();

				std::string serialized = uim.SerializePublic();
				sock.writePacket(serialized);
			}
		});
	}

	void ImpressionUpdater::cancel() {
		QMutexLocker lg(&lock);
		ids.clear();
	}

} /* namespace spine */
