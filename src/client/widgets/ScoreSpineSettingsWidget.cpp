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

#include "widgets/ScoreSpineSettingsWidget.h"

#include "Config.h"
#include "UpdateLanguage.h"

#include "models/SpineEditorModel.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	ScoreSpineSettingsWidget::ScoreSpineSettingsWidget(GeneralSettingsWidget * generalSettingsWidget, models::SpineEditorModel * model, QWidget * par) : QWidget(par), _model(model), _generalSettingsWidget(generalSettingsWidget), _scores() {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		setLayout(l);

		addNewScore();
	}

	ScoreSpineSettingsWidget::~ScoreSpineSettingsWidget() {
	}

	void ScoreSpineSettingsWidget::save() {
		QList<models::ScoreModel> scores;
		for (Score s : _scores) {
			models::ScoreModel sm;
			sm.name = s.lineEdit->text();
			scores.append(sm);
		}
		if (!scores.empty() && scores.back().name.isEmpty()) {
			scores.pop_back(); // last one is always empty
		}
		_model->setScores(scores);
	}

	void ScoreSpineSettingsWidget::addNewScore() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		Score s;
		s.layout = new QHBoxLayout();
		s.lineEdit = new QLineEdit(this);
		s.lineEdit->setPlaceholderText(QApplication::tr("Name"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_generalSettingsWidget, s.lineEdit, "Name");
		s.addButton = new QPushButton("+", this);
		s.removeButton = new QPushButton("-", this);
		s.layout->addWidget(s.lineEdit);
		s.layout->addWidget(s.removeButton);
		s.layout->addWidget(s.addButton);

		connect(s.addButton, SIGNAL(released()), this, SLOT(addNewScore()));
		connect(s.removeButton, SIGNAL(released()), this, SLOT(removeScore()));

		if (pb) {
			int index = 0;
			for (Score oldS : _scores) {
				if (oldS.addButton == pb) {
					dynamic_cast<QVBoxLayout *>(layout())->insertLayout(index + 1, s.layout);
					_scores.insert(index + 1, s);
					break;
				} else {
					index++;
				}
			}
		} else {
			dynamic_cast<QVBoxLayout *>(layout())->addLayout(s.layout);
			_scores.append(s);
		}

		if (_scores.size() == 1) {
			s.removeButton->setDisabled(true);
		} else {
			_scores[0].removeButton->setEnabled(true);
		}
	}

	void ScoreSpineSettingsWidget::removeScore() {
		QPushButton * pb = qobject_cast<QPushButton *>(sender());
		if (pb) {
			int index = 0;
			for (Score s : _scores) {
				if (s.removeButton == pb) {
					dynamic_cast<QVBoxLayout *>(layout())->removeItem(s.layout);
					_scores.removeAt(index);
					s.addButton->deleteLater();
					s.layout->deleteLater();
					s.removeButton->deleteLater();
					s.lineEdit->deleteLater();
					break;
				} else {
					index++;
				}
			}
		}

		if (_scores.size() == 1) {
			_scores[0].removeButton->setDisabled(true);
		}
	}

	void ScoreSpineSettingsWidget::updateFromModel() {
		clear();
		for (const models::ScoreModel sm : _model->getScores()) {
			addNewScore();
			_scores.back().lineEdit->setText(sm.name);
		}
		if (_scores.empty()) {
			addNewScore();
		}
	}

	void ScoreSpineSettingsWidget::clear() {
		for (Score s : _scores) {
			dynamic_cast<QVBoxLayout *>(layout())->removeItem(s.layout);
			delete s.addButton;
			delete s.layout;
			delete s.removeButton;
			delete s.lineEdit;
		}
		_scores.clear();
	}

} /* namespace widgets */
} /* namespace spine */
