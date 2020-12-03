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

#include "KeyMapping.h"

#include <map>

namespace spine {
namespace input {
namespace {

	std::map<DIK_KeyCodes, WindowsKeyCodes> keyMap = {
		{ DIK_KeyCodes::KEY_0, WindowsKeyCodes::KEY_0 },
		{ DIK_KeyCodes::KEY_1, WindowsKeyCodes::KEY_1 },
		{ DIK_KeyCodes::KEY_2, WindowsKeyCodes::KEY_2 },
		{ DIK_KeyCodes::KEY_3, WindowsKeyCodes::KEY_3 },
		{ DIK_KeyCodes::KEY_4, WindowsKeyCodes::KEY_4 },
		{ DIK_KeyCodes::KEY_5, WindowsKeyCodes::KEY_5 },
		{ DIK_KeyCodes::KEY_6, WindowsKeyCodes::KEY_6 },
		{ DIK_KeyCodes::KEY_7, WindowsKeyCodes::KEY_7 },
		{ DIK_KeyCodes::KEY_8, WindowsKeyCodes::KEY_8 },
		{ DIK_KeyCodes::KEY_9, WindowsKeyCodes::KEY_9 },

		{ DIK_KeyCodes::KEY_A, WindowsKeyCodes::KEY_A },
		{ DIK_KeyCodes::KEY_B, WindowsKeyCodes::KEY_B },
		{ DIK_KeyCodes::KEY_C, WindowsKeyCodes::KEY_C },
		{ DIK_KeyCodes::KEY_D, WindowsKeyCodes::KEY_D },
		{ DIK_KeyCodes::KEY_E, WindowsKeyCodes::KEY_E },
		{ DIK_KeyCodes::KEY_F, WindowsKeyCodes::KEY_F },
		{ DIK_KeyCodes::KEY_G, WindowsKeyCodes::KEY_G },
		{ DIK_KeyCodes::KEY_H, WindowsKeyCodes::KEY_H },
		{ DIK_KeyCodes::KEY_I, WindowsKeyCodes::KEY_I },
		{ DIK_KeyCodes::KEY_J, WindowsKeyCodes::KEY_J },
		{ DIK_KeyCodes::KEY_K, WindowsKeyCodes::KEY_K },
		{ DIK_KeyCodes::KEY_L, WindowsKeyCodes::KEY_L },
		{ DIK_KeyCodes::KEY_M, WindowsKeyCodes::KEY_M },
		{ DIK_KeyCodes::KEY_N, WindowsKeyCodes::KEY_N },
		{ DIK_KeyCodes::KEY_O, WindowsKeyCodes::KEY_O },
		{ DIK_KeyCodes::KEY_P, WindowsKeyCodes::KEY_P },
		{ DIK_KeyCodes::KEY_Q, WindowsKeyCodes::KEY_Q },
		{ DIK_KeyCodes::KEY_R, WindowsKeyCodes::KEY_R },
		{ DIK_KeyCodes::KEY_S, WindowsKeyCodes::KEY_S },
		{ DIK_KeyCodes::KEY_T, WindowsKeyCodes::KEY_T },
		{ DIK_KeyCodes::KEY_U, WindowsKeyCodes::KEY_U },
		{ DIK_KeyCodes::KEY_V, WindowsKeyCodes::KEY_V },
		{ DIK_KeyCodes::KEY_W, WindowsKeyCodes::KEY_W },
		{ DIK_KeyCodes::KEY_X, WindowsKeyCodes::KEY_X },
		{ DIK_KeyCodes::KEY_Y, WindowsKeyCodes::KEY_Y },
		{ DIK_KeyCodes::KEY_Z, WindowsKeyCodes::KEY_Z },

		{ DIK_KeyCodes::KEY_ESCAPE, WindowsKeyCodes::KEY_ESCAPE },
		{ DIK_KeyCodes::KEY_RETURN, WindowsKeyCodes::KEY_RETURN },

		{ DIK_KeyCodes::KEY_LEFTARROW, WindowsKeyCodes::KEY_LEFTARROW },
		{ DIK_KeyCodes::KEY_RIGHTARROW, WindowsKeyCodes::KEY_RIGHTARROW },
		{ DIK_KeyCodes::KEY_UPARROW, WindowsKeyCodes::KEY_UPARROW },
		{ DIK_KeyCodes::KEY_DOWNARROW, WindowsKeyCodes::KEY_DOWNARROW },

		{ DIK_KeyCodes::KEY_F1, WindowsKeyCodes::KEY_F1 },
		{ DIK_KeyCodes::KEY_F2, WindowsKeyCodes::KEY_F2 },
		{ DIK_KeyCodes::KEY_F3, WindowsKeyCodes::KEY_F3 },
		{ DIK_KeyCodes::KEY_F4, WindowsKeyCodes::KEY_F4 },
		{ DIK_KeyCodes::KEY_F5, WindowsKeyCodes::KEY_F5 },
		{ DIK_KeyCodes::KEY_F6, WindowsKeyCodes::KEY_F6 },
		{ DIK_KeyCodes::KEY_F7, WindowsKeyCodes::KEY_F7 },
		{ DIK_KeyCodes::KEY_F8, WindowsKeyCodes::KEY_F8 },
		{ DIK_KeyCodes::KEY_F9, WindowsKeyCodes::KEY_F9 },
		{ DIK_KeyCodes::KEY_F10, WindowsKeyCodes::KEY_F10 },
		{ DIK_KeyCodes::KEY_F11, WindowsKeyCodes::KEY_F11 },
		{ DIK_KeyCodes::KEY_F12, WindowsKeyCodes::KEY_F12 },

		{ DIK_KeyCodes::KEY_LCONTROL, WindowsKeyCodes::KEY_LCONTROL },
		{ DIK_KeyCodes::KEY_LMENU, WindowsKeyCodes::KEY_LMENU },
		{ DIK_KeyCodes::KEY_RCONTROL, WindowsKeyCodes::KEY_RCONTROL },
		{ DIK_KeyCodes::KEY_RMENU, WindowsKeyCodes::KEY_RMENU },
		{ DIK_KeyCodes::KEY_TAB, WindowsKeyCodes::KEY_TAB },
		{ DIK_KeyCodes::KEY_LSHIFT, WindowsKeyCodes::KEY_LSHIFT },
		{ DIK_KeyCodes::KEY_RSHIFT, WindowsKeyCodes::KEY_RSHIFT },
		{ DIK_KeyCodes::KEY_CAPITAL, WindowsKeyCodes::KEY_CAPITAL },
		{ DIK_KeyCodes::KEY_SPACE, WindowsKeyCodes::KEY_SPACE }
	};
}

	void updateState(int32_t keyCode, int32_t pressed) {
#ifdef WIN32
		auto kc = static_cast<DIK_KeyCodes>(keyCode);
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wVk = 0;
		ip.ki.wScan = static_cast<WORD>(MapVirtualKey(static_cast<WORD>(keyMap[kc]), MAPVK_VK_TO_VSC));
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		bool extended = false;

		if (kc == DIK_KeyCodes::KEY_UPARROW || kc == DIK_KeyCodes::KEY_DOWNARROW || kc == DIK_KeyCodes::KEY_LEFTARROW || kc == DIK_KeyCodes::KEY_RIGHTARROW) {
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
#endif
	}

} /* namespace input */
} /* namespace spine */
