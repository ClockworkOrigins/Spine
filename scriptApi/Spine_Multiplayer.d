var int Spine_SetHostnameFunc;
var int Spine_SearchMatchFunc;
var int Spine_SearchMatchWithFriendFunc;
var int Spine_StopSearchMatchFunc;
var int Spine_IsInMatchFunc;
var int Spine_CreateMessageFunc;
var int Spine_DeleteMessageFunc;
var int Spine_SendMessageFunc;
var int Spine_ReceiveMessageFunc;
var int Spine_GetPlayerCountFunc;
var int Spine_GetPlayerUsernameFunc;
var int Spine_IsOnlineFunc;

// sets a hostname used for the MP
// overrides general server for own multiplayers
// server has to implement the server API
// TODO: create open source server API
func void Spine_SetHostname(var string hostname) {
	if (Spine_Initialized && Spine_SetHostnameFunc) {
		CALL_cStringPtrParam(hostname);
		CALL__cdecl(Spine_SetHostnameFunc);
	};
};

// searches for a multiplayer match
// numPlayers specifies the amount of players for this match
// identifier is the identifier for the mode/level or anything else specific for the modification
func void Spine_SearchMatch(var int numPlayers, var int identifier) {
	if (Spine_Initialized && Spine_SearchMatchFunc) {
		CALL_IntParam(identifier);
		CALL_IntParam(numPlayers);
		CALL__cdecl(Spine_SearchMatchFunc);
	};
};

// searches for a multiplayer match with a friend
// identifier is the identifier for the mode/level or anything else specific for the modification
func void Spine_SearchMatchWithFriend(var int identifier, var string friendName) {
	if (Spine_Initialized && Spine_SearchMatchWithFriendFunc) {
		CALL_cStringPtrParam(friendName);
		CALL_IntParam(identifier);
		CALL__cdecl(Spine_SearchMatchWithFriendFunc);
	};
};

// stops the search for a match
func void Spine_StopSearchMatch() {
	if (Spine_Initialized && Spine_StopSearchMatchFunc) {
		CALL__cdecl(Spine_StopSearchMatchFunc);
	};
};

// returns TRUE in case SearchMatch was called and a match was found, otherwise FALSE
func int Spine_IsInMatch() {
	if (Spine_Initialized && Spine_IsInMatchFunc) {
		CALL__cdecl(Spine_IsInMatchFunc);
		
		return CALL_RetValAsInt();
	};
	return FALSE;
};

// creates a new message. See Spine_Message.d for possible messages
// messageType is one of the entries in Spine_Constants.d
func int Spine_CreateMessage(var int messageType) {
	if (Spine_Initialized && Spine_CreateMessageFunc) {
		CALL_IntParam(messageType);
		CALL__cdecl(Spine_CreateMessageFunc);
		
		return CALL_RetValAsPtr();
	};
	return 0;
};

// deletes a message. Call this only for received messages after you handled your stuff with it
func int Spine_DeleteMessage(var int messagePtr) {
	if (Spine_Initialized && Spine_DeleteMessageFunc) {
		CALL_PtrParam(messagePtr);
		CALL__cdecl(Spine_DeleteMessageFunc);
		
		return CALL_RetValAsPtr();
	};
	return 0;
};

// sends a message created with Spine_CreateMessage to all connected players
// messagePtr is invalid after this call!
func void Spine_SendMessage(var int messagePtr) {
	if (Spine_Initialized && Spine_SendMessageFunc) {
		CALL_PtrParam(messagePtr);
		CALL__cdecl(Spine_SendMessageFunc);
	};
};

// returns a message from message buffer if one is available, otherwise returns 0
func int Spine_ReceiveMessage() {
	if (Spine_Initialized && Spine_ReceiveMessageFunc) {
		CALL__cdecl(Spine_ReceiveMessageFunc);
		return CALL_RetValAsPtr();
	};
	return 0;
};

// returns amount of players in current match
func int Spine_GetPlayerCount() {
	if (Spine_Initialized && Spine_GetPlayerCountFunc) {
		CALL__cdecl(Spine_GetPlayerCountFunc);
		
		return CALL_RetValAsInt();
	};
	return FALSE;
};

// returns the username of the player at position position
func string Spine_GetPlayerUsername(var int position) {
	if (Spine_Initialized && Spine_GetPlayerUsernameFunc) {
		const string STR_BUFFER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

		CALL_cStringPtrParam(STR_BUFFER);
		CALL_IntParam(position);
		CALL__cdecl(Spine_GetPlayerUsernameFunc);
		
		return STR_BUFFER;
	};
	return "";
};

// returns TRUE if in online mode, otherwise FALSE
func int Spine_IsOnline() {
	if (Spine_Initialized && Spine_IsOnlineFunc) {
		CALL__cdecl(Spine_IsOnlineFunc);
		
		return CALL_RetValAsInt();
	};
	return FALSE;
};
