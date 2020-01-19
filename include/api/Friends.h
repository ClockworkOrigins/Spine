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

#include "api/API.h"

namespace spine {
namespace api {

	bool initializeFriends();

	/**
	 * \brief return the amount of friends
	 */
	SPINEAPI_EXPORTS int32_t getFriendCount();

	/**
	 * \brief returns the name of the friend at position index
	 */
	SPINEAPI_EXPORTS void getFriendName(int32_t index, char * str);

} /* namespace api */
} /* namespace spine */
