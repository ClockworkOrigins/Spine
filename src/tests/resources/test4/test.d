//Menü-Update-Script

// [INTERAL]
    func void Update_Menu_Item(var string name, var string val) {
        var int itPtr;
        itPtr = MEM_GetMenuItemByString(name);
        
        if (!itPtr) {
            MEM_Error(ConcatStrings("Update_Menu_Item: Invalid Menu Item: ", name));
            return;
        };
        
        //void __thiscall zCMenuItem::SetText(val = val, line = 0, drawNow = true)
        const int SetText = 5114800;
        
        CALL_IntParam(true);
        CALL_IntParam(0);
        CALL_zStringPtrParam(val);
        CALL__thiscall(itPtr, SetText);
    };
    
    //Call this is INIT_GLOBAL after LeGo_Init.
    func void Install_Character_Menu_Hook() {
        //at oCMenu_Status::SetLearnPoints
        const int done = false;
        if(!done) {
			HookEngineF(7592320, 7, changeStatusPage);
			HookEngineF(4714092, 6, CloseStatusScreen);
			/*HookEngineF(7742032, 6, startInfoMan);
			HookEngineF(7742480, 9, closeInfoMan);
			HookEngineF(7387888, 7, closeInfoMan);*/
			//HookEngineF(7621056, 6, Npc_TakeVob);
            done = true;
        };
    };
// [/INTERNAL]

// Fill in this function according to your needs:
/*func void Update_Character_Menu() {
    /* Usage:
        Update_Menu_Item([Name of the Menu Item],
                         [new String value]);
    */
    
    //Replace heading with number of coins
    /*Update_Menu_Item ("MENU_BERUF_TITLE", "BERUFE");
    var string skilllevel;
    if(jobCounter == 0)
    {
    	Update_Menu_Item("MENU_BERUF1_TITLE", "Keine Berufe gelernt!");	
    }    
    else
    {
    	if(learnedBergbau == TRUE)
	    {
	    	skilllevel = IntToString(bergbauSkill);
	    	skilllevel = ConcatStrings(skilllevel, "/60");
	    	Update_Menu_Item ("MENU_BERUF1_TITLE", "Bergbau");
	    	Update_Menu_Item ("MENU_BERUF1_SKILL", skilllevel);
	    }; 
		if(learnedKuerschner == TRUE)
	    {
	    	skilllevel = IntToString(kuerschnerSkill);
	    	skilllevel = ConcatStrings(skilllevel, "/60");
	    	Update_Menu_Item ("MENU_BERUF1_TITLE", "Kürschnerei");
	    	Update_Menu_Item ("MENU_BERUF1_SKILL", skilllevel);
	    };
		if(learnedHerb == TRUE)
	    {
	    	skilllevel = IntToString(herbSkill);
	    	skilllevel = ConcatStrings(skilllevel, "/60");
	    	Update_Menu_Item ("MENU_BERUF1_TITLE", "Kräuterkunde");
	    	Update_Menu_Item ("MENU_BERUF1_SKILL", skilllevel);
	    };
		if(learnedSmith == TRUE)
	    {
	    	skilllevel = IntToString(smithSkill);
	    	skilllevel = ConcatStrings(skilllevel, "/60");
	    	Update_Menu_Item ("MENU_BERUF2_TITLE", "Schmiedekunst");
	    	Update_Menu_Item ("MENU_BERUF2_SKILL", skilllevel);
	    };
		if(learnedLeather == TRUE)
	    {
	    	skilllevel = IntToString(leatherSkill);
	    	skilllevel = ConcatStrings(skilllevel, "/60");
	    	Update_Menu_Item ("MENU_BERUF2_TITLE", "Lederverarbeitung");
	    	Update_Menu_Item ("MENU_BERUF2_SKILL", skilllevel);
	    };
		if(learnedAlch == TRUE)
	    {
	    	skilllevel = IntToString(alchSkill);
	    	skilllevel = ConcatStrings(skilllevel, "/60");
	    	Update_Menu_Item ("MENU_BERUF2_TITLE", "Alchemie");
	    	Update_Menu_Item ("MENU_BERUF2_SKILL", skilllevel);
	    };
    };    
};*/

var int pageCounter;
const int maxStatPages = 3;

