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
#include "common/Mod.h"
#include "common/ProjectStats.h"
#include "common/ModUpdate.h"
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
		UpdateFileHeaderMessage() : Message(), name(), hash(), remove() {
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
		UpdateFileMessage() : Message(), data() {
			type = MessageType::UPDATEFILE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & data;
		}
	};

	struct RequestAllModsMessage : public Message {
		std::string language;
		std::string username;
		std::string password;
		RequestAllModsMessage() : Message(), language(), username(), password() {
			type = MessageType::REQUESTALLMODS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & language;
			ar & username;
			ar & password;
		}
	};

	struct UpdateAllModsMessage : public Message {
		std::vector<Mod> mods;
		std::vector<int32_t> playedProjects;
		UpdateAllModsMessage() : Message() {
			type = MessageType::UPDATEALLMODS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & mods;
			ar & playedProjects;
		}
	};

	struct RequestModFilesMessage : public Message {
		int32_t modID;
		std::string language;
		RequestModFilesMessage() : Message(), modID(), language("Deutsch") {
			type = MessageType::REQUESTMODFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			try { // TODO (Daniel): drop this code sometime in the future
				ar & language;
			} catch (boost::archive::archive_exception&) {
				language = "Deutsch";
			}
		}
	};

	struct ListModFilesMessage : public Message {
		std::vector<std::pair<std::string, std::string>> fileList;
		std::string fileserver;
		ListModFilesMessage() : Message() {
			type = MessageType::LISTMODFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & fileList;
			ar & fileserver;
		}
	};

	struct DownloadSucceededMessage : public Message {
		int32_t modID;
		DownloadSucceededMessage() : Message(), modID() {
			type = MessageType::DOWNLOADSUCCEEDED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
		}
	};

	struct [[deprecated("Remove in Spine 1.28.0")]] UpdatePlayTimeMessage : public Message {
		std::string username;
		std::string password;
		int32_t modID;
		int32_t duration;
		UpdatePlayTimeMessage() : Message(), modID(), duration() {
			type = MessageType::UPDATEPLAYTIME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & modID;
			ar & duration;
		}
	};

	struct UpdateDownloadSizesMessage : public Message {
		std::vector<std::pair<int32_t, uint64_t>> downloadSizes;
		UpdateDownloadSizesMessage() : Message() {
			type = MessageType::UPDATEDOWNLOADSIZES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & downloadSizes;
		}
	};

	struct RequestPlayTimeMessage : public Message {
		std::string username;
		std::string password;
		int32_t modID;
		RequestPlayTimeMessage() : Message(), modID() {
			type = MessageType::REQUESTPLAYTIME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & modID;
		}
	};

	struct SendPlayTimeMessage : public Message {
		int32_t duration;
		SendPlayTimeMessage() : Message(), duration() {
			type = MessageType::SENDPLAYTIME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & duration;
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
		int32_t [[deprecated("Remove in Spine 1.28.0")]] modID;
		int32_t identifier;
		std::string [[deprecated("Remove in Spine 1.28.0")]] username;
		std::string [[deprecated("Remove in Spine 1.28.0")]] password;
		int32_t score;
		UpdateScoreMessage() : Message(), modID(), identifier(), score() {
			type = MessageType::UPDATESCORE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & identifier;
			ar & username;
			ar & password;
			ar & score;
		}
	};

	struct RequestAchievementsMessage : public Message {
		int32_t [[deprecated("Remove in Spine 1.28.0")]] modID;
		std::string [[deprecated("Remove in Spine 1.28.0")]] username;
		std::string [[deprecated("Remove in Spine 1.28.0")]] password;
		RequestAchievementsMessage() : Message(), modID() {
			type = MessageType::REQUESTACHIEVEMENTS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & username;
			ar & password;
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
		int32_t [[deprecated("Remove in Spine 1.28.0")]] modID;
		int32_t identifier;
		std::string [[deprecated("Remove in Spine 1.28.0")]] username;
		std::string [[deprecated("Remove in Spine 1.28.0")]] password;
		uint32_t [[deprecated("Remove in Spine 1.28.0")]] duration;
		UnlockAchievementMessage() : Message(), modID(), identifier(), duration() {
			type = MessageType::UNLOCKACHIEVEMENT;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & identifier;
			ar & username;
			ar & password;
			ar & duration;
		}
	};

	struct ModVersionCheckMessage : public Message {
		std::string language;
		std::vector<ModVersion> modVersions;
		std::string username;
		std::string password;
		ModVersionCheckMessage() : Message() {
			type = MessageType::MODVERSIONCHECK;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & language;
			ar & modVersions;
			ar & username;
			ar & password;
		}
	};

	struct SendModsToUpdateMessage : public Message {
		std::vector<ModUpdate> updates;
		SendModsToUpdateMessage() : Message() {
			type = MessageType::SENDMODSTOUPDATE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & updates;
		}
	};

	struct SearchMatchMessage : public Message {
		int32_t numPlayers;
		int32_t identifier;
		int32_t modID;
		std::string username;
		std::string password;
		std::string friendName;
		SearchMatchMessage() : Message(), numPlayers(), identifier(), modID(), username(), password(), friendName() {
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
		FoundMatchMessage() : Message(), users() {
			type = MessageType::FOUNDMATCH;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & users;
		}
	};

	struct FeedbackMessage : public Message {
		std::string text;
		uint8_t majorVersion;
		uint8_t minorVersion;
		uint8_t patchVersion;
		std::string username;
		int32_t projectID;
		
		FeedbackMessage() : Message(), majorVersion(), minorVersion(), patchVersion(), projectID(-1) {
			type = MessageType::FEEDBACK;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & text;
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
			ar & username;
			ar & projectID;
		}
	};

	struct RequestOriginalFilesMessage : public Message {
		std::vector<std::pair<int32_t, std::string>> files;
		RequestOriginalFilesMessage() : Message(), files() {
			type = MessageType::REQUESTORIGINALFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & files;
		}
	};

	struct SendOriginalFilesMessage : public Message {
		std::vector<std::pair<int32_t, std::pair<std::string, std::string>>> files;
		SendOriginalFilesMessage() : Message(), files() {
			type = MessageType::SENDORIGINALFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & files;
		}
	};

	struct UpdateFilesMessage : public Message {
		std::vector<std::pair<std::string, std::string>> files;
		UpdateFilesMessage() : Message(), files() {
			type = MessageType::UPDATEFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & files;
		}
	};

	struct UpdateLoginTimeMessage : public Message {
		std::string username;
		std::string password;
		UpdateLoginTimeMessage() : Message(), username(), password() {
			type = MessageType::UPDATELOGINTIME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
		}
	};

	struct RequestPackageFilesMessage : public Message {
		int32_t packageID;
		std::string language;
		RequestPackageFilesMessage() : Message(), packageID(), language() {
			type = MessageType::REQUESTPACKAGEFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & packageID;
			ar & language;
		}
	};

	struct UpdatePackageListMessage : public Message {
		struct Package {
			int32_t modID;
			int32_t packageID;
			std::string name;
			uint64_t downloadSize;

			Package() : modID(), packageID(), name(), downloadSize(0) {
			}

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & packageID;
				ar & name;
				ar & downloadSize;
			}
		};
		std::vector<Package> packages;
		UpdatePackageListMessage() : Message(), packages() {
			type = MessageType::UPDATEPACKAGELIST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & packages;
		}
	};

	struct PackageDownloadSucceededMessage : public Message {
		int32_t packageID;
		PackageDownloadSucceededMessage() : Message(), packageID() {
			type = MessageType::PACKAGEDOWNLOADSUCCEEDED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & packageID;
		}
	};

	struct RequestAllModStatsMessage : public Message {
		std::string username;
		std::string password;
		std::string language;
		RequestAllModStatsMessage() : Message() {
			type = MessageType::REQUESTALLMODSTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & language;
		}
	};

	struct SendAllModStatsMessage : public Message {
		std::vector<ProjectStats> mods;
		SendAllModStatsMessage() : Message(), mods() {
			type = MessageType::SENDALLMODSTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & mods;
		}
	};

	struct [[deprecated("Remove in Spine 1.28.0")]] RequestSingleModStatMessage : public Message {
		std::string username;
		std::string password;
		std::string language;
		int32_t modID;
		RequestSingleModStatMessage() : Message(), modID() {
			type = MessageType::REQUESTSINGLEMODSTAT;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & language;
			ar & modID;
		}
	};

	struct [[deprecated("Remove in Spine 1.28.0")]] SendSingleModStatMessage : public Message {
		ProjectStats mod;
		SendSingleModStatMessage() : Message() {
			type = MessageType::SENDSINGLEMODSTAT;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & mod;
		}
	};

	struct RequestAllAchievementStatsMessage : public Message {
		std::string username;
		std::string password;
		std::string language;
		int32_t modID;
		RequestAllAchievementStatsMessage() : Message(), modID() {
			type = MessageType::REQUESTALLACHIEVEMENTSTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & language;
			ar & modID;
		}
	};

	struct SendAllAchievementStatsMessage : public Message {
		struct AchievementStats {
			std::string name = "";
			std::string description = "";
			std::string iconLocked = "";
			std::string iconLockedHash = "";
			std::string iconUnlocked = "";
			std::string iconUnlockedHash = "";
			bool unlocked = false;
			double unlockedPercent = 0.0;
			bool hidden = false;
			int currentProgress = 0;
			int maxProgress = 0;
			bool canSeeHidden = false;

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & name;
				ar & description;
				ar & iconLocked;
				ar & iconLockedHash;
				ar & iconUnlocked;
				ar & iconUnlockedHash;
				ar & unlocked;
				ar & unlockedPercent;
				ar & hidden;
				ar & currentProgress;
				ar & maxProgress;
				ar & canSeeHidden;
			}
		};
		std::vector<AchievementStats> achievements;
		SendAllAchievementStatsMessage() : Message(), achievements() {
			type = MessageType::SENDALLMODSTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & achievements;
		}
	};

	struct RequestAllScoreStatsMessage : public Message {
		std::string username;
		std::string password;
		std::string language;
		int32_t modID;
		RequestAllScoreStatsMessage() : Message(), username(), password(), language(), modID() {
			type = MessageType::REQUESTALLSCORESTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & language;
			ar & modID;
		}
	};

	struct SendAllScoreStatsMessage : public Message {
		struct ScoreStats {
			std::string name = "";
			std::vector<std::pair<std::string, int32_t>> scores;

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & name;
				ar & scores;
			}
		};
		std::vector<ScoreStats> scores;
		SendAllScoreStatsMessage() : Message(), scores() {
			type = MessageType::SENDALLSCORESTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & scores;
		}
	};

	struct RequestAllNewsMessage : public Message {
		int32_t lastNewsID;
		std::string language;
		RequestAllNewsMessage() : Message(), lastNewsID(0), language() {
			type = MessageType::REQUESTALLNEWS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & lastNewsID;
			ar & language;
		}
	};

	struct SendAllNewsMessage : public Message {
		struct News {
			int32_t id;
			std::string title = "";
			std::string body;
			int32_t timestamp;
			std::vector<std::pair<int32_t, std::string>> referencedMods;
			std::vector<std::pair<std::string, std::string>> imageFiles;

			News() : timestamp(0) {
			}

			News(std::string s1, std::string s2, std::string s3, std::string s4) : id(std::stoi(s1)), title(s2), body(s3), timestamp(std::stoi(s4)) {
			}

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & id;
				ar & title;
				ar & body;
				ar & timestamp;
				ar & referencedMods;
				ar & imageFiles;
			}
		};

		struct NewsTicker {
			NewsTickerType type;
			std::string name;
			int32_t projectID;
			int32_t timestamp;
			int8_t majorVersion;
			int8_t minorVersion;
			int8_t patchVersion;

			NewsTicker() : type(NewsTickerType::Update), projectID(0), timestamp(0), majorVersion(0), minorVersion(0), patchVersion(0) {}

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & type;
				ar & name;
				ar & projectID;
				ar & timestamp;
				ar & majorVersion;
				ar & minorVersion;
				ar & patchVersion;
			}
		};
		
		std::vector<News> news;
		std::vector<NewsTicker> newsTicker;
		
		SendAllNewsMessage() : Message() {
			type = MessageType::SENDALLNEWS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & news;
			ar & newsTicker;
		}
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

	struct SubmitNewsMessage : public Message {
		std::string username;
		std::string password;
		SendAllNewsMessage::News news;
		std::vector<int32_t> mods;
		std::vector<std::vector<uint8_t>> images;
		std::string language;
		SubmitNewsMessage() : Message(), username(), password(), news(), mods(), images(), language() {
			type = MessageType::SUBMITNEWS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & news;
			ar & mods;
			ar & images;
			ar & language;
		}
	};

	struct LinkClickedMessage : public Message {
		int32_t newsID;
		std::string url;
		LinkClickedMessage() : Message(), newsID(), url() {
			type = MessageType::LINKCLICKED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & newsID;
			ar & url;
		}
	};

	struct SubmitScriptFeaturesMessage : public Message {
		struct Achievement {
			std::string name;
			std::string description;
			bool hidden;
			std::string lockedImageName;
			std::string unlockedImageName;
			int maxProgress;

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & name;
				ar & description;
				ar & hidden;
				ar & lockedImageName;
				ar & unlockedImageName;
				ar & maxProgress;
			}
		};
		struct Score {
			std::string name;

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & name;
			}
		};
		int32_t modID;
		std::string language;
		std::string username;
		std::string password;
		std::vector<Achievement> achievements;
		std::vector<Score> scores;
		std::vector<std::pair<std::pair<std::string, std::string>, std::vector<uint8_t>>> achievementImages;
		SubmitScriptFeaturesMessage() : Message(), modID(), language(), username(), password(), achievements(), scores(), achievementImages() {
			type = MessageType::SUBMITSCRIPTFEATURES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & language;
			ar & username;
			ar & password;
			ar & achievements;
			ar & scores;
			ar & achievementImages;
		}
	};

	struct RequestInfoPageMessage : public Message {
		int32_t modID;
		std::string language;
		std::string username;
		std::string password;
		RequestInfoPageMessage() : Message(), modID(), language(), username(), password() {
			type = MessageType::REQUESTINFOPAGE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & language;
			ar & username;
			ar & password;
		}
	};

	struct SendInfoPageMessage : public Message {
		struct History {
			int32_t timestamp;
			int8_t majorVersion;
			int8_t minorVersion;
			int8_t patchVersion;
			bool savegameCompatible;
			std::string changelog;

			History() : timestamp(0), majorVersion(0), minorVersion(0), patchVersion(0), savegameCompatible(false) {}

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & timestamp;
				ar & majorVersion;
				ar & minorVersion;
				ar & patchVersion;
				ar & savegameCompatible;
				ar & changelog;
			}
		};
		
		std::string modname;
		std::vector<std::pair<std::string, std::string>> screenshots;
		std::string description;
		std::vector<std::string> features;
		int32_t spineFeatures;
		bool editRights;
		bool installAllowed;
		std::vector<std::pair<int32_t, std::string>> optionalPackages;
		uint8_t majorVersion;
		uint8_t minorVersion;
		uint8_t patchVersion;
		uint32_t releaseDate;
		uint32_t updateDate;
		GameType gameType;
		std::vector<History> history;

		SendInfoPageMessage() : Message(), spineFeatures(0), editRights(false), installAllowed(true), majorVersion(), minorVersion(), patchVersion(), releaseDate(0), updateDate(0), gameType(GameType::Gothic) {
			type = MessageType::SENDINFOPAGE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modname;
			ar & screenshots;
			ar & description;
			ar & features;
			ar & spineFeatures;
			ar & editRights;
			ar & installAllowed;
			ar & optionalPackages;
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
			ar & releaseDate;
			ar & updateDate;
			ar & gameType;
			ar & history;
		}
	};

	struct SubmitInfoPageMessage : public Message {
		int32_t modID;
		std::string language;
		std::vector<std::pair<std::string, std::string>> screenshots;
		std::vector<std::pair<std::string, std::vector<uint8_t>>> imageFiles;
		std::string description;
		std::vector<std::string> features;
		int32_t spineFeatures;
		SubmitInfoPageMessage() : Message(), modID(), spineFeatures(0) {
			type = MessageType::SUBMITINFOPAGE;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & language;
			ar & screenshots;
			ar & imageFiles;
			ar & description;
			ar & features;
			ar & spineFeatures;
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

	struct [[deprecated("Remove in Spine 1.28.0")]] SendUserInfosMessage : public Message {
		std::string username;
		std::string password;
		std::string hash;
		std::string mac;
		std::vector<std::pair<std::string, std::string>> settings;
		int ownID;
		std::string language;
		static int id;
		SendUserInfosMessage() : Message(), ownID(id++) {
			type = MessageType::SENDUSERINFOS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & hash;
			ar & mac;
			ar & ownID;
			ar & settings;
			ar & language;
		}
	};

	struct RequestRandomModMessage : public Message {
		std::string language;
		std::string username;
		std::string password;
		RequestRandomModMessage() : Message() {
			type = MessageType::REQUESTRANDOMMOD;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & language;
			ar & username;
			ar & password;
		}
	};

	struct SendRandomModMessage : public Message {
		int32_t modID;
		SendRandomModMessage() : Message(), modID() {
			type = MessageType::SENDRANDOMMOD;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
		}
	};

	struct UpdateAchievementProgressMessage : public Message {
		int32_t modID;
		int32_t identifier;
		int32_t progress;
		std::string username;
		std::string password;
		UpdateAchievementProgressMessage() : Message(), modID(), identifier(), progress() {
			type = MessageType::UPDATEACHIEVEMENTPROGRESS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & identifier;
			ar & progress;
			ar & username;
			ar & password;
		}
	};

	struct SubmitCompatibilityMessage : public Message {
		int32_t modID;
		int32_t patchID;
		std::string username;
		std::string password;
		bool compatible;
		SubmitCompatibilityMessage() : Message(), modID(), patchID(), username(), password(), compatible() {
			type = MessageType::SUBMITCOMPATIBILITY;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & patchID;
			ar & username;
			ar & password;
			ar & compatible;
		}
	};

	struct RequestOwnCompatibilitiesMessage : public Message {
		std::string username;
		std::string password;
		RequestOwnCompatibilitiesMessage() : Message(), username(), password() {
			type = MessageType::REQUESTOWNCOMPATIBILITIES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
		}
	};

	struct SendOwnCompatibilitiesMessage : public Message {
		struct Compatibility {
			int32_t modID;
			int32_t patchID;
			bool compatible;

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & patchID;
				ar & compatible;
			}
		};
		std::vector<Compatibility> compatibilities;
		SendOwnCompatibilitiesMessage() : Message() {
			type = MessageType::SENDOWNCOMPATIBILITIES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & compatibilities;
		}
	};

	struct [[deprecated("Remove in Spine 1.28.0")]] RequestCompatibilityListMessage : public Message {
		int32_t modID;
		RequestCompatibilityListMessage() : Message(), modID() {
			type = MessageType::REQUESTCOMPATIBILITYLIST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
		}
	};

	struct [[deprecated("Remove in Spine 1.28.0")]] SendCompatibilityListMessage : public Message {
		std::vector<int32_t> impossiblePatches;
		std::vector<int32_t> forbiddenPatches;
		SendCompatibilityListMessage() : Message() {
			type = MessageType::SENDCOMPATIBILITYLIST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & impossiblePatches;
			ar & forbiddenPatches;
		}
	};

	struct SubmitRatingMessage : public Message {
		int32_t modID;
		int32_t rating;
		std::string username;
		std::string password;
		SubmitRatingMessage() : Message(), modID(), rating() {
			type = MessageType::SUBMITRATING;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & rating;
			ar & username;
			ar & password;
		}
	};

	struct UpdateRequestEncryptedMessage : public Message {
		uint8_t majorVersion;
		uint8_t minorVersion;
		uint8_t patchVersion;
		UpdateRequestEncryptedMessage() : Message(), majorVersion(), minorVersion(), patchVersion() {
			type = MessageType::UPDATEREQUESTENCRYPTED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & majorVersion;
			ar & minorVersion;
			ar & patchVersion;
		}
	};

	struct RequestOverallSaveDataMessage : public Message {
		int32_t modID;
		std::string username;
		std::string password;
		RequestOverallSaveDataMessage() : Message(), modID(), username(), password() {
			type = MessageType::REQUESTOVERALLSAVEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & username;
			ar & password;
		}
	};

	struct SendOverallSaveDataMessage : public Message {
		std::vector<std::pair<std::string, std::string>> data;
		SendOverallSaveDataMessage() : Message(), data() {
			type = MessageType::SENDOVERALLSAVEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & data;
		}
	};

	struct UpdateOverallSaveDataMessage : public Message {
		int32_t modID;
		std::string username;
		std::string password;
		std::string entry;
		std::string value;
		UpdateOverallSaveDataMessage() : Message(), modID(), username(), password(), entry(), value() {
			type = MessageType::UPDATEOVERALLSAVEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & username;
			ar & password;
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
		int32_t size;

		ModFile() : filename(), hash(), language(), changed(false), deleted(false), size(0) {
		}

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
		UploadModfilesMessage() : Message(), modID(), files() {
			type = MessageType::UPLOADMODFILES;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & files;
		}
	};

	struct RequestModsForEditorMessage : public Message {
		std::string username;
		std::string password;
		std::string language;
		RequestModsForEditorMessage() : Message(), username(), password(), language() {
			type = MessageType::REQUESTMODSFOREDITOR;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & language;
		}
	};

	struct SendModsForEditorMessage : public Message {
		struct ModForEditor {
			int32_t modID;
			std::string name;
			std::vector<std::pair<std::string, std::string>> images;

			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & name;
				ar & images;
			}
		};
		std::vector<ModForEditor> modList;
		SendModsForEditorMessage() : Message(), modList() {
			type = MessageType::SENDMODSFOREDITOR;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modList;
		}
	};

	struct UpdateOfflineDataMessage : public Message {
		struct AchievementData {
			int32_t modID;
			int32_t identifier;
			int32_t current;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & identifier;
				ar & current;
			}
		};
		struct ScoreData {
			int32_t modID;
			int32_t identifier;
			int32_t score;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & identifier;
				ar & score;
			}
		};
		struct OverallSaveData {
			int32_t modID;
			std::string entry;
			std::string value;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & entry;
				ar & value;
			}
		};
		std::string username;
		std::string password;
		std::vector<AchievementData> achievements;
		std::vector<ScoreData> scores;
		std::vector<OverallSaveData> overallSaves;
		std::vector<std::pair<int32_t, int32_t>> playTimes;
		UpdateOfflineDataMessage() : Message() {
			type = MessageType::UPDATEOFFLINEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & achievements;
			ar & scores;
			ar & overallSaves;
			ar & playTimes;
		}
	};

	struct RequestOfflineDataMessage : public Message {
		std::string username;
		std::string password;
		RequestOfflineDataMessage() : Message() {
			type = MessageType::REQUESTOFFLINEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
		}
	};

	struct SendOfflineDataMessage : public Message {
		struct AchievementData {
			int32_t modID;
			int32_t identifier;
			int32_t current;
			int32_t max;
			std::string username;
			bool unlocked;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & identifier;
				ar & current;
				ar & max;
				ar & username;
				ar & unlocked;
			}
		};
		struct ScoreData {
			int32_t modID;
			int32_t identifier;
			std::string username;
			int32_t score;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & identifier;
				ar & username;
				ar & score;
			}
		};
		struct OverallSaveData {
			int32_t modID;
			std::string username;
			std::string entry;
			std::string value;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & modID;
				ar & username;
				ar & entry;
				ar & value;
			}
		};
		std::vector<AchievementData> achievements;
		std::vector<ScoreData> scores;
		std::vector<OverallSaveData> overallSaves;
		SendOfflineDataMessage() : Message() {
			type = MessageType::SENDOFFLINEDATA;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & achievements;
			ar & scores;
			ar & overallSaves;
		}
	};

	struct UpdateStartTimeMessage : public Message {
		int16_t dayOfTheWeek;
		int16_t hour;
		UpdateStartTimeMessage() : Message(), dayOfTheWeek(), hour() {
			type = MessageType::UPDATESTARTTIME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & dayOfTheWeek;
			ar & hour;
		}
	};

	struct [[deprecated("Remove in Spine 1.28.0")]] UpdatePlayingTimeMessage : public Message {
		int16_t dayOfTheWeek;
		int16_t hour;
		UpdatePlayingTimeMessage() : Message(), dayOfTheWeek(), hour() {
			type = MessageType::UPDATEPLAYINGTIME;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & dayOfTheWeek;
			ar & hour;
		}
	};

	struct RequestAllFriendsMessage : public Message {
		std::string username;
		std::string password;
		RequestAllFriendsMessage() : Message() {
			type = MessageType::REQUESTALLFRIENDS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
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
		std::vector<Friend> pendingFriends;
		std::vector<Friend> friendRequests;
		std::vector<std::string> nonFriends;
		SendAllFriendsMessage() : Message() {
			type = MessageType::SENDALLFRIENDS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & friends;
			ar & pendingFriends;
			ar & friendRequests;
			ar & nonFriends;
		}
	};

	struct SendFriendRequestMessage : public Message {
		std::string username;
		std::string password;
		std::string friendname;
		SendFriendRequestMessage() : Message(), username(), password(), friendname() {
			type = MessageType::SENDFRIENDREQUEST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & friendname;
		}
	};

	struct AcceptFriendRequestMessage : public Message {
		std::string username;
		std::string password;
		std::string friendname;
		AcceptFriendRequestMessage() : Message(), username(), password(), friendname() {
			type = MessageType::ACCEPTFRIENDREQUEST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & friendname;
		}
	};

	struct DeclineFriendRequestMessage : public Message {
		std::string username;
		std::string password;
		std::string friendname;
		DeclineFriendRequestMessage() : Message(), username(), password(), friendname() {
			type = MessageType::DECLINEFRIENDREQUEST;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
			ar & friendname;
		}
	};

	struct RequestUserLevelMessage : public Message {
		std::string username;
		std::string password;
		RequestUserLevelMessage() : Message(), username(), password() {
			type = MessageType::REQUESTUSERLEVEL;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
		}
	};

	struct SendUserLevelMessage : public Message {
		uint32_t level;
		uint32_t currentXP;
		uint32_t nextXP;
		SendUserLevelMessage() : Message(), level(), currentXP(), nextXP() {
			type = MessageType::SENDUSERLEVEL;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & level;
			ar & currentXP;
			ar & nextXP;
		}
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

	struct UpdateSucceededMessage : public Message {
		int32_t modID;

		UpdateSucceededMessage() : Message(), modID() {
			type = MessageType::UPDATESUCCEEDED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
		}
	};

	struct UpdateChapterStatsMessage : public Message {
		int32_t modID;
		int32_t identifier;
		int32_t guild;
		std::string statName;
		int32_t statValue;

		UpdateChapterStatsMessage() : Message(), modID(), identifier(), guild(), statName(), statValue() {
			type = MessageType::UPDATECHAPTERSTATS;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & modID;
			ar & identifier;
			ar & guild;
			ar & statName;
			ar & statValue;
		}
	};

	struct IsAchievementUnlockedMessage : public Message {
		std::string username;
		std::string password;
		int32_t modID;
		int32_t achievementID;

		IsAchievementUnlockedMessage() : Message(), username(), password(), modID(), achievementID() {
			type = MessageType::ISACHIEVEMENTUNLOCKED;
		}
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<Message>(*this);
			ar & username;
			ar & password;
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

} /* namespace common */
} /* namespace spine */
