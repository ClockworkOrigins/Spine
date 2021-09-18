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

#include "MariaDBWrapper.h"

using namespace spine::server;

MariaDBWrapper::MariaDBWrapper() : _database(mysql_init(nullptr)) {
}

bool MariaDBWrapper::connect(const std::string & host, const std::string & user, const std::string & password, const std::string & database, uint16_t port) {
	_database = mysql_real_connect(_database, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, "/var/lib/mysql/mysql.sock", 0);
	return _database != nullptr;
}

bool MariaDBWrapper::query(const std::string & q) const {
	return _database && mysql_real_query(_database, q.c_str(), static_cast<unsigned long>(q.size())) == 0;
}

std::string MariaDBWrapper::getLastError() const {
	return _database ? mysql_error(_database) : "";
}

void MariaDBWrapper::close() {
	if (_database) {
		mysql_close(_database);
		_database = nullptr;
	}
}
