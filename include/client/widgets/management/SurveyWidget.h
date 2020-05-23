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

#include "widgets/management/IManagementWidget.h"

#include <QModelIndex>
#include <QWidget>

class QComboBox;
class QSpinBox;
class QStackedWidget;
class QVBoxLayout;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class SurveyWidget : public QWidget, public IManagementWidget {
		Q_OBJECT

	public:
		SurveyWidget(QWidget * par);

		void updateModList(QList<ManagementMod> modList);
		void selectedMod(int index);
		void updateView() override;

	signals:
		void removeSpinner();
		void loadedData(ManagementSurveys);
		void triggerUpdate();

	private slots:
		void updateData(ManagementSurveys content);
		void createSurvey();

	private:
		QList<ManagementMod> _mods;
		int _modIndex;
		QStackedWidget * _stackedWidget;
		QVBoxLayout * _surveysLayout;
		QVBoxLayout * _questionsLayout;
		QVBoxLayout * _answersLayout;
		gui::WaitSpinner * _waitSpinner;

		QList<QWidget *> _surveyWidgetList;
		QList<QWidget *> _questionsWidgetList;
		QList<QWidget *> _answersWidgetList;

		QSpinBox * _majorVersionBox;
		QSpinBox * _minorVersionBox;
		QSpinBox * _patchVersionBox;
		QComboBox * _languageBox;

		ManagementSurveys _managementSurveys;
		int _surveyIndex;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
