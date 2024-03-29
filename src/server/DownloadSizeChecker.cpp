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
#include <thread>

#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

#include "boost/filesystem.hpp"

using namespace spine::server;
std::map<std::tuple<int32_t, std::string, uint32_t>, uint64_t> DownloadSizeChecker::_cache;
std::map<std::tuple<int32_t, std::string, uint32_t>, uint64_t> DownloadSizeChecker::_packageCache;
std::map<int32_t, uint64_t> DownloadSizeChecker::_fileSizes;
std::map<int32_t, uint64_t> DownloadSizeChecker::_packageFileSizes;
std::map<std::pair<int32_t, std::string>, int32_t> DownloadSizeChecker::_fileToFileIDs;
std::map<std::pair<int32_t, std::string>, int32_t> DownloadSizeChecker::_fileToPackageIDs;
std::recursive_mutex DownloadSizeChecker::_lock;

uint64_t DownloadSizeChecker::getBytes(int32_t modID, const std::string & language, uint32_t version) {
	std::lock_guard<std::recursive_mutex> lg(_lock);
	const auto it = _cache.find(std::make_tuple(modID, language, version));
	if (it != _cache.end()) {
		return it->second;
	}
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("PREPARE selectStmt FROM \"SELECT FileID, Path FROM modfiles WHERE ModID = ? AND (Language = ? OR Language = 'All')\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("PREPARE insertStmt FROM \"INSERT INTO fileSizes (FileID, CompressedSize, UncompressedSize) VALUES (?, ?, 0) ON DUPLICATE KEY UPDATE CompressedSize = ?\";")) {
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
		const auto fileID = std::stoi(vec[0]);
		const auto path = vec[1];

		auto it2 = _fileSizes.find(fileID);

		if (it2 != _fileSizes.end()) {
			bytes += it2->second;
			continue;
		}

		std::ifstream in("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + path, std::ifstream::ate | std::ifstream::binary);
		if (!in.good()) {
			std::cout << "Couldn't open file: " << "/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + path << std::endl;
			break;
		}
		const auto size = in.tellg();
		bytes += size;

		_fileSizes.insert(std::make_pair(fileID, size));

		if (!database.query("SET @paramFileID=" + std::to_string(fileID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			continue;
		}
		if (!database.query("SET @paramSize=" + std::to_string(size) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			continue;
		}
		if (!database.query("EXECUTE insertStmt USING @paramFileID, @paramSize, @paramSize;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			continue;
		}
	}
	_cache.insert(std::make_pair(std::make_tuple(modID, language, version), bytes));
	return bytes;
}

uint64_t DownloadSizeChecker::getBytesForPackage(int32_t modID, int32_t optionalID, const std::string & language, uint32_t version) {
	std::lock_guard<std::recursive_mutex> lg(_lock);
	const auto it = _packageCache.find(std::make_tuple(optionalID, language, version));
	if (it != _packageCache.end()) {
		return it->second;
	}
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("PREPARE selectStmt FROM \"SELECT FileID, Path FROM optionalpackagefiles WHERE PackageID = ? AND (Language = ? OR Language = 'All')\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return 0;
	}
	if (!database.query("PREPARE insertStmt FROM \"INSERT INTO packageFileSizes (FileID, CompressedSize, UncompressedSize) VALUES (?, ?, 0) ON DUPLICATE KEY UPDATE CompressedSize = ?\";")) {
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
		const auto fileID = std::stoi(vec[0]);
		const auto path = vec[1];

		auto it2 = _packageFileSizes.find(fileID);

		if (it2 != _packageFileSizes.end()) {
			bytes += it2->second;
			continue;
		}

		std::ifstream in("/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + path, std::ifstream::ate | std::ifstream::binary);
		if (!in.good()) {
			std::cout << "Couldn't open file: " << "/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/" + std::to_string(modID) + "/" + path << std::endl;
			break;
		}
		const auto size = in.tellg();
		bytes += size;

		_packageFileSizes.insert(std::make_pair(fileID, size));

		if (!database.query("SET @paramFileID=" + std::to_string(fileID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			continue;
		}
		if (!database.query("SET @paramSize=" + std::to_string(size) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			continue;
		}
		if (!database.query("EXECUTE insertStmt USING @paramFileID, @paramSize, @paramSize;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
			continue;
		}
	}
	_packageCache.insert(std::make_pair(std::make_tuple(optionalID, language, version), bytes));
	return bytes;
}

uint64_t DownloadSizeChecker::getSizeForFile(int32_t projectID, const std::string& path) {
	std::lock_guard<std::recursive_mutex> lg(_lock);
	const auto p = std::make_pair(projectID, path);

	const auto it = _fileToFileIDs.find(p);

	// this case should never happen
	if (it == _fileToFileIDs.end()) return 0;

	const auto fileID = it->second;

	const auto it2 = _fileSizes.find(fileID);

	// this could happen for the first call before database got cached a first time
	if (it2 == _fileSizes.end()) return 0;

	return it2->second;
}

uint64_t DownloadSizeChecker::getSizeForPackageFile(int32_t packageID, const std::string& path) {
	std::lock_guard<std::recursive_mutex> lg(_lock);
	const auto p = std::make_pair(packageID, path);

	const auto it = _fileToPackageIDs.find(p);

	// this case should never happen
	if (it == _fileToPackageIDs.end()) return 0;

	const auto fileID = it->second;

	const auto it2 = _packageFileSizes.find(fileID);

	// this could happen for the first call before database got cached a first time
	if (it2 == _packageFileSizes.end()) return 0;

	return it2->second;
}

void DownloadSizeChecker::init() {
	std::lock_guard<std::recursive_mutex> lg(_lock);
	MariaDBWrapper database;
	if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		std::cout << "Couldn't connect to database: " << __LINE__ << " " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("PREPARE selectStmt FROM \"SELECT FileID, CompressedSize FROM fileSizes\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("PREPARE selectPackagesStmt FROM \"SELECT FileID, CompressedSize FROM packageFileSizes\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("PREPARE selectPackageFileIDsStmt FROM \"SELECT PackageID, FileID, Path FROM optionalpackagefiles\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("PREPARE selectFileIDsStmt FROM \"SELECT ModID, FileID, Path FROM modfiles\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	if (!database.query("EXECUTE selectStmt;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	auto lastResults = database.getResults<std::vector<std::string>>();

	for (const auto & vec : lastResults) {
		const auto fileID = std::stoi(vec[0]);
		const auto size = std::stoull(vec[1]);
		_fileSizes.insert(std::make_pair(fileID, size));
	}

	if (!database.query("EXECUTE selectPackagesStmt;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	lastResults = database.getResults<std::vector<std::string>>();

	for (const auto & vec : lastResults) {
		const auto fileID = std::stoi(vec[0]);
		const auto size = std::stoull(vec[1]);
		_packageFileSizes.insert(std::make_pair(fileID, size));
	}

	if (!database.query("EXECUTE selectFileIDsStmt;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	lastResults = database.getResults<std::vector<std::string>>();

	for (const auto & vec : lastResults) {
		const auto projectID = std::stoi(vec[0]);
		const auto fileID = std::stoi(vec[1]);
		const auto path = vec[2];

		_fileToFileIDs.insert(std::make_pair(std::make_pair(projectID, path), fileID));
	}

	if (!database.query("EXECUTE selectPackageFileIDsStmt;")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << database.getLastError() << std::endl;
		return;
	}
	lastResults = database.getResults<std::vector<std::string>>();

	for (const auto & vec : lastResults) {
		const auto projectID = std::stoi(vec[0]);
		const auto packageFileID = std::stoi(vec[1]);
		const auto path = vec[2];

		_fileToPackageIDs.insert(std::make_pair(std::make_pair(projectID, path), packageFileID));
	}
}

void DownloadSizeChecker::clear() {
	std::lock_guard<std::recursive_mutex> lg(_lock);
	_cache.clear();
	_packageCache.clear();
}

void DownloadSizeChecker::refresh() {
	std::thread([] {
		std::lock_guard<std::recursive_mutex> lg(_lock);

		clear();

		_fileSizes.clear();
		_packageFileSizes.clear();
		_fileToFileIDs.clear();
		_fileToPackageIDs.clear();

		init();
	}).detach();
}
