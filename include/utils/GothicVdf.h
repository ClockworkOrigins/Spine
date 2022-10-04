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
		typedef struct Result {
			enum class Status {
				Success,
				Fail
			};

			Status status = Status::Success;
			quint64 fileCount = 0;
			quint64 strippedFileCount = 0;
			quint64 strippedSize = 0;
			QString log;
			QString resultPath;
		} Result;

		GothicVdf(const QString & path);
		~GothicVdf();

		bool parse();
		void close();

		void write(const QString & outPath) const;

		void remove(const QString & file);
		void remove(const QStringList & fileList);

		QStringList getFiles() const;

		QString getHash(int idx) const;

		QStringList getDeletableFiles(const QMap<QString, QString> & modkitFiles) const;

		static Result optimize(const QString & path, const QString & gothicVersion);
		static Result optimize(const QString & path, const QString & outFolder, const QString & gothicVersion);

		static QMap<QString, QString> parseResource(const QString & file);

		static bool updateTimestamp(const QString & path, quint32 timestamp);

		static void fromMsDosTime(quint32 msDosTimestamp, quint32 & year, quint32 & month, quint32 & day, quint32 & hour, quint32 & minute, quint32 & second);
		static quint32 toMsDosTime(quint32 year, quint32 month, quint32 day, quint32 hour, quint32 minute, quint32 second);

	private:
		typedef struct {
			QString comment;
			QString signature;
			quint32 numEntries;
			quint32 numFiles;
			quint32 timestamp;
			quint32 dataSize;
			quint32 rootOffset;
			quint32 entrySize;
		} Header;

		typedef struct {
			QString name;
			quint32 offset;
			quint32 size;
			quint32 type;
			quint32 attributes;
			QString path;
			bool isDir;
		} EntryHeader;

		mutable QFile _file;

		Header _header;

		QList<EntryHeader> _entries;

		static quint32 toInt(const QByteArray & bytes);
		static QByteArray fromInt(quint32 v);

		void createHeaders(const QString & path, const QMap<QString, QStringList> & fileTree, QList<EntryHeader> & headers, Header & header) const;
		void adjustHeader(QList<EntryHeader> & headers, Header & header) const;

		void writeHeader(QFile & f, const Header & header) const;
		void writeEntryHeaders(QFile & f, const QList<EntryHeader> & entryHeaders) const;
		void writeEntries(QFile & f, const QList<EntryHeader> & entryHeaders) const;
	};

} /* namespace common */
} /* namespace utils */
