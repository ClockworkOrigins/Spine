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

#ifndef __SPINE_API_MULTIPLAYER_H__
#define __SPINE_API_MULTIPLAYER_H__

#include <cstdint>
#include <string>

#include "api/API.h"

namespace spine {
namespace api {

	struct APIMessage;

	bool initializeMultiplayer();

	/**
	 * \brief sets server hostname to override default
	 */
	SPINEAPI_EXPORTS void setHostname(const char * hostname);

	/**
	 * \brief search for a match
	 */
	SPINEAPI_EXPORTS void searchMatch(int32_t numPlayers, int32_t identifier);

	/**
	 * \brief search for a match with a friend
	 */
	SPINEAPI_EXPORTS void searchMatchWithFriend(int32_t identifier, const char * friendName);

	/**
	 * \brief stop search
	 */
	SPINEAPI_EXPORTS void stopSearchMatch();

	/**
	 * \brief is current user in a match
	 */
	SPINEAPI_EXPORTS int32_t isInMatch();

	/**
	 * \brief returns amount of players in current multiplayer match
	 */
	SPINEAPI_EXPORTS int32_t getPlayerCount();

	/**
	 * \brief returns name of player with number
	 */
	SPINEAPI_EXPORTS void getPlayerUsername(int32_t player, char * str);
	
	/**
	 * \brief creates a new Message
	 */
	SPINEAPI_EXPORTS APIMessage * createMessage(int32_t messageType);
	
	/**
	 * \brief deletes a Message
	 */
	SPINEAPI_EXPORTS void deleteMessage(APIMessage * message);
	
	/**
	 * \brief sends a Message
	 */
	SPINEAPI_EXPORTS void sendMessage(APIMessage * message);
	
	/**
	 * \brief returns a Message
	 */
	SPINEAPI_EXPORTS APIMessage * receiveMessage();
	
	/**
	 * \brief returns true or false depending if in online mode or not
	 */
	SPINEAPI_EXPORTS int32_t isOnline();

} /* namespace api */
} /* namespace spine */

#endif /* __SPINE_API_MULTIPLAYER_H__ */

