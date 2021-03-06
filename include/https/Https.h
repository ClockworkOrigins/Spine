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

#pragma once

#include <cstdint>
#include <functional>

#include <QFuture>

class QJsonObject;
class QString;

namespace spine {
namespace https {

	class Http {
	public:
		static void post(const QString & host, uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback);
	};

	class Https {
	public:
		static void post(uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback);
		static QFuture<void> postAsync(uint16_t port, const QString & f, const QString & data, const std::function<void(const QJsonObject &, int statusCode)> & callback);
		
        static void cancelAll();
	};

} /* namespace https */
} /* namespace spine */

