var string Spine_FirstStart;
var int Spine_Initialized;
var int Spine_Dll;
var int Spine_InitFunc;
var int Spine_GetUsernameFunc;
var int Spine_StartedWithoutSpine;

func void Spine_ResetBeforeInit() {
	if (Hlp_IsValidHandle(Spine_AchievementView)) {
		View_Delete(Spine_AchievementView);
		Spine_AchievementView = 0;
	};
	if (Hlp_IsValidHandle(Spine_AchievementImageView)) {
		View_Delete(Spine_AchievementImageView);
		Spine_AchievementImageView = 0;
	};
	SPINE_ACHIEVEMENTSQUEUE[0] = -1;
	SPINE_ACHIEVEMENTSQUEUE[1] = -1;
	SPINE_ACHIEVEMENTSQUEUE[2] = -1;
	SPINE_ACHIEVEMENTSQUEUE[3] = -1;
	SPINE_ACHIEVEMENTSQUEUE[4] = -1;
	SPINE_ACHIEVEMENTSQUEUE[5] = -1;
	SPINE_ACHIEVEMENTSQUEUE[6] = -1;
	SPINE_ACHIEVEMENTSQUEUE[7] = -1;
	SPINE_ACHIEVEMENTSQUEUE[8] = -1;
	SPINE_ACHIEVEMENTSQUEUE[9] = -1;
	SPINE_SHOWACHIEVEMENTS = TRUE;
	
	FF_Remove(Spine_ShowAchievementView);
	FF_Remove(Spine_RemoveAchievementView);
	
	Spine_Initialized = FALSE;
	Spine_StartedWithoutSpine = FALSE;
};

