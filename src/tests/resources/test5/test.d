/*#################################################################
###################################################################
	Monster-Respawn
###################################################################
#################################################################*/



class RespawnObject {
	var int inst;
	var string wp;
	var int respawnDay;
	var string worldName;
};

instance RespawnObject@(RespawnObject);

func void AddToRespawnArray(var c_npc slf) {
	var int hndl; hndl = new(RespawnObject@);
	var RespawnObject myRespawnObject; myRespawnObject = get(hndl);
	myRespawnObject.inst = Hlp_GetInstanceID(slf);
	myRespawnObject.wp = slf.spawnPoint;
	myRespawnObject.respawnDay = Wld_GetDay() + r_MinMax(2,8);
	myRespawnObject.worldName = MEM_World.worldName;
	//PrintS("Zum Respawn-Array hinzugefügt!");
};

func void checkRespawns() {
	var int respawnOpt;
	respawnOpt = STR_ToInt(MEM_GetGothOpt("MAHLENDUR", "respawn"));
	if (respawnOpt == 1) {
		ForEachHndl(RespawnObject@, _CheckRespawns);
	};
};

func int _CheckRespawns(var int hndl) {
	var RespawnObject myRespawnObject; myRespawnObject = get(hndl);
	if ((myRespawnObject.respawnDay <= Wld_GetDay()) && (Hlp_StrCmp(myRespawnObject.worldName, MEM_World.worldName))) {
		Wld_InsertNpc(myRespawnObject.inst, myRespawnObject.wp);
		delete(hndl);
	};
	return rContinue;
};




/*#################################################################
###################################################################
	Pflanzen-Respawn
###################################################################
#################################################################*/

class PlantRespawnObject {
	var int inst;
	var int spawnPosition[16];
	var int respawnDay;
	var string WorldName;
};

instance PlantRespawnObject@(PlantRespawnObject);

func void AddToPlantRespawnArray(var oCItem slf) {
	var int hndl; hndl = new(PlantRespawnObject@);
	var PlantRespawnObject myPlantRespawnObject; myPlantRespawnObject = get(hndl);
	myPlantRespawnObject.inst = Hlp_GetInstanceID(slf);
	MEM_CopyWords(_@(slf._zCVob_trafoObjToWorld), _@(myPlantRespawnObject.spawnPosition), 16);
	myPlantRespawnObject.respawnDay = Wld_GetDay() + r_MinMax(2, 8);
	myPlantRespawnObject.WorldName = MEM_World.worldName;
};

///////////////////////////////////////////////////////////////////////////////
// Hook: Item aufheben
//////////////////////////////////////////////////////////////////////////////

func void Hook_oCNpc__DoTakeVob() {
    const int oCNpc__DoTakeVob = 7621056; //0x7449C0
    HookEngine(oCNpc__DoTakeVob, 6, "EVT_NPCTAKEVOB");
};



var int sSld; var int sPir;
var int toldBoth;

func void snitchSld () {
	if (Npc_GetDistToWP(hero, "WP_SNITCHSLD") <= 2000) {
		B_LogEntry(TOPIC_snitchPirSld, "Das ist nicht gut. Die Söldner scheinen aufzurüsten und sich für irgendetwas bereit zu machen. Hoffentlich ist die Stadt nicht Teil ihrer Pläne.");
		FF_Remove(snitchSld);
		sSld = TRUE;
	};
};

func void snitchPir () {
	if (Npc_GetDistToWP(hero, "WP_SNITCHPIR") <= 2000) {
		B_LogEntry(TOPIC_snitchPirSld, "Scheinbar ist ein Teil der Mannschaft zurückgekehrt. Und jetzt bereiten sie sich auf irgendetwas vor. Ob das gut geht...");
		FF_Remove(snitchPir);
		sPir = TRUE;
	};
};

func void snitchPir_NotThisWay () {
	if (Npc_GetDistToWP(hero, "WP_SNITCHPIR_WRONGWAY") <= 2000) {
		AI_StandUp(hero);
		AI_Output(hero, hero, "SNITCHPIR_WRONGWAY_01"); //Von hier aus sehe ich nur den Zaun.
		AI_Output(hero, hero, "SNITCHPIR_WRONGWAY_02"); //Ich sollte mir einen anderen Weg suchen.
		FF_Remove(snitchPir_NotThisWay);
	};
};

func void changeSldRtnPreAttack () {
	Npc_ExchangeRoutine(SLD_815_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_817_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_818_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_819_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_820_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_821_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_824_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_825_Soeldner, "PREATTACK");
	Npc_ExchangeRoutine(SLD_826_Soeldner, "PREATTACK");
};
