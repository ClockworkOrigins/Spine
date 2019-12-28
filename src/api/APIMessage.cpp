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

#include "api/APIMessage.h"

#include <sstream>

namespace spine {
namespace api {

	std::string APIMessage::serialize() const {
		std::stringstream ss;
		boost::archive::text_oarchive arch(ss);
		APIMessage * m = const_cast<APIMessage *>(this);
		arch << m;
		return ss.str();
	}

	APIMessage * APIMessage::deserialize(const std::string & s) {
		APIMessage * m = nullptr;
		std::stringstream ss(s);
		boost::archive::text_iarchive arch(ss);
		arch >> m;
		return m;
	}

} /* namespace api */
} /* namespace spine */

BOOST_CLASS_EXPORT_GUID(spine::api::APIMessage, "SPA_0")
BOOST_CLASS_IMPLEMENTATION(spine::api::APIMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::api::APIMessage_I, "SPA_1")
BOOST_CLASS_IMPLEMENTATION(spine::api::APIMessage_I, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::api::APIMessage_S, "SPA_2")
BOOST_CLASS_IMPLEMENTATION(spine::api::APIMessage_S, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::api::APIMessage_I4, "SPA_3")
BOOST_CLASS_IMPLEMENTATION(spine::api::APIMessage_I4, boost::serialization::object_serializable)