func int Spine_InitWithDll(var int modules) {
	MEM_Info("Spine: Loading init function");
	Spine_InitFunc = GetProcAddress(Spine_Dll, "init");
	
	if (!Spine_InitFunc) {
		MEM_Info("Spine: init function not found");
		FreeLibrary(Spine_Dll);
		return FALSE;
	};
	
	MEM_Info("Spine: Loading getTestMode function");
	Spine_GetTestModeFunc = GetProcAddress(Spine_Dll, "getTestMode");
	
	if (!Spine_GetTestModeFunc) {
		MEM_Info("Spine: getTestMode function not found");
		FreeLibrary(Spine_Dll);
		return FALSE;
	};
	
	if (modules & SPINE_MODULE_GETCURRENTUSERNAME) {
		MEM_Info("Spine: Loading getUsername function");
		Spine_GetUsernameFunc = GetProcAddress(Spine_Dll, "getUsername");
		
		if (!Spine_GetUsernameFunc) {
			MEM_Info("Spine: getUsername function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_GetUsernameFunc = 0;
	};
	
	if (modules & SPINE_MODULE_SCORES) {
		MEM_Info("Spine: Loading updateScore function");
		Spine_UpdateScoreFunc = GetProcAddress(Spine_Dll, "updateScore");
		
		if (!Spine_UpdateScoreFunc) {
			MEM_Info("Spine: updateScore function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getUserScore function");
		Spine_GetUserScoreFunc = GetProcAddress(Spine_Dll, "getUserScore");
		
		if (!Spine_GetUserScoreFunc) {
			MEM_Info("Spine: getUserScore function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getUserRank function");
		Spine_GetUserRankFunc = GetProcAddress(Spine_Dll, "getUserRank");
		
		if (!Spine_GetUserRankFunc) {
			MEM_Info("Spine: getUserRank function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getScoreForRank function");
		Spine_GetScoreForRankFunc = GetProcAddress(Spine_Dll, "getScoreForRank");
		
		if (!Spine_GetScoreForRankFunc) {
			MEM_Info("Spine: getScoreForRank function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getScoreForUsername function");
		Spine_GetScoreForUsernameFunc = GetProcAddress(Spine_Dll, "getScoreForUsername");
		
		if (!Spine_GetScoreForUsernameFunc) {
			MEM_Info("Spine: getScoreForUsername function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getUsernameForRank function");
		Spine_GetUsernameForRankFunc = GetProcAddress(Spine_Dll, "getUsernameForRank");
		
		if (!Spine_GetUsernameForRankFunc) {
			MEM_Info("Spine: getUsernameForRank function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_UpdateScoreFunc = 0;
		Spine_GetUserScoreFunc = 0;
		Spine_GetUserRankFunc = 0;
		Spine_GetScoreForRankFunc = 0;
		Spine_GetUsernameForRankFunc = 0;
	};
	
	if (modules & SPINE_MODULE_ACHIEVEMENTS) {
		if (!(_LeGo_Flags & LeGo_FrameFunctions) || !(_LeGo_Flags & LeGo_View)) {
			MEM_ErrorBox("For Spine Achievement Module you need to initialize LeGo with both FrameFunctions and View");
			FreeLibrary(Spine_Dll);
			return FALSE;
			
		};
		MEM_Info("Spine: Loading unlockAchievement function");
		Spine_UnlockAchievementFunc = GetProcAddress(Spine_Dll, "unlockAchievement");
		
		if (!Spine_UnlockAchievementFunc) {
			MEM_Info("Spine: unlockAchievement function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading isAchievementUnlocked function");
		Spine_IsAchievementUnlockedFunc = GetProcAddress(Spine_Dll, "isAchievementUnlocked");
		
		if (!Spine_IsAchievementUnlockedFunc) {
			MEM_Info("Spine: isAchievementUnlocked function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading updateAchievementProgress function");
		Spine_UpdateAchievementProgressFunc = GetProcAddress(Spine_Dll, "updateAchievementProgress");
		
		if (!Spine_UpdateAchievementProgressFunc) {
			MEM_Info("Spine: updateAchievementProgress function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getAchievementProgress function");
		Spine_GetAchievementProgressFunc = GetProcAddress(Spine_Dll, "getAchievementProgress");
		
		if (!Spine_GetAchievementProgressFunc) {
			MEM_Info("Spine: getAchievementProgress function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getAchievementMaxProgress function");
		Spine_GetAchievementMaxProgressFunc = GetProcAddress(Spine_Dll, "getAchievementMaxProgress");
		
		if (!Spine_GetAchievementMaxProgressFunc) {
			MEM_Info("Spine: getAchievementMaxProgress function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getShowAchievements function");
		Spine_GetShowAchievementsFunc = GetProcAddress(Spine_Dll, "getShowAchievements");
		
		if (!Spine_GetShowAchievementsFunc) {
			MEM_Info("Spine: getShowAchievements function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading isAchievementOfOtherModUnlocked function");
		Spine_IsAchievementOfOtherModUnlockedFunc = GetProcAddress(Spine_Dll, "isAchievementOfOtherModUnlocked");
		
		if (!Spine_IsAchievementOfOtherModUnlockedFunc) {
			MEM_Info("Spine: isAchievementOfOtherModUnlocked function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_UnlockAchievementFunc = 0;
		Spine_IsAchievementUnlockedFunc = 0;
		Spine_IsAchievementOfOtherModUnlockedFunc = 0;
	};
	
	if (modules & SPINE_MODULE_MULTIPLAYER) {
		MEM_Info("Spine: Loading createMessage function");
		Spine_CreateMessageFunc = GetProcAddress(Spine_Dll, "createMessage");
		
		if (!Spine_CreateMessageFunc) {
			MEM_Info("Spine: createMessage function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading deleteMessage function");
		Spine_DeleteMessageFunc = GetProcAddress(Spine_Dll, "deleteMessage");
		
		if (!Spine_DeleteMessageFunc) {
			MEM_Info("Spine: deleteMessage function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading sendMessage function");
		Spine_SendMessageFunc = GetProcAddress(Spine_Dll, "sendMessage");
		
		if (!Spine_SendMessageFunc) {
			MEM_Info("Spine: sendMessage function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading receiveMessage function");
		Spine_ReceiveMessageFunc = GetProcAddress(Spine_Dll, "receiveMessage");
		
		if (!Spine_ReceiveMessageFunc) {
			MEM_Info("Spine: receiveMessage function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading searchMatch function");
		Spine_SearchMatchFunc = GetProcAddress(Spine_Dll, "searchMatch");
		
		if (!Spine_SearchMatchFunc) {
			MEM_Info("Spine: searchMatch function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading searchMatchWithFriend function");
		Spine_SearchMatchWithFriendFunc = GetProcAddress(Spine_Dll, "searchMatchWithFriend");
		
		if (!Spine_SearchMatchWithFriendFunc) {
			MEM_Info("Spine: searchMatchWithFriend function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading stopSearchMatch function");
		Spine_StopSearchMatchFunc = GetProcAddress(Spine_Dll, "stopSearchMatch");
		
		if (!Spine_StopSearchMatchFunc) {
			MEM_Info("Spine: stopSearchMatch function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading isInMatch function");
		Spine_IsInMatchFunc = GetProcAddress(Spine_Dll, "isInMatch");
		
		if (!Spine_IsInMatchFunc) {
			MEM_Info("Spine: isInMatch function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getPlayerCount function");
		Spine_GetPlayerCountFunc = GetProcAddress(Spine_Dll, "getPlayerCount");
		
		if (!Spine_GetPlayerCountFunc) {
			MEM_Info("Spine: getPlayerCount function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getPlayerUsername function");
		Spine_GetPlayerUsernameFunc = GetProcAddress(Spine_Dll, "getPlayerUsername");
		
		if (!Spine_GetPlayerUsernameFunc) {
			MEM_Info("Spine: getPlayerUsername function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading setHostname function");
		Spine_SetHostnameFunc = GetProcAddress(Spine_Dll, "setHostname");
		
		if (!Spine_SetHostnameFunc) {
			MEM_Info("Spine: setHostname function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading isOnline function");
		Spine_IsOnlineFunc = GetProcAddress(Spine_Dll, "isOnline");
		
		if (!Spine_IsOnlineFunc) {
			MEM_Info("Spine: isOnline function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_CreateMessageFunc = 0;
		Spine_SendMessageFunc = 0;
		Spine_ReceiveMessageFunc = 0;
		Spine_SearchMatchFunc = 0;
		Spine_IsInMatchFunc = 0;
		Spine_GetPlayerCountFunc = 0;
		Spine_GetPlayerUsernameFunc = 0;
	};
	
	if (modules & SPINE_MODULE_OVERALLSAVE) {
		MEM_Info("Spine: Loading setOverallSaveValue function");
		Spine_OverallSaveSetStringFunc = GetProcAddress(Spine_Dll, "setOverallSaveValue");
		
		if (!Spine_OverallSaveSetStringFunc) {
			MEM_Info("Spine: setOverallSaveValue function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getOverallSaveValue function");
		Spine_OverallSaveGetStringFunc = GetProcAddress(Spine_Dll, "getOverallSaveValue");
		
		if (!Spine_OverallSaveGetStringFunc) {
			MEM_Info("Spine: getOverallSaveValue function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		MEM_Info("Spine: Loading setOverallSaveValueInt function");
		Spine_OverallSaveSetIntFunc = GetProcAddress(Spine_Dll, "setOverallSaveValueInt");
		
		if (!Spine_OverallSaveSetIntFunc) {
			MEM_Info("Spine: setOverallSaveValueInt function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getOverallSaveValueInt function");
		Spine_OverallSaveGetIntFunc = GetProcAddress(Spine_Dll, "getOverallSaveValueInt");
		
		if (!Spine_OverallSaveGetIntFunc) {
			MEM_Info("Spine: getOverallSaveValueInt function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_OverallSaveSetStringFunc = 0;
		Spine_OverallSaveGetStringFunc = 0;
		Spine_OverallSaveSetIntFunc = 0;
		Spine_OverallSaveGetIntFunc = 0;
	};
	
	if (modules & SPINE_MODULE_GAMEPAD) {
		MEM_Info("Spine: Loading vibrateGamepad function");
		Spine_VibrateGamepadFunc = GetProcAddress(Spine_Dll, "vibrateGamepad");
		
		if (!Spine_VibrateGamepadFunc) {
			MEM_Info("Spine: vibrateGamepad function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading isGamepadEnabled function");
		Spine_IsGamepadEnabledFunc = GetProcAddress(Spine_Dll, "isGamepadEnabled");
		
		if (!Spine_IsGamepadEnabledFunc) {
			MEM_Info("Spine: isGamepadEnabled function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading isGamepadActive function");
		Spine_IsGamepadActiveFunc = GetProcAddress(Spine_Dll, "isGamepadActive");
		
		if (!Spine_IsGamepadActiveFunc) {
			MEM_Info("Spine: isGamepadActive function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getGamepadButtonState function");
		Spine_GetGamepadButtonStateFunc = GetProcAddress(Spine_Dll, "getGamepadButtonState");
		
		if (!Spine_GetGamepadButtonStateFunc) {
			MEM_Info("Spine: getGamepadButtonState function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getGamepadTriggerState function");
		Spine_GetGamepadTriggerStateFunc = GetProcAddress(Spine_Dll, "getGamepadTriggerState");
		
		if (!Spine_GetGamepadTriggerStateFunc) {
			MEM_Info("Spine: getGamepadTriggerState function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getGamepadStickState function");
		Spine_GetGamepadStickStateFunc = GetProcAddress(Spine_Dll, "getGamepadStickState");
		
		if (!Spine_GetGamepadStickStateFunc) {
			MEM_Info("Spine: getGamepadStickState function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading changeGamepadMode function");
		Spine_ChangeRawModeFunc = GetProcAddress(Spine_Dll, "changeGamepadMode");
		
		if (!Spine_ChangeRawModeFunc) {
			MEM_Info("Spine: changeGamepadMode function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_VibrateGamepadFunc = 0;
		Spine_IsGamepadEnabledFunc = 0;
		Spine_IsGamepadActiveFunc = 0;
		Spine_GetGamepadButtonStateFunc = 0;
		Spine_GetGamepadTriggerStateFunc = 0;
		Spine_GetGamepadStickStateFunc = 0;
		Spine_ChangeRawModeFunc = 0;
	};
	
	if (modules & SPINE_MODULE_FRIENDS) {
		MEM_Info("Spine: Loading getFriendCount function");
		Spine_GetFriendCountFunc = GetProcAddress(Spine_Dll, "getFriendCount");
		
		if (!Spine_GetFriendCountFunc) {
			MEM_Info("Spine: getFriendCount function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
		
		MEM_Info("Spine: Loading getFriendName function");
		Spine_GetFriendNameFunc = GetProcAddress(Spine_Dll, "getFriendName");
		
		if (!Spine_GetFriendNameFunc) {
			MEM_Info("Spine: getFriendName function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_GetFriendCountFunc = 0;
		Spine_GetFriendNameFunc = 0;
	};
	
	if (modules & SPINE_MODULE_STATISTICS) {
		MEM_Info("Spine: Loading updateStatistic function");
		Spine_UpdateStatisticFunc = GetProcAddress(Spine_Dll, "updateStatistic");
		
		if (!Spine_UpdateStatisticFunc) {
			MEM_Info("Spine: updateStatistic function not found");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_UpdateStatisticFunc = 0;
	};
	
	if (STR_Len(Spine_FirstStart) == 0) {
		Spine_FirstStart = "Initialized";
		
		if (modules & SPINE_MODULE_GAMEPAD) {
			Spine_InitEarthquakeHooks();
		};
		
		MEM_Info("Spine: Calling init function");
		CALL_IntParam(modules);
		CALL__cdecl(Spine_InitFunc);
		Spine_Initialized = CALL_RetValAsInt();
		
		if (!Spine_Initialized) {
			MEM_Info("Spine: init function failed");
			FreeLibrary(Spine_Dll);
			return FALSE;
		};
	} else {
		Spine_Initialized = TRUE;
	};
	
	if (Spine_GetShowAchievementsFunc) {
		CALL__cdecl(Spine_GetShowAchievementsFunc);
		SPINE_SHOWACHIEVEMENTS = CALL_RetValAsInt();
	};
	
	return Spine_Initialized;
};

func int Spine_InitWithoutDll(var int modules) {
	MEM_Info("Spine: Loading init function");
	Spine_InitFunc = 0;
	
	Spine_GetTestModeFunc = 0;
	Spine_GetUsernameFunc = 0;
	
	Spine_UpdateScoreFunc = 0;
	Spine_GetUserScoreFunc = 0;
	Spine_GetUserRankFunc = 0;
	Spine_GetScoreForRankFunc = 0;
	Spine_GetUsernameForRankFunc = 0;
	
	if (modules & SPINE_MODULE_ACHIEVEMENTS) {
		if (!(_LeGo_Flags & LeGo_FrameFunctions) || !(_LeGo_Flags & LeGo_View)) {
			MEM_ErrorBox("For Spine Achievement Module you need to initialize LeGo with both FrameFunctions and View");
			return FALSE;
			
		};
		
		var int i; i = 0;
		var int pos; pos = MEM_StackPos.position;
		if (i < MAX_ACHIEVEMENTS) {
			var int achievementUnlocked; achievementUnlocked = STR_ToInt(MEM_GetGothOpt("SPINE", ConcatStrings(ConcatStrings(SPINE_MODNAME, "_achievement_"), IntToString(i))));
			MEM_WriteStatArr(SPINE_ACHIEVEMENTS_LOCAL_UNLOCKED, i, achievementUnlocked);
			i += 1;
			MEM_StackPos.position = pos;
		};
	};
	Spine_UnlockAchievementFunc = 0;
	Spine_IsAchievementUnlockedFunc = 0;
	
	Spine_CreateMessageFunc = 0;
	Spine_SendMessageFunc = 0;
	Spine_ReceiveMessageFunc = 0;
	Spine_SearchMatchFunc = 0;
	Spine_IsInMatchFunc = 0;
	Spine_GetPlayerCountFunc = 0;
	Spine_GetPlayerUsernameFunc = 0;
	
	Spine_OverallSaveSetStringFunc = 0;
	Spine_OverallSaveGetStringFunc = 0;
	Spine_OverallSaveSetIntFunc = 0;
	Spine_OverallSaveGetIntFunc = 0;
	
	Spine_VibrateGamepadFunc = 0;
	Spine_IsGamepadEnabledFunc = 0;
	Spine_IsGamepadActiveFunc = 0;
	Spine_GetGamepadButtonStateFunc = 0;
	Spine_GetGamepadTriggerStateFunc = 0;
	Spine_GetGamepadStickStateFunc = 0;
	Spine_ChangeRawModeFunc = 0;
	
	Spine_GetFriendCountFunc = 0;
	Spine_GetFriendNameFunc = 0;
	
	Spine_UpdateStatisticFunc = 0;
	
	if (STR_Len(Spine_FirstStart) == 0) {
		Spine_FirstStart = "Initialized";
	};
	Spine_Initialized = TRUE;
	Spine_StartedWithoutSpine = TRUE;
	
	SPINE_SHOWACHIEVEMENTS = STR_ToInt(MEM_GetGothOpt("SPINE", "showAchievements"));
	
	return Spine_Initialized;
};

func int Spine_Init(var int modules) {
	Spine_ResetBeforeInit();
	
	MEM_Info("Spine: Initializing");
	
	MEM_Info("Spine: Loading SpineAPI.dll");
	Spine_Dll = LoadLibrary("SpineAPI.dll");
	
	if (!Spine_Dll) {
		MEM_Info("Spine: SpineAPI.dll couldn't be loaded");
		return Spine_InitWithoutDll(modules);
	};
	
	return Spine_InitWithDll(modules);
};

// returns the username of the user currently logged in
// if played without account/login, empty string is returned
func string Spine_GetCurrentUsername() {
	if (Spine_Initialized && Spine_GetUsernameFunc) {
		const string STR_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

		CALL_cStringPtrParam(STR_BUFFER);
		CALL__cdecl(Spine_GetUsernameFunc);
		
		return STR_BUFFER;
	};
	return "";
};
