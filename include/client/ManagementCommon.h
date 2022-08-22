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

#pragma once

#include <cstdint>
#include <vector>

#include "common/GameType.h"
#include "common/ModType.h"
#include "common/ScoreOrder.h"

#include <QDate>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <QUrl>

class QJsonObject;

namespace spine {
namespace client {

	struct ManagementMod;

	typedef struct ManagementMod  {
		QString name;
		int id = 0;
		int packageID = -1;

		QList<ManagementMod> read(const QJsonObject & json);
	} ManagementMod;

	typedef struct ManagementTranslation {
		QString language;
		QString text;

		void read(const QJsonObject & json);
		void write(QJsonObject & json) const;
	} ManagementTranslation;

	typedef struct ManagementAchievement {
		QList<ManagementTranslation> names;
		QList<ManagementTranslation> descriptions;

		int32_t maxProgress = 0;
		bool hidden = false;

		QString lockedImageName;
		QString lockedImageHash;
		std::vector<uint8_t> lockedImageData; // <= :(

		QString unlockedImageName;
		QString unlockedImageHash;
		std::vector<uint8_t> unlockedImageData; // <= :(

		void read(const QJsonObject & json);
		void write(QJsonObject & json) const;

		bool isValid() const;
	} ManagementAchievement;

	typedef struct ManagementGeneralData {
		bool enabled = false;
		common::GameType gothicVersion;
		common::ModType modType;
		int32_t duration = 0;
		QDate releaseDate;
		QString feedbackMail;
		QUrl discussionUrl;
		QString keywords;

		void read(const QJsonObject & json);
		void write(QJsonObject & json) const;
	} ManagementGeneralData;

	typedef struct ManagementCustomStatistic {
		QString name;
		int32_t value = 0;

		void read(const QJsonObject & json);
	} ManagementCustomStatistic;

	typedef struct ManagementCustomStatistics {
		QMap<QPair<int, int>, QList<ManagementCustomStatistic>> stats;

		void read(const QJsonObject & json);
	} ManagementCustomStatistics;

	typedef struct ManagementModFile {
		QString filename;
		QString oldHash;
		QString hash;
		QString language;
		bool changed = false;
		bool deleted = false;
		int32_t size = 0;
		bool newFile = false;

		void read(const QJsonObject & json);

		bool operator==(const ManagementModFile & other) const;
	} ManagementModFile;

	typedef struct ManagementModFilesData {
		int32_t versionMajor;
		int32_t versionMinor;
		int32_t versionPatch;
		int32_t versionSpine;
		common::GameType gameType;

		QList<ManagementModFile> files;

		void read(const QJsonObject & json);
	} ManagementModFilesData;

	typedef struct ManagementVersionDownload {
		QString version;
		int32_t downloads;

		void read(const QJsonObject & json);
	} ManagementVersionDownload;

	typedef struct ManagementStatistic {
		int32_t maximum = 0;
		int32_t average = 0;
		int32_t median = 0;

		void read(const QJsonObject & json);
	} ManagementStatistic;

	typedef struct ManagementAchievementStatistic  {
		QString name;
		ManagementStatistic statistic;

		void read(const QJsonObject & json);
	} ManagementAchievementStatistic;

	typedef struct ManagementStatistics {
		int32_t overallDownloads;
		QList<ManagementVersionDownload> downloadsPerVersion;

		int32_t overallPlayerCount = 0;
		int32_t last24HoursPlayerCount = 0;
		int32_t last7DaysPlayerCount = 0;

		ManagementStatistic playTime;
		ManagementStatistic sessionTime;

		QList<ManagementAchievementStatistic> achievementStatistics;

		void read(const QJsonObject & json);
	} ManagementStatistics;

	typedef struct ManagementScore {
		QList<ManagementTranslation> names;
		common::ScoreOrder scoreOrder;

		void read(const QJsonObject & json);
		void write(QJsonObject & json) const;
	} ManagementScore;

	typedef struct ManagementSurvey {
		int32_t surveyID = 0;
		QString language;
		int32_t majorVersion = 0;
		int32_t minorVersion = 0;
		int32_t patchVersion = 0;
		int32_t questionCount = 0;
		int32_t answerCount = 0;
		bool enabled = false;

		void read(const QJsonObject & json);
	} ManagementSurvey;

	typedef struct ManagementSurveys  {
		QList<ManagementSurvey> surveys;

		void read(const QJsonObject & json);
	} ManagementSurveys;

	typedef struct ManagementSurveyAnswer {
		QString question;
		QStringList answers;

		void read(const QJsonObject & json);
	} ManagementSurveyAnswer;

	typedef struct ManagementSurveyAnswers {
		QList<ManagementSurveyAnswer> answers;

		void read(const QJsonObject & json);
	} ManagementSurveyAnswers;

	typedef struct ManagementSurveyQuestion {
		QString question;

		void read(const QJsonObject & json);
	} ManagementSurveyQuestion;

	typedef struct ManagementSurveyQuestions {
		QList<ManagementSurveyQuestion> questions;

		void read(const QJsonObject & json);
	} ManagementSurveyQuestions;

} /* namespace client */
} /* namespace spine */

Q_DECLARE_METATYPE(spine::client::ManagementAchievement)
Q_DECLARE_METATYPE(spine::client::ManagementAchievementStatistic)
Q_DECLARE_METATYPE(spine::client::ManagementCustomStatistic)
Q_DECLARE_METATYPE(spine::client::ManagementCustomStatistics)
Q_DECLARE_METATYPE(spine::client::ManagementGeneralData)
Q_DECLARE_METATYPE(spine::client::ManagementMod)
Q_DECLARE_METATYPE(spine::client::ManagementModFile)
Q_DECLARE_METATYPE(spine::client::ManagementModFilesData)
Q_DECLARE_METATYPE(spine::client::ManagementScore)
Q_DECLARE_METATYPE(spine::client::ManagementStatistic)
Q_DECLARE_METATYPE(spine::client::ManagementStatistics)
Q_DECLARE_METATYPE(spine::client::ManagementSurvey)
Q_DECLARE_METATYPE(spine::client::ManagementSurveyAnswer)
Q_DECLARE_METATYPE(spine::client::ManagementSurveyAnswers)
Q_DECLARE_METATYPE(spine::client::ManagementSurveys)
Q_DECLARE_METATYPE(spine::client::ManagementTranslation)
Q_DECLARE_METATYPE(spine::client::ManagementVersionDownload)
Q_DECLARE_METATYPE(QList<spine::client::ManagementMod>)
