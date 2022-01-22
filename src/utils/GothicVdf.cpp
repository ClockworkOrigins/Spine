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

#include "utils/GothicVdf.h"

#include "utils/Hashing.h"

#include <QFileInfo>
#include <QQueue>

using namespace spine::utils;

namespace {
	constexpr auto HEADER_SIZE = 296;
	constexpr auto ENTRY_HEADER_SIZE = 80;
}

GothicVdf::GothicVdf(const QString & path) : _file(path) {}

GothicVdf::~GothicVdf() {
	close();
}

bool GothicVdf::parse() {
	const auto suffix = QFileInfo(_file).suffix().toLower();

	if (suffix != "vdf" && suffix != "mod")
		return false;

	if (!_file.open(QIODevice::ReadWrite))
		return false;

	const auto commentData = _file.read(256);

	_header.comment = QString::fromLatin1(commentData);

	const auto signature = _file.read(16);

	_header.signature = QString::fromLatin1(signature);

	if (_header.signature != "PSVDSC_V2.00\n\r\n\r" && _header.signature != "PSVDSC_V2.00\r\n\r\n")
		return false;

	const auto numEntries = _file.read(4);

	_header.numEntries = toInt(numEntries);

	const auto numFiles = _file.read(4);

	_header.numFiles = toInt(numFiles);

	const auto timestamp = _file.read(4);

	_header.timestamp = toInt(timestamp);

	const auto dataSize = _file.read(4);

	_header.dataSize = toInt(dataSize);

	const auto rootOffset = _file.read(4);

	_header.rootOffset = toInt(rootOffset);

	const auto entrySize = _file.read(4);

	_header.entrySize = toInt(entrySize);

	_entries.clear();

	QList<QQueue<QString>> dirList;

	qint32 parsedEntries = 0;

	bool wasMultiDir = false;

	while (!_file.atEnd()) {
		EntryHeader entryHeader;

		const auto name = _file.read(64);

		entryHeader.name = QString::fromLatin1(name).trimmed();

		const auto offset = _file.read(4);

		entryHeader.offset = toInt(offset);

		const auto size = _file.read(4);

		entryHeader.size = toInt(size);

		const auto type = _file.read(4);

		const auto typeStr = type.toHex();

		const auto isDir = toInt(type) == 0xc0000000;
		const auto isMultiDir = toInt(type) == 0x80000000;
		const auto isLastFileInFolder = toInt(type) == 0x40000000;

		if ((isDir || isMultiDir) && !wasMultiDir) {
			QQueue<QString> q;
			q << entryHeader.name.trimmed();
			dirList << q;

			if (isMultiDir) {
				wasMultiDir = true;
			}
		} else if (isMultiDir || isDir) {
			dirList.back() << entryHeader.name.trimmed();

			if (isDir) {
				wasMultiDir = false;
			}
		}

		if (wasMultiDir && !isMultiDir) {
			wasMultiDir = false;
		}

		entryHeader.isDir = isDir;

		QString dirPath;

		for (int i = 0; i < dirList.count(); i++) {
			dirPath += dirList[i].front();

			if (i < dirList.count() - 1) {
				dirPath += "/";
			}
		}

		entryHeader.path = dirPath;

		if (isLastFileInFolder) {
			dirList.back().dequeue();

			while (!dirList.isEmpty() && dirList.back().isEmpty()) {
				dirList.pop_back();

				if (dirList.isEmpty()) break;

				dirList.back().dequeue();
			}
		}

		const auto attributes = _file.read(4);

		entryHeader.attributes = toInt(attributes);

		_entries << entryHeader;

		parsedEntries++;

		if (parsedEntries == _header.numEntries)
			break;
	}

	return true;
}

void GothicVdf::close() {
	_file.close();
}

QStringList GothicVdf::getFiles() const {
	QStringList files;

	for (const auto & header : _entries) {
		if (header.isDir) continue;

		files << header.path + "/" + header.name;
	}

	return files;
}

QString GothicVdf::getHash(int idx) {
	quint64 offset = HEADER_SIZE;

	for (int i = 0; i < _entries.count(); i++) {
		const auto header = _entries[i];

		offset += ENTRY_HEADER_SIZE;

		if (header.isDir) continue;

		if (idx > 0) {
			offset += header.size;

			idx--;
			continue;
		}

		_file.seek(header.offset);

		const auto data = _file.read(header.size);

		QString hash;
		Hashing::hash(data, hash);

		return hash;
	}

	return QString();
}

qint32 GothicVdf::toInt(const QByteArray & bytes) const {
	if (bytes.count() != 4)
		return -1;

	const auto v1 = static_cast<qint32>(static_cast<uint8_t>(bytes[0]));
	const auto v2 = static_cast<qint32>(static_cast<uint8_t>(bytes[1]));
	const auto v3 = static_cast<qint32>(static_cast<uint8_t>(bytes[2]));
	const auto v4 = static_cast<qint32>(static_cast<uint8_t>(bytes[3]));

	qint32 result = 0;
	result += v1;
	result += v2 * 256;
	result += v3 * 256 * 256;
	result += v4 * 256 * 256 * 256;

	return result;
}
