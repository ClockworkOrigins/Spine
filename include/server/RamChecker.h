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
 // Copyright 2022 Clockwork Origins

#pragma once

#include <mutex>

namespace spine {
namespace server {

	class RamChecker {
	public:
		RamChecker(const std::string & label);
		~RamChecker();

	private:
		static std::mutex _lock;
		std::string _label;
		int64_t _lastRamUsage;
		bool _enabled;

		int64_t getRamUsage() const;
	};

} /* namespace server */
} /* namespace spine */
