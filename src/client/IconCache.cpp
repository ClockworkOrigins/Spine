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

#include "IconCache.h"

#include "utils/Config.h"

#include <QDirIterator>

using namespace spine::client;
using namespace spine::utils;

IconCache * IconCache::getInstance() {
	static IconCache factory;
	return &factory;
}

QPixmap IconCache::getIcon(int32_t projectID) const {
	return _iconCache.value(projectID, QPixmap());
}

bool IconCache::hasIcon(int32_t projectID) const {
	return _iconCache.contains(projectID);
}

void IconCache::cacheIcon(int32_t projectID, const QString & icon) {
	if (icon.isEmpty()) return;
	
	const QFileInfo fi(icon);

	if (!fi.exists()) return;

	QFile::copy(icon, QString("%1/icons/%2.%3").arg(Config::DOWNLOADDIR).arg(projectID).arg(fi.suffix()));
	
	const auto fileName = fi.fileName();
	const auto split = fileName.split('.', QString::SkipEmptyParts);
	if (!fileName.isEmpty() && split.count() == 2) {
		_iconCache[projectID] = QPixmap(icon);
	}
	_iconCache[projectID] = icon;
}

IconCache::IconCache() {
	loadCache();
}

void IconCache::loadCache() {
	if (!QDir(QString("%1/icons").arg(Config::DOWNLOADDIR)).exists()) {
		const auto b = QDir().mkpath(QString("%1/icons").arg(Config::DOWNLOADDIR));
		Q_UNUSED(b);
	}
	QDirIterator it(QString("%1/icons").arg(Config::DOWNLOADDIR), { "*.bmp", "*.ico", "*.png", "*.jpg" }, QDir::Files);
	while (it.hasNext()) {
		it.next();
		const QString filePath = it.filePath();
		const QString fileName = it.fileName();
		const auto split = fileName.split('.', QString::SkipEmptyParts);
		if (!filePath.isEmpty() && !fileName.isEmpty() && split.count() == 2) {
			_iconCache[split[0].toInt()] = QPixmap(filePath);
		}
	}
}
