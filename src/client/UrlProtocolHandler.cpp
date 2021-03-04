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

#include "UrlProtocolHandler.h"

#include <QLocalSocket>
#include <QLocalServer>

using namespace spine::client;

void UrlProtocolHandler::listen() {
    _server = new QLocalServer(this);

    connect(_server, &QLocalServer::newConnection, this, &UrlProtocolHandler::handleConnection);
	
    _server->listen("Spine");
}

void UrlProtocolHandler::handle(const QString & command) {
    auto cmd = command;

    if (cmd.isEmpty()) return;

    if (!cmd.startsWith("spine://")) return;
	
    cmd = cmd.remove("spine://");

    auto list = cmd.split("/");

    if (list.isEmpty()) return;

    cmd = list[0];
    list.removeAt(0);

	if (cmd == "start" && !list.isEmpty()) {
        const int projectID = list[0].toInt();
        emit start(projectID);
	} else if (cmd == "install" && !list.isEmpty()) {
        const int projectID = list[0].toInt();
        emit install(projectID);
    }
}

void UrlProtocolHandler::send(const QString & command) {
    QLocalSocket sock;
    sock.connectToServer("Spine", QIODevice::WriteOnly);
    sock.waitForConnected();
    sock.write(command.toUtf8());
    sock.waitForDisconnected();
    sock.close();
}

void UrlProtocolHandler::handleConnection() {
    Q_ASSERT(_server->hasPendingConnections());

    if (!_server->hasPendingConnections()) return;

    auto * connection = _server->nextPendingConnection();
    connection->waitForReadyRead();
    const auto data = connection->readAll();

    const auto command = QString::fromUtf8(data);

    handle(command);

    connection->close();
}
