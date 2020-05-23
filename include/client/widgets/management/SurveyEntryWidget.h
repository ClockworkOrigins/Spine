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

#include "ManagementCommon.h"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace spine {
namespace client {
namespace widgets {

	class SurveyEntryWidget : public QWidget {
		Q_OBJECT

	public:
		SurveyEntryWidget(int projectID, ManagementSurvey survey, QWidget * par);

	signals:
		void releaseClicked();
		void editClicked(int surveyID);
		void showAnswersClicked(int surveyID);
		void deleteClicked();

	private slots:
		void wantToDelete();
		void release();

	private:
		int _projectID;
		ManagementSurvey _survey;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
