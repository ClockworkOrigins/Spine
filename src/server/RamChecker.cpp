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

#include "RamChecker.h"

#ifndef WIN32
#include "sys/sysinfo.h"
#include "sys/types.h"
#endif

#include <iostream>
#include <string>

using namespace spine::server;

std::mutex RamChecker::_lock;

RamChecker::RamChecker(const std::string & label) : _label(label) {
	const char * val = std::getenv("SPINE_RAMCHECK");
	_enabled = val && std::stoi(val) == 1;

	if (!_enabled) return;

	_lock.lock();
	_lastRamUsage = getRamUsage();
}

RamChecker::~RamChecker() {
	if (!_enabled) return;

	const auto currentRam = getRamUsage();

	if (currentRam > _lastRamUsage) {
		std::cout << "RAM increased by " << currentRam - _lastRamUsage << " (" << _label << ")" << std::endl;
	}

	_lock.unlock();
}

int64_t RamChecker::getRamUsage() const {
#ifndef WIN32
	struct sysinfo memInfo;

	sysinfo(&memInfo);

	int64_t virtualMemUsed = memInfo.totalram - memInfo.freeram;
	//Add other values in next statement to avoid int overflow on right hand side...
	virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
	virtualMemUsed *= memInfo.mem_unit;

	return virtualMemUsed;
#else
	return 0;
#endif
}
