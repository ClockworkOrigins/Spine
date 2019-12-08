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

#pragma once

#include "ManagementCommon.h"

#include <QWidget>

class QGridLayout;
class QLineEdit;

namespace spine {
namespace client {
namespace widgets {

	class ScoresWidget : public QWidget {
		Q_OBJECT

	public:
		ScoresWidget(QWidget * par);
		~ScoresWidget();

		void updateModList(QList<client::ManagementMod> modList);
		void selectedMod(int index);

	private slots:
		void updateScores();
		void addScore();

	private:
		QList<client::ManagementMod> _mods;
		int _modIndex;
		QGridLayout * _layout;
		int _rowCount;
		QList<std::tuple<QLineEdit *, QLineEdit *, QLineEdit *, QLineEdit *>> _scoreEdits;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
