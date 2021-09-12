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

#include <QApplication>
#include <QMap>

using namespace spine::common;
using namespace spine::utils;

QString LanguageConverter::convert(Language language) {
	static QMap<Language, QString> languageStrings = {
		{ Language::German, QStringLiteral("Deutsch") },
		{ Language::English, QStringLiteral("English") },
		{ Language::Polish, QStringLiteral("Polish") },
		{ Language::Russian, QStringLiteral("Russian") },
	};

	return languageStrings.value(language);
}

Language LanguageConverter::convert(const QString & language) {
	static QMap<QString, Language> languageStrings = {
		{ QStringLiteral("Deutsch"), Language::German },
		{ QStringLiteral("English"), Language::English },
		{ QStringLiteral("Polish"), Language::Polish },
		{ QStringLiteral("Russian"), Language::Russian },
	};

	return languageStrings.value(language, None);
}

QStringList LanguageConverter::getLanguages()
{
	QStringList languages;
	languages << QApplication::tr("German") << QApplication::tr("English") << QApplication::tr("Polish") << QApplication::tr("Russian");

	return languages;
}
