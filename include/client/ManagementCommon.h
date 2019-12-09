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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
// Copyright 2019 Clockwork Origins

#pragma once

#include <cstdint>
#include <vector>

#include "common/GothicVersion.h"
#include "common/ModType.h"

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

class QJsonObject;

namespace spine {
namespace client {

	typedef struct {
		QString name;
		int id;

		void read(const QJsonObject &json);
	} ManagementMod;

	typedef struct {
		QString language;
		QString text;

		void read(const QJsonObject &json);
		void write(QJsonObject & json) const;
	} ManagementTranslation;

	typedef struct {
		QList<ManagementTranslation> names;
		QList<ManagementTranslation> descriptions;

		int32_t maxProgress;
		bool hidden;

		QString lockedImageName;
		QString lockedImageHash;
		std::vector<uint8_t> lockedImageData; // <= :(

		QString unlockedImageName;
		QString unlockedImageHash;
		std::vector<uint8_t> unlockedImageData; // <= :(

		void read(const QJsonObject &json);
		void write(QJsonObject & json) const;

		bool isValid() const;
	} ManagementAchievement;

	typedef struct {
		bool enabled;
		common::GothicVersion gothicVersion;
		common::ModType modType;
		int32_t duration;
		QDate releaseDate;

		void read(const QJsonObject &json);
		void write(QJsonObject & json) const;
	} ManagementGeneralData;

	typedef struct {
		QString name;
		int32_t value;

		void read(const QJsonObject &json);
	} ManagementCustomStatistic;

	typedef struct {
		QMap<QPair<int, int>, QList<ManagementCustomStatistic>> stats;

		void read(const QJsonObject &json);
	} ManagementCustomStatistics;

} /* namespace client */
} /* namespace spine */
