// base message
class Spine_Message {
	var int _vtbl;
	var int messageType; // type of the message, NEVER change manually. The message type specifies the instance this class contains (SPINE_MESSAGETYPE_INT means you can use it as Spine_IntMessage)
	var int userType; // this type can be used to specify modification specific stuff
	var string username; // sender of this message, NEVER change manually
};

class Spine_IntMessage {
	var int _vtbl;
	var int messageType; // type of the message, NEVER change manually. The message type specifies the instance this class contains (SPINE_MESSAGETYPE_INT means you can use it as Spine_IntMessage)
	var int userType; // this type can be used to specify modification specific stuff
	var string username; // sender of this message, NEVER change manually
	var int param; // some int value one can use for whatever he likes
};

class Spine_StringMessage {
	var int _vtbl;
	var int messageType; // type of the message, NEVER change manually. The message type specifies the instance this class contains (SPINE_MESSAGETYPE_STRING means you can use it as Spine_StringMessage)
	var int userType; // this type can be used to specify modification specific stuff
	var string username; // sender of this message, NEVER change manually
	var string param; // some string value one can use for whatever he likes, maximum of 255 Characters!
};

class Spine_Int3Message {
	var int _vtbl;
	var int messageType; // type of the message, NEVER change manually. The message type specifies the instance this class contains (SPINE_MESSAGETYPE_INT3 means you can use it as Spine_IntMessage)
	var int userType; // this type can be used to specify modification specific stuff
	var string username; // sender of this message, NEVER change manually
	var int param1; // some int value one can use for whatever he likes
	var int param2; // some int value one can use for whatever he likes
	var int param3; // some int value one can use for whatever he likes
};

class Spine_Int4Message {
	var int _vtbl;
	var int messageType; // type of the message, NEVER change manually. The message type specifies the instance this class contains (SPINE_MESSAGETYPE_INT4 means you can use it as Spine_Int4Message)
	var int userType; // this type can be used to specify modification specific stuff
	var string username; // sender of this message, NEVER change manually
	var int param1; // some int value one can use for whatever he likes
	var int param2; // some int value one can use for whatever he likes
	var int param3; // some int value one can use for whatever he likes
	var int param4; // some int value one can use for whatever he likes
};
