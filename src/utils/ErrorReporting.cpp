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

#include "ErrorReporting.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Conversion.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace spine {
namespace utils {

	QSet<QString> errorList;

	void ErrorReporting::report(const QString & message) {
		QString errorMessage = message;
        errorMessage = errorMessage.replace("\r\n", "<br>");
        errorMessage = errorMessage.replace("\n", "<br>");
        errorMessage = errorMessage.replace("\"", "&quot;");
        errorMessage = errorMessage.replace("\'", "&apos;");
		
		if (errorList.contains(errorMessage)) return;

		errorList.insert(errorMessage);

		QJsonObject json;
		json["project"] = 0;
		json["version"] = s2q(VERSION_STRING);
		json["report"] = errorMessage;

		https::Https::postAsync(19101, "reportCrash", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});
	}
	
} /* namespace utils */
} /* namespace spine */
