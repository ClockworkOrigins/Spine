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

#include "widgets/management/ScoresWidget.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "widgets/WaitSpinner.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace spine {
namespace client {
namespace widgets {

	ScoresWidget::ScoresWidget(const QString & username, const QString & password, QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _rowCount(1), _scoreEdits(), _username(username), _password(password), _waitSpinner(nullptr) {
		QVBoxLayout * vl = new QVBoxLayout();

		{
			_layout = new QGridLayout();
			_layout->setAlignment(Qt::AlignTop);

			_layout->addWidget(new QLabel(QApplication::tr("German"), this), 0, 0);
			_layout->addWidget(new QLabel(QApplication::tr("English"), this), 0, 1);
			_layout->addWidget(new QLabel(QApplication::tr("Polish"), this), 0, 2);
			_layout->addWidget(new QLabel(QApplication::tr("Russian"), this), 0, 3);

			vl->addLayout(_layout);
		}

		{
			QHBoxLayout * hl = new QHBoxLayout();
			hl->addStretch(1);

			QPushButton * pb = new QPushButton("+", this);
			hl->addWidget(pb);
			connect(pb, &QPushButton::released, this, &ScoresWidget::addScore);

			vl->addLayout(hl);
		}

		QDialogButtonBox * dbb = new QDialogButtonBox(this);
		QPushButton * submitButton = new QPushButton(QApplication::tr("Submit"), this);
		dbb->addButton(submitButton, QDialogButtonBox::ButtonRole::AcceptRole);
		connect(submitButton, &QPushButton::released, this, &ScoresWidget::updateScores);

		qRegisterMetaType<QList<ManagementScore>>("QList<ManagementScore>");

		connect(this, &ScoresWidget::removeSpinner, [this]() {
			if (!_waitSpinner) return;
			
			_waitSpinner->deleteLater();
			_waitSpinner = nullptr;
		});

		connect(this, &ScoresWidget::loadedData, this, &ScoresWidget::updateData);

		vl->addStretch(1);

		vl->addWidget(dbb);

		setLayout(vl);
	}

	ScoresWidget::~ScoresWidget() {
	}

	void ScoresWidget::updateModList(QList<client::ManagementMod> modList) {
		_mods = modList;
	}

	void ScoresWidget::selectedMod(int index) {
		_modIndex = index;

		for (auto t : _scoreEdits) {
			QLineEdit * germanEdit = std::get<0>(t);
			QLineEdit * englishEdit = std::get<1>(t);
			QLineEdit * polishEdit = std::get<2>(t);
			QLineEdit * russianEdit = std::get<3>(t);

			_layout->removeWidget(germanEdit);
			_layout->removeWidget(englishEdit);
			_layout->removeWidget(polishEdit);
			_layout->removeWidget(russianEdit);

			germanEdit->deleteLater();
			englishEdit->deleteLater();
			polishEdit->deleteLater();
			russianEdit->deleteLater();
		}
		_scoreEdits.clear();
	}

	void ScoresWidget::updateView() {
		if (_modIndex == -1 || _modIndex >= _mods.size()) return;
		
		delete _waitSpinner;
		_waitSpinner = new spine::widgets::WaitSpinner(QApplication::tr("Updating"), this);

		QJsonObject requestData;
		requestData["Username"] = _username;
		requestData["Password"] = _password;
		requestData["ModID"] = _mods[_modIndex].id;
		
		https::Https::postAsync(MANAGEMENTSERVER_PORT, "getScores", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) {
				emit removeSpinner();
				return;
			}
			QList<ManagementScore> scoreList;

			const auto it = json.find("Scores");
			if (it != json.end()) {
				const auto scoreArr = it->toArray();
				for (auto scoreV : scoreArr) {
					const auto s = scoreV.toObject();
					
					if (s.isEmpty()) continue;

					ManagementScore ms;
					ms.read(s);

					scoreList.append(ms);
				}
			}
			
			emit loadedData(scoreList);
			emit removeSpinner();
		});
	}

