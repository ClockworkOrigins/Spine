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

#include "api/zString.h"

namespace spine {
namespace api {

	zString::zString() : _vtbl(nullptr), _allocater(0), _ptr(new char[256]), _len(256), _res(256) {
	}

	zString::zString(const zString & other) : _vtbl(other._vtbl), _allocater(other._allocater), _ptr(other._ptr), _len(other._len), _res(other._res) {
	}

	zString::zString(const std::string & other) : _vtbl(nullptr), _allocater(0), _ptr(new char[256]), _len(other.size()), _res(other.size()) {
		strcpy(_ptr, other.data());
	}

	zString::~zString() {
		delete[] _ptr;
	}

} /* namespace api */
} /* namespace spine */
