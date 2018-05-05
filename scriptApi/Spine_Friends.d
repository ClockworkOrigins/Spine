var int Spine_GetFriendCountFunc;
var int Spine_GetFriendNameFunc;

// returns the amount of friends the current player has
// will be 0 when offline, no account or no friends are found
func int Spine_GetFriendCount() {
	if (Spine_Initialized && Spine_GetFriendCountFunc) {
		CALL__cdecl(Spine_GetFriendCountFunc);
		return CALL_RetValAsInt();
	};
	return 0;
};

// returns the name of a friend
// index specifies the index in the friend list which will be ordered alphabetically
// index has to be in range [0;Spine_GetFriendCount() - 1]
func string Spine_GetFriendName(var int index) {
	if (Spine_Initialized && Spine_GetFriendNameFunc) {
		const string STR_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
		
		CALL_cStringPtrParam(STR_BUFFER);
		CALL_IntParam(index);
		CALL__cdecl(Spine_GetFriendNameFunc);
		return STR_BUFFER;
	};
	return "";
};
