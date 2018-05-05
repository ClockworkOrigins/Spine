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

#ifndef __SPINE_GAMEPAD_XBOXCONTROLLER_H__
#define __SPINE_GAMEPAD_XBOXCONTROLLER_H__

#include <map>
#include <mutex>
#include <thread>

#define WIN32_LEAN_AND_MEAN // We don't want the extra stuff like MFC and such
#include <windows.h>
#include <XInput.h>     // XInput API
#pragma comment(lib, "XInput.lib")

namespace clockUtils {
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {
namespace gamepad {

	class KeyMapping;
	enum class DIK_KeyCodes;

	struct Vector2 {
		void set(float f) {
			X = f;
			Y = f;
		}

		float X;
		float Y;
	};

	enum GamePadButton {
		GamePad_Button_DPAD_UP = 0,
		GamePad_Button_DPAD_DOWN = 1,
		GamePad_Button_DPAD_LEFT = 2,
		GamePad_Button_DPAD_RIGHT = 3,
		GamePad_Button_START = 4,
		GamePad_Button_BACK = 5,
		GamePad_Button_LEFT_THUMB = 6,
		GamePad_Button_RIGHT_THUMB = 7,
		GamePad_Button_LEFT_SHOULDER = 8,
		GamePad_Button_RIGHT_SHOULDER = 9,
		GamePad_Button_A = 10,
		GamePad_Button_B = 11,
		GamePad_Button_X = 12,
		GamePad_Button_Y = 13,
		GamePad_LTrigger = 14,
		GamePad_RTrigger = 15,
		GamePad_LStick_X_Pos = 16,
		GamePad_LStick_X_Neg = 17,
		GamePad_LStick_Y_Pos = 18,
		GamePad_LStick_Y_Neg = 19,
		GamePad_RStick_X_Pos = 20,
		GamePad_RStick_X_Neg = 21,
		GamePad_RStick_Y_Pos = 22,
		GamePad_RStick_Y_Neg = 23,
		GamePadButton_Max = 24
	};

	// GamePad Indexes
	enum GamePadIndex {
		GamePadIndex_One = 0,
		GamePadIndex_Two = 1,
		GamePadIndex_Three = 2,
		GamePadIndex_Four = 3,
		COUNT
	};

	// The GamePad State Stuct, were we store the buttons positions
	struct GamePadState {
		bool _buttons[GamePadButton_Max];
		Vector2 _left_thumbstick;               // <= I'm using a Vector2 (floats) class but you can replaced it with a float X and Y or whatever your Vector2 class is
		Vector2 _right_thumbstick;
		float _left_trigger;
		float _right_trigger;

		// Just to clear all values to default
		void reset() {
			for (int i = 0; i < (int) GamePadButton_Max; ++i) {
				_buttons[i] = false;
			}
			_left_thumbstick.set(0.0f);
			_right_thumbstick.set(0.0f);
			_left_trigger = 0.0f;
			_right_trigger = 0.0f;
		}
	};

	class GamePadXbox {
	public:
		GamePadXbox(GamePadIndex idx);
		GamePadXbox(GamePadIndex idx, int keyDelayRate, const std::map<GamePadButton, DIK_KeyCodes> gamepadButtons, GamePadButton previousSpellButton, GamePadButton nextSpellButton, GamePadButton drawSpellButton);

		~GamePadXbox();

		static bool isConnected(GamePadIndex idx);
		bool isConnected();
		void vibrate(float leftmotor, float rightmotor);
		void update();
		static GamePadButton getButtonPressed(GamePadIndex idx);

	public:
		GamePadState State;

	private:
		XINPUT_STATE _controllerState;
		GamePadIndex _index;
		int _keyDelayRate;
		KeyMapping * _keyMapping;
		std::map<GamePadButton, bool> _pressedKeys;
		std::map<GamePadButton, DIK_KeyCodes> _gamepadButtons;
		std::map<GamePadButton, DIK_KeyCodes> _specialButtons;
		int _leftTrigger;
		int _rightTrigger;
		int _leftStickX;
		int _leftStickY;
		int _rightStickX;
		int _rightStickY;
		bool _rawMode;
		GamePadButton _previousSpellButton;
		GamePadButton _nextSpellButton;
		GamePadButton _drawSpellButton;
		int _currentSpellIndex;

		bool _running;
		bool _active;
		mutable std::mutex _lock;
		clockUtils::sockets::TcpSocket * _sock;
		std::thread _worker;

		void execute();
		void activate();
		DIK_KeyCodes indexToSpellButton();
	};

} /* namespace gamepad */
} /* namespace spine */

#endif /* __SPINE_GAMEPAD_XBOXCONTROLLER_H__ */