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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#include "api/Gamepad.h"

#include <algorithm>
#include <thread>

#include "SpineConfig.h"

#include "api/APIMessage.h"

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

#include "clockUtils/sockets/TcpSocket.h"

namespace spine {
namespace api {
namespace {
	enum GamepadSide {
		GAMEPAD_LEFT,
		GAMEPAD_RIGHT
	};
	enum GamepadAxis {
		GAMEPAD_X,
		GAMEPAD_Y
	};

	clockUtils::sockets::TcpSocket * gamepadSocket = nullptr;
	std::mutex gamepadLock;
	bool gamepadEnabled = false;
	bool gamepadActive = false;
	std::map<int32_t, bool> gamepadStates;
	int32_t leftTrigger = 0;
	int32_t rightTrigger = 0;
	int32_t leftStickX = 0;
	int32_t leftStickY = 0;
	int32_t rightStickX = 0;
	int32_t rightStickY = 0;
}

	void initializeGamepad() {
		gamepadSocket = new clockUtils::sockets::TcpSocket();
		if (gamepadSocket->connectToIP("127.0.0.1", GAMEPAD_PORT, 10000) != clockUtils::ClockError::SUCCESS) {
			delete gamepadSocket;
			gamepadSocket = nullptr;
		} else {
			gamepadSocket->receiveCallback([](std::vector<uint8_t> message, clockUtils::sockets::TcpSocket *, clockUtils::ClockError err) {
				if (err == clockUtils::ClockError::SUCCESS) {
					try {
						std::string serialized(message.begin(), message.end());
						common::Message * msg = common::Message::DeserializeBlank(serialized);
						if (msg) {
							if (msg->type == common::MessageType::GAMEPADENABLED) {
								common::GamepadEnabledMessage * gem = dynamic_cast<common::GamepadEnabledMessage *>(msg);
								if (gem) {
									gamepadEnabled = gem->enabled;
								}
							} else if (msg->type == common::MessageType::GAMEPADACTIVE) {
								common::GamepadActiveMessage * gam = dynamic_cast<common::GamepadActiveMessage *>(msg);
								if (gam) {
									gamepadActive = gam->active;
								}
							} else if (msg->type == common::MessageType::GAMEPADBUTTONSTATE) {
								common::GamepadButtonStateMessage * gbsm = dynamic_cast<common::GamepadButtonStateMessage *>(msg);
								if (gbsm) {
									std::lock_guard<std::mutex> lg(gamepadLock);
									gamepadStates[gbsm->button] = gbsm->state;
								}
							} else if (msg->type == common::MessageType::GAMEPADTRIGGERSTATE) {
								common::GamepadTriggerStateMessage * gtsm = dynamic_cast<common::GamepadTriggerStateMessage *>(msg);
								if (gtsm) {
									if (gtsm->trigger == GAMEPAD_LEFT) {
										leftTrigger = gtsm->value;
									} else {
										rightTrigger = gtsm->value;
									}
								}
							} else if (msg->type == common::MessageType::GAMEPADSTICKSTATE) {
								common::GamepadStickStateMessage * gssm = dynamic_cast<common::GamepadStickStateMessage *>(msg);
								if (gssm) {
									if (gssm->stick == GAMEPAD_LEFT) {
										if (gssm->axis) {
											leftStickX = gssm->value;
										} else {
											leftStickY = gssm->value;
										}
									} else {
										if (gssm->axis) {
											rightStickX = gssm->value;
										} else {
											rightStickY = gssm->value;
										}
									}
								}
							}
						}
						delete msg;
					} catch (...) {
						return;
					}
				}
			});
		}
	}

	void vibrateGamepad(int leftMotor, int rightMotor) {
		if (initialized && (activatedModules & common::SpineModules::Gamepad) && gamepadSocket) {
			leftMotor = std::max(std::min(leftMotor, 65535), 0);
			rightMotor = std::max(std::min(rightMotor, 65535), 0);
			common::VibrateGamepadMessage vgm;
			vgm.leftMotor = leftMotor / 65535.0f;
			vgm.rightMotor = rightMotor / 65535.0f;
			const std::string serialized = vgm.SerializeBlank();
			gamepadSocket->writePacket(serialized);
		}
	}

	int32_t isGamepadEnabled() {
		if (initialized && (activatedModules & common::SpineModules::Gamepad) && gamepadSocket) {
			return gamepadEnabled;
		} else {
			return 0;
		}
	}

	int32_t isGamepadActive() {
		if (initialized && (activatedModules & common::SpineModules::Gamepad)) {
			return gamepadActive;
		} else {
			return 0;
		}
	}

	int32_t getGamepadButtonState(int32_t button) {
		if (initialized && (activatedModules & common::SpineModules::Gamepad)) {
			std::lock_guard<std::mutex> lg(gamepadLock);
			return gamepadStates[button];
		} else {
			return 0;
		}
	}

	int32_t getGamepadTriggerState(int32_t trigger) {
		if (initialized && (activatedModules & common::SpineModules::Gamepad)) {
			return (trigger == GAMEPAD_LEFT) ? leftTrigger : rightTrigger;
		} else {
			return 0;
		}
	}

	int32_t getGamepadStickState(int32_t stick, int32_t axis) {
		if (initialized && (activatedModules & common::SpineModules::Gamepad)) {
			if (stick == GAMEPAD_LEFT) {
				return axis == GAMEPAD_X ? leftStickX : leftStickY;
			} else {
				return axis == GAMEPAD_X ? rightStickX : rightStickY;
			}
		} else {
			return 0;
		}
	}

	void changeGamepadMode(int32_t rawMode) {
		if (initialized && (activatedModules & common::SpineModules::Gamepad) && gamepadSocket) {
			common::GamepadRawModeMessage grmm;
			grmm.enabled = rawMode == 1;
			const std::string serialized = grmm.SerializeBlank();
			gamepadSocket->writePacket(serialized);
		}
	}

} /* namespace api */
} /* namespace spine */