func void clearStatusPage() {
	Update_Menu_Item("MENU_STATUS_ZEILE1_TITLE", "");
	
	Update_Menu_Item("MENU_STATUS_ZEILE3_TITLE", "");
	Update_Menu_Item("MENU_STATUS_ZEILE4_TITLE", "");	
	Update_Menu_Item("MENU_STATUS_ZEILE5_TITLE", "");
	Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "");
	Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "");
	Update_Menu_Item("MENU_STATUS_ZEILE5_SKILL", "");
	
	Update_Menu_Item("MENU_ITEM_TALENTS_HEADING", "FERTIGKEITEN");
	
};

func void changePage(var int nextPage) {
	clearStatusPage();
	if (nextPage == 0) {
		Update_Menu_Item("MENU_STATUS_ZEILE1_TITLE", "BERUFE");
		var string skilllevel;
		if(jobCounter == 0)
		{
			Update_Menu_Item("MENU_STATUS_ZEILE3_TITLE", "Keine Berufe gelernt!");	
		}    
		else
		{
			if(learnedBergbau == TRUE)
			{
				skilllevel = IntToString(bergbauSkill);
				skilllevel = ConcatStrings(skilllevel, "/100");
				Update_Menu_Item ("MENU_STATUS_ZEILE3_TITLE", "Bergbau");
				Update_Menu_Item ("MENU_STATUS_ZEILE3_SKILL", skilllevel);
			}; 
			if(learnedKuerschner == TRUE)
			{
				skilllevel = IntToString(kuerschnerSkill);
				skilllevel = ConcatStrings(skilllevel, "/100");
				Update_Menu_Item ("MENU_STATUS_ZEILE3_TITLE", "Kürschnerei");
				Update_Menu_Item ("MENU_STATUS_ZEILE3_SKILL", skilllevel);
			};
			if(learnedHerb == TRUE)
			{
				skilllevel = IntToString(herbSkill);
				skilllevel = ConcatStrings(skilllevel, "/100");
				Update_Menu_Item ("MENU_STATUS_ZEILE3_TITLE", "Kräuterkunde");
				Update_Menu_Item ("MENU_STATUS_ZEILE3_SKILL", skilllevel);
			};
			if(learnedSmith == TRUE)
			{
				skilllevel = IntToString(smithSkill);
				skilllevel = ConcatStrings(skilllevel, "/100");
				Update_Menu_Item ("MENU_STATUS_ZEILE4_TITLE", "Schmiedekunst");
				Update_Menu_Item ("MENU_STATUS_ZEILE4_SKILL", skilllevel);
			};
			if(learnedLeather == TRUE)
			{
				skilllevel = IntToString(leatherSkill);
				skilllevel = ConcatStrings(skilllevel, "/100");
				Update_Menu_Item ("MENU_STATUS_ZEILE4_TITLE", "Lederverarbeitung");
				Update_Menu_Item ("MENU_STATUS_ZEILE4_SKILL", skilllevel);
			};
			if(learnedAlch == TRUE)
			{
				skilllevel = IntToString(alchSkill);
				skilllevel = ConcatStrings(skilllevel, "/100");
				Update_Menu_Item ("MENU_STATUS_ZEILE4_TITLE", "Alchemie");
				Update_Menu_Item ("MENU_STATUS_ZEILE4_SKILL", skilllevel);
			};
		};
	}
	else if (nextPage == 1) {
		//Anzeige von Jagdtalenten
		Update_Menu_Item("MENU_STATUS_ZEILE1_TITLE", "JAGDTALENTE");
		Update_Menu_Item("MENU_STATUS_ZEILE3_TITLE", "Zähne nehmen");
		Update_Menu_Item("MENU_STATUS_ZEILE4_TITLE", "Klauen nehmen");
		Update_Menu_Item("MENU_STATUS_ZEILE5_TITLE", "Trophäen nehmen");
		if (PLAYER_TALENT_TAKEANIMALTROPHY [TROPHY_Teeth] == FALSE) { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "-"); } else { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "gelernt"); };
		if (PLAYER_TALENT_TAKEANIMALTROPHY [TROPHY_Claws] == FALSE) { Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "-"); } else { Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "gelernt"); };
		if (PLAYER_TALENT_TAKEANIMALTROPHY [TROPHY_ShadowHorn] == FALSE) { Update_Menu_Item("MENU_STATUS_ZEILE5_SKILL", "-"); } else { Update_Menu_Item("MENU_STATUS_ZEILE5_SKILL", "gelernt"); };
	}
	else if (nextPage == 2) {
		//Anzeige von Magietalenten: Schriftrollen/Runen erschaffen
		Update_Menu_Item("MENU_STATUS_ZEILE1_TITLE", "MAGIETALENTE");
		Update_Menu_Item("MENU_STATUS_ZEILE3_TITLE", "Spruchrollen");
		Update_Menu_Item("MENU_STATUS_ZEILE4_TITLE", "Runen erschaffen");
		if (PLAYER_TALENT_SCROLLS_LEARNED == FALSE) { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "-"); } else { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "gelernt"); };
		if (PLAYER_TALENT_RUNES_LEARNED == FALSE) { Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "-"); } else { Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "gelernt"); };
	}
	else if (nextPage == 3) {
		Update_Menu_Item("MENU_STATUS_ZEILE1_TITLE", "DIEBESTALENTE");
		Update_Menu_Item("MENU_STATUS_ZEILE3_TITLE", "Schleichen");
		Update_Menu_Item("MENU_STATUS_ZEILE4_TITLE", "Taschendiebstahl");
		if (Npc_GetTalentSkill(hero, NPC_TALENT_SNEAK)) { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "gelernt"); } else { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "-"); };
		if (Npc_GetTalentSkill(hero, NPC_TALENT_PICKPOCKET)) { Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "gelernt"); } else { Update_Menu_Item("MENU_STATUS_ZEILE4_SKILL", "-"); };
	}
	else if (nextPage == 4) {
		Update_Menu_Item("MENU_STATUS_ZEILE1_TITLE", "SPEZIALTALENTE");
		Update_Menu_Item("MENU_STATUS_ZEILE3_TITLE", "Kraut drehen");
		if (learnedPot == TRUE) { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "gelernt"); } else { Update_Menu_Item("MENU_STATUS_ZEILE3_SKILL", "-"); };
	}
	else if (nextPage == 5) { nextPage = 0; pageCounter = 0; changePage(nextPage);  };
};

