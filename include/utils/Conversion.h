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

#include <cfloat>
#include <cmath>
#include <string>

#include <QApplication>
#include <QString>

namespace spine {
namespace utils {

#define decodeString(str) str.replace("&apos;", "'")
#define encodeString(str) str.replace("'", "&apos;").trimmed()
#define s2q(str) decodeString(QString::fromUtf8(std::string(str).c_str()))
#define q2s(str) encodeString(QString(str)).toUtf8().toStdString()
#define q2ws(str) encodeString(QString(str)).toStdWString()
#define i2s(number) QLocale::system().toString(number)

	inline QString byteToString(qint64 value) {
		QString unit = "B";
		double dSize = static_cast<double>(value);
		while (dSize > 1024 && unit != "GB") {
			dSize /= 1024.0;
			if (unit == "B") {
				unit = "KB";
			} else if (unit == "KB") {
				unit = "MB";
			} else if (unit == "MB") {
				unit = "GB";
			}
		}
		return QString::number(dSize, 'f', 1) + " " + unit;
	}

	inline QString bytePerTimeToString(qint64 value, double time) {
		QString unit = "B";
		double dSize = value / (time / 1000.0);
		while (dSize > 1024 && unit != "GB") {
			dSize /= 1024.0;
			if (unit == "B") {
				unit = "KB";
			} else if (unit == "KB") {
				unit = "MB";
			} else if (unit == "MB") {
				unit = "GB";
			}
		}
		return QString::number(dSize, 'f', 1) + " " + unit + "/s";
	}

	inline QString timeToString(double duration) {
		QString timeString;
		if (std::abs(duration + 1) < std::numeric_limits<double>::epsilon() || std::abs(duration) < std::numeric_limits<double>::epsilon()) {
			timeString = "-";
		} else {
			if (duration > 90) {
				timeString = i2s((static_cast<int>(duration) + 30) / 60) + " " + (duration >= 90 ? QApplication::tr("Hours") : QApplication::tr("Hour"));
			} else {
				timeString = QString::number(static_cast<int>(duration)) + " " + ((duration > 1 || std::abs(duration) < std::numeric_limits<double>::epsilon()) ? QApplication::tr("Minutes") : QApplication::tr("Minute"));
			}
		}
		return timeString;
	}

} /* namespace utils */
} /* namespace spine */
