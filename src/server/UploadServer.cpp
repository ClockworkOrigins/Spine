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

#include "UploadServer.h"

#include <fstream>

#include "FileSynchronizer.h"
#include "MariaDBWrapper.h"
#include "SpineServerConfig.h"

#include "common/MessageStructs.h"

#include "boost/filesystem.hpp"

#include "clockUtils/sockets/TcpSocket.h"

#ifdef TEST_CONFIG
	const std::string PATH_PREFIX = "./downloads/mods/";
#else
	const std::string PATH_PREFIX = "/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods/";
#endif
	
using namespace spine::server;

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

UploadServer::UploadServer() : _listenUploadServer(new clockUtils::sockets::TcpSocket()) {
#ifdef TEST_CONFIG
	if (!boost::filesystem::exists(PATH_PREFIX)) {
		boost::filesystem::create_directories(PATH_PREFIX);
	}
#endif
}

UploadServer::~UploadServer() {
	delete _listenUploadServer;
}

int UploadServer::run() {
	if (_listenUploadServer->listen(UPLOADSERVER_PORT, 10, true, std::bind(&UploadServer::handleUploadFiles, this, std::placeholders::_1)) != clockUtils::ClockError::SUCCESS) {
		return 1;
	}

	return 0;
}

void UploadServer::handleUploadFiles(clockUtils::sockets::TcpSocket * sock) const {
	bool error = false;
	MariaDBWrapper spineDatabase;
	if (!spineDatabase.connect("localhost", DATABASEUSER, DATABASEPASSWORD, SPINEDATABASE, 0)) {
		delete sock;
		error = true;
	}

	if (!spineDatabase.query("PREPARE deleteFileStmt FROM \"DELETE FROM modfiles WHERE ModID = ? AND Path = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE selectFileStmt FROM \"SELECT FileID FROM modfiles WHERE ModID = ? AND Path = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE updateFileStmt FROM \"UPDATE modfiles SET Hash = ?, Language = ? WHERE ModID = ? AND Path = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE insertFileStmt FROM \"INSERT INTO modfiles (ModID, Path, Language, Hash) VALUES (?, ?, ?, ?)\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE updateLanguageStmt FROM \"UPDATE modfiles SET Language = ? WHERE ModID = ? AND Path = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE selectVersionStmt FROM \"SELECT MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods WHERE ModID = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE selectFileIDStmt FROM \"SELECT FileID FROM modfiles WHERE ModID = ? AND Path = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE insertSizeStmt FROM \"INSERT INTO fileSizes (FileID, CompressedSize, UncompressedSize) VALUES (?, ?, 0) ON DUPLICATE KEY UPDATE CompressedSize = ?\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}
	if (!spineDatabase.query("PREPARE deleteSizeStmt FROM \"DELETE FROM fileSizes WHERE FileID = ? LIMIT 1\";")) {
		std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
		delete sock;
		error = true;
	}

	std::string newBuffer;
	std::string unreadBuffer;
	enum States {
		MetadataSize,
		Metadata,
		File
	};
	int64_t currentSize = 0;
	int currentIndex = 0;
	int state = MetadataSize;
	int versionMajor = 0;
	int versionMinor = 0;
	int versionPatch = 0;
	int versionSpine = 0;
	common::UploadModfilesMessage * umm = nullptr;
	std::ofstream fs;
	while (sock->read(newBuffer) == clockUtils::ClockError::SUCCESS && !error) {
		unreadBuffer.append(newBuffer);
		if (state == MetadataSize) {
			if (unreadBuffer.size() >= 4) {
				memcpy(&currentSize, unreadBuffer.c_str(), 4);
				unreadBuffer = unreadBuffer.substr(4);
				state = Metadata;
			}
		}
		if (state == Metadata) {
			if (unreadBuffer.size() >= static_cast<size_t>(currentSize)) {
				const std::string serialized = unreadBuffer.substr(0, currentSize);
				unreadBuffer = unreadBuffer.substr(currentSize);
				common::Message * msg = common::Message::DeserializeBlank(serialized);
				umm = dynamic_cast<common::UploadModfilesMessage *>(msg);
				std::cout << "Updating " << umm->files.size() << std::endl;
				state = File;
				currentIndex = 0;
				if (!spineDatabase.query("SET @paramModID=" + std::to_string(umm->modID) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
					error = true;
					break;
				}
				if (!spineDatabase.query("EXECUTE selectVersionStmt USING @paramModID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
					error = true;
					break;
				}
				auto lastResults = spineDatabase.getResults<std::vector<std::string>>();
				if (lastResults.empty()) {
					error = true;
					break;
				}
				versionMajor = std::stoi(lastResults[0][0]);
				versionMinor = std::stoi(lastResults[0][1]);
				versionPatch = std::stoi(lastResults[0][2]);
				versionSpine = std::stoi(lastResults[0][3]);
				
				do {
					if (currentIndex == static_cast<int>(umm->files.size())) {
						break;
					}
					const common::ModFile mf = umm->files[currentIndex];
					if (mf.deleted) {
						if (boost::filesystem::exists(PATH_PREFIX + std::to_string(umm->modID) + "/" + mf.filename) && !boost::filesystem::remove(PATH_PREFIX + std::to_string(umm->modID) + "/" + mf.filename)) {
							error = true;
							break;
						}
						if (!spineDatabase.query("SET @paramPath='" + mf.filename + "';")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						if (!spineDatabase.query("EXECUTE selectFileIDStmt USING @paramModID, @paramPath;")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						lastResults = spineDatabase.getResults<std::vector<std::string>>();
						if (lastResults.empty()) {
							error = true;
							break;
						}
						if (!spineDatabase.query("SET @paramFileID=" + lastResults[0][0] + ";")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}

						if (!spineDatabase.query("EXECUTE deleteFileStmt USING @paramModID, @paramPath;")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						if (!spineDatabase.query("EXECUTE deleteSizeStmt USING @paramFileID;")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						currentIndex++;

						FileSynchronizer::AddJob job;
						job.operation = FileSynchronizer::Operation::Delete;
						job.majorVersion = versionMajor;
						job.minorVersion = versionMinor;
						job.patchVersion = versionPatch;
						job.spineVersion = versionSpine;
						job.projectID = umm->modID;
						job.path = mf.filename;

						FileSynchronizer::addJob(job);
					} else if (mf.changed && mf.size == 0) {
						if (!spineDatabase.query("SET @paramLanguage='" + mf.language + "';")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						if (!spineDatabase.query("EXECUTE updateLanguageStmt USING @paramLanguage, @paramModID, @paramPath;")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						currentIndex++;
					} else {
						// now we have to receive a new file, so set currentSize and go on
						currentSize = mf.size;
						std::cout << "Receiving File '" << mf.filename << "' with size " << mf.size << std::endl;
						std::vector<std::string> path = split(mf.filename, "/");
						std::string currentPath = PATH_PREFIX + std::to_string(umm->modID) + "/";

						for (size_t i = 0; i < path.size() - 1; i++) {
							currentPath += path[i] + "/";
							if (!boost::filesystem::exists(currentPath) && !boost::filesystem::create_directories(currentPath)) {
								error = true;
								break;
							}
						}
						fs.open(currentPath + path.back(), std::ios::out | std::ios::binary);
						break;
					}
				} while (true);
			}
		}
		if (state == File) {
			while (!unreadBuffer.empty()) {
				if (currentSize > 0) {
					const size_t diff = std::min(unreadBuffer.size(), static_cast<size_t>(currentSize));
					fs.write(unreadBuffer.c_str(), diff);
					currentSize -= diff;
					unreadBuffer = unreadBuffer.substr(diff);
				}

				common::ModFile mf = umm->files[currentIndex];

				if (currentSize == 0) {
					fs.close();

					if (!spineDatabase.query("SET @paramPath='" + mf.filename + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}
					if (!spineDatabase.query("SET @paramHash='" + mf.hash + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}
					if (!spineDatabase.query("SET @paramLanguage='" + mf.language + "';")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}
					if (mf.changed) {
						if (!spineDatabase.query("EXECUTE selectFileStmt USING @paramModID, @paramPath;")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}
						auto lastResults = spineDatabase.getResults<std::vector<std::string>>();
						if (lastResults.empty()) {
							if (!spineDatabase.query("EXECUTE insertFileStmt USING @paramModID, @paramPath, @paramLanguage, @paramHash;")) {
								std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
								error = true;
								break;
							}

							FileSynchronizer::AddJob job;
							job.operation = FileSynchronizer::Operation::Add;
							job.majorVersion = versionMajor;
							job.minorVersion = versionMinor;
							job.patchVersion = versionPatch;
							job.spineVersion = versionSpine;
							job.projectID = umm->modID;
							job.path = mf.filename;

							FileSynchronizer::addJob(job);
						} else {
							if (!spineDatabase.query("EXECUTE updateFileStmt USING @paramHash, @paramLanguage, @paramModID, @paramPath;")) {
								std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
								error = true;
								break;
							}

							FileSynchronizer::AddJob job;
							job.operation = FileSynchronizer::Operation::Update;
							job.majorVersion = versionMajor;
							job.minorVersion = versionMinor;
							job.patchVersion = versionPatch;
							job.spineVersion = versionSpine;
							job.projectID = umm->modID;
							job.path = mf.filename;

							FileSynchronizer::addJob(job);
						}
					} else if (!mf.changed) {
						if (!spineDatabase.query("EXECUTE insertFileStmt USING @paramModID, @paramPath, @paramLanguage, @paramHash;")) {
							std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
							error = true;
							break;
						}

						FileSynchronizer::AddJob job;
						job.operation = FileSynchronizer::Operation::Add;
						job.majorVersion = versionMajor;
						job.minorVersion = versionMinor;
						job.patchVersion = versionPatch;
						job.spineVersion = versionSpine;
						job.projectID = umm->modID;
						job.path = mf.filename;

						FileSynchronizer::addJob(job);
					}
					if (!spineDatabase.query("EXECUTE selectFileIDStmt USING @paramModID, @paramPath;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}
					auto lastResults = spineDatabase.getResults<std::vector<std::string>>();
					if (lastResults.empty()) {
						error = true;
						break;
					}
					if (!spineDatabase.query("SET @paramFileID=" + lastResults[0][0] + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}
					if (!spineDatabase.query("SET @paramSize=" + std::to_string(umm->files[currentIndex].size) + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}
					if (!spineDatabase.query("EXECUTE insertSizeStmt USING @paramFileID, @paramSize, @paramSize;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << ": " << spineDatabase.getLastError() << std::endl;
						error = true;
						break;
					}

					currentIndex++;

					if (umm && currentIndex == static_cast<int>(umm->files.size())) {
						break; // finished uploads
					}

					mf = umm->files[currentIndex];
					currentSize = mf.size;
					std::cout << "Receiving File '" << mf.filename << "' with size " << mf.size << std::endl;
					std::vector<std::string> path = split(mf.filename, "/");
					std::string currentPath = PATH_PREFIX + std::to_string(umm->modID) + "/";
					for (size_t i = 0; i < path.size() - 1; i++) {
						currentPath += path[i] + "/";
						if (!boost::filesystem::exists(currentPath) && !boost::filesystem::create_directories(currentPath)) {
							error = true;
							break;
						}
					}
					fs.open(currentPath + path.back(), std::ios::out | std::ios::binary);
				}
			}
		}
		
		if (umm && currentIndex == static_cast<int>(umm->files.size())) break; // finished uploads
	}
	std::cout << "Finished Upload: " << !error << std::endl;
	delete umm;
	common::AckMessage am;
	am.success = !error;
	const std::string serialized = am.SerializeBlank();
	sock->writePacket(serialized);
	delete sock;
}
