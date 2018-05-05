// a maximum of 11 (1 active + 10 waiting in queue) achievements can be unlocked at once
// should be enough, otherwise I'll increase the amount
var int SPINE_ACHIEVEMENTSQUEUE[10];
var int Spine_AchievementView;
var int Spine_AchievementImageView;

var int Spine_UnlockAchievementFunc;
var int Spine_IsAchievementUnlockedFunc;
var int Spine_UpdateAchievementProgressFunc;
var int Spine_GetAchievementProgressFunc;
var int Spine_GetAchievementMaxProgressFunc;
var int Spine_GetShowAchievementsFunc;

var int SPINE_ACHIEVEMENTS_LOCAL_UNLOCKED[MAX_ACHIEVEMENTS];
var int SPINE_ACHIEVEMENTS_LOCAL_PROGRESS[MAX_ACHIEVEMENTS];

// return TRUE or FALSE whether the achievement for the given id is already unlocked or not
func int Spine_IsAchievementUnlocked(var int identifier) {
	if (Spine_Initialized && Spine_IsAchievementUnlockedFunc) {
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_IsAchievementUnlockedFunc);
		return CALL_RetValAsInt();
	} else if (Spine_Initialized && Spine_StartedWithoutSpine) {
		return MEM_ReadStatArr(SPINE_ACHIEVEMENTS_LOCAL_UNLOCKED, identifier);
	};
	return TRUE;
};

// private, don't call from outside
func void Spine_ShowAchievementView(var int identifier) {
	var zCPar_Symbol sym; sym = _^(MEMINT_SwitchG1G2(MEM_GetSymbol("_STR_FONT_ONSCREEN"), // Font constant G1
                                                 MEM_GetSymbol("FONT_SCREENSMALL"))); // Font constant G2
	var string Spine_Font; Spine_Font = MEM_ReadString(sym.content);
	var int startPosX;
	var int startPosY;
	var zCView screen; screen = _^(MEM_Game._zCSession_viewport);
	if (SPINE_ACHIEVEMENTORIENTATION == SPINE_TOPLEFT) {
		startPosX = 0;
		startPosY = 0;
	} else if (SPINE_ACHIEVEMENTORIENTATION == SPINE_TOPRIGHT) {
		startPosX = screen.psizex - SPINE_ACHIEVEMENT_WIDTH;
		startPosY = 0;
	} else if (SPINE_ACHIEVEMENTORIENTATION == SPINE_BOTTOMLEFT) {
		startPosX = 0;
		startPosY = screen.psizey - SPINE_ACHIEVEMENT_HEIGHT;
	} else if (SPINE_ACHIEVEMENTORIENTATION == SPINE_BOTTOMRIGHT) {
		startPosX = screen.psizex - SPINE_ACHIEVEMENT_WIDTH;
		startPosY = screen.psizey - SPINE_ACHIEVEMENT_HEIGHT;
	};
	Spine_AchievementView = View_CreatePxl(startPosX, startPosY, startPosX + SPINE_ACHIEVEMENT_WIDTH, startPosY + SPINE_ACHIEVEMENT_HEIGHT);
	View_SetTexture(Spine_AchievementView, "ACHIEVEMENT_BACKGROUND.TGA");
	View_Open(Spine_AchievementView);
	
	var int imageStartPosX;
	var int imageStartPosY;
	var int imageOffset;
	imageOffset = (SPINE_ACHIEVEMENT_HEIGHT - SPINE_ACHIEVEMENT_IMAGE_HEIGHT) / 2;
	imageStartPosX = startPosX + imageOffset;
	imageStartPosY = startPosY + imageOffset;
	Spine_AchievementImageView = View_CreatePxl(imageStartPosX, imageStartPosY, imageStartPosX + SPINE_ACHIEVEMENT_IMAGE_WIDTH, imageStartPosY + SPINE_ACHIEVEMENT_IMAGE_HEIGHT);
	var string achievementTexture;
	achievementTexture = MEM_ReadStatStringArr(SPINE_ACHIEVEMENT_TEXTURES, identifier);
	View_SetTexture(Spine_AchievementImageView, achievementTexture);
	View_Open(Spine_AchievementImageView);
	
	var string achievementName;
	achievementName = MEM_ReadStatStringArr(SPINE_ACHIEVEMENT_NAMES, identifier);
	var int textLength; textLength = Print_GetStringWidth(achievementName, Spine_Font);
	if (textLength <= SPINE_ACHIEVEMENT_WIDTH - (SPINE_ACHIEVEMENT_IMAGE_WIDTH + imageOffset * 3)) {
		View_AddText(Spine_AchievementView, Print_ToVirtual(SPINE_ACHIEVEMENT_IMAGE_WIDTH + imageOffset * 2, SPINE_ACHIEVEMENT_WIDTH), Print_ToVirtual(imageOffset, SPINE_ACHIEVEMENT_HEIGHT), achievementName, Spine_Font);
	} else { // Text is too long for the field
		var int height; height = 0;
		var int splitCount; splitCount = STR_SplitCount(achievementName, " ");
		var int i; i = 0;
		var string currentLine; currentLine = "";
		var int pos; pos = MEM_StackPos.position;
		if (i < splitCount) {
			var string currentLineTest;
			if (STR_Len(currentLine) > 0) {
				currentLine = ConcatStrings(currentLine, " ");
			};
			currentLineTest = ConcatStrings(currentLine, STR_Split(achievementName, " ", i));
			textLength = Print_GetStringWidth(currentLineTest, Spine_Font);
			if (textLength > SPINE_ACHIEVEMENT_WIDTH - (SPINE_ACHIEVEMENT_IMAGE_WIDTH + imageOffset * 3)) {
				if (STR_Len(currentLine) == 0) {
					View_AddText(Spine_AchievementView, Print_ToVirtual(SPINE_ACHIEVEMENT_IMAGE_WIDTH + imageOffset * 2, SPINE_ACHIEVEMENT_WIDTH), Print_ToVirtual(imageOffset + height, SPINE_ACHIEVEMENT_HEIGHT), currentLineTest, Spine_Font);
					currentLine = "";
					currentLineTest = "";
				} else {
					View_AddText(Spine_AchievementView, Print_ToVirtual(SPINE_ACHIEVEMENT_IMAGE_WIDTH + imageOffset * 2, SPINE_ACHIEVEMENT_WIDTH), Print_ToVirtual(imageOffset + height, SPINE_ACHIEVEMENT_HEIGHT), currentLine, Spine_Font);
					currentLine = STR_Split(achievementName, " ", i);
					currentLineTest = "";
				};
				height += Print_GetFontHeight(Spine_Font);
			} else {
				currentLine = currentLineTest;
			};
			i += 1;
			MEM_StackPos.position = pos;
		};
		if (STR_Len(currentLine) > 0) {
			View_AddText(Spine_AchievementView, Print_ToVirtual(SPINE_ACHIEVEMENT_IMAGE_WIDTH + imageOffset * 2, SPINE_ACHIEVEMENT_WIDTH), Print_ToVirtual(imageOffset + height, SPINE_ACHIEVEMENT_HEIGHT), currentLine, Spine_Font);
		};
	};
	
	FF_Remove(Spine_RemoveAchievementView);
	FF_ApplyOnceExt(Spine_RemoveAchievementView, SPINE_ACHIEVEMENT_DISPLAY_TIME, 1);
};

