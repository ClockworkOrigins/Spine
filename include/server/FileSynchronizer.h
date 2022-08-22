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

#pragma once

#include <mutex>
#include <string>

namespace spine {
namespace server {

	class FileSynchronizer {
	public:
		enum class Operation {
			Add,
			Update,
			Delete,
			RaiseVersion
		};

		typedef struct BaseJob {
			int projectID = 0;
			std::string path;
			int majorVersion = 0;
			int minorVersion = 0;
			int patchVersion = 0;
			int spineVersion = 0;
			Operation operation = Operation::Update;
		} BaseJob;

		typedef struct AddJob : BaseJob {} AddJob;

		typedef struct AddForServerJob : BaseJob {
			int serverID = 0;

			AddForServerJob() = default;

			explicit AddForServerJob(const AddJob & job) {
				projectID = job.projectID;
				path = job.path;
				majorVersion = job.majorVersion;
				minorVersion = job.minorVersion;
				patchVersion = job.patchVersion;
				spineVersion = job.spineVersion;
				operation = job.operation;
			}
		} AddForServerJob;

		typedef struct ExecuteJob : BaseJob {
			bool valid = false;
			int jobID = 0;
			int serverID = 0;
			std::string username;
			std::string password;
			std::string ftpHost;
			std::string rootFolder;
		} ExecuteJob;

		static void init();

		static void addJob(const AddJob & job);

	private:
		static std::mutex lock;

		// main loop
		static void exec();

		// checks if for some server jobs are missing, e.g. new server got added and needs to synchronize everything now
		static void addMissing();

		// executes a single job
		static void executeJob(const ExecuteJob & job);

		// called after job has been successfully finished
		static void finishJob(const ExecuteJob & job);

		// check if that was the last job for the projectID and if yes unlock fileserver
		static void updateFileserver(const ExecuteJob & job);

		// helper
		// returns the first job in the queue
		static ExecuteJob getFirstJobInQueue();

		// adds a job for a specific server
		static void addJob(const AddForServerJob & job);

		// updates a job != first job to represent latest state in case a file gets updated again before previous operation finished
		static void updateJob(const AddForServerJob & job, int jobID);
	};

} /* namespace server */
} /* namespace spine */
