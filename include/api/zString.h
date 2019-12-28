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

#ifndef __SPINE_API_ZSTRING_H__
#define __SPINE_API_ZSTRING_H__

#include <cstdint>
#include <cstring>
#include <string>

#include "boost/serialization/split_member.hpp"

namespace spine {
namespace api {

	class zString {
		friend class boost::serialization::access;

	public:
		zString();
		zString(const zString & other);
		zString(const std::string & other);
		~zString();

		template<class Archive>
		void save(Archive & ar, const unsigned int) const {
			std::string s;
			s.resize(_len);
			strcpy(const_cast<char *>(s.data()), _ptr);
			ar & s;
		}

		template<class Archive>
		void load(Archive & ar, const unsigned int) {
			std::string s;
			ar & s;
		}

	private:
		void * _vtbl;
		int32_t _allocater; //immer 0
		char * _ptr; //pointer zu den Daten
		int32_t _len; //Länge des Strings
		int32_t _res; //Anzahl allozierter Bytes

		BOOST_SERIALIZATION_SPLIT_MEMBER()
	};

} /* namespace api */
} /* namespace spine */

#endif /* __SPINE_API_ZSTRING_H__ */

