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
 // Copyright 2021 Clockwork Origins

#include "FileSynchronizer.h"

#include <iostream>
#include <map>
#include <set>
#include <thread>

#include "ServerCommon.h"
#include "MariaDBWrapper.h"

namespace {
#ifdef TEST_CONFIG
	const std::string PATH_PREFIX = "./downloads/mods";
#else
	const std::string PATH_PREFIX = "/var/www/vhosts/clockwork-origins.de/httpdocs/Gothic/downloads/mods";
#endif
}

using namespace spine;

std::mutex FileSynchronizer::lock;

void FileSynchronizer::init() {
	std::thread(&FileSynchronizer::exec).detach();
}

void FileSynchronizer::addJob(const AddJob & job) {
	std::lock_guard<std::mutex> lg(lock);
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectStmt FROM \"SELECT ServerID FROM fileserverList\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		auto results = database.getResults<std::vector<std::string>>();

		for (const auto & vec : results) {
			const auto serverID = std::stoi(vec[0]);

			AddForServerJob serverJob(job);
			serverJob.serverID = serverID;
			
			addJob(serverJob);
		}
	} while (false);
}

void FileSynchronizer::exec() {
	while (true) {
		// 1. add jobs for all missing files on servers
		addMissing();

		// 2. while a job is available, execute it
		while (true) {
			lock.lock();
			const auto job = getFirstJobInQueue();
			lock.unlock();

			if (!job.valid) break;

			executeJob(job);
		}

		std::this_thread::sleep_for(std::chrono::hours(1));
	}
}

