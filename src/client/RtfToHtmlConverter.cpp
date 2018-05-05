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
// Copyright 2018 Clockwork Origins

#include "RtfToHtmlConverter.h"

#include <QDebug>

namespace spine {

	QString RtfToHtmlConverter::convert(QString rtfText) {
		if (!rtfText.isEmpty()) {
			rtfText = rtfText.replace("\\b0", "</b>");
			rtfText = rtfText.replace("\\b", "<b>");
			rtfText = rtfText.replace("\\i0", "</i>");
			rtfText = rtfText.replace("\\i", "<i>");
			rtfText = rtfText.replace("\\par", "<br/>");
			rtfText = rtfText.replace("\\fs", "<br/>");
			rtfText = rtfText.replace("\\tab", "&#09;");
			rtfText = rtfText.replace("d\\qc\\lang1031", "");
			rtfText = rtfText.replace(R"(\rtf1\ansi\ansicpg1252)", "");
			rtfText = rtfText.replace("\\ ", "");
			rtfText = rtfText.replace("{", "");
			rtfText = rtfText.replace("}", "");
			if (rtfText.contains("\\")) {
				rtfText.clear();
			}
		}
		rtfText.insert(0, "<center>");
		rtfText.append("</center>");
		return rtfText;
	}

} /* namespace spine */
