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
// Copyright 2021 Clockwork Origins

#include "MissingMetadataChecker.h"

#include "SpineConfig.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include <QDirIterator>
#include <QtConcurrentRun>

using namespace spine::client;
using namespace spine::utils;

MissingMetadataChecker::MissingMetadataChecker() {
    connect(this, &MissingMetadataChecker::finished, this, &QObject::deleteLater);
}

void MissingMetadataChecker::check() {
    QtConcurrent::run([this]() {
        Database::DBError err;
        const auto projects = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods", err);

        static QStringList folderWhitelist = { "screens", "achievements" };

        QDirIterator it(Config::DOWNLOADDIR + "/mods/", QDir::Filter::Dirs);
        while (it.hasNext()) {
            it.next();

            const auto dirName = it.fileName();

            bool b = true;
            const auto projectID = dirName.toInt(&b);

            if (!b) continue;

            const auto filePath = it.filePath();
            QFileInfoList fiList = QDir(filePath).entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);

            bool allInWhiteList = !fiList.isEmpty();
        	for (const auto & fi : fiList) {
                const auto fileName = fi.fileName();
        		
                if (fi.isDir() && folderWhitelist.contains(fileName)) continue;

                allInWhiteList = false;
        		
                break;
        	}

            if (allInWhiteList) continue;

            if (std::find_if(projects.begin(), projects.end(), [projectID](int i) {
                return i == projectID;
            }) != projects.end()) continue;

            emit install(projectID);
        }

        emit finished();
	});
}
