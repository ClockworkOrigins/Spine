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

#include "ManagementCommon.h"

#include <QJsonArray>
#include <QJsonObject>

namespace spine {
namespace client {

	void ManagementMod::read(const QJsonObject & json) {
		if (json.isEmpty()) return;

		if (!json.contains("Name")) return;
				
		if (!json.contains("ID")) return;

		name = json["Name"].toString();
		id = json["ID"].toInt();
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
			maxProgress = it->toInt();
		}
		
		// hidden
		it = json.find("Hidden");
		if (it != json.end()) {
			hidden = it->toBool();
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

		enabled = json["Enabled"].toBool();
		gothicVersion = static_cast<common::GothicVersion>(json["GothicVersion"].toInt());
		modType = static_cast<common::ModType>(json["ModType"].toInt());
		duration = json["Duration"].toInt();
		const int dateOffset = json["ReleaseDate"].toInt();

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
		value = json["Value"].toInt();
	}

	void ManagementCustomStatistics::read(const QJsonObject & json) {
		if (!json.contains("Stats")) return;
				
		const auto arr = json["Stats"].toArray();
		for (const auto entry : arr) {
			const auto jo = entry.toObject();

			if (!jo.contains("ID")) continue;
			
			if (!jo.contains("Guild")) continue;
			
			if (!jo.contains("Entries")) continue;

			QPair<int32_t, int32_t> p = qMakePair(jo["ID"].toInt(), jo["Guild"].toInt());
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

		versionMajor = json["VersionMajor"].toInt();
		versionMinor = json["VersionMinor"].toInt();
		versionPatch = json["VersionPatch"].toInt();
				
		const auto arr = json["Files"].toArray();
		for (const auto entry : arr) {
			const auto jo = entry.toObject();

			ManagementModFile mmf;
			mmf.read(jo);

			files.append(mmf);
		}
	}

} /* namespace client */
} /* namespace spine */
