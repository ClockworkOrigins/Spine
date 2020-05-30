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

#include "widgets/management/SurveyEditWidget.h"

#include "FontAwesome.h"
#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;

SurveyEditWidget::SurveyEditWidget(QWidget * par) : QWidget(par), _projectID(0), _surveyID(0), _waitSpinner(nullptr) {
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
		
		QPushButton * addQuestionButton = new QPushButton(QApplication::tr("AddQuestion"));
		QPushButton * backButton = new QPushButton(QApplication::tr("Back"));
		QPushButton * saveButton = new QPushButton(QApplication::tr("Save"));

		hl->addWidget(addQuestionButton);
		hl->addStretch(1);
		hl->addWidget(backButton);
		hl->addWidget(saveButton);

		vlOuter->addLayout(hl);

		connect(addQuestionButton, &QPushButton::released, this, &SurveyEditWidget::addQuestionClicked);
		connect(backButton, &QPushButton::released, this, &SurveyEditWidget::backClicked);
		connect(saveButton, &QPushButton::released, this, &SurveyEditWidget::saveClicked);
	}

	setLayout(vlOuter);

	connect(this, &SurveyEditWidget::removeSpinner, [this]() {
		if (!_waitSpinner) return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});
}

void SurveyEditWidget::updateView(int projectID, int surveyID) {
	qDeleteAll(_widgetList);
	_widgetList.clear();

	_editList.clear();

	_projectID = projectID;
	_surveyID = surveyID;

	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = _projectID;
	requestData["SurveyID"] = _surveyID;
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "getPlayTestSurvey", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		ManagementSurveyQuestions msq;
		msq.read(json);
		
		emit loadedData(msq);
		emit removeSpinner();
	});
}

void SurveyEditWidget::updateData(ManagementSurveyQuestions content) {
	for (const auto & msa : content.questions) {
		QWidget * w = new QWidget(this);
		QHBoxLayout * hl = new QHBoxLayout();

		QLineEdit * le = new QLineEdit(msa.question, w);
		hl->addWidget(le);
		
		QPushButton * deleteButton = new QPushButton(QChar(static_cast<int>(FontAwesome::trasho)), w);
		deleteButton->setProperty("fontAwesome", true);
		hl->addWidget(deleteButton);

		w->setLayout(hl);

		_widgetList.append(w);
		_editList.append(le);

		connect(deleteButton, &QPushButton::released, [this, w, le]() {
			_widgetList.removeAll(w);
			_editList.removeAll(le);

			w->deleteLater();
		});
	}
}

void SurveyEditWidget::saveClicked() {
	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = _projectID;
	requestData["SurveyID"] = _surveyID;

	QJsonArray questionArray;
	for (const auto & question : _editList) {
		auto text = question->text();

		text = text.trimmed();

		text = text.replace("\"", "&quot;");
		text = text.replace("\'", "&apos;");
		text = text.replace("\n", "<br>");

		if (text.isEmpty()) continue;
		
		questionArray.append(text);
	}
	requestData["Questions"] = questionArray;
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "updatePlayTestSurvey", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});
}

void SurveyEditWidget::addQuestionClicked() {
	QWidget * w = new QWidget(this);
	QHBoxLayout * hl = new QHBoxLayout();

	QLineEdit * le = new QLineEdit(w);
	hl->addWidget(le);
	
	QPushButton * deleteButton = new QPushButton(QChar(static_cast<int>(FontAwesome::trasho)), w);
	deleteButton->setProperty("fontAwesome", true);
	hl->addWidget(deleteButton);

	w->setLayout(hl);

	_widgetList.append(w);
	_editList.append(le);

	connect(deleteButton, &QPushButton::released, [this, w, le]() {
		_widgetList.removeAll(w);
		_editList.removeAll(le);

		w->deleteLater();
	});
}
