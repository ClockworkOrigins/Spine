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

#include "Database.h"

std::mutex Database::_lock;
std::map<std::string, sqlite3 *> Database::_databases = std::map<std::string, sqlite3 *>();

void Database::execute(const std::string & dbpath, const std::string & query, DBError & error) {
	sqlite3 * db;
	bool selfOpened = false;
	std::lock_guard<std::mutex> lg(_lock); // lock the database vector and sqlite
	if (_databases.find(dbpath) != _databases.end()) {
		db = _databases[dbpath];
	} else {
		const int r = sqlite3_open(dbpath.c_str(), &db);
		if (r != SQLITE_OK) {
			error.error = true;
			error.errMsg = std::string("open(): ") + sqlite3_errmsg(db);
			return;
		}
		selfOpened = true;
	}
	const int r = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (r != SQLITE_OK) {
		error.error = true;
		error.errMsg = std::string("exec(): ") + sqlite3_errmsg(db);
		return;
	}
	error.error = false;

	if (selfOpened) {
		sqlite3_close(db);
	}
}

void Database::open(const std::string & dbpath, DBError & error) {
	sqlite3 * db;
	std::lock_guard<std::mutex> lg(_lock); // lock the database vector and sqlite
	const int r = sqlite3_open(dbpath.c_str(), &db);
	if (r != SQLITE_OK) {
		error.error = true;
		error.errMsg = std::string("open(): ") + sqlite3_errmsg(db);
		return;
	}
	if (_databases.find(dbpath) != _databases.end()) {
		error.error = true;
		error.errMsg = "Database already open!";
		return;
	}
	error.error = false;
	_databases.insert(std::make_pair(dbpath, db));
}

void Database::close(const std::string & dbpath, DBError & error) {
	std::lock_guard<std::mutex> lg(_lock); // lock the database vector and sqlite
	if (_databases.find(dbpath) == _databases.end()) {
		error.error = true;
		error.errMsg = "Database not open!";
		return;
	}
	error.error = false;
	sqlite3_close(_databases[dbpath]);

	_databases.erase(dbpath);
}

template<>
int Database::queryColumn<int>(const int column, sqlite3_stmt * stmt) {
	return sqlite3_column_int(stmt, column);
}

template<>
std::string Database::queryColumn<std::string>(const int column, sqlite3_stmt * stmt) {
	return std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, column)));
}
