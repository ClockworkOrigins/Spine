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

#include "widgets/SurveyDialog.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/WindowsExtensions.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

SurveyDialog::SurveyDialog(const Survey & survey, int majorVersion, int minorVersion, int patchVersion, QWidget * par) : QDialog(par), _survey(survey), _majorVersion(majorVersion), _minorVersion(minorVersion), _patchVersion(patchVersion) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);
		
	QScrollArea * sa = new QScrollArea(this);
	QWidget * cw = new QWidget(sa);
	QVBoxLayout * vl = new QVBoxLayout();
	vl->setAlignment(Qt::AlignTop);
	cw->setLayout(vl);
	sa->setWidget(cw);
	sa->setWidgetResizable(true);
	sa->setProperty("default", true);
	cw->setProperty("default", true);

	for (const auto & q : _survey.questions) {
		QLabel * lbl = new QLabel(q.question, cw);
		lbl->setProperty("big", true);
		lbl->setProperty("bold", true);
		vl->addWidget(lbl);

		QTextEdit * te = new QTextEdit(q.answer, cw);
		vl->addWidget(te);
	}

	l->addWidget(sa, 1);

	QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);

	auto * submitButton = dbb->button(QDialogButtonBox::StandardButton::Ok);
	submitButton->setText(QApplication::tr("Submit"));

	auto * cancelButton = dbb->button(QDialogButtonBox::StandardButton::Cancel);
	cancelButton->setText(QApplication::tr("Cancel"));

	l->addWidget(dbb);

	setLayout(l);

	connect(dbb, &QDialogButtonBox::clicked, [this, submitButton, cancelButton](QAbstractButton * btn) {
		if (btn == submitButton) {
			QJsonObject requestData;
			requestData["Username"] = Config::Username;
			requestData["Password"] = Config::Password;
			requestData["SurveyID"] = _survey.surveyID;
			requestData["MajorVersion"] = _majorVersion;
			requestData["MinorVersion"] = _minorVersion;
			requestData["PatchVersion"] = _patchVersion;

			QJsonArray answerArray;
			for (int i = 0; i < _survey.questions.count(); i++) {
				auto text = _editList[i]->toPlainText();

				text = text.trimmed();

				text = text.replace("\"", "&quot;");
				text = text.replace("\'", "&apos;");
				text = text.replace("\n", "<br>");

				if (text.isEmpty()) continue;

				QJsonObject json;
				json["QuestionID"] = _survey.questions[i].questionID;
				json["Answer"] = text;
				
				answerArray.append(json);
			}
			requestData["Answers"] = answerArray;
			
			https::Https::postAsync(MANAGEMENTSERVER_PORT, "submitPlayTestSurveyAnswers", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});
			
			accept();
		} else {
			reject();
		}
	});
}
