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
// Copyright 2021 Clockwork Origins

#pragma once

#include <QObject>

class QLocalServer;

namespace spine {
namespace client {

	class UrlProtocolHandler : public QObject {
		Q_OBJECT
	
	public:
        void listen();
        void send(const QString & command);
        void handle(const QString & command);

	signals:
        void start(int projectID);
        void install(int projectID);

	private:
        QLocalServer * _server;
		
        void handleConnection();
	};

} /* namespace client */
} /* namespace spine */
