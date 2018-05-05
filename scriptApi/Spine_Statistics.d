var int Spine_UpdateStatisticFunc;

// updates a statistic value with given identifier, guild, name and value
// general usage case for this is to create statistics that will help to balance the mod
// that's why identifier is intended to be the chapter
func void Spine_UpdateStatistic(var int identifier, var int guild, var string name, var int value) {
	if (Spine_Initialized && Spine_UpdateStatisticFunc) {
		CALL_IntParam(value);
		CALL_cStringPtrParam(name);
		CALL_IntParam(guild);
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_UpdateStatisticFunc);
	};
};
