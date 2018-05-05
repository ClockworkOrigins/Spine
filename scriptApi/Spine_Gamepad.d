const int SPINE_VIBRATION_MINIMUM = 0;
const int SPINE_VIBRATION_MAXIMUM = 65535;

var int Spine_VibrateGamepadFunc;
var int Spine_IsGamepadEnabledFunc;
var int Spine_IsGamepadActiveFunc;
var int Spine_GetGamepadButtonStateFunc;
var int Spine_GetGamepadTriggerStateFunc;
var int Spine_GetGamepadStickStateFunc;
var int Spine_ChangeRawModeFunc;

// Gamepad buttons
const int SPINE_GAMEPAD_BUTTON_DPAD_UP = 0;
const int SPINE_GAMEPAD_BUTTON_DPAD_DOWN = 1;
const int SPINE_GAMEPAD_BUTTON_DPAD_LEFT = 2;
const int SPINE_GAMEPAD_BUTTON_DPAD_RIGHT = 3;
const int SPINE_GAMEPAD_BUTTON_START = 4;
const int SPINE_GAMEPAD_BUTTON_BACK = 5;
const int SPINE_GAMEPAD_BUTTON_LEFT_THUMB = 6;
const int SPINE_GAMEPAD_BUTTON_RIGHT_THUMB = 7;
const int SPINE_GAMEPAD_BUTTON_LEFT_SHOULDER = 8;
const int SPINE_GAMEPAD_BUTTON_RIGHT_SHOULDER = 9;
const int SPINE_GAMEPAD_BUTTON_A = 10;
const int SPINE_GAMEPAD_BUTTON_B = 11;
const int SPINE_GAMEPAD_BUTTON_X = 12;
const int SPINE_GAMEPAD_BUTTON_Y = 13;
const int SPINE_GAMEPAD_LTrigger = 14;
const int SPINE_GAMEPAD_RTrigger = 15;
const int SPINE_GAMEPAD_LSTICK_X_POS = 16;
const int SPINE_GAMEPAD_LSTICK_X_NEG = 17;
const int SPINE_GAMEPAD_LSTICK_Y_POS = 18;
const int SPINE_GAMEPAD_LSTICK_Y_NEG = 19;
const int SPINE_GAMEPAD_RSTICK_X_POS = 20;
const int SPINE_GAMEPAD_RSTICK_X_NEG = 21;
const int SPINE_GAMEPAD_RSTICK_Y_POS = 22;
const int SPINE_GAMEPAD_RSTICK_Y_NEG = 23;

// constants to use for identifying triggers and sticks
const int SPINE_GAMEPAD_LEFT = 0;
const int SPINE_GAMEPAD_RIGHT = 1;
const int SPINE_GAMEPAD_X = 0;
const int SPINE_GAMEPAD_Y = 1;

// cause vibration on Controller
// both leftMotor and rightMotor are in range [0, 65535]
// values smaller than 0 and bigger 65535 are clamped
func void Spine_VibrateGamepad(var int leftMotor, var int rightMotor) {
	if (Spine_Initialized && Spine_VibrateGamepadFunc) {
		leftMotor = clamp(leftMotor, SPINE_VIBRATION_MINIMUM, SPINE_VIBRATION_MAXIMUM);
		rightMotor = clamp(rightMotor, SPINE_VIBRATION_MINIMUM, SPINE_VIBRATION_MAXIMUM);
		CALL_IntParam(rightMotor);
		CALL_IntParam(leftMotor);
		CALL__cdecl(Spine_VibrateGamepadFunc);
	};
};

// returns TRUE if a Gamepad is configured and enabled
// this doesn't mean the Gamepad is really in use, it's just available
func int Spine_IsGamepadEnabled() {
	if (Spine_Initialized && Spine_IsGamepadEnabledFunc) {
		CALL__cdecl(Spine_IsGamepadEnabledFunc);
		return CALL_RetValAsInt();
	};
	return FALSE;
};

// returns TRUE if a Gamepad is actively used
func int Spine_IsGamepadActive() {
	if (Spine_Initialized && Spine_IsGamepadActiveFunc) {
		CALL__cdecl(Spine_IsGamepadActiveFunc);
		return CALL_RetValAsInt();
	};
	return FALSE;
};

// returns the state of the given gamepad button
func int Spine_GetGamepadButtonState(var int button) {
	if (Spine_Initialized && Spine_GetGamepadButtonStateFunc) {
		CALL_IntParam(button);
		CALL__cdecl(Spine_GetGamepadButtonStateFunc);
		var int result; result = CALL_RetValAsInt();
		if (result) {
			return KEY_PRESSED;
		} else {
			return KEY_UP;
		};
	};
	return KEY_UP;
};

