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

#include "ManagementCommon.h"

#include <QJsonArray>
#include <QJsonObject>

namespace spine {
namespace client {

	// boost ptree serializes everything as string, so this has be taken into account here
	
	void ManagementMod::read(const QJsonObject & json) {
		if (json.isEmpty()) return;

		if (!json.contains("Name")) return;
				
		if (!json.contains("ID")) return;

		name = json["Name"].toString();
		id = json["ID"].toString().toInt();
	}

	void ManagementTranslation::read(const QJsonObject & json) {
		if (!json.contains("Language")) return;
				
		if (!json.contains("Text")) return;

		language = json["Language"].toString();
		text = json["Text"].toString();
	}

	void ManagementTranslation::write(QJsonObject & json) const {
		json["Language"] = language;
		json["Text"] = text;
	}

	void ManagementAchievement::read(const QJsonObject & json) {
		// name translations
		auto it = json.find("Names");
		if (it != json.end()) {
			const auto n = it->toArray();
			for (auto nameEntry : n) {
				const auto name = nameEntry.toObject();

				if (name.isEmpty()) continue;

				ManagementTranslation mt;
				mt.read(name);
				names.append(mt);
			}
		}
		
		// description translations
		it = json.find("Descriptions");
		if (it != json.end()) {
			const auto d = it->toArray();
			for (auto descriptionEntry : d) {
				const auto description = descriptionEntry.toObject();

				if (description.isEmpty()) continue;

				ManagementTranslation mt;
				mt.read(description);
				descriptions.append(mt);
			}
		}
		
		// maxProgress
		it = json.find("MaxProgress");
		if (it != json.end()) {
			maxProgress = it->toString().toInt();
		}
		
		// hidden
		it = json.find("Hidden");
		if (it != json.end()) {
			hidden = it->toString() == "true";
		}
		
		// lockedImageName
		it = json.find("LockedImageName");
		if (it != json.end()) {
			lockedImageName = it->toString();
		}
		
		// lockedImageHash
		it = json.find("LockedImageHash");
		if (it != json.end()) {
			lockedImageHash = it->toString();
		}
		
		// unlockedImageName
		it = json.find("UnlockedImageName");
		if (it != json.end()) {
			unlockedImageName = it->toString();
		}
		
		// unlockedImageHash
		it = json.find("UnlockedImageHash");
		if (it != json.end()) {
			unlockedImageHash = it->toString();
		}
	}

	void ManagementAchievement::write(QJsonObject & json) const {
		QJsonArray nameArray;
		for (const auto & name : names) {
			QJsonObject n;
			name.write(n);
			nameArray.append(n);
		}
		json["Names"] = nameArray;
		
		QJsonArray descriptionArray;
		for (const auto & description : descriptions) {
			QJsonObject d;
			description.write(d);
			descriptionArray.append(d);
		}
		json["Descriptions"] = descriptionArray;

		json["MaxProgress"] = maxProgress;
		json["Hidden"] = hidden;
		
		json["LockedImageName"] = lockedImageName;
		json["LockedImageHash"] = lockedImageHash;
		json["UnlockedImageName"] = unlockedImageName;
		json["UnlockedImageHash"] = unlockedImageHash;
	}

	bool ManagementAchievement::isValid() const {
		return !names.empty();
	}

	void ManagementGeneralData::read(const QJsonObject & json) {
		if (!json.contains("Enabled")) return;
				
		if (!json.contains("GothicVersion")) return;
		
		if (!json.contains("ModType")) return;
		
		if (!json.contains("ReleaseDate")) return;
		
		if (!json.contains("Duration")) return;

		enabled = json["Enabled"].toString() == "true";
		gothicVersion = static_cast<common::GameType>(json["GothicVersion"].toString().toInt());
		modType = static_cast<common::ModType>(json["ModType"].toString().toInt());
		duration = json["Duration"].toString().toInt();
		const int dateOffset = json["ReleaseDate"].toString().toInt();

		QDate rd(2000, 1, 1);
		rd = rd.addDays(dateOffset);
		releaseDate = rd;
	}

	void ManagementGeneralData::write(QJsonObject & json) const {
		json["Enabled"] = enabled;
		json["GothicVersion"] = static_cast<int32_t>(gothicVersion);
		json["ModType"] = static_cast<int32_t>(modType);
		json["Duration"] = duration;

		const QDate date(2000, 1, 1);
		const auto rd = static_cast<int32_t>(date.daysTo(releaseDate));
		
		json["ReleaseDate"] = rd;
	}

	void ManagementCustomStatistic::read(const QJsonObject & json) {
		if (!json.contains("Name")) return;
				
		if (!json.contains("Value")) return;

		name = json["Name"].toString();
		value = json["Value"].toString().toInt();
	}

