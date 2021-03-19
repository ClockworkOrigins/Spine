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
// Copyright 2020 Clockwork Origins

#pragma once

#include <memory>

#include <QFuture>

namespace discord {
	class Core;
	class User;
}

namespace std {
	class thread;
}

namespace spine {
namespace discord {

	class DiscordManager : public QObject {
		Q_OBJECT
		
	public:
		static DiscordManager * instance();

		void updatePresence(const QString & details, const QString & state);

		bool isConnected() const;

		int64_t getUserID() const;

		void stop();

	signals:
		void connected();

	private:
		std::shared_ptr<::discord::Core> _core;
		::discord::User * _currentUser;
		bool _running;
		std::thread * _thread;
		
		DiscordManager();
		~DiscordManager();
	};

} /* namespace client */
} /* namespace spine */
