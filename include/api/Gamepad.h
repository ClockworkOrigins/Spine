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

	void initializeGamepad();

	/**
	 * \brief vibrates gamepad
	 */
	SPINEAPI_EXPORTS void vibrateGamepad(int leftMotor, int rightMotor);

	/**
	 * \brief returns true if a Gamepad is configured and enabled (not in use)
	 */
	SPINEAPI_EXPORTS int32_t isGamepadEnabled();

	/**
	 * \brief returns true if a Gamepad is in use (at least one key was used)
	 */
	SPINEAPI_EXPORTS int32_t isGamepadActive();

	/**
	 * \brief returns the state of the given button
	 */
	SPINEAPI_EXPORTS int32_t getGamepadButtonState(int32_t button);

	/**
	 * \brief returns the state of the given trigger
	 */
	SPINEAPI_EXPORTS int32_t getGamepadTriggerState(int32_t trigger);

	/**
	 * \brief returns the state of the stick and direction
	 */
	SPINEAPI_EXPORTS int32_t getGamepadStickState(int32_t stick, int32_t axis);

	/**
	 * \brief changes mode
	 */
	SPINEAPI_EXPORTS void changeGamepadMode(int32_t rawMode);

} /* namespace api */
} /* namespace spine */