	void ManagementCustomStatistics::read(const QJsonObject & json) {
		if (!json.contains("Stats")) return;
				
		const auto arr = json["Stats"].toArray();
		for (const auto entry : arr) {
			const auto jo = entry.toObject();

			if (!jo.contains("ID")) continue;
			
			if (!jo.contains("Guild")) continue;
			
			if (!jo.contains("Entries")) continue;

			QPair<int32_t, int32_t> p = qMakePair(jo["ID"].toString().toInt(), jo["Guild"].toString().toInt());
			QList<ManagementCustomStatistic> list;

			const auto entries = jo["Entries"].toArray();
			for (const auto entry2 : entries) {
				const auto e = entry2.toObject();

				ManagementCustomStatistic mcs;
				mcs.read(e);

				list.append(mcs);
			}

			stats.insert(p, list);
		}
	}

	void ManagementModFile::read(const QJsonObject & json) {
		if (!json.contains("Name")) return;
				
		if (!json.contains("Hash")) return;
		
		if (!json.contains("Language")) return;

		filename = json["Name"].toString();
		hash = json["Hash"].toString();
		language = json["Language"].toString();

		changed = false;
		deleted = false;
		size = 0;
	}

	void ManagementModFilesData::read(const QJsonObject & json) {
		if (!json.contains("VersionMajor")) return;
		
		if (!json.contains("VersionMinor")) return;
		
		if (!json.contains("VersionPatch")) return;
		
		if (!json.contains("Files")) return;

		versionMajor = json["VersionMajor"].toString().toInt();
		versionMinor = json["VersionMinor"].toString().toInt();
		versionPatch = json["VersionPatch"].toString().toInt();
				
		const auto arr = json["Files"].toArray();
		for (const auto entry : arr) {
			const auto jo = entry.toObject();

			ManagementModFile mmf;
			mmf.read(jo);

			files.append(mmf);
		}
	}

	void ManagementVersionDownload::read(const QJsonObject & json) {
		if (!json.contains("Version")) return;
		
		if (!json.contains("Count")) return;

		version = json["Version"].toString();
		downloads = json["Count"].toString().toInt();
	}

	void ManagementStatistic::read(const QJsonObject & json) {
		if (!json.contains("Minimum")) return;
		
		if (!json.contains("Maximum")) return;
		
		if (!json.contains("Median")) return;
		
		if (!json.contains("Average")) return;

		minimum = json["Minimum"].toString().toInt();
		maximum = json["Maximum"].toString().toInt();
		median = json["Median"].toString().toInt();
		average = json["Average"].toString().toInt();
	}

	void ManagementAchievementStatistic::read(const QJsonObject & json) {
		if (!json.contains("Name")) return;

		name = json["Name"].toString();

		statistic.read(json);
	}

	void ManagementStatistics::read(const QJsonObject & json) {
		if (!json.contains("OverallDownloads")) return;
		
		if (!json.contains("DownloadsPerVersion")) return;
		
		if (!json.contains("OverallPlayerCount")) return;
		
		if (!json.contains("Last24HoursPlayerCount")) return;
		
		if (!json.contains("Last7DaysPlayerCount")) return;
		
		if (!json.contains("PlayTime")) return;
		
		if (!json.contains("SessionTime")) return;
		
		if (!json.contains("Achievements")) return;

		overallDownloads = json["OverallDownloads"].toString().toInt();

		const auto downloadsPerVersionArr = json["DownloadsPerVersion"].toArray();
		for (const auto & entry : downloadsPerVersionArr) {
			const auto e = entry.toObject();

			ManagementVersionDownload mvd;
			mvd.read(e);

			downloadsPerVersion.append(mvd);
		}

		overallPlayerCount = json["OverallPlayerCount"].toString().toInt();
		last24HoursPlayerCount = json["Last24HoursPlayerCount"].toString().toInt();
		last7DaysPlayerCount = json["Last7DaysPlayerCount"].toString().toInt();

		const auto playTimeJson = json["PlayTime"].toObject();
		playTime.read(playTimeJson);

		const auto sessionTimeJson = json["SessionTime"].toObject();
		sessionTime.read(sessionTimeJson);

		const auto achievementStatisticsArr = json["Achievements"].toArray();
		for (const auto & entry : achievementStatisticsArr) {
			const auto e = entry.toObject();

			ManagementAchievementStatistic mas;
			mas.read(e);

			achievementStatistics.append(mas);
		}
	}

	void ManagementScore::read(const QJsonObject & json) {
		// name translations
		auto it = json.find("Names");
		if (it != json.end()) {
			const auto n = it->toArray();
			for (auto nameEntry : n) {
				const auto name = nameEntry.toObject();

				if (name.isEmpty()) continue;

				ManagementTranslation mt;
				mt.read(name);
				names.append(mt);
			}
		}
	}

	void ManagementScore::write(QJsonObject & json) const {
		QJsonArray nameArray;
		for (const auto & name : names) {
			QJsonObject n;
			name.write(n);
			nameArray.append(n);
		}
		json["Names"] = nameArray;
	}

} /* namespace client */
} /* namespace spine */
