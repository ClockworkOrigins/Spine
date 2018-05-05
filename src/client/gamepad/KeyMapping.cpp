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

#include "gamepad/KeyMapping.h"

namespace spine {
namespace gamepad {

	KeyMapping::KeyMapping() : _keyMap() {
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_0, WindowsKeyCodes::KEY_0));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_1, WindowsKeyCodes::KEY_1));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_2, WindowsKeyCodes::KEY_2));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_3, WindowsKeyCodes::KEY_3));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_4, WindowsKeyCodes::KEY_4));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_5, WindowsKeyCodes::KEY_5));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_6, WindowsKeyCodes::KEY_6));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_7, WindowsKeyCodes::KEY_7));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_8, WindowsKeyCodes::KEY_8));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_9, WindowsKeyCodes::KEY_9));

		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_A, WindowsKeyCodes::KEY_A));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_B, WindowsKeyCodes::KEY_B));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_C, WindowsKeyCodes::KEY_C));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_D, WindowsKeyCodes::KEY_D));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_E, WindowsKeyCodes::KEY_E));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F, WindowsKeyCodes::KEY_F));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_G, WindowsKeyCodes::KEY_G));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_H, WindowsKeyCodes::KEY_H));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_I, WindowsKeyCodes::KEY_I));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_J, WindowsKeyCodes::KEY_J));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_K, WindowsKeyCodes::KEY_K));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_L, WindowsKeyCodes::KEY_L));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_M, WindowsKeyCodes::KEY_M));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_N, WindowsKeyCodes::KEY_N));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_O, WindowsKeyCodes::KEY_O));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_P, WindowsKeyCodes::KEY_P));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_Q, WindowsKeyCodes::KEY_Q));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_R, WindowsKeyCodes::KEY_R));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_S, WindowsKeyCodes::KEY_S));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_T, WindowsKeyCodes::KEY_T));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_U, WindowsKeyCodes::KEY_U));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_V, WindowsKeyCodes::KEY_V));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_W, WindowsKeyCodes::KEY_W));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_X, WindowsKeyCodes::KEY_X));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_Y, WindowsKeyCodes::KEY_Y));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_Z, WindowsKeyCodes::KEY_Z));

		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_ESCAPE, WindowsKeyCodes::KEY_ESCAPE));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_RETURN, WindowsKeyCodes::KEY_RETURN));

		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_LEFTARROW, WindowsKeyCodes::KEY_LEFTARROW));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_RIGHTARROW, WindowsKeyCodes::KEY_RIGHTARROW));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_UPARROW, WindowsKeyCodes::KEY_UPARROW));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_DOWNARROW, WindowsKeyCodes::KEY_DOWNARROW));

		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F1, WindowsKeyCodes::KEY_F1));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F2, WindowsKeyCodes::KEY_F2));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F3, WindowsKeyCodes::KEY_F3));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F4, WindowsKeyCodes::KEY_F4));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F5, WindowsKeyCodes::KEY_F5));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F6, WindowsKeyCodes::KEY_F6));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F7, WindowsKeyCodes::KEY_F7));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F8, WindowsKeyCodes::KEY_F8));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F9, WindowsKeyCodes::KEY_F9));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F10, WindowsKeyCodes::KEY_F10));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F11, WindowsKeyCodes::KEY_F11));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_F12, WindowsKeyCodes::KEY_F12));

		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_LCONTROL, WindowsKeyCodes::KEY_LCONTROL));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_LMENU, WindowsKeyCodes::KEY_LMENU));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_RCONTROL, WindowsKeyCodes::KEY_RCONTROL));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_RMENU, WindowsKeyCodes::KEY_RMENU));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_TAB, WindowsKeyCodes::KEY_TAB));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_LSHIFT, WindowsKeyCodes::KEY_LSHIFT));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_RSHIFT, WindowsKeyCodes::KEY_RSHIFT));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_CAPITAL, WindowsKeyCodes::KEY_CAPITAL));
		_keyMap.insert(std::make_pair(DIK_KeyCodes::KEY_SPACE, WindowsKeyCodes::KEY_SPACE));
	}

	void KeyMapping::updateState(DIK_KeyCodes keyCode, bool pressed) {
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wVk = 0;
		ip.ki.wScan = (WORD) MapVirtualKey((WORD) _keyMap[keyCode], MAPVK_VK_TO_VSC);
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		bool extended = false;

		if (keyCode == DIK_KeyCodes::KEY_UPARROW || keyCode == DIK_KeyCodes::KEY_DOWNARROW || keyCode == DIK_KeyCodes::KEY_LEFTARROW || keyCode == DIK_KeyCodes::KEY_RIGHTARROW) {
			extended = true;
		}

		// Press the "A" key
		ip.ki.dwFlags = KEYEVENTF_SCANCODE;
		if (extended) {
			ip.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		}
		if (!pressed) {
			ip.ki.dwFlags |= KEYEVENTF_KEYUP;
		}
		SendInput(1, &ip, sizeof(INPUT));
	}

} /* namespace gamepad */
} /* namespace spine */
