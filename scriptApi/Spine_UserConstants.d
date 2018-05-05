// The values for this variables can be specified by the user

const int SPINE_ACHIEVEMENTORIENTATION = SPINE_BOTTOMRIGHT; // position of the achievement widget
const int SPINE_ACHIEVEMENT_DISPLAY_TIME = 5000; // show achievement for 5 seconds

const int SPINE_EARTHQUAKE_VIBRATION = TRUE; // earthquake triggers vibration of gamepad, only relevant for Gothic 2

const string SPINE_MODNAME = "MODNAME"; // modname is necessary for achievements to work without using Spine

// define the strings for the achievements
// don't use an identifier for unlockAchievement greater than the maximum index of the array
const int MAX_ACHIEVEMENTS = 3;
const string SPINE_ACHIEVEMENT_NAMES[MAX_ACHIEVEMENTS] = {
	"PLACEHOLDER1",
	"PLACEHOLDER2",
	"PLACEHOLDER3"
};

const string SPINE_ACHIEVEMENT_TEXTURES[MAX_ACHIEVEMENTS] = {
	"SPINE_ACHIEVEMENT_DEFAULT.TGA",
	"SPINE_ACHIEVEMENT_DEFAULT.TGA",
	"SPINE_ACHIEVEMENT_DEFAULT.TGA"
};

const string SPINE_ACHIEVEMENT_PROGRESS[MAX_ACHIEVEMENTS] = {
	"0",
	"0",
	"3"
};
