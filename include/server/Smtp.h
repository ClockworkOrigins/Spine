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

#include <string>

namespace clockUtils {
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {

	class Smtp {
	public:
		/**
		 * \brief connects socket to this mail sever
		 */
		explicit Smtp(const std::string & mailServerIp);
		~Smtp();

		/**
		 * \brief sends mail, don't call from multiple threads, this is NOT threadsafe
		 * \return returns true on success, otherwise false
		 */
		bool sendMail(const std::string & from, const std::string & to, const std::string & subject, const std::string & body, const std::string & replyTo) const;

	private:
		clockUtils::sockets::TcpSocket * _socket;
		bool _connected;
	};

} /* namespace spine */
