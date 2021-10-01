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

#include "DownloadSizeChecker.h"

#include <iostream>

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

#include "boost/filesystem.hpp"

using namespace spine::server;

uint64_t DownloadSizeChecker::getBytes(int32_t modID, const std::string & language, uint32_t version) {
	std::lock_guard<std::mutex> lg(_lock);
	const auto it = _cache.find(std::make_tuple(modID, language, version));
	if (it != _cache.end()) {
		return it->second;
	}
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("PREPARE selectStmt FROM \"SELECT Path FROM modfiles WHERE ModID = ? AND (Language = ? OR Language = 'All')\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("SET @paramModID=" + std::to_string(modID) + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("SET @paramLanguage='" + language + "';")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("EXECUTE selectStmt USING @paramModID, @paramLanguage;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	const auto lastResults = database.getResults<std::vector<std::string>>();
	uint64_t bytes = 0;
	for (const auto & vec : lastResults) {
		std::ifstream in("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + vec[0], std::ifstream::ate | std::ifstream::binary);
		if (!in.good()) {
			std::cout << "Couldn't open file: " << "/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + vec[0] << std::endl;
			break;
		}
		const auto size = in.tellg();
		bytes += size;
	}
	_cache.insert(std::make_pair(std::make_tuple(modID, language, version), bytes));
	return bytes;
}

uint64_t DownloadSizeChecker::getBytesForPackage(int32_t modID, int32_t optionalID, const std::string & language, uint32_t version) {
	std::lock_guard<std::mutex> lg(_lock);
	const auto it = _packageCache.find(std::make_tuple(optionalID, language, version));
	if (it != _packageCache.end()) {
		return it->second;
	}
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("PREPARE selectStmt FROM \"SELECT Path FROM optionalpackagefiles WHERE PackageID = ? AND (Language = ? OR Language = 'All')\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("SET @paramPackageID=" + std::to_string(optionalID) + ";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("SET @paramLanguage='" + language + "';")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("EXECUTE selectStmt USING @paramPackageID, @paramLanguage;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	const auto lastResults = database.getResults<std::vector<std::string>>();
	uint64_t bytes = 0;
	for (const auto & vec : lastResults) {
		std::ifstream in("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + vec[0], std::ifstream::ate | std::ifstream::binary);
		if (!in.good()) {
			std::cout << "Couldn't open file: " << "/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + vec[0] << std::endl;
			break;
		}
		const auto size = in.tellg();
		bytes += size;
	}
	_packageCache.insert(std::make_pair(std::make_tuple(optionalID, language, version), bytes));
	return bytes;
}

void DownloadSizeChecker::clear() {
	std::lock_guard<std::mutex> lg(_lock);
	_cache.clear();
	_packageCache.clear();
}
