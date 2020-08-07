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

#include "simple-web-server/client_https.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>

using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

using namespace spine::https;

void Https::post(uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback) {
	HttpsClient client("clockwork-origins.com:" + std::to_string(port), false);

	// Synchronous request... maybe make it asynchronous?
	client.request("POST", "/" + q2s(f), data.toStdString(), [callback](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &) {
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
	client.io_service->run();
}

QFuture<void> Https::postAsync(uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback) {
	auto future = QtConcurrent::run(post, port, f, data, callback);
	return future;
}
