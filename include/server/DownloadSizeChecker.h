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

#include <cstdint>
#include <map>
#include <mutex>

namespace spine {
namespace server {

	class DownloadSizeChecker {
	public:
		static uint64_t getBytes(int32_t modID, const std::string & language, uint32_t version);
		static uint64_t getBytesForPackage(int32_t modID, int32_t optionalID, const std::string & language, uint32_t version);
		static uint64_t getSizeForFile(int32_t projectID, const std::string & path);
		static uint64_t getSizeForPackageFile(int32_t packageID, const std::string & path);

		static void init();
		static void clear();
		static void refresh();

	private:
		static std::map<std::tuple<int32_t, std::string, uint32_t>, uint64_t> _cache;
		static std::map<std::tuple<int32_t, std::string, uint32_t>, uint64_t> _packageCache;
		static std::map<int32_t, uint64_t> _fileSizes;
		static std::map<int32_t, uint64_t> _packageFileSizes;
		static std::map<std::pair<int32_t, std::string>, int32_t> _fileToFileIDs;
		static std::map<std::pair<int32_t, std::string>, int32_t> _fileToPackageIDs;
		static std::recursive_mutex _lock;
	};

} /* namespace server */
} /* namespace spine */
