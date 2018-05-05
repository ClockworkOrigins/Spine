const string SPINE_VERSION_STRING = "@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_PATCH@";

// Spine modules

const int SPINE_MODULE_GETCURRENTUSERNAME	= 1<<0;
const int SPINE_MODULE_ACHIEVEMENTS			= 1<<1;
const int SPINE_MODULE_SCORES				= 1<<2;
const int SPINE_MODULE_MULTIPLAYER			= 1<<3;
const int SPINE_MODULE_OVERALLSAVE			= 1<<4;
const int SPINE_MODULE_GAMEPAD				= 1<<5;
const int SPINE_MODULE_FRIENDS				= 1<<6;
const int SPINE_MODULE_STATISTICS			= 1<<7;

const int SPINE_ALL							= (1<<8) - 1;

const int SPINE_TOPLEFT = 0;
const int SPINE_TOPRIGHT = 1;
const int SPINE_BOTTOMLEFT = 2;
const int SPINE_BOTTOMRIGHT = 3;

const int SPINE_ACHIEVEMENT_WIDTH = 400;
const int SPINE_ACHIEVEMENT_HEIGHT = 80;
const int SPINE_ACHIEVEMENT_IMAGE_WIDTH = 64;
const int SPINE_ACHIEVEMENT_IMAGE_HEIGHT = 64;

// MessageTypes for Spine_Multiplayer
const int SPINE_MESSAGETYPE_BASE = 0;
const int SPINE_MESSAGETYPE_INT = 1;
const int SPINE_MESSAGETYPE_STRING = 2;
const int SPINE_MESSAGETYPE_INT4 = 3;
const int SPINE_MESSAGETYPE_INT3 = 4;

var int SPINE_SHOWACHIEVEMENTS; // show achievement (you can set this to FALSE to disable the achievement widget, but internally the achievement will be unlocked, so you still can see it in Spine), configured in Spine GUI

const int SPINE_ADDRESS_TRIGGEREARTHQUAKE_G1 = 6189088; // 0x005E7020
const int SPINE_LENGTH_TRIGGEREARTHQUAKE_G1 = 4;
const int SPINE_ADDRESS_UNTRIGGEREARTHQUAKE_G1 = 6189168; // 0x005E7070
const int SPINE_LENGTH_UNTRIGGEREARTHQUAKE_G1 = 4;

const int SPINE_ADDRESS_TRIGGEREARTHQUAKE_G2 = 6373264; // 0x00613F90
const int SPINE_LENGTH_TRIGGEREARTHQUAKE_G2 = 6;
const int SPINE_ADDRESS_UNTRIGGEREARTHQUAKE_G2 = 6373344; // 0x00613FE0
const int SPINE_LENGTH_UNTRIGGEREARTHQUAKE_G2 = 6;

const int SPINE_GAME_DAMAGE_VIBRATION_TIME = 1000; // 1 seconds vibration for every hit?
