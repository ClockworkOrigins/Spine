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

#include "widgets/management/SurveyEntryWidget.h"

#include "FontAwesome.h"
#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Compression.h"
#include "utils/Config.h"
#include "utils/FileDownloader.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::utils;

SurveyEntryWidget::SurveyEntryWidget(int projectID, ManagementSurvey survey, QWidget * par) : QWidget(par), _projectID(projectID), _survey(survey) {
	QHBoxLayout * hl = new QHBoxLayout();
	
	QPushButton * deleteButton = new QPushButton(QChar(static_cast<int>(FontAwesome::trasho)), this);
	deleteButton->setProperty("fontAwesome", true);
	connect(deleteButton, &QPushButton::released, this, &SurveyEntryWidget::wantToDelete);

	QLabel * versionLbl = new QLabel(QApplication::tr("Version") + ":", this);
	QLabel * versionValueLbl = new QLabel(QString("%1.%2.%3").arg(survey.majorVersion).arg(survey.minorVersion).arg(survey.patchVersion), this);

	QLabel * languageLbl = new QLabel(QApplication::tr("Language") + ":", this);
	QLabel * languageValueLbl = new QLabel(survey.language, this);

	QLabel * questionCountLbl = new QLabel(QApplication::tr("Questions") + ":", this);
	QLabel * questionCountValueLbl = new QLabel(QString::number(survey.questionCount), this);

	QLabel * answerCountLbl = new QLabel(QApplication::tr("Answers") + ":", this);
	QLabel * answerCountValueLbl = new QLabel(QString::number(survey.answerCount), this);

	hl->addWidget(deleteButton);
	
	hl->addWidget(versionLbl);
	hl->addWidget(versionValueLbl);
	
	hl->addWidget(languageLbl);
	hl->addWidget(languageValueLbl);
	
	hl->addWidget(questionCountLbl);
	hl->addWidget(questionCountValueLbl);
	
	hl->addWidget(answerCountLbl);
	hl->addWidget(answerCountValueLbl);

	if (!survey.enabled) {
		QPushButton * pb = new QPushButton(QApplication::tr("Edit"), this);

		connect(pb, &QPushButton::released, [this]() {
			emit editClicked(_survey.surveyID);
		});
		
		hl->addWidget(pb);
	}

	if (!survey.enabled) {
		QPushButton * pb = new QPushButton(QApplication::tr("Release"), this);
		pb->setEnabled(survey.questionCount > 0);

		connect(pb, &QPushButton::released, this, &SurveyEntryWidget::release);
		
		hl->addWidget(pb);
	}

	if (survey.enabled) {
		QPushButton * pb = new QPushButton(QApplication::tr("ShowAnswers"), this);

		connect(pb, &QPushButton::released, [this]() {
			emit showAnswersClicked(_survey.surveyID);
		});
		
		hl->addWidget(pb);
	}

	setLayout(hl);
}

void SurveyEntryWidget::wantToDelete() {
	QMessageBox msgBox(QMessageBox::Icon::NoIcon, QApplication::tr("DeleteSurvey"), QApplication::tr("DeleteSurveyDescription"), QMessageBox::Ok | QMessageBox::Cancel, this);
	const auto btn = msgBox.exec();

	if (btn != QMessageBox::Ok) return;

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = _projectID;
	requestData["SurveyID"] = _survey.surveyID;
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "deletePlayTestSurvey", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int statusCode) {
		if (statusCode != 200) return;
		
		emit deleteClicked();
	});
}

void SurveyEntryWidget::release() {
	QMessageBox msgBox(QMessageBox::Icon::NoIcon, QApplication::tr("ReleaseSurvey"), QApplication::tr("ReleaseSurveyDescription"), QMessageBox::Ok | QMessageBox::Cancel, this);
	const auto btn = msgBox.exec();

	if (btn != QMessageBox::Ok) return;

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = _projectID;
	requestData["SurveyID"] = _survey.surveyID;
	requestData["Enabled"] = 1;
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "enablePlayTestSurvey", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int statusCode) {
		if (statusCode != 200) return;
		
		emit releaseClicked();
	});
}