// unlocks achievement for this mod for given id
// contact Bonne to get your achievements on the server
// will also automatically create a view informing about unlocking the achievement
// unless it is disabled via setting SPINE_SHOWACHIEVEMENTS = FALSE
// place on the screen can be changed setting SPINE_ACHIEVEMENTORIENTATION
func void Spine_UnlockAchievement(var int identifier) {
	var int i;
	var int pos;
	var int hndl;
	if (Spine_Initialized && Spine_UnlockAchievementFunc) {
		if (!Spine_IsAchievementUnlocked(identifier)) {
			CALL_IntParam(identifier);
			CALL__cdecl(Spine_UnlockAchievementFunc);
			
			if (Spine_AchievementView == 0) {
				Spine_ShowAchievementView(identifier);
			} else {
				i = 0;
				pos = MEM_StackPos.position;
	
				hndl = MEM_ReadStatArr(SPINE_ACHIEVEMENTSQUEUE, i);
				
				if (hndl == -1) {
					MEM_WriteStatArr(SPINE_ACHIEVEMENTSQUEUE, i, identifier);
				} else {
					i += 1;
					if (i < 10) {
						MEM_StackPos.position = pos;
					};
				};
			};
		};
	} else if (Spine_Initialized && Spine_StartedWithoutSpine) {
		if (!Spine_IsAchievementUnlocked(identifier)) {
			MEM_WriteStatArr(SPINE_ACHIEVEMENTS_LOCAL_UNLOCKED, identifier, TRUE);
			MEM_SetGothOpt("SPINE", ConcatStrings(ConcatStrings(SPINE_MODNAME, "_achievement_"), IntToString(identifier)), "1");
			
			if (Spine_AchievementView == 0) {
				Spine_ShowAchievementView(identifier);
			} else {
				i = 0;
				pos = MEM_StackPos.position;
	
				hndl = MEM_ReadStatArr(SPINE_ACHIEVEMENTSQUEUE, i);
				
				if (hndl == -1) {
					MEM_WriteStatArr(SPINE_ACHIEVEMENTSQUEUE, i, identifier);
				} else {
					i += 1;
					if (i < 10) {
						MEM_StackPos.position = pos;
					};
				};
			};
		};
	};
};

