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
// Copyright 2019 Clockwork Origins

#include "ServerCommon.h"

#include <regex>

namespace spine {
namespace server {

	std::string ServerCommon::convertString(const std::string & str) {
		std::string result = str;
		result = std::regex_replace(result, std::regex("%20"), " ");
		result = std::regex_replace(result, std::regex("%21"), "!");
		result = std::regex_replace(result, std::regex("%22"), "\"");
		result = std::regex_replace(result, std::regex("%23"), "#");
		result = std::regex_replace(result, std::regex("%24"), "$");
		result = std::regex_replace(result, std::regex("%25"), "%");
		result = std::regex_replace(result, std::regex("%26"), "&");
		result = std::regex_replace(result, std::regex("%27"), "'");
		result = std::regex_replace(result, std::regex("%28"), "(");
		result = std::regex_replace(result, std::regex("%29"), ")");
		result = std::regex_replace(result, std::regex("%2a"), "*");
		result = std::regex_replace(result, std::regex("%2b"), "+");
		result = std::regex_replace(result, std::regex("%2c"), ",");
		result = std::regex_replace(result, std::regex("%2d"), "-");
		result = std::regex_replace(result, std::regex("%2e"), ".");
		result = std::regex_replace(result, std::regex("%2f"), "/");
		result = std::regex_replace(result, std::regex("%3a"), ":");
		result = std::regex_replace(result, std::regex("%3b"), ";");
		result = std::regex_replace(result, std::regex("%3c"), "<");
		result = std::regex_replace(result, std::regex("%3d"), "=");
		result = std::regex_replace(result, std::regex("%3e"), ">");
		result = std::regex_replace(result, std::regex("%3f"), "?");
		result = std::regex_replace(result, std::regex("%40"), "@");
		result = std::regex_replace(result, std::regex("%5b"), "[");
		result = std::regex_replace(result, std::regex("%5c"), "\\");
		result = std::regex_replace(result, std::regex("%5d"), "]");
		result = std::regex_replace(result, std::regex("%5e"), "^");
		result = std::regex_replace(result, std::regex("%5f"), "_");
		result = std::regex_replace(result, std::regex("%7b"), "{");
		result = std::regex_replace(result, std::regex("%7d"), "}");
		return result;
	}
	
} /* namespace server */
} /* namespace spine */
