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

#include "Conversion.h"
#include "SpineConfig.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	ScoresWidget::ScoresWidget(QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _rowCount(1), _scoreEdits() {
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

		vl->addStretch(1);

		vl->addWidget(dbb);

		setLayout(vl);
	}

	ScoresWidget::~ScoresWidget() {
	}

	void ScoresWidget::updateModList(std::vector<common::SendModManagementMessage::ModManagement> modList) {
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
		
		for (auto score : _mods[_modIndex].scores) {
			QLineEdit * germanEdit = new QLineEdit(this);
			QLineEdit * englishEdit = new QLineEdit(this);
			QLineEdit * polishEdit = new QLineEdit(this);
			QLineEdit * russianEdit = new QLineEdit(this);

			for (auto tt : score.names) {
				if (tt.language == "Deutsch") {
					germanEdit->setText(s2q(tt.text));
				} else if (tt.language == "English") {
					englishEdit->setText(s2q(tt.text));
				} else if (tt.language == "Polish") {
					polishEdit->setText(s2q(tt.text));
				} else if (tt.language == "Russian") {
					russianEdit->setText(s2q(tt.text));
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
		if (_modIndex == -1) {
			return;
		}
		common::UpdateScoresMessage usm;
		usm.modID = _mods[_modIndex].modID;

		for (int i = 0; i < _scoreEdits.size(); i++) {
			bool toAdd = false;

			common::UpdateScoresMessage::Score score;

			QLineEdit * germanEdit = std::get<0>(_scoreEdits[i]);
			QLineEdit * englishEdit = std::get<1>(_scoreEdits[i]);
			QLineEdit * polishEdit = std::get<2>(_scoreEdits[i]);
			QLineEdit * russianEdit = std::get<3>(_scoreEdits[i]);

			if (!germanEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Deutsch";
				tt.text = q2s(germanEdit->text());
				score.names.push_back(tt);
				toAdd = true;
			}
			if (!englishEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "English";
				tt.text = q2s(englishEdit->text());
				score.names.push_back(tt);
				toAdd = true;
			}
			if (!polishEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Polish";
				tt.text = q2s(polishEdit->text());
				score.names.push_back(tt);
				toAdd = true;
			}
			if (!russianEdit->text().isEmpty()) {
				common::TranslatedText tt;
				tt.language = "Russian";
				tt.text = q2s(russianEdit->text());
				score.names.push_back(tt);
				toAdd = true;
			}
			if (toAdd) {
				usm.scores.push_back(score);
			}
		}

		std::string serialized = usm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			sock.writePacket(serialized);
		}
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
} /* namespace spine */
