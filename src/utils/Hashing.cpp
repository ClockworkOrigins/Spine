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
// Copyright 2019 Clockwork Origins

#include "Hashing.h"

#include <QCryptographicHash>
#include <QFile>

using namespace spine::utils;

bool Hashing::hash(const QString & file, QString & hash) {
	QFile f(file);
	
	if (!f.open(QFile::ReadOnly)) return false;
	
	QCryptographicHash cryptoHash(QCryptographicHash::Sha512);
	if (cryptoHash.addData(&f)) {
		hash = QString::fromLatin1(cryptoHash.result().toHex());
		return true;
	}
	
	return false;
}

bool Hashing::hash(const QByteArray & bytes, QString & hash) {
	QCryptographicHash cryptoHash(QCryptographicHash::Sha512);
	cryptoHash.addData(bytes);

	hash = QString::fromLatin1(cryptoHash.result().toHex());
	return true;
}

bool Hashing::checkHash(const QString & file, const QString & referenceHash) {
	QString h;
	const bool b = hash(file, h);

	return b && h == referenceHash;
}
