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

#include "Smtp.h"

#include <iostream>

#include "clockUtils/sockets/TcpSocket.h"

namespace spine {
namespace {
	std::vector<std::string> split(const std::string & str, const std::string & delim) {
		std::vector<std::string> ret;

		size_t n = 0;
		size_t n2 = str.find(delim);

		while (n2 != std::string::npos) {
			std::string s = str.substr(n, n2 - n);
			n = n2 + 1;
			n2 = str.find(delim, n);

			if (!s.empty()) {
				ret.push_back(s);
			}
		}

		if (str.size() - n > 0) {
			ret.push_back(str.substr(n, str.size() - n));
		}

		return ret;
	}
}

	Smtp::Smtp(const std::string & mailServerIp) : _socket(new clockUtils::sockets::TcpSocket()), _connected(false) {
		_connected = _socket->connectToIP(mailServerIp, 25, 2000) == clockUtils::ClockError::SUCCESS;
	}

	Smtp::~Smtp() {
		if (_connected) {
			std::string writeMsg = "QUIT\r\n.\r\n";
			_socket->write(writeMsg.c_str(), writeMsg.length());
			_socket->close();
		}
		delete _socket;
	}

	bool Smtp::sendMail(const std::string & from, const std::string & to, const std::string & subject, const std::string & body, const std::string & replyTo) const {
		if (!_connected) {
			return false;
		}
		// the SMTP looks like the following
		// > connect to mailServerIP by Client
		// < 220 service ready									|| server is ready to get mail
		// > HELO <name>										|| start mail
		// < 250 ok												|| server answers
		// > MAIL FROM: <from>									|| client informs about sender
		// < 250 ok
		// > RCPT TO: <to>										|| client informs about receiver
		// < 250 ok
		// > DATA												|| client informs about next message will contain the data
		// < 354 start mail input								|| server informs client he's ready to receive mail data
		// > From: <sender@example.org>
		//   To: <receiver@example.com>
		//   Subject : Testmail
		//	 Date : Thu, 26 Oct 2006 13 : 10 : 50 + 0200
		//														|| this blank line between header and body is important
		//	 Lorem ipsum dolor sit amet, consectetur
		//   adipisici elit, sed eiusmod tempor incidunt
		//   ut labore et dolore magna aliqua.
		//	 .													|| . signals end of body
		// < 250 ok												|| server takes control over mail and new one can be send
		clockUtils::ClockError error;
		std::string buffer;
		enum class State {
			Init,
			Mail,
			Rcpt,
			Data,
			Body,
			Quit
		};
		State state = State::Init;
		bool ret = true;
		do {
			error = _socket->read(buffer);
			if (error != clockUtils::ClockError::SUCCESS) {
				std::cout << "Error reading message" << std::endl;
				ret = false;
				break;
			}
			buffer.resize(3);
			if (state == State::Init && (buffer == "220" || buffer == "250")) { // 220 if started first, 250 if last mail was sent successfully
				// handshake
				std::string writeMsg = "HELO there\r\n";
				error = _socket->write(writeMsg.c_str(), writeMsg.length());
				state = State::Mail;
			} else if (state == State::Mail && buffer == "250") {
				// inform server about sender
				std::string writeMsg = "MAIL FROM: " + from + "\r\n";
				error = _socket->write(writeMsg.c_str(), writeMsg.length());
				state = State::Rcpt;
			} else if (state == State::Rcpt && buffer == "250") {
				// inform server about receiver
				std::string writeMsg = "RCPT TO: " + to + "\r\n";
				error = _socket->write(writeMsg.c_str(), writeMsg.length());
				state = State::Data;
			} else if (state == State::Data && buffer == "250") {
				// inform server you want to send data
				std::string writeMsg = "DATA\r\n";
				error = _socket->write(writeMsg.c_str(), writeMsg.length());
				state = State::Body;
			} else if (state == State::Body && buffer == "354") {
				// give server data
				std::vector<std::string> splitBody = split(body, "\n");
				std::string newBody;
				for (const std::string & s : splitBody) {
					newBody += s + "\r\n";
				}
				std::string writeMsg = "To: " + to + "\r\nFrom: " + from + "\r\nSubject: " + subject + "\r\nReply-To:" + replyTo + "\r\n\r\n" + newBody + "\r\n.\r\n";
				error = _socket->write(writeMsg.c_str(), writeMsg.length());
				state = State::Quit;
			} else {
				std::cout << "Received unexpected mail from server: " << buffer << std::endl;
				state = State::Quit;
				ret = false;
			}
		} while (error == clockUtils::ClockError::SUCCESS && !buffer.empty() && state != State::Quit);

		return ret;
	}

} /* namespace spine */
