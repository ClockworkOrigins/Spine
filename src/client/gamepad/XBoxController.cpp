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

#include "gamepad/XBoxController.h"

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "gamepad/KeyMapping.h"

#include "clockUtils/sockets/TcpSocket.h"

namespace spine {
namespace gamepad {
namespace {
	enum GamepadSide {
		GAMEPAD_LEFT,
		GAMEPAD_RIGHT
	};
	enum GamepadAxis {
		GAMEPAD_X,
		GAMEPAD_Y
	};
}

	GamePadXbox::GamePadXbox(GamePadIndex idx) : _keyDelayRate(), _keyMapping(new KeyMapping()), _pressedKeys(), _gamepadButtons(), _specialButtons(), _leftTrigger(0), _rightTrigger(0), _leftStickX(0), _leftStickY(0), _rightStickX(0), _rightStickY(0), _rawMode(false), _running(true), _active(false), _sock(nullptr) {
		_index = idx;
		State.reset();
	}

	GamePadXbox::GamePadXbox(GamePadIndex idx, int keyDelayRate, const std::map<GamePadButton, DIK_KeyCodes> gamepadButtons, GamePadButton previousSpellButton, GamePadButton nextSpellButton, GamePadButton drawSpellButton) : _keyDelayRate(keyDelayRate), _keyMapping(new KeyMapping()), _pressedKeys(), _gamepadButtons(gamepadButtons), _specialButtons(), _leftTrigger(0), _rightTrigger(0), _leftStickX(0), _leftStickY(0), _rightStickX(0), _rightStickY(0), _rawMode(false), _previousSpellButton(previousSpellButton), _nextSpellButton(nextSpellButton), _drawSpellButton(drawSpellButton), _currentSpellIndex(0), _running(true), _active(false), _sock(nullptr) {
		_index = idx;
		State.reset();
		_specialButtons.insert(std::make_pair(GamePadButton::GamePad_Button_BACK, DIK_KeyCodes::KEY_ESCAPE));
		_worker = std::thread(std::bind(&GamePadXbox::execute, this));
	}

	GamePadXbox::~GamePadXbox() {
		// We don't want the controller to be vibrating accidentally when we exit the app
		if (isConnected()) {
			vibrate(0.0f, 0.0f);
		}
		_running = false;
		if (_worker.joinable()) {
			_worker.join();
		}
	}

	bool GamePadXbox::isConnected(GamePadIndex idx) {
		// clean the state
		XINPUT_STATE controllerState;
		memset(&controllerState, 0, sizeof(XINPUT_STATE));

		// Get the state
		DWORD Result = XInputGetState(idx, &controllerState);

		if (Result == ERROR_SUCCESS) {
			return true;
		} else {
			return false;
		}
	}

	bool GamePadXbox::isConnected() {
		// clean the state
		memset(&_controllerState, 0, sizeof(XINPUT_STATE));

		// Get the state
		DWORD Result = XInputGetState(_index, &_controllerState);

		if (Result == ERROR_SUCCESS) {
			return true;
		} else {
			return false;
		}
	}

	void GamePadXbox::vibrate(float leftmotor, float rightmotor) {
		// Create a new Vibraton 
		XINPUT_VIBRATION Vibration;

		memset(&Vibration, 0, sizeof(XINPUT_VIBRATION));

		int leftVib = (int) (leftmotor * 65535.0f);
		int rightVib = (int) (rightmotor * 65535.0f);

		// Set the Vibration Values
		Vibration.wLeftMotorSpeed = (WORD) leftVib;
		Vibration.wRightMotorSpeed = (WORD) rightVib;
		// Vibrate the controller
		XInputSetState((int) _index, &Vibration);
	}

	void GamePadXbox::update() {
		State.reset();
		// The values of the Left and Right Triggers go from 0 to 255. We just convert them to 0.0f=>1.0f
		if (_controllerState.Gamepad.bRightTrigger && (int) _controllerState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			State._right_trigger = _controllerState.Gamepad.bRightTrigger / 255.0f;
		}

		if (_controllerState.Gamepad.bLeftTrigger && (int) _controllerState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			State._left_trigger = _controllerState.Gamepad.bLeftTrigger / 255.0f;
		}

		// Get the Buttons
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
			State._buttons[GamePad_Button_A] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
			State._buttons[GamePad_Button_B] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) {
			State._buttons[GamePad_Button_X] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) {
			State._buttons[GamePad_Button_Y] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
			State._buttons[GamePad_Button_DPAD_DOWN] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
			State._buttons[GamePad_Button_DPAD_UP] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
			State._buttons[GamePad_Button_DPAD_LEFT] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
			State._buttons[GamePad_Button_DPAD_RIGHT] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) {
			State._buttons[GamePad_Button_START] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) {
			State._buttons[GamePad_Button_BACK] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
			State._buttons[GamePad_Button_LEFT_THUMB] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			State._buttons[GamePad_Button_LEFT_SHOULDER] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
			State._buttons[GamePad_Button_RIGHT_THUMB] = true;
		}
		if (_controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			State._buttons[GamePad_Button_RIGHT_SHOULDER] = true;
		}
		// The Rest is missing, you can figure out the rest :)

