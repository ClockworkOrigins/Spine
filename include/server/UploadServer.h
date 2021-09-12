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

#pragma once

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {
namespace server {

	class UploadServer {
	public:
		UploadServer();
		~UploadServer();

		int run();

	private:
		clockUtils::sockets::TcpSocket * _listenUploadServer;

		void handleUploadFiles(clockUtils::sockets::TcpSocket * sock) const;
	};

} /* namespace server */
} /* namespace spine */
