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
// Copyright 2020 Clockwork Origins

#include "LanguageConverter.h"

#include "common/Language.h"

#include <map>

using namespace spine::common;
using namespace spine::server;

std::string LanguageConverter::convert(Language language) {
	static std::map<Language, std::string> languageStrings = {
		{ Language::German, "Deutsch" },
		{ Language::English, "English" },
		{ Language::Polish, "Polish" },
		{ Language::Russian, "Russian" },
	};

	return languageStrings[language];
}

Language LanguageConverter::convert(const std::string & language) {
	static std::map<std::string, Language> languageStrings = {
		{ "Deutsch", Language::German },
		{ "English", Language::English },
		{ "Polish", Language::Polish },
		{ "Russian", Language::Russian },
	};

	return languageStrings[language];
}