void FileSynchronizer::addMissing() {
	std::lock_guard<std::mutex> lg(lock);
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectServersStmt FROM \"SELECT ServerID FROM fileserverList\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectFileServerStmt FROM \"SELECT ServerID, ProjectID FROM projectsOnFileservers\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectJobsStmt FROM \"SELECT ServerID, ProjectID FROM fileserverSynchronizationQueue\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectProjectsStmt FROM \"SELECT ModID, MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectProjectFilesStmt FROM \"SELECT Path FROM modfiles WHERE ModID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectPackagesStmt FROM \"SELECT PackageID FROM optionalpackages WHERE ModID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectPackageFilesStmt FROM \"SELECT Path FROM optionalpackagefiles WHERE PackageID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectServersStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();

		std::vector<int> servers;

		servers.reserve(results.size());
		
		for (const auto & vec : results) {
			servers.push_back(std::stoi(vec[0]));
		}

		struct ProjectConfig {
			int majorVersion;
			int minorVersion;
			int patchVersion;
			int spineVersion;
		};

		std::map<int, ProjectConfig> projects;

		if (!database.query("EXECUTE selectProjectsStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();

		for (const auto & vec : results) {
			const auto projectID = std::stoi(vec[0]);
			const auto majorVersion = std::stoi(vec[1]);
			const auto minorVersion = std::stoi(vec[2]);
			const auto patchVersion = std::stoi(vec[3]);
			const auto spineVersion = std::stoi(vec[4]);

			ProjectConfig cfg {};
			cfg.majorVersion = majorVersion;
			cfg.minorVersion = minorVersion;
			cfg.patchVersion = patchVersion;
			cfg.spineVersion = spineVersion;

			projects[projectID] = cfg;
		}

		std::map<int, std::set<int>> projectsOnServer;

		if (!database.query("EXECUTE selectFileServerStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();

		for (const auto & vec : results) {
			const auto serverID = std::stoi(vec[0]);
			const auto projectID = std::stoi(vec[1]);

			projectsOnServer[serverID].insert(projectID);
		}

		if (!database.query("EXECUTE selectJobsStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();

		for (const auto & vec : results) {
			const auto serverID = std::stoi(vec[0]);
			const auto projectID = std::stoi(vec[1]);

			projectsOnServer[serverID].insert(projectID);
		}

		int counter = 0;

		for (const auto serverID : servers) {
			for (auto & project : projects) {
				const auto & set = projectsOnServer[serverID];
				const auto it2 = set.find(project.first);

				if (it2 != set.end()) continue;
				
				if (!database.query("SET @paramProjectID=" + std::to_string(project.first) + ";")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}

				if (!database.query("EXECUTE selectProjectFilesStmt USING @paramProjectID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				for (const auto & vec : results) {
					AddForServerJob job = {};
					job.serverID = serverID;
					job.projectID = project.first;
					job.majorVersion = project.second.majorVersion;
					job.minorVersion = project.second.minorVersion;
					job.patchVersion = project.second.patchVersion;
					job.spineVersion = project.second.spineVersion;
					job.operation = Operation::Add;
					job.path = vec[0];

					addJob(job);

					counter++;
				}

				if (!database.query("EXECUTE selectPackagesStmt USING @paramProjectID;")) {
					std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
					break;
				}
				results = database.getResults<std::vector<std::string>>();

				for (const auto & vec : results) {
					if (!database.query("SET @paramPackageID=" + vec[0] + ";")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectPackageFilesStmt USING @paramPackageID;")) {
						std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
						break;
					}
					auto results2 = database.getResults<std::vector<std::string>>();

					for (const auto & vec2 : results2) {
						AddForServerJob job = {};
						job.serverID = serverID;
						job.projectID = project.first;
						job.majorVersion = project.second.majorVersion;
						job.minorVersion = project.second.minorVersion;
						job.patchVersion = project.second.patchVersion;
						job.spineVersion = project.second.spineVersion;
						job.operation = Operation::Add;
						job.path = vec2[0];

						addJob(job);

						counter++;
					}
				}
			}
		}

		if (counter > 0) {
			std::cout << "Added " << counter << " synchronization jobs" << std::endl;
		}
	} while (false);
}

void FileSynchronizer::executeJob(const ExecuteJob & job) {
	switch (job.operation) {
	case Operation::Add:
	case Operation::Update: {
		const auto cmd = "curl --ftp-create-dirs -Q \'SITE UMASK 022\' -T \"" + PATH_PREFIX + "/" + std::to_string(job.projectID) + "/" + job.path + "\" \"ftp://" + job.username + ":" + job.password + "@" + job.ftpHost + "/" + job.rootFolder + std::to_string(job.projectID) + "/" + job.path + "\"";

		const auto result = system(cmd.c_str());

		if (result) {
			std::cout << "Update command: " << cmd << std::endl;
			std::cout << "Update result: " << result << std::endl;
		}

		if (result) return;
			
		break;
	}
	case Operation::Delete: {
		const auto cmd = "curl -v -u " + job.username + ":" + job.password + " ftp://" + job.ftpHost + " -Q \'DELE " + job.rootFolder + std::to_string(job.projectID) + "/" + job.path + "\'";

			
		const auto result = system(cmd.c_str());

		if (result) {
			std::cout << "Delete command: " << cmd << std::endl;
			std::cout << "Delete result: " << result << std::endl;
		}

		if (result) return;
			
		break;
	}
	case Operation::RaiseVersion: {
		break;
	}
	}

	finishJob(job);
}

void FileSynchronizer::finishJob(const ExecuteJob & job) {
	std::lock_guard<std::mutex> lg(lock);
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM fileserverSynchronizationQueue WHERE JobID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramJobID=" + std::to_string(job.jobID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE deleteStmt USING @paramJobID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		updateFileserver(job);
	} while (false);
}

void FileSynchronizer::updateFileserver(const ExecuteJob & job) {
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectStmt FROM \"SELECT JobID FROM fileserverSynchronizationQueue WHERE ServerID = ? AND ProjectID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE updateStmt FROM \"INSERT INTO projectsOnFileservers (ServerID, ProjectID, MajorVersion, MinorVersion, PatchVersion, SpineVersion) VALUES (?, ?, ?, ?, ?, ?) ON DUPLICATE KEY UPDATE MajorVersion = ?, MinorVersion = ?, PatchVersion = ?, SpineVersion = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramServerID=" + std::to_string(job.serverID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramProjectID=" + std::to_string(job.projectID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectStmt USING @paramServerID, @paramProjectID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		const auto results = database.getResults<std::vector<std::string>>();

		if (!results.empty()) break;

		if (!database.query("SET @paramMajorVersion=" + std::to_string(job.majorVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramMinorVersion=" + std::to_string(job.minorVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramPatchVersion=" + std::to_string(job.patchVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramSpineVersion=" + std::to_string(job.spineVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE updateStmt USING @paramServerID, @paramProjectID, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion, @paramSpineVersion, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion, @paramSpineVersion;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
	} while (false);
}

FileSynchronizer::ExecuteJob FileSynchronizer::getFirstJobInQueue() {
	ExecuteJob job;
	job.valid = false;
	
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectStmt FROM \"SELECT JobID, ServerID, ProjectID, MajorVersion, MinorVersion, PatchVersion, SpineVersion, Path, Operation FROM fileserverSynchronizationQueue ORDER BY JobID LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE selectServerDetailsStmt FROM \"SELECT Username, Password, FtpHost, RootFolder FROM fileserverList WHERE ServerID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectStmt;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		auto results = database.getResults<std::vector<std::string>>();

		if (results.empty()) break;

		auto vec = results[0];

		job.jobID = std::stoi(vec[0]);
		job.serverID = std::stoi(vec[1]);
		job.projectID = std::stoi(vec[2]);
		job.majorVersion = std::stoi(vec[3]);
		job.minorVersion = std::stoi(vec[4]);
		job.patchVersion = std::stoi(vec[5]);
		job.spineVersion = std::stoi(vec[6]);
		job.path = vec[7];
		job.operation = static_cast<Operation>(std::stoi(vec[8]));

		if (!database.query("SET @paramServerID=" + std::to_string(job.serverID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectServerDetailsStmt USING @paramServerID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		results = database.getResults<std::vector<std::string>>();

		if (results.empty()) break;

		vec = results[0];

		job.username = vec[0];
		job.password = vec[1];
		job.ftpHost = vec[2];
		job.rootFolder = vec[3];

		job.valid = true;
	} while (false);

	return job;
}

void FileSynchronizer::addJob(const AddForServerJob & job) {
	const auto firstJob = getFirstJobInQueue(); // might be processed right now

	const bool add = !firstJob.valid; // in this case there is nothing to update anyway, so always add
	bool update = firstJob.valid && (firstJob.serverID != job.serverID || firstJob.projectID != job.projectID || firstJob.path != job.path); // in this case we can update the file in theory, but we need to check whether a job for it is queued at all
	
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectStmt FROM \"SELECT JobID, Path FROM fileserverSynchronizationQueue WHERE ServerID = ? AND ProjectID = ? AND Path = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE insertStmt FROM \"INSERT INTO fileserverSynchronizationQueue (ServerID, ProjectID, MajorVersion, MinorVersion, PatchVersion, SpineVersion, Path, Operation) VALUES (?, ?, ?, ?, ?, ?, ?, ?)\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramServerID=" + std::to_string(job.serverID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramProjectID=" + std::to_string(job.projectID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramPath='" + job.path + "';")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectStmt USING @paramServerID, @paramProjectID, @paramPath;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		const auto results = database.getResults<std::vector<std::string>>();
		auto jobID = -1;

		for (const auto vec : results) {
			if (vec[1] != job.path) continue;

			jobID = std::stoi(vec[0]);

			break;
		}

		if (results.empty() || add || (!update && results.size() == 1) || jobID == -1) {
			update = false;
		} else {
			update = true;
		}

		if (update) {
			updateJob(job, jobID);
			break;
		}
		if (!database.query("SET @paramMajorVersion=" + std::to_string(job.majorVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramMinorVersion=" + std::to_string(job.minorVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramPatchVersion=" + std::to_string(job.patchVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramSpineVersion=" + std::to_string(job.spineVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramOperation=" + std::to_string(static_cast<int>(job.operation)) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE insertStmt USING @paramServerID, @paramProjectID, @paramMajorVersion, @paramMinorVersion, @paramPatchVersion, @paramSpineVersion, @paramPath, @paramOperation;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
	} while (false);
}

void FileSynchronizer::updateJob(const AddForServerJob & job, int jobID) {
	do {
		CONNECTTODATABASE(__LINE__)

		if (!database.query("PREPARE selectOperationStmt FROM \"SELECT Operation FROM fileserverSynchronizationQueue WHERE JobID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE deleteStmt FROM \"DELETE FROM fileserverSynchronizationQueue WHERE JobID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("PREPARE updateStmt FROM \"UPDATE fileserverSynchronizationQueue SET MajorVersion = ?, MinorVersion = ?, PatchVersion = ?, SpineVersion = ?, Operation = ? WHERE JobID = ?\";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
		
		if (!database.query("SET @paramJobID=" + std::to_string(jobID) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramMajorVersion=" + std::to_string(job.majorVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramMinorVersion=" + std::to_string(job.minorVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramPatchVersion=" + std::to_string(job.patchVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("SET @paramSpineVersion=" + std::to_string(job.spineVersion) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectOperationStmt USING @paramJobID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		const auto results = database.getResults<std::vector<std::string>>();

		if (results.empty()) break; // can't happen, but defensive programming and so

		const auto vec = results[0];

		const auto operation = static_cast<Operation>(std::stoi(vec[0]));

		if (job.operation == Operation::Delete && operation == Operation::Add) {
			if (!database.query("EXECUTE deleteStmt USING @paramJobID;")) {
				std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
				break;
			}
			break;
		}

		auto newOperation = job.operation;

		if (job.operation == Operation::Update && operation == Operation::Add) {
			newOperation = Operation::Add;
		}

		if (!database.query("SET @paramOperation=" + std::to_string(static_cast<int>(newOperation)) + ";")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE updateStmt USING @paramMajorVersion, @paramMinorVersion, @paramPatchVersion, @paramSpineVersion, @paramOperation, @paramJobID;")) {
			std::cout << "Query couldn't be started: " << __FILE__ << ": " << __LINE__ << std::endl;
			break;
		}
	} while (false);
}