// returns the state of the given gamepad trigger
// SPINE_GAMEPAD_LEFT and SPINE_GAMEPAD_RIGHT
// result in range [0, 255]
func int Spine_GetGamepadTriggerState(var int trigger) {
	if (Spine_Initialized && Spine_GetGamepadTriggerStateFunc) {
		CALL_IntParam(trigger);
		CALL__cdecl(Spine_GetGamepadTriggerStateFunc);
		var int result; result = CALL_RetValAsInt();
		return result;
	};
	return 0;
};

// returns the state of the given gamepad stick
// SPINE_GAMEPAD_LEFT and SPINE_GAMEPAD_RIGHT for the two sticks
// SPINE_GAMEPAD_X and SPINE_GAMEPAD_Y for the two directions
// result is an integer value
func int Spine_GetGamepadStickState(var int stick, var int axis) {
	if (Spine_Initialized && Spine_GetGamepadStickStateFunc) {
		CALL_IntParam(axis);
		CALL_IntParam(stick);
		CALL__cdecl(Spine_GetGamepadStickStateFunc);
		var int result; result = CALL_RetValAsInt();
		return result;
	};
	return 0;
};

// sets whether raw mode shall be used or not
// raw mode means that pressing a gamepad button will be handled only by the modification
// so no automatic keymapping is done by Spine itself
func void Spine_ChangeRawMode(var int rawModeEnabled) {
	if (Spine_Initialized && Spine_ChangeRawModeFunc) {
		CALL_IntParam(rawModeEnabled);
		CALL__cdecl(Spine_ChangeRawModeFunc);
	};
};

// call this to trigger vibration when hero gets damage
// vibration strength depends on damage taken
func void Spine_DoDamageVibration(var int dmg) {
	var int oldDmg;
	if (FF_Active(Spine_StopDamageVibration)) {
		FF_Remove(Spine_StopDamageVibration);
	} else {
		oldDmg = 0;
	};
	dmg = max(dmg, oldDmg);
	Spine_VibrateGamepad(clamp((dmg * SPINE_VIBRATION_MAXIMUM) / 100, 0, SPINE_VIBRATION_MAXIMUM), clamp((dmg * SPINE_VIBRATION_MAXIMUM) / 100, 0, SPINE_VIBRATION_MAXIMUM));
	if (FF_Active(Spine_StopDamageVibration)) {
		FF_Remove(Spine_StopDamageVibration);
	};
	FF_ApplyOnceExt(Spine_StopDamageVibration, SPINE_GAME_DAMAGE_VIBRATION_TIME, 1);
};

// internal function, don't call from outside
func void Spine_InitEarthquakeHooks() {
	if (GOTHIC_BASE_VERSION == 1 && SPINE_EARTHQUAKE_VIBRATION) {
		HookEngine (SPINE_ADDRESS_TRIGGEREARTHQUAKE_G1, SPINE_LENGTH_TRIGGEREARTHQUAKE_G1, "SPINE_EARTHQUAKE_TRIGGER");
		HookEngine (SPINE_ADDRESS_UNTRIGGEREARTHQUAKE_G1, SPINE_LENGTH_UNTRIGGEREARTHQUAKE_G1, "SPINE_EARTHQUAKE_UNTRIGGER");
	} else if (GOTHIC_BASE_VERSION == 2 && SPINE_EARTHQUAKE_VIBRATION) {
		HookEngine (SPINE_ADDRESS_TRIGGEREARTHQUAKE_G2, SPINE_LENGTH_TRIGGEREARTHQUAKE_G2, "SPINE_EARTHQUAKE_TRIGGER");
		HookEngine (SPINE_ADDRESS_UNTRIGGEREARTHQUAKE_G2, SPINE_LENGTH_UNTRIGGEREARTHQUAKE_G2, "SPINE_EARTHQUAKE_UNTRIGGER");
	};
};

// internal function, don't call from outside
func void Spine_Earthquake_Trigger() {
	Spine_VibrateGamepad(SPINE_VIBRATION_MAXIMUM, SPINE_VIBRATION_MAXIMUM);
	if (FF_Active(Spine_Earthquake_Untrigger)) {
		FF_Remove(Spine_Earthquake_Untrigger);
	};
	FF_ApplyOnceExt(Spine_Earthquake_Untrigger, 2000, 1);
};

// internal function, don't call from outside
func void Spine_Earthquake_Untrigger() {
	Spine_VibrateGamepad(SPINE_VIBRATION_MINIMUM, SPINE_VIBRATION_MINIMUM);
};

// internal function, don't call from outside
func void Spine_StopDamageVibration() {
	Spine_VibrateGamepad(SPINE_VIBRATION_MINIMUM, SPINE_VIBRATION_MINIMUM);
};