// updates achievement progress
// in case maximum achievement progress is reached, the achievement is displayed as unlocked automatically
func void Spine_UpdateAchievementProgress(var int identifier, var int progress) {
	var int i;
	var int ret;
	var int pos;
	var int hndl;
	var int maxProgress;
	var string maxProgressString;
	
	if (Spine_Initialized && Spine_UpdateAchievementProgressFunc) {
		if (!Spine_IsAchievementUnlocked(identifier)) {
			CALL_IntParam(progress);
			CALL_IntParam(identifier);
			CALL__cdecl(Spine_UpdateAchievementProgressFunc);
			ret = CALL_RetValAsInt();
			
			if (Spine_GetTestMode()) {
				maxProgressString = MEM_ReadStatStringArr(SPINE_ACHIEVEMENT_PROGRESS, identifier);
				maxProgress = STR_ToInt(maxProgressString);
				if (maxProgress <= progress) {
					ret = TRUE;
				};
			};
			
			if (ret) {
				if (Spine_AchievementView == 0) {
					Spine_ShowAchievementView(identifier);
				} else {
					i = 0;
					pos = MEM_StackPos.position;
		
					hndl = MEM_ReadStatArr(SPINE_ACHIEVEMENTSQUEUE, i);
					
					if (hndl == -1) {
						MEM_WriteStatArr(SPINE_ACHIEVEMENTSQUEUE, i, identifier);
					} else {
						i += 1;
						if (i < 10) {
							MEM_StackPos.position = pos;
						};
					};
				};
			};
		};
	} else if (Spine_Initialized && Spine_StartedWithoutSpine) {
		if (!Spine_IsAchievementUnlocked(identifier)) {
			MEM_WriteStatArr(SPINE_ACHIEVEMENTS_LOCAL_PROGRESS, identifier, progress);
			
			if (Spine_GetTestMode() || Spine_StartedWithoutSpine) {
				maxProgressString = MEM_ReadStatStringArr(SPINE_ACHIEVEMENT_PROGRESS, identifier);
				maxProgress = STR_ToInt(maxProgressString);
				if (maxProgress <= progress) {
					ret = TRUE;
				};
			};
			
			if (ret) {
				Spine_UnlockAchievement(identifier);
			};
		};
	};
};

// returns the current progress of an achievement or 0 in case it is an achievement without progress
func int Spine_GetAchievementProgress(var int identifier) {
	if (Spine_Initialized && Spine_GetAchievementProgressFunc) {
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_GetAchievementProgressFunc);
		return CALL_RetValAsInt();
	} else if (Spine_Initialized && Spine_StartedWithoutSpine) {
		return MEM_ReadStatArr(SPINE_ACHIEVEMENTS_LOCAL_PROGRESS, identifier);
	};
	return FALSE;
};

// returns the maximum progress to reach of an achievement or 0 in case it is an achievement without progress
func int Spine_GetAchievementMaxProgress(var int identifier) {
	if (Spine_Initialized && (Spine_GetAchievementMaxProgressFunc || Spine_StartedWithoutSpine)) {
		if (Spine_GetTestMode() || Spine_StartedWithoutSpine) {
			var string maxProgressString; maxProgressString = MEM_ReadStatStringArr(SPINE_ACHIEVEMENT_PROGRESS, identifier);
			var int maxProgress; maxProgress = STR_ToInt(maxProgressString);
			return maxProgress;
		} else {
			CALL_IntParam(identifier);
			CALL__cdecl(Spine_GetAchievementMaxProgressFunc);
			return CALL_RetValAsInt();
		};
	};
	return FALSE;
};

// private, don't call from outside
func void Spine_RemoveAchievementView() {
	if (Hlp_IsValidHandle(Spine_AchievementView)) {
		View_Delete(Spine_AchievementView);
		Spine_AchievementView = 0;
	};
	if (Hlp_IsValidHandle(Spine_AchievementImageView)) {
		View_Delete(Spine_AchievementImageView);
		Spine_AchievementImageView = 0;
	};
	FF_Remove(Spine_ShowAchievementView);
	if (SPINE_ACHIEVEMENTSQUEUE[0] != -1) {
		var int identifier;
		identifier = SPINE_ACHIEVEMENTSQUEUE[0];
		var int i; i = 0;
		var int pos; pos = MEM_StackPos.position;

		var int hndl; hndl = MEM_ReadStatArr(SPINE_ACHIEVEMENTSQUEUE, i);
		
		if (hndl != -1) {
			MEM_WriteStatArr(SPINE_ACHIEVEMENTSQUEUE, i, MEM_ReadStatArr(SPINE_ACHIEVEMENTSQUEUE, i + 1));
			i += 1;
			if (i < 9) {
				MEM_StackPos.position = pos;
			};
		};
		MEM_WriteStatArr(SPINE_ACHIEVEMENTSQUEUE, i, -1);
		FF_ApplyOnceExtData(Spine_ShowAchievementView, 0, 1, identifier);
	};
};
