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
// Copyright 2022 Clockwork Origins

#pragma once

#include <QFile>
#include <QList>
#include <QString>

namespace spine {
namespace utils {

	class GothicVdf {
	public:
		GothicVdf(const QString & path);
		~GothicVdf();

		bool parse();
		void close();

		QStringList getFiles() const;

		QString getHash(int idx);

	private:
		typedef struct {
			QString comment;
			QString signature;
			qint32 numEntries;
			qint32 numFiles;
			qint32 timestamp;
			qint32 dataSize;
			qint32 rootOffset;
			qint32 entrySize;
		} Header;

		typedef struct {
			QString name;
			qint32 offset;
			qint32 size;
			qint32 type;
			qint32 attributes;
			QString path;
			bool isDir;
		} EntryHeader;

		QFile _file;

		Header _header;

		QList<EntryHeader> _entries;

		qint32 toInt(const QByteArray & bytes) const;
	};

} /* namespace common */
} /* namespace utils */
