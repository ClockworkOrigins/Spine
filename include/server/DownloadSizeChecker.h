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

	class DownloadSizeChecker {
	public:
		DownloadSizeChecker();

		uint64_t getBytes(int32_t modID, const std::string & language, uint32_t version);
		uint64_t getBytesForPackage(int32_t modID, int32_t optionalID, const std::string & language, uint32_t version);
		void clear();

	private:
		std::map<std::tuple<int32_t, std::string, uint32_t>, uint64_t> _cache;
		std::map<std::tuple<int32_t, std::string, uint32_t>, uint64_t> _packageCache;
		std::mutex _lock;
	};

} /* namespace spine */
