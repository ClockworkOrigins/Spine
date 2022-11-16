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

#include <QDir>
#include <QFileInfo>
#include <QQueue>
#include <QRegularExpression>
#include <QTextStream>

using namespace spine::utils;

namespace {
	constexpr auto HEADER_SIZE = 296;
	constexpr auto ENTRY_HEADER_SIZE = 80;
}

GothicVdf::GothicVdf(const QString & path) : _file(path), _header() {}

GothicVdf::~GothicVdf() {
	close();
}

bool GothicVdf::parse() {
	const auto suffix = QFileInfo(_file).suffix().toLower();

	if (suffix != "vdf" && suffix != "mod")
		return false;

	if (!_file.open(QIODevice::ReadOnly))
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

	// must be 80 for valid G1/G2 files, other value implies some compressed/encrypted Union stuff which is not supported as of now
	if (_header.entrySize != 80)
		return false;

	_entries.clear();

	QList<QQueue<QString>> dirList;

	quint32 parsedEntries = 0;

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

		entryHeader.type = toInt(type);

		const auto isDir = entryHeader.type == 0xc0000000;
		const auto isMultiDir = entryHeader.type == 0x80000000;
		const auto isLastFileInFolder = entryHeader.type == 0x40000000;

		if ((isDir || isMultiDir) && !wasMultiDir) {
			QQueue<QString> q;
			q << entryHeader.name;
			dirList << q;

			if (isMultiDir) {
				wasMultiDir = true;
			}
		} else if (isDir || isMultiDir) {
			dirList.back() << entryHeader.name;
		}

		entryHeader.isDir = isDir || isMultiDir;

		QString dirPath;

		for (int i = 0; i < dirList.count(); i++) {
			dirPath += dirList[i].front();

			if (i < dirList.count() - 1) {
				dirPath += "/";
			}

			if (wasMultiDir && i == dirList.count() - 2) {
				dirPath += entryHeader.name;
				break;
			}
		}

		if (wasMultiDir && !isMultiDir) {
			wasMultiDir = false;
		}

		quint32 closestBelow = 0;
		int closestBelowIdx = 0;

		for (int i = 0; i < _entries.count(); i++) {
			const auto e = _entries[i];

			if (!e.isDir)
				continue;

			if (e.offset >= static_cast<quint32>(_entries.count() + 1))
				continue;

			if (e.offset <= closestBelow)
				continue;

			closestBelow = e.offset;
			closestBelowIdx = i;
		}

		if (!_entries.isEmpty()) {
			dirPath = _entries[closestBelowIdx].path;

			if (entryHeader.isDir)
				dirPath += "/" + entryHeader.name;
		}

		entryHeader.path = (dirList.count() == 0 && (isDir || isMultiDir)) || closestBelow == 0 ? entryHeader.name : dirPath;

		if (isLastFileInFolder && !dirList.isEmpty()) {
			if (entryHeader.path.endsWith(dirList.back().head())) {
				dirList.back().dequeue();
			}

			while (!dirList.isEmpty() && dirList.back().isEmpty()) {
				dirList.pop_back();

				if (dirList.isEmpty())
					break;

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

	return parsedEntries == _header.numEntries;
}

void GothicVdf::close() {
	_file.close();
}

void GothicVdf::write(const QString & outPath) const {
	QFile f(outPath);
	if (!f.open(QIODevice::WriteOnly))
		return;

	auto files = getFiles();

	files.sort();

	QMap<QString, QStringList> fileTree;

	for (const auto & file : files) {
		const auto splits = file.split("/");

		if (splits[0] == "SPLASH.BMP")
			continue;

		QString path = "/";
		for (int i = 0; i < splits.count(); i++) {
			const auto newPath = path + "/" + splits[i];

			if (!fileTree.contains(path)) {
				fileTree.insert(path, { newPath });
			} else {
				auto & list = fileTree[path];

				if (!list.contains(newPath)) {
					list << newPath;
				}
			}

			path = newPath;
		}
	}

	Header header = _header;
	header.numFiles = files.count();
	header.dataSize = 0;
	header.numEntries = 0;

	for (auto it = fileTree.begin(); it != fileTree.end(); ++it) {
		header.numEntries += it.value().count();
	}

	QList<EntryHeader> entryHeaders;

	createHeaders("/", fileTree, entryHeaders, header);
	adjustHeader(entryHeaders, header);

	writeHeader(f, header);
	writeEntryHeaders(f, entryHeaders);
	writeEntries(f, entryHeaders);
}

void GothicVdf::remove(const QString & file) {
	auto split = file.split("/");

	const auto name = split.back();
	split.pop_back();

	const auto path = split.join("/");

	for (auto it = _entries.begin(); it != _entries.end(); ++it) {
		if (it->name != name)
			continue;

		if (it->path != path)
			continue;

		_entries.erase(it);

		break;
	}
}

void GothicVdf::remove(const QStringList & fileList) {
	for (const auto & file : fileList) {
		remove(file);
	}
}

QStringList GothicVdf::getFiles() const {
	QStringList files;

	for (const auto & header : _entries) {
		if (header.isDir) continue;

		files << header.path + "/" + header.name;
	}

	return files;
}

QString GothicVdf::getHash(int idx) const {
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

QStringList GothicVdf::getDeletableFiles(const QMap<QString, QString> & modkitFiles) const {
	QStringList deletableFiles;

	const auto files = getFiles();

	auto containsOuBin = false;

	for (const auto & f : files) {
		if (!f.contains("OU.BIN", Qt::CaseInsensitive))
			continue;

		containsOuBin = true;

		break;
	}

	for (int i = 0; i < files.count(); i++) {
		const auto & f = files[i];

		if (!f.startsWith("_WORK")) {
			if (!f.startsWith("NINJA") && !f.startsWith("SYSTEM/AUTORUN") && !f.startsWith("/") && !f.contains("SPLASH.BMP", Qt::CaseInsensitive)) { // skip Ninja and Union folders
				deletableFiles << f;
			}
		} else {
			auto copyF = f;
			copyF = copyF.replace("_WORK/", "");
			copyF = copyF.toLower();

			const auto it2 = modkitFiles.find(copyF);

			const auto suffix = QFileInfo(copyF).suffix();

			if (it2 != modkitFiles.end()) {
				QString modFileHash = getHash(i);

				if (modFileHash == it2.value()) {
					deletableFiles << f;
				}
			} else if (suffix == "src" || suffix == "d") {
				deletableFiles << f;
			} else if (suffix == "bak") {
				deletableFiles << f;
			} else if (suffix == "csl" && containsOuBin) {
				deletableFiles << f;
			} else if (copyF.endsWith("ouinfo.inf")) {
				deletableFiles << f;
			} else if (copyF.startsWith("demo/")) {
				deletableFiles << f;
			} else if (copyF.startsWith("tools/")) {
				deletableFiles << f;
			} else if (copyF.contains(QRegularExpression("_compiled/[^/]+/"))) {
				deletableFiles << f;
			} else if (copyF.startsWith("data/scripts/content/cutscene/") && !copyF.endsWith("ou.bin")) {
				deletableFiles << f;
			}
		}
	}

	return deletableFiles;
}

GothicVdf::Result GothicVdf::optimize(const QString & path, const QString & gothicVersion) {
	const auto tmpPath = QDir::tempPath();

	auto result = optimize(path, tmpPath, gothicVersion);

	if (result.status == Result::Status::Fail)
		return result;

	QFile::moveToTrash(path);

	QFile::copy(result.resultPath, path);

	QFile::remove(result.resultPath);

	return result;
}

GothicVdf::Result GothicVdf::optimize(const QString & path, const QString & outFolder, const QString & gothicVersion) {
	Result result;
	result.status = Result::Status::Success;

	GothicVdf vdf(path);
	const auto b = vdf.parse();
	if (!b) {
		result.status = Result::Status::Fail;
		return result;
	}

	const auto files = vdf.getFiles();

	const auto modkitFiles = parseResource(QString(":/%1.txt").arg(gothicVersion));

	const auto deletableFiles = vdf.getDeletableFiles(modkitFiles);

	result.fileCount = files.count();
	result.strippedFileCount = deletableFiles.count();

	for (const auto & f : deletableFiles) {
		vdf.remove(f);

		result.log += f + "\n";
	}

	result.resultPath = outFolder + "/" + path.split("/").back();
	if (!QDir(outFolder).exists()) {
		const auto b2 = QDir(outFolder).mkpath(outFolder);
		Q_UNUSED(b2)
	}
	if (result.strippedFileCount > 0) {
		vdf.write(result.resultPath);
		vdf.close();
	} else {
		vdf.close();
		QFile::copy(path, result.resultPath);
	}
	result.strippedSize = QFileInfo(path).size() - QFileInfo(result.resultPath).size();

	return result;
}

QMap<QString, QString> GothicVdf::parseResource(const QString & file) {
	QFile f(file);

	QMap<QString, QString> results;

	if (!f.open(QIODevice::ReadOnly))
		return results;

	QTextStream ts(&f);

	while (!ts.atEnd()) {
		const auto line = ts.readLine();

		const auto split = line.split(";");

		Q_ASSERT(split.count() == 2);

		if (split.count() != 2)
			continue;

		results.insert(split[0].toLower(), split[1]);
	}

	return results;
}

bool GothicVdf::updateTimestamp(const QString & path, quint32 timestamp) {
	QFile file(path);

	const auto suffix = QFileInfo(path).suffix().toLower();

	if (suffix != "vdf" && suffix != "mod")
		return false;

	if (!file.open(QIODevice::ReadWrite))
		return false;

	const auto commentData = file.read(256);

	const auto signature = file.read(16);

	if (signature != "PSVDSC_V2.00\n\r\n\r" && signature != "PSVDSC_V2.00\r\n\r\n")
		return false;

	const auto numEntries = file.read(4);

	const auto numFiles = file.read(4);

	const auto ts = file.read(4);

	file.seek(256 + 16 + 4 + 4);

	const auto b = file.write(fromInt(timestamp));
	Q_UNUSED(b)

	file.close();

	return true;
}

void GothicVdf::fromMsDosTime(quint32 msDosTimestamp, quint32 & year, quint32 & month, quint32 & day, quint32 & hour, quint32 & minute, quint32 & second) {
	hour = (msDosTimestamp & 63488) / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2;
	minute = (msDosTimestamp & 2016) / 2 / 2 / 2 / 2 / 2;
	second = (msDosTimestamp & 31) * 2;

	msDosTimestamp = msDosTimestamp / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2;

	year = ((msDosTimestamp & 65024) / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2 / 2) + 1980;
	month = (msDosTimestamp & 480) / 2 / 2 / 2 / 2 / 2;
	day = msDosTimestamp & 31;
}

quint32 GothicVdf::toInt(const QByteArray & bytes) {
	if (bytes.count() != 4)
		return 0;

	const auto v1 = static_cast<quint32>(static_cast<uint8_t>(bytes[0]));
	const auto v2 = static_cast<quint32>(static_cast<uint8_t>(bytes[1]));
	const auto v3 = static_cast<quint32>(static_cast<uint8_t>(bytes[2]));
	const auto v4 = static_cast<quint32>(static_cast<uint8_t>(bytes[3]));

	quint32 result = 0;
	result += v1;
	result += v2 * 256;
	result += v3 * 256 * 256;
	result += v4 * 256 * 256 * 256;

	return result;
}

QByteArray GothicVdf::fromInt(quint32 v) {
	const auto v1 = v % 256;
	v /= 256;
	const auto v2 = v % 256;
	v /= 256;
	const auto v3 = v % 256;
	v /= 256;
	const auto v4 = v;

	const auto v1_char = static_cast<char>(static_cast<uint8_t>(v1));
	const auto v2_char = static_cast<char>(static_cast<uint8_t>(v2));
	const auto v3_char = static_cast<char>(static_cast<uint8_t>(v3));
	const auto v4_char = static_cast<char>(static_cast<uint8_t>(v4));

	QByteArray bytes(4, '0');
	bytes[0] = v1_char;
	bytes[1] = v2_char;
	bytes[2] = v3_char;
	bytes[3] = v4_char;

	return bytes;
}

void GothicVdf::createHeaders(const QString & path, const QMap<QString, QStringList> & fileTree, QList<EntryHeader> & headers, Header & header) const {
	const auto it = fileTree.find(path);

	if (it == fileTree.end())
		return;

	auto list = it.value();
	std::sort(list.begin(), list.end(), [](const QString & a, const QString & b) {
		if (a.contains(".") && !b.contains("."))
			return false;

		if (!a.contains(".") && b.contains("."))
			return true;

		return a < b;
	});

	QQueue<QString> subPaths;

	for (int i = 0; i < list.count(); i++) {
		const auto p = list[i];

		const auto it2 = fileTree.find(p);

		if (it2 == fileTree.end()) {
			// it's a file
			auto list2 = p.split("/", Qt::SkipEmptyParts);

			auto name = list2.back();
			list2.pop_back();

			const auto p2 = list2.join("/");

			for (const auto & entry : _entries) {
				if (entry.name != name)
					continue;

				if (entry.path != p2)
					continue;

				auto e = entry;
				e.type = p == list.back() ? 0x40000000 : 0x00000000;

				headers << e;

				break;
			}
		} else {
			// it's a folder
			auto list2 = p.split("/", Qt::SkipEmptyParts);
			auto name = list2.back();

			const auto p2 = list2.join("/");

			for (const auto & entry : _entries) {
				if (entry.name != name)
					continue;

				if (entry.path != p2)
					continue;

				auto e = entry;
				e.type = p == list.back() ? 0xc0000000 : 0x80000000;

				headers << e;

				subPaths << p;

				break;
			}
		}
	}

	while (!subPaths.isEmpty()) {
		const auto p = subPaths.dequeue();
		createHeaders(p, fileTree, headers, header);
	}
}

void GothicVdf::writeHeader(QFile & f, const Header & header) const {
	f.write(header.comment.toLatin1(), 256);
	f.write(header.signature.toLatin1(), 16);
	f.write(fromInt(header.numEntries));
	f.write(fromInt(header.numFiles));
	f.write(fromInt(header.timestamp));
	f.write(fromInt(header.dataSize));
	f.write(fromInt(header.rootOffset));
	f.write(fromInt(header.entrySize));
}

void GothicVdf::writeEntryHeaders(QFile & f, const QList<EntryHeader> & entryHeaders) const {
	for (const auto & header : entryHeaders) {
		f.write(header.name.toLatin1());
		f.write(QByteArray(64 - header.name.toLatin1().size(), ' '));
		f.write(fromInt(header.offset));
		f.write(fromInt(header.size));
		f.write(fromInt(header.type));
		f.write(fromInt(header.attributes));
	}
}

void GothicVdf::adjustHeader(QList<EntryHeader> & headers, Header & header) const {
	header.dataSize = 0;

	const auto fixOffset = ENTRY_HEADER_SIZE * headers.count() + HEADER_SIZE;

	for (auto i = 0; i < headers.count(); i++) {
		if (headers[i].isDir) {
			for (auto j = 0; j < headers.count(); j++) {
				if (headers[j].name == headers[i].name)
					continue;

				if (!headers[j].isDir && headers[j].path != headers[i].path)
					continue;

				if (headers[j].isDir && !headers[j].path.contains(headers[i].path + "/"))
					continue;

				headers[i].offset = j;

				break;
			}
		} else {
			headers[i].offset = fixOffset + header.dataSize;
		}
		header.dataSize += headers[i].size;
	}
}

void GothicVdf::writeEntries(QFile & f, const QList<EntryHeader> & entryHeaders) const {
	for (const auto & header : entryHeaders) {
		if (header.isDir)
			continue;

		for (const auto & entry : _entries) {
			if (entry.name != header.name)
				continue;

			if (entry.path != header.path)
				continue;

			_file.seek(entry.offset);

			f.write(_file.read(entry.size));
		}
	}
}

quint32 GothicVdf::toMsDosTime(quint32 year, quint32 month, quint32 day, quint32 hour, quint32 minute, quint32 second) {
	const auto y = (year - 1980) * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2;
	const auto mon = month * 2 * 2 * 2 * 2 * 2;
	const auto d = day;
	const auto date = (y + mon + d) * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2;

	const auto h = hour * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2;
	const auto min = minute * 2 * 2 * 2 * 2 * 2;
	const auto sec = second / 2;
	const auto time = h + min + sec;

	return time + date;
}
