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
// Copyright 2020 Clockwork Origins

#pragma once

#include <QDialog>

class QTextEdit;

namespace spine {
namespace widgets {

	typedef struct {
		int questionID;
		QString question;
		QString answer;
	} SurveyQuestion;
	
	typedef struct {
		int surveyID;
		QList<SurveyQuestion> questions;
	} Survey;

	class SurveyDialog : public QDialog {
		Q_OBJECT

	public:
		SurveyDialog(const Survey & survey, int majorVersion, int minorVersion, int patchVersion, QWidget * par);

	private:
		Survey _survey;
		int _majorVersion;
		int _minorVersion;
		int _patchVersion;
		QList<QTextEdit * > _editList;
	};

} /* namespace widgets */
} /* namespace spine */
