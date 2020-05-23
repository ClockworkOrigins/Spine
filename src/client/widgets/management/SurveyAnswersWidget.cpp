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

#include "widgets/management/SurveyAnswersWidget.h"

#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;

SurveyAnswersWidget::SurveyAnswersWidget(QWidget * par) : QWidget(par), _projectID(0), _surveyID(0), _waitSpinner(nullptr) {
	QVBoxLayout * vlOuter = new QVBoxLayout();
		
	QScrollArea * sa = new QScrollArea(this);
	QWidget * cw = new QWidget(sa);
	_layout = new QVBoxLayout();
	_layout->setAlignment(Qt::AlignTop);
	cw->setLayout(_layout);
	sa->setWidget(cw);
	sa->setWidgetResizable(true);
	sa->setProperty("default", true);
	cw->setProperty("default", true);

	vlOuter->addWidget(sa, 1);

	{
		QHBoxLayout * hl = new QHBoxLayout();

		QPushButton * pb = new QPushButton(QApplication::tr("Back"));

		hl->addStretch(1);
		hl->addWidget(pb);

		vlOuter->addLayout(hl);

		connect(pb, &QPushButton::released, this, &SurveyAnswersWidget::backClicked);
	}

	setLayout(vlOuter);

	connect(this, &SurveyAnswersWidget::removeSpinner, [this]() {
		if (!_waitSpinner) return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});
}

void SurveyAnswersWidget::updateView(int projectID, int surveyID) {
	qDeleteAll(_widgetList);
	_widgetList.clear();

	_projectID = projectID;
	_surveyID = surveyID;

	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = _projectID;
	requestData["SurveyID"] = _surveyID;
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "getAllPlayTestSurveyAnswers", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		ManagementSurveyAnswers msa;
		msa.read(json);
		
		emit loadedData(msa);
		emit removeSpinner();
	});
}

void SurveyAnswersWidget::updateData(ManagementSurveyAnswers content) {
	for (const auto & msa : content.answers) {
		QLabel * questionLbl = new QLabel(msa.question, this);
		questionLbl->setProperty("big", true);
		questionLbl->setProperty("bold", true);
		_layout->addWidget(questionLbl);

		for (const auto & answer : msa.answers) {
			if (answer.isEmpty()) continue;
			
			QLabel * answerLbl = new QLabel(answer, this);
			_layout->addWidget(answerLbl);
		}
	}
}
