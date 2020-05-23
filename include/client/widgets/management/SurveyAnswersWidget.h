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

class QVBoxLayout;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class SurveyAnswersWidget : public QWidget {
		Q_OBJECT

	public:
		SurveyAnswersWidget(QWidget * par);

		void updateView(int projectID, int surveyID);

	signals:
		void backClicked();
		void removeSpinner();
		void loadedData(ManagementSurveyAnswers);

	private slots:
		void updateData(ManagementSurveyAnswers content);

	private:
		int _projectID;
		int _surveyID;

		QVBoxLayout * _layout;

		QList<QWidget *> _widgetList;
		
		gui::WaitSpinner * _waitSpinner;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
