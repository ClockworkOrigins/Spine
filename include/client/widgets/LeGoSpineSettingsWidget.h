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
// Copyright 2018 Clockwork Origins

#pragma once

#include "models/SpineEditorModel.h"

#include <QMap>
#include <QWidget>

class QCheckBox;

namespace spine {
namespace widgets {

	class LeGoSpineSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		LeGoSpineSettingsWidget(models::SpineEditorModel * model, QWidget * par);

		void save();

	public slots:
		void achievementStateChanged(int state);
		void gamepadStateChanged(int state);

	public slots:
		void updateFromModel();

	private:
		models::SpineEditorModel * _model;
		QMap<models::LeGoModules, QCheckBox *> _legoModuleCheckBoxes;
		bool _achievementsEnabled;
		bool _gamepadEnabled;
	};

} /* namespace widgets */
} /* namespace spine */
