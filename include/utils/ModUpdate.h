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

#include "common/GameType.h"

#include <QList>
#include <QPair>
#include <QString>

namespace spine {
namespace utils {

	struct ModUpdate {
		int32_t modID;
		QString name;
		int8_t majorVersion;
		int8_t minorVersion;
		int8_t patchVersion;
		int8_t spineVersion;
		QList<QPair<QString, QString>> files;
		QList<QPair<int32_t, QList<QPair<QString, QString>>>> packageFiles;
		QString fileserver;
		QString fallbackFileserver;
		common::GameType gothicVersion;
		bool savegameCompatible;
		QString changelog;

		ModUpdate() : modID(0), majorVersion(0), minorVersion(0), patchVersion(0), spineVersion(0), gothicVersion(common::GameType::Gothic), savegameCompatible(false) {}
	};

} /* namespace common */
} /* namespace utils */
