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
#include "utils/Hashing.h"

#include <QDirIterator>
#include <QPainter>

using namespace spine::client;
using namespace spine::utils;

IconCache * IconCache::getInstance() {
	static IconCache factory;
	return &factory;
}

QPixmap IconCache::getIcon(int32_t projectID) const {
	return _iconCache.value(projectID, Icon()).icon;
}

bool IconCache::hasIcon(int32_t projectID) const {
	return _iconCache.contains(projectID);
}

void IconCache::cacheIcon(int32_t projectID, const QString & icon) {
	if (icon.isEmpty()) return;
	
	const QFileInfo fi(icon);

	if (!fi.exists()) return;

	QString hash;
	Hashing::hash(icon, hash);

	if (hasIcon(projectID)) {
		if (hash == _iconCache.value(projectID).hash) return;
	}

	QFile::copy(icon, QString("%1/icons/%2.%3").arg(Config::DOWNLOADDIR).arg(projectID).arg(fi.suffix()));
	
	const auto fileName = fi.fileName();
	const auto split = fileName.split('.', Qt::SkipEmptyParts);

	Icon i;
	i.hash = hash;
	i.icon = icon;

	_iconCache[projectID] = i;
}

QIcon IconCache::getOrLoadIcon(const QString & path) {
	if (_pathIconCache.contains(path)) return _pathIconCache[path];

	QIcon icon(path);

	_pathIconCache.insert(path, icon);

	return icon;
}

QImage IconCache::getOrLoadIconAsImage(const QString & path) {
	const auto icon = getOrLoadIcon(path);
	const auto pixmap = icon.pixmap(icon.availableSizes()[0]);
	return pixmap.toImage();
}

IconCache::IconCache() {
	loadCache();
}

void IconCache::loadCache() {
	if (!QDir(QString("%1/icons").arg(Config::DOWNLOADDIR)).exists()) {
		const auto b = QDir().mkpath(QString("%1/icons").arg(Config::DOWNLOADDIR));
		Q_UNUSED(b)
	}
	QDirIterator it(QString("%1/icons").arg(Config::DOWNLOADDIR), { "*.bmp", "*.ico", "*.png", "*.jpg" }, QDir::Files);
	while (it.hasNext()) {
		it.next();
		const QString filePath = it.filePath();
		const QString fileName = it.fileName();
		const auto split = fileName.split('.', Qt::SkipEmptyParts);
		if (!filePath.isEmpty() && !fileName.isEmpty() && split.count() == 2) {
			const auto id = split[0].toInt();

			QString hash;
			Hashing::hash(filePath, hash);

			Icon i;
			i.icon = filePath;
			i.hash = hash;

			_iconCache[id] = i;

			const auto icoPath = Config::DOWNLOADDIR + "/icons/" + QString::number(id) + ".ico";
			
			if (!QFileInfo::exists(icoPath)) {
				QIcon ico(filePath);
				const auto b = _iconCache[id].icon.save(icoPath);
				Q_UNUSED(b)
			}
		}
	}
}