func void changePageLoop() {
	Timer_SetPauseInMenu(false);
	changePage(pageCounter);
	/*var int returnStateNext; returnStateNext = MEM_KeyPressed(KEY_RIGHTARROW);
	var int returnStatePrev; returnStatePrev = MEM_KeyPressed(KEY_LEFTARROW);*/
	if (MEM_KeyState(KEY_RIGHTARROW) == KEY_PRESSED) {
		pageCounter += 1;
		changePage(pageCounter);
	} else if (MEM_KeyState(KEY_LEFTARROW) == KEY_PRESSED) {
		if (pageCounter != 0) {
			pageCounter = pageCounter - 1;
		} else {
			pageCounter = maxStatPages;
			changePage(pageCounter);
		};
	};
};

func void changeStatusPage () {
	pageCounter = 0;
	FF_Apply(changePageLoop);
	//startInfoMan();
};

func void closeStatusScreen() {
	FF_Remove(changePageLoop);
	//closeInfoMan();
};


func void playBuffAni_Mana() {
	var oCNpc slf; slf = _^(ECX);
	Wld_PlayEffect("spellFX_INCOVATION_BLUE",  hero, hero, 0, 0, 0, FALSE );
	//AI_PlayAni (slf,"T_PRACTICEMAGIC5");	
    
};

func void showBuildScreen() {
	var int journalBtn; journalBtn = MEM_KeyState(KEY_F11);
	if (journalBtn == KEY_PRESSED) {
		View_Close(journalView);
		//View_Delete(journalView);
		FF_Remove(showBuildScreen);
		FF_ApplyOnce(showJobInterface);
	};
};

func void showGatherScreen() {

};

func void showJobInterface() {
	if (MEM_KeyState(KEY_F11) == KEY_PRESSED) {
		if !View_GetPtr(journalView) {
			journalView = View_Create(256, 256, PS_VMax-256, PS_VMax-256);
			View_SetTexture(journalView, "Log_Paper.tga");
		};
		FF_Remove(showJobInterface);
		//NDC_SetInput(0);
		View_Open(journalView);
		FF_ApplyOnce(showBuildScreen);
		return;
	};
};







