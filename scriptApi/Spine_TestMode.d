var int Spine_GetTestModeFunc;

// returns TRUE if mod is started in testmode, otherwise FALSE
func int Spine_GetTestMode() {
	if (Spine_Initialized && Spine_GetTestModeFunc) {
		CALL__cdecl(Spine_GetTestModeFunc);
		return CALL_RetValAsInt();
	};
	return FALSE;
};
