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
// Copyright 2019 Clockwork Origins

#include "https/Https.h"

#include "utils/Conversion.h"
#include "utils/WindowsExtensions.h"

#include "clockUtils/log/Log.h"

#include "simple-web-server/client_https.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>

using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

using namespace spine::https;
using namespace spine::utils;

namespace {
	void purgeClients();

	QList<HttpsClient *> activeClientList;
	QList<HttpsClient *> purgeClientList;
	QMutex clientLock;
	bool purgerRunning = true;
	std::thread * purgeThread = new std::thread(&purgeClients);

	void purgeClients() {
		while (purgerRunning) {
			{
				QMutexLocker ml(&clientLock);
				while (!purgeClientList.isEmpty()) {
					delete purgeClientList[0];
					purgeClientList.removeAt(0);
				}
			}
			
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	}
}

void Http::post(const QString & host, uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback) {
	auto * client = new HttpClient(q2s(host) + ":" + std::to_string(port));

	// Synchronous request... maybe make it asynchronous?
	client->request("POST", "/" + q2s(f), data.toStdString(), [callback](std::shared_ptr<HttpClient::Response> response, const SimpleWeb::error_code &) {
		const QString code = s2q(response->status_code).split(" ")[0];

		const int statusCode = code.toInt();

		if (statusCode == 200) {
			const std::string content = response->content.string();
			const QJsonDocument doc(QJsonDocument::fromJson(s2q(content).toUtf8()));
			callback(doc.object(), code.toInt());
		} else {
			callback(QJsonObject(), statusCode);
		}
	});
	client->io_service->run();
}

void Https::post(uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback) {
	auto * client = new HttpsClient("clockwork-origins.com:" + std::to_string(port), false);
	{
		QMutexLocker ml(&clientLock);
		activeClientList << client;
	}

	// Synchronous request... maybe make it asynchronous?
	client->request("POST", "/" + q2s(f), data.toStdString(), [callback, client, f](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &) {
		LOGINFO("Size before " << q2s(f) << ": " << getPRAMValue())
		
		const QString code = s2q(response->status_code).split(" ")[0];

		const int statusCode = code.toInt();

		if (statusCode == 200) {
			const std::string content = response->content.string();
			const QJsonDocument doc(QJsonDocument::fromJson(s2q(content).toUtf8()));
			callback(doc.object(), code.toInt());
			
			LOGINFO("Size after " << q2s(f) << ": " << getPRAMValue())
		} else {
			callback(QJsonObject(), statusCode);
		}

		{
			QMutexLocker ml(&clientLock);
			activeClientList.removeAll(client);
			purgeClientList << client;
		}
	});
	client->io_service->run();
}

QFuture<void> Https::postAsync(uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback) {
	auto future = QtConcurrent::run(post, port, f, data, callback);
	return future;
}

void Https::cancelAll() {
	purgerRunning = false;

	{
		QMutexLocker ml(&clientLock);
		for (const auto * client : activeClientList) {
			client->io_service->stop();
			delete client;
		}
		for (const auto * client : purgeClientList) {
			delete client;
		}
	}

	purgeThread->join();
	delete purgeThread;
}
