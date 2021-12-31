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

#pragma once

#include <cstdint>

#include <QIcon>
#include <QMap>
#include <QPixmap>

namespace spine {
namespace client {

	class IconCache {
	public:
		static IconCache * getInstance();

		QPixmap getIcon(int32_t projectID) const;
		bool hasIcon(int32_t projectID) const;

		void cacheIcon(int32_t projectID, const QString & icon);

		QIcon getOrLoadIcon(const QString & path);
		QImage getOrLoadIconAsImage(const QString & path);

	private:
		struct Icon {
			QPixmap icon;
			QString hash;
		};

		QMap<int32_t, Icon> _iconCache;
		QMap<QString, QIcon> _pathIconCache;

		IconCache();
		
		void loadCache();
	};

} /* namespace client */
} /* namespace spine */
