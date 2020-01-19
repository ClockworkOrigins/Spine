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

#include <cstdint>

#include "api/API.h"
#include "api/zString.h"

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"

namespace spine {
namespace api {

	enum APIMessageType {
		BASE,
		INT,
		STRING,
		INT4,
		INT3
	};

	struct APIMessage {
		int32_t messageType;
		int32_t userType;
		zString username;

		APIMessage() : messageType(APIMessageType::BASE), userType(0), username() {
		}

		APIMessage(APIMessageType type) : messageType(type), userType(0), username() {
		}

		virtual ~APIMessage() {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & messageType;
			ar & userType;
			ar & username;
		}

		std::string serialize() const;
		static APIMessage * deserialize(const std::string & s);
	};

	struct APIMessage_I : APIMessage {
		int param;

		APIMessage_I() : APIMessage(APIMessageType::INT) {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<APIMessage>(*this);
			ar & param;
		}
	};

	struct APIMessage_S : APIMessage {
		zString param;

		APIMessage_S() : APIMessage(APIMessageType::STRING) {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<APIMessage>(*this);
			ar & param;
		}
	};

	struct APIMessage_I3 : APIMessage {
		int param1;
		int param2;
		int param3;

		APIMessage_I3() : APIMessage(APIMessageType::INT3) {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<APIMessage>(*this);
			ar & param1;
			ar & param2;
			ar & param3;
		}
	};

	struct APIMessage_I4 : APIMessage {
		int param1;
		int param2;
		int param3;
		int param4;

		APIMessage_I4() : APIMessage(APIMessageType::INT4) {
		}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & boost::serialization::base_object<APIMessage>(*this);
			ar & param1;
			ar & param2;
			ar & param3;
			ar & param4;
		}
	};

} /* namespace api */
} /* namespace spine */
