var int Spine_UpdateScoreFunc;
var int Spine_GetUserScoreFunc;
var int Spine_GetUserRankFunc;
var int Spine_GetScoreForRankFunc;
var int Spine_GetScoreForUsernameFunc;
var int Spine_GetUsernameForRankFunc;

// adds a score for a specific identifier
// contact Bonne to get your rankings on the server
func void Spine_UpdateScore(var int identifier, var int score) {
	if (Spine_Initialized && Spine_UpdateScoreFunc) {
		CALL_IntParam(score);
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_UpdateScoreFunc);
	};
};

// returns the score for the given identifier
// if no score exists for the player/identifier combination, 0 is returned
func int Spine_GetUserScore(var int identifier) {
	if (Spine_Initialized && Spine_GetUserScoreFunc) {
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_GetUserScoreFunc);
		return CALL_RetValAsInt();
	};
	return 0;
};

// returns the rank for the given identifier
// if no score exists for the player/identifier combination, 0 is returned
func int Spine_GetUserRank(var int identifier) {
	if (Spine_Initialized && Spine_GetUserRankFunc) {
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_GetUserRankFunc);
		return CALL_RetValAsInt();
	};
	return -1;
};

// returns the score for the given identifier and rank
// if no score exists for the rank/identifier combination, 0 is returned
func int Spine_GetScoreForRank(var int identifier, var int rank) {
	if (Spine_Initialized && Spine_GetScoreForRankFunc) {
		CALL_IntParam(rank);
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_GetScoreForRankFunc);
		return CALL_RetValAsInt();
	};
	return -1;
};

// returns the score for the given identifier and username
// if no score exists for the username/identifier combination, 0 is returned
func int Spine_GetScoreForUsername(var int identifier, var string username) {
	if (Spine_Initialized && Spine_GetScoreForUsernameFunc) {
		CALL_cStringPtrParam(username);
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_GetScoreForUsernameFunc);
		return CALL_RetValAsInt();
	};
	return -1;
};

// returns the username for the given identifier and rank
// if no score exists for the rank/identifier combination, empty string is returned
func string Spine_GetUsernameForRank(var int identifier, var int rank) {
	if (Spine_Initialized && Spine_GetUsernameForRankFunc) {
		const string STR_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
		
		CALL_cStringPtrParam(STR_BUFFER);
		CALL_IntParam(rank);
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_GetUsernameForRankFunc);
		return STR_BUFFER;
	};
	return "";
};
