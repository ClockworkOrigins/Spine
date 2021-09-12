/*
	This file is part of Spine.

    Spine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Spine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Spine.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#pragma once

#include "common/MessageTypes.h"
#include "common/ProjectStats.h"
#include "common/ModVersion.h"
#include "common/NewsTickerTypes.h"

#include "boost/serialization/map.hpp"
#include "boost/serialization/vector.hpp"

namespace spine {
namespace common {
	
	struct Message {
		MessageType type;
		Message() : type() {}
		virtual ~Message() {}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & type;
		}

		std::string SerializeBlank() const;
		static Message * DeserializeBlank(const std::string & s);

		std::string SerializePublic() const;
		static Message * DeserializePublic(const std::string & s);

		std::string SerializePrivate() const;
		static Message * DeserializePrivate(const std::string & s);
	};

	struct UpdateRequestMessage : public Message {
		uint8_t majorVersion;
		uint8_t minorVersion;
		uint8_t patchVersion;
		UpdateRequestMessage() : Message(), majorVersion(), minorVersion(), patchVersion() {
			type = MessageType::UPDATEREQUEST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
		}
	};

	struct UpdateFileCountMessage : public Message {
		uint32_t count;
		UpdateFileCountMessage() : Message(), count() {
			type = MessageType::UPDATEFILECOUNT;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & count;
		}
	};

	struct UpdateFileHeaderMessage : public Message {
		std::string name;
		uint64_t hash;
		bool remove;
		UpdateFileHeaderMessage() : Message(), hash(), remove() {
			type = MessageType::UPDATEFILEHEADER;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & name;
			ar & hash;
			ar & remove;
		}
	};

	struct UpdateFileMessage : public Message {
		std::vector<uint8_t> data;
		UpdateFileMessage() : Message() {
			type = MessageType::UPDATEFILE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & data;
		}
	};

	struct RequestUsernameMessage : public Message {
		RequestUsernameMessage() : Message() {
			type = MessageType::REQUESTUSERNAME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
		}
	};

	struct SendUsernameMessage : public Message {
		std::string username;
		int32_t modID;
		int32_t userID;
		SendUsernameMessage() : Message(), modID(), userID(-1) {
			type = MessageType::SENDUSERNAME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & modID;
			ar & userID;
		}
	};
	
	struct RequestScoresMessage : public Message {
		int32_t modID;
		RequestScoresMessage() : Message(), modID() {
			type = MessageType::REQUESTSCORES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
		}
	};
	
	struct SendScoresMessage : public Message {
		std::vector<std::pair<int32_t, std::vector<std::pair<std::string, int32_t>>>> scores;
		SendScoresMessage() : Message() {
			type = MessageType::SENDSCORES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & scores;
		}
	};

	struct UpdateScoreMessage : public Message {
		int32_t identifier;
		int32_t score;
		UpdateScoreMessage() : Message(), identifier(), score() {
			type = MessageType::UPDATESCORE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & identifier;
			ar & score;
		}
	};

	struct RequestAchievementsMessage : public Message {
		RequestAchievementsMessage() : Message() {
			type = MessageType::REQUESTACHIEVEMENTS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
		}
	};

	struct SendAchievementsMessage : public Message {
		std::vector<int32_t> achievements;
		std::vector<std::pair<int32_t, std::pair<int32_t, int32_t>>> achievementProgress;
		bool showAchievements;
		SendAchievementsMessage() : Message(), showAchievements(true) {
			type = MessageType::SENDACHIEVEMENTS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & achievements;
			ar & achievementProgress;
			ar & showAchievements;
		}
	};

	struct UnlockAchievementMessage : public Message {
		int32_t identifier;
		UnlockAchievementMessage() : Message(), identifier() {
			type = MessageType::UNLOCKACHIEVEMENT;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & identifier;
		}
	};

	struct SearchMatchMessage : public Message {
		int32_t numPlayers;
		int32_t identifier;
		int32_t modID;
		std::string username;
		std::string password;
		std::string friendName;
		SearchMatchMessage() : Message(), numPlayers(), identifier(), modID() {
			type = MessageType::SEARCHMATCH;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & numPlayers;
			ar & identifier;
			ar & modID;
			ar & username;
			ar & password;
			ar & friendName;
		}
	};

	struct FoundMatchMessage : public Message {
		std::vector<std::string> users;
		FoundMatchMessage() : Message() {
			type = MessageType::FOUNDMATCH;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & users;
		}
	};

	struct UpdateFilesMessage : public Message {
		std::vector<std::pair<std::string, std::string>> files;
		UpdateFilesMessage() : Message() {
			type = MessageType::UPDATEFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & files;
		}
	};
	
	struct AchievementStats {
		std::string name;
		std::string description;
		std::string iconLocked;
		std::string iconLockedHash;
		std::string iconUnlocked;
		std::string iconUnlockedHash;
		bool unlocked = false;
		double unlockedPercent = 0.0;
		bool hidden = false;
		int currentProgress = 0;
		int maxProgress = 0;
		bool canSeeHidden = false;
	};
	
	struct ScoreStats {
		std::string name;
		std::vector<std::pair<std::string, int32_t>> scores;
	};

	struct Package {
		int32_t modID;
		int32_t packageID;
		std::string name;
		uint64_t downloadSize;
		std::string language;

		Package() : modID(), packageID(), downloadSize(0) {}
	};

	struct News{
		std::string title;
		std::string body;
		int64_t timestamp;
		std::vector<std::pair<int32_t, std::string>> referencedMods;
		std::vector<std::pair<std::string, std::string>> imageFiles;
	};

	struct NewsTicker{
		NewsTickerType type;
		std::string name;
		int32_t projectID;
		int32_t timestamp;
		int8_t majorVersion;
		int8_t minorVersion;
		int8_t patchVersion;

		NewsTicker() : type(NewsTickerType::Update), projectID(0), timestamp(0), majorVersion(0), minorVersion(0), patchVersion(0) {}
	};

	struct RequestOverallSavePathMessage : public Message {
		RequestOverallSavePathMessage() : Message() {
			type = MessageType::REQUESTOVERALLSAVEPATH;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
		}
	};

	struct SendOverallSavePathMessage : public Message {
#ifdef WIN32
		std::wstring path;
#else
		std::string path;
#endif
		
		SendOverallSavePathMessage() : Message(), path() {
			type = MessageType::SENDOVERALLSAVEPATH;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & path;
		}
	};

	struct AckMessage : public Message {
		bool success;
		AckMessage() : Message(), success() {
			type = MessageType::ACK;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & success;
		}
	};

	struct UpdateAchievementProgressMessage : public Message {
		int32_t identifier;
		int32_t progress;
		UpdateAchievementProgressMessage() : Message(), identifier(), progress() {
			type = MessageType::UPDATEACHIEVEMENTPROGRESS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & identifier;
			ar & progress;
		}
	};

	struct RequestOverallSaveDataMessage : public Message {
		RequestOverallSaveDataMessage() : Message() {
			type = MessageType::REQUESTOVERALLSAVEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
		}
	};

	struct SendOverallSaveDataMessage : public Message {
		std::vector<std::pair<std::string, std::string>> data;
		SendOverallSaveDataMessage() : Message() {
			type = MessageType::SENDOVERALLSAVEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & data;
		}
	};

	struct UpdateOverallSaveDataMessage : public Message {
		std::string entry;
		std::string value;
		UpdateOverallSaveDataMessage() : Message() {
			type = MessageType::UPDATEOVERALLSAVEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & entry;
			ar & value;
		}
	};

	struct VibrateGamepadMessage : public Message {
		float leftMotor;
		float rightMotor;
		VibrateGamepadMessage() : Message(), leftMotor(), rightMotor() {
			type = MessageType::VIBRATEGAMEPAD;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & leftMotor;
			ar & rightMotor;
		}
	};

	struct GamepadEnabledMessage : public Message {
		bool enabled;
		GamepadEnabledMessage() : Message(), enabled() {
			type = MessageType::GAMEPADENABLED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & enabled;
		}
	};

	struct GamepadActiveMessage : public Message {
		bool active;
		GamepadActiveMessage() : Message(), active() {
			type = MessageType::GAMEPADACTIVE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & active;
		}
	};

	struct GamepadButtonStateMessage : public Message {
		int32_t button;
		bool state;
		GamepadButtonStateMessage() : Message(), button(), state() {
			type = MessageType::GAMEPADBUTTONSTATE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & button;
			ar & state;
		}
	};

	struct GamepadTriggerStateMessage : public Message {
		int32_t trigger;
		int32_t value;
		GamepadTriggerStateMessage() : Message(), trigger(), value() {
			type = MessageType::GAMEPADTRIGGERSTATE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & trigger;
			ar & value;
		}
	};

	struct GamepadStickStateMessage : public Message {
		int32_t stick;
		int32_t axis;
		int32_t value;
		GamepadStickStateMessage() : Message(), stick(), axis(), value() {
			type = MessageType::GAMEPADSTICKSTATE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & stick;
			ar & axis;
			ar & value;
		}
	};

	struct GamepadRawModeMessage : public Message {
		bool enabled;
		GamepadRawModeMessage() : Message(), enabled() {
			type = MessageType::GAMEPADRAWMODE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & enabled;
		}
	};

	struct ModFile {
		std::string filename;
		std::string hash;
		std::string language;
		bool changed;
		bool deleted;
		int64_t size;

		ModFile() : changed(false), deleted(false), size(0) {}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & filename;
			ar & hash;
			ar & language;
			ar & changed;
			ar & deleted;
			ar & size;
		}
	};

	struct TranslatedText {
		std::string language;
		std::string text;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & language;
			ar & text;
		}
	};

	struct UploadModfilesMessage : public Message {
		int32_t modID;
		std::vector<ModFile> files;
		UploadModfilesMessage() : Message(), modID() {
			type = MessageType::UPLOADMODFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & files;
		}
	};

	struct RequestAllFriendsMessage : public Message {
		RequestAllFriendsMessage() : Message() {
			type = MessageType::REQUESTALLFRIENDS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
		}
	};

	struct Friend {
		std::string name;
		uint32_t level;

		Friend() : level(0) {}
		Friend(const std::string & n, uint32_t l) : name(n), level(l) {}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & name;
			ar & level;
		}
	};

	struct SendAllFriendsMessage : public Message {
		std::vector<Friend> friends;
		SendAllFriendsMessage() : Message() {
			type = MessageType::SENDALLFRIENDS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & friends;
		}
	};

	struct SendUserLevelMessage {
		uint32_t level;
		uint32_t currentXP;
		uint32_t nextXP;
		SendUserLevelMessage() : level(), currentXP(), nextXP() {}
	};

	struct UploadAchievementIconsMessage : public Message {
		int32_t modID;

		struct Icon {
			std::string name;
			std::vector<uint8_t> data;
			
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & name;
				ar & data;
			}
		};

		std::vector<Icon> icons;

		UploadAchievementIconsMessage() : Message(), modID() {
			type = MessageType::UPLOADACHIEVEMENTICONS;
		}
		
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & icons;
		}
	};

	struct UpdateChapterStatsMessage : public Message {
		int32_t identifier;
		int32_t guild;
		std::string statName;
		int32_t statValue;

		UpdateChapterStatsMessage() : Message(), identifier(), guild(), statValue() {
			type = MessageType::UPDATECHAPTERSTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & identifier;
			ar & guild;
			ar & statName;
			ar & statValue;
		}
	};

	struct IsAchievementUnlockedMessage : public Message {
		int32_t modID;
		int32_t achievementID;

		IsAchievementUnlockedMessage() : Message(), modID(), achievementID() {
			type = MessageType::ISACHIEVEMENTUNLOCKED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & achievementID;
		}
	};

	struct SendAchievementUnlockedMessage : public Message {
		bool unlocked;

		SendAchievementUnlockedMessage() : Message(), unlocked() {
			type = MessageType::SENDISACHIEVEMENTUNLOCKED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & unlocked;
		}
	};

	struct UploadScreenshotsMessage : public Message {
		int32_t projectID;
		std::string username;
		std::string password;
		// filename, data
		std::vector<std::pair<std::string, std::vector<uint8_t>>> screenshots;

		UploadScreenshotsMessage() : Message(), projectID(-1) {
			type = MessageType::UPLOADSCREENSHOTS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & projectID;
			ar & username;
			ar & password;
			ar & screenshots;
		}
	};

} /* namespace common */
} /* namespace spine */