	void ScoresWidget::updateData(QList<ManagementScore> scores) {		
		for (const auto & score : scores) {
			QLineEdit * germanEdit = new QLineEdit(this);
			QLineEdit * englishEdit = new QLineEdit(this);
			QLineEdit * polishEdit = new QLineEdit(this);
			QLineEdit * russianEdit = new QLineEdit(this);

			for (const auto & tt : score.names) {
				if (tt.language == "Deutsch") {
					germanEdit->setText(tt.text);
				} else if (tt.language == "English") {
					englishEdit->setText(tt.text);
				} else if (tt.language == "Polish") {
					polishEdit->setText(tt.text);
				} else if (tt.language == "Russian") {
					russianEdit->setText(tt.text);
				}
			}

			_layout->addWidget(germanEdit, _rowCount, 0);
			_layout->addWidget(englishEdit, _rowCount, 1);
			_layout->addWidget(polishEdit, _rowCount, 2);
			_layout->addWidget(russianEdit, _rowCount, 3);

			_scoreEdits.push_back(std::make_tuple(germanEdit, englishEdit, polishEdit, russianEdit));

			_rowCount++;
		}
	}

	void ScoresWidget::updateScores() {
		if (_modIndex == -1) return;

		QJsonObject json;
		json["ModID"] = _mods[_modIndex].id;
		json["Username"] = _username;
		json["Password"] = _password;

		QJsonArray scoreArray;

		for (auto & scoreEdit : _scoreEdits) {
			bool toAdd = false;

			QLineEdit * germanEdit = std::get<0>(scoreEdit);
			QLineEdit * englishEdit = std::get<1>(scoreEdit);
			QLineEdit * polishEdit = std::get<2>(scoreEdit);
			QLineEdit * russianEdit = std::get<3>(scoreEdit);

			ManagementScore ms;

			if (!germanEdit->text().isEmpty()) {
				ManagementTranslation mt;
				mt.language = "Deutsch";
				mt.text = germanEdit->text();
				ms.names.append(mt);
				toAdd = true;
			}
			if (!englishEdit->text().isEmpty()) {
				ManagementTranslation mt;
				mt.language = "English";
				mt.text = englishEdit->text();
				ms.names.push_back(mt);
				toAdd = true;
			}
			if (!polishEdit->text().isEmpty()) {
				ManagementTranslation mt;
				mt.language = "Polish";
				mt.text = polishEdit->text();
				ms.names.push_back(mt);
				toAdd = true;
			}
			if (!russianEdit->text().isEmpty()) {
				ManagementTranslation mt;
				mt.language = "Russian";
				mt.text = russianEdit->text();
				ms.names.push_back(mt);
				toAdd = true;
			}
			if (toAdd) {
				QJsonObject j;
				ms.write(j);
				scoreArray.append(j);
			}
		}

		if (scoreArray.isEmpty()) return; // no need to synchronize something if there are no achievements
		
		json["Scores"] = scoreArray;

		const QJsonDocument doc(json);
		const QString content = doc.toJson(QJsonDocument::Compact);

		https::Https::postAsync(MANAGEMENTSERVER_PORT, "updateScores", content, [](const QJsonObject &, int) {
			// we could do some error handling here
		});
	}

	void ScoresWidget::addScore() {
		QLineEdit * germanEdit = new QLineEdit(this);
		QLineEdit * englishEdit = new QLineEdit(this);
		QLineEdit * polishEdit = new QLineEdit(this);
		QLineEdit * russianEdit = new QLineEdit(this);

		_layout->addWidget(germanEdit, _rowCount, 0);
		_layout->addWidget(englishEdit, _rowCount, 1);
		_layout->addWidget(polishEdit, _rowCount, 2);
		_layout->addWidget(russianEdit, _rowCount, 3);

		_scoreEdits.push_back(std::make_tuple(germanEdit, englishEdit, polishEdit, russianEdit));

		_rowCount++;
	}

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