		// Check to make sure we are not moving during the dead zone
		// Let's check the Left DeadZone
		if ((_controllerState.Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) &&
			(_controllerState.Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) {
			_controllerState.Gamepad.sThumbLX = 0;
			_controllerState.Gamepad.sThumbLY = 0;
		}

		// Check left thumbStick

		float leftThumbY = _controllerState.Gamepad.sThumbLY;
		if (leftThumbY) {
			State._left_thumbstick.Y = leftThumbY;
		}
		float leftThumbX = _controllerState.Gamepad.sThumbLX;
		if (leftThumbX) {
			State._left_thumbstick.X = leftThumbX;
		}

		// For the rightThumbstick it's pretty much the same.
		if ((_controllerState.Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) &&
			(_controllerState.Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE &&
			_controllerState.Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) {
			_controllerState.Gamepad.sThumbRX = 0;
			_controllerState.Gamepad.sThumbRY = 0;
		}

		// Check left thumbStick

		float rightThumbY = _controllerState.Gamepad.sThumbRY;
		if (rightThumbY) {
			State._right_thumbstick.Y = rightThumbY;
		}
		float rightThumbX = _controllerState.Gamepad.sThumbRX;
		if (rightThumbX) {
			State._right_thumbstick.X = rightThumbX;
		}

		State._buttons[GamePadButton::GamePad_LTrigger] = State._left_trigger > 0;
		State._buttons[GamePadButton::GamePad_RTrigger] = State._right_trigger > 0;
		State._buttons[GamePadButton::GamePad_LStick_X_Pos] = State._left_thumbstick.X > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_LStick_X_Neg] = State._left_thumbstick.X < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_LStick_Y_Pos] = State._left_thumbstick.Y > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_LStick_Y_Neg] = State._left_thumbstick.Y < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_RStick_X_Pos] = State._right_thumbstick.X > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_RStick_X_Neg] = State._right_thumbstick.X < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_RStick_Y_Pos] = State._right_thumbstick.Y > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		State._buttons[GamePadButton::GamePad_RStick_Y_Neg] = State._right_thumbstick.Y < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	}

	GamePadButton GamePadXbox::getButtonPressed(GamePadIndex idx) {
		GamePadXbox gpx(idx);
		while (gpx.isConnected()) {
			gpx.update();
			for (int i = 0; i < GamePadButton_Max; i++) {
				if (gpx.State._buttons[i]) {
					return GamePadButton(i);
				}
			}
		}
		return GamePadButton_Max;
	}

	void GamePadXbox::execute() {
		clockUtils::sockets::TcpSocket listenSocket;
		listenSocket.listen(GAMEPAD_PORT, 1, false, [this](clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError err) {
			delete _sock;
			_sock = nullptr;
			if (err == clockUtils::ClockError::SUCCESS) {
				_sock = sock;
				std::string serialized;
				// start synchronisation
				{
					std::lock_guard<std::mutex> lg(_lock);
					{
						common::GamepadEnabledMessage gem;
						gem.enabled = isConnected();
						serialized = gem.SerializeBlank();
						_sock->writePacket(serialized);
					}
					{
						common::GamepadActiveMessage gem;
						gem.active = _active;
						serialized = gem.SerializeBlank();
						_sock->writePacket(serialized);
					}
					for (int i = 0; i < GamePadButton_Max; i++) {
						common::GamepadButtonStateMessage gbsm;
						gbsm.button = i;
						gbsm.state = State._buttons[i];
						serialized = gbsm.SerializeBlank();
						_sock->writePacket(serialized);
					}
				}
				while (clockUtils::ClockError::SUCCESS == _sock->receivePacket(serialized)) {
					try {
						common::Message * msg = common::Message::DeserializeBlank(serialized);
						if (msg) {
							// dispatch message type
							if (msg->type == common::MessageType::VIBRATEGAMEPAD) {
								common::VibrateGamepadMessage * vgm = dynamic_cast<common::VibrateGamepadMessage *>(msg);
								if (vgm) {
									vibrate(vgm->leftMotor, vgm->rightMotor);
								}
							} else if (msg->type == common::MessageType::GAMEPADRAWMODE) {
								common::GamepadRawModeMessage * grmm = dynamic_cast<common::GamepadRawModeMessage *>(msg);
								if (grmm) {
									_rawMode = grmm->enabled;
								}
							}
						}
						delete msg;
					} catch (...) {
						break;
					}
				}
			}
		});
		while (_running) {
			if (isConnected()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(_keyDelayRate));

				update();

				{
					if (_leftTrigger != int(State._left_trigger)) {
						common::GamepadTriggerStateMessage gtsm;
						gtsm.trigger = GAMEPAD_LEFT;
						gtsm.value = int(State._left_trigger);
						_leftTrigger = int(State._left_trigger);
						if (_sock) {
							std::string serialized = gtsm.SerializeBlank();
							_sock->writePacket(serialized);
						}

					}
				}
				{
					if (_rightTrigger != int(State._right_trigger)) {
						common::GamepadTriggerStateMessage gtsm;
						gtsm.trigger = GAMEPAD_RIGHT;
						gtsm.value = int(State._right_trigger);
						_leftTrigger = int(State._right_trigger);
						if (_sock) {
							std::string serialized = gtsm.SerializeBlank();
							_sock->writePacket(serialized);
						}

					}
				}
				{
					if (_leftStickX != int(State._left_thumbstick.X)) {
						common::GamepadStickStateMessage gssm;
						gssm.stick = GAMEPAD_LEFT;
						gssm.axis = GAMEPAD_X;
						gssm.value = int(State._left_thumbstick.X);
						_leftStickX = int(State._left_thumbstick.X);
						if (_sock) {
							std::string serialized = gssm.SerializeBlank();
							_sock->writePacket(serialized);
						}

					}
				}
				{
					if (_leftStickY != int(State._left_thumbstick.Y)) {
						common::GamepadStickStateMessage gssm;
						gssm.stick = GAMEPAD_LEFT;
						gssm.axis = GAMEPAD_Y;
						gssm.value = int(State._left_thumbstick.Y);
						_leftStickY = int(State._left_thumbstick.Y);
						if (_sock) {
							std::string serialized = gssm.SerializeBlank();
							_sock->writePacket(serialized);
						}

					}
				}
				{
					if (_rightStickX != int(State._right_thumbstick.X)) {
						common::GamepadStickStateMessage gssm;
						gssm.stick = GAMEPAD_RIGHT;
						gssm.axis = GAMEPAD_X;
						gssm.value = int(State._right_thumbstick.X);
						_rightStickX = int(State._right_thumbstick.X);
						if (_sock) {
							std::string serialized = gssm.SerializeBlank();
							_sock->writePacket(serialized);
						}

					}
				}
				{
					if (_rightStickY != int(State._right_thumbstick.Y)) {
						common::GamepadStickStateMessage gssm;
						gssm.stick = GAMEPAD_RIGHT;
						gssm.axis = GAMEPAD_Y;
						gssm.value = int(State._right_thumbstick.Y);
						_rightStickY = int(State._right_thumbstick.Y);
						if (_sock) {
							std::string serialized = gssm.SerializeBlank();
							_sock->writePacket(serialized);
						}

					}
				}
				for (int i = 0; i < GamePadButton_Max; i++) {
					GamePadButton button = GamePadButton(i);
					auto it = _pressedKeys.find(button);
					if (it == _pressedKeys.end()) {
						it = _pressedKeys.insert(std::make_pair(button, false)).first;
					}
					if (it->second != State._buttons[i]) {
						it->second = State._buttons[i];
						common::GamepadButtonStateMessage gbsm;
						gbsm.button = i;
						gbsm.state = it->second;
						if (_sock) {
							std::string serialized = gbsm.SerializeBlank();
							_sock->writePacket(serialized);
						}
						if (!_rawMode) {
							if (_gamepadButtons.find(button) != _gamepadButtons.end()) {
								_keyMapping->updateState(_gamepadButtons[button], it->second);
							}
							if (_specialButtons.find(button) != _specialButtons.end()) {
								_keyMapping->updateState(_specialButtons[button], it->second);
							}
							if (button == _previousSpellButton) {
								if (it->second) {
									_currentSpellIndex = (_currentSpellIndex > 0) ? _currentSpellIndex - 1 : 6;
								}
								_keyMapping->updateState(indexToSpellButton(), it->second);
							}
							if (button == _nextSpellButton) {
								if (it->second) {
									_currentSpellIndex = (_currentSpellIndex < 6) ? _currentSpellIndex + 1 : 0;
								}
								_keyMapping->updateState(indexToSpellButton(), it->second);
							}
							if (button == _drawSpellButton) {
								_keyMapping->updateState(indexToSpellButton(), it->second);
							}
						}
						activate();
					}
				}
			}
		}
	}

	void GamePadXbox::activate() {
		if (!_active) {
			_active = true;
			if (_sock) {
				common::GamepadActiveMessage gem;
				gem.active = _active;
				std::string serialized = gem.SerializeBlank();
				_sock->writePacket(serialized);
			}
		}
	}

	DIK_KeyCodes GamePadXbox::indexToSpellButton() {
		switch (_currentSpellIndex) {
		case 0: {
			return DIK_KeyCodes::KEY_4;
		}
		case 1: {
			return DIK_KeyCodes::KEY_5;
		}
		case 2: {
			return DIK_KeyCodes::KEY_6;
		}
		case 3: {
			return DIK_KeyCodes::KEY_7;
		}
		case 4: {
			return DIK_KeyCodes::KEY_8;
		}
		case 5: {
			return DIK_KeyCodes::KEY_9;
		}
		case 6: {
			return DIK_KeyCodes::KEY_0;
		}
		default: {
			return DIK_KeyCodes::KEY_3;
		}
		}
	}

} /* namespace gamepad */
} /* namespace spine */
