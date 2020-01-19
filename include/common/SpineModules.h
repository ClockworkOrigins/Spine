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

namespace spine {
namespace common {

	enum SpineModules {
		GetCurrentUsername = 1 << 0,
		Achievements = 1 << 1,
		Scores = 1 << 2,
		Multiplayer = 1 << 3,
		OverallSave = 1 << 4,
		Gamepad = 1 << 5,
		Friends = 1 << 6,
		Statistics = 1 << 7,

		All = (1 << 8) - 1
	};

} /* namespace common */
} /* namespace spine */
