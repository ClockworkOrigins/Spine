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
// Copyright 2019 Clockwork Origins

#pragma once

#include "simple-web-server/server_https.hpp"

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

namespace spine {
namespace server {

	class ManagementServer {
	public:
		ManagementServer();
		~ManagementServer();

		int run();
		void stop();

	private:
		HttpsServer * _server;
		std::thread * _runner;
	};

} /* namespace server */
} /* namespace spine */
