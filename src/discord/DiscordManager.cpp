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
// Copyright 2020 Clockwork Origins

#include "DiscordManager.h"

#include <thread>

#include "SpineConfig.h"

#include "utils/Conversion.h"

#include "discord.h"

using namespace spine::discord;
using namespace discord;

DiscordManager * DiscordManager::instance() {
	static DiscordManager discordManager;
	return &discordManager;
}

void DiscordManager::updatePresence(const QString & details, const QString & state) {
	if (!_core) return;
	
	Activity activity {};
	activity.SetDetails(details.toStdString().c_str());
	activity.SetState(state.toStdString().c_str());
	activity.SetInstance(false);
	activity.GetAssets().SetLargeImage("spine_1024x1024");
	activity.SetType(ActivityType::Playing);
	_core->ActivityManager().UpdateActivity(activity, [](Result) {});
}

bool DiscordManager::isConnected() const {
	return _core && _currentUser->GetId() > 0;
}

int64_t DiscordManager::getUserID() const {
	return _currentUser->GetId();
}

DiscordManager::DiscordManager() : _currentUser(new User()), _running(true), _thread(nullptr) {
	Core * core = nullptr;

	try {
		Core::Create(DISCORD_CLIENTID, DiscordCreateFlags_NoRequireDiscord, &core);

		_core.reset(core);

		if (!_core) return;

		core->UserManager().OnCurrentUserUpdate.Connect([this]() {
	        _core->UserManager().GetCurrentUser(_currentUser);

			emit connected();
		});

		_thread = new std::thread([this]() {
			while (_running) {
				_core->RunCallbacks();

				std::this_thread::sleep_for(std::chrono::milliseconds(16));
			}
		});
	} catch (...) {
		
	}
}

DiscordManager::~DiscordManager() {
	_running = false;

	if (_thread) {
		_thread->join();
		delete _thread;
	}
}
