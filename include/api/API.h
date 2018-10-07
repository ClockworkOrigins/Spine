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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2018 Clockwork Origins

#ifndef __SPINE_API_SPINEAPI_H__
#define __SPINE_API_SPINEAPI_H__

#include <cstdint>
#include <string>

#if WIN32
	#define SPINEAPI_EXPORTS extern "C" __declspec(dllexport)
#else
	#define SPINEAPI_EXPORTS
#endif

namespace clockUtils {
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace spine {
namespace api {

	extern int32_t activatedModules;
	extern bool initialized;
	extern std::string username;
	extern clockUtils::sockets::TcpSocket * sock;
	extern int32_t modID;

	void reconnect();

	/**
	 * \brief initializes all stuff
	 */
	SPINEAPI_EXPORTS int32_t init(int32_t modules);

	/**
	 * \brief returns the username of the user currently logged into Spine
	 */
	SPINEAPI_EXPORTS void getUsername(char * str);

	/**
	 * \brief updates score for given id
	 */
	SPINEAPI_EXPORTS void updateScore(int32_t id, int32_t score);

	/**
	 * \brief get score of current user for given id
	 */
	SPINEAPI_EXPORTS int32_t getUserScore(int32_t id);

	/**
	 * \brief get rank of current user for given id
	 */
	SPINEAPI_EXPORTS int32_t getUserRank(int32_t id);

	/**
	 * \brief gets score for given id and rank
	 */
	SPINEAPI_EXPORTS int32_t getScoreForRank(int32_t id, int32_t rank);

	/**
	 * \brief gets username for given id and rank
	 */
	SPINEAPI_EXPORTS void getUsernameForRank(int32_t id, int32_t rank, char * str);

	/**
	 * \brief gets score for given id and username
	 */
	SPINEAPI_EXPORTS int32_t getScoreForUsername(int32_t id, const char * user);

	/**
	 * \brief unlocks achievement with given id
	 */
	SPINEAPI_EXPORTS void unlockAchievement(int32_t id);

	/**
	 * \brief checks whether achievement with given id is already unlocked
	 */
	SPINEAPI_EXPORTS int32_t isAchievementUnlocked(int32_t id);

	/**
	 * \brief set achievement progress
	 */
	SPINEAPI_EXPORTS int32_t updateAchievementProgress(int32_t id, int32_t progress);

	/**
	 * \brief get current progress of the achievement
	 */
	SPINEAPI_EXPORTS int32_t getAchievementProgress(int32_t id);

	/**
	 * \brief get maximum progress of the achievement
	 */
	SPINEAPI_EXPORTS int32_t getAchievementMaxProgress(int32_t id);

	/**
	 * \brief checks whether achievement with given id of another modification is already unlocked
	 */
	SPINEAPI_EXPORTS int32_t isAchievementOfOtherModUnlocked(int32_t id, int32_t achievementID);

	/**
	 * \brief sets string to savegame
	 */
	SPINEAPI_EXPORTS void setOverallSaveValue(const char * key, const char * value);

	/**
	 * \brief returns string from savegame
	 */
	SPINEAPI_EXPORTS void getOverallSaveValue(const char * key, char * value);

	/**
	 * \brief sets int to savegame
	 */
	SPINEAPI_EXPORTS void setOverallSaveValueInt(const char * key, int value);

	/**
	 * \brief returns int from savegame
	 */
	SPINEAPI_EXPORTS int getOverallSaveValueInt(const char * key);

	/**
	 * \brief returns if achievement view is enabled or disabled
	 */
	SPINEAPI_EXPORTS int getShowAchievements();

	/**
	 * \brief returns if game currently is in testmode or if it's valid
	 */
	SPINEAPI_EXPORTS int getTestMode();

} /* namespace api */
} /* namespace spine */

#endif /* __SPINE_API_SPINEAPI_H__ */

