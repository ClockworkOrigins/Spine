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

#ifndef __SPINE_WIDGETS_GENERALSPINESETTINGSWIDGET_H__
#define __SPINE_WIDGETS_GENERALSPINESETTINGSWIDGET_H__

#include "common/SpineModules.h"

#include <QMap>
#include <QWidget>

class QCheckBox;
class QLineEdit;

namespace spine {
namespace models {
	class SpineEditorModel;
} /* namespace models */
namespace widgets {

	class GeneralSpineSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		GeneralSpineSettingsWidget(models::SpineEditorModel * model, QWidget * par);
		~GeneralSpineSettingsWidget();

		void save();

	signals:
		void changedAchievementState(int);
		void changedScoreState(int);
		void changedGamepadState(int);

	private slots:
		void updateFromModel();

	private:
		models::SpineEditorModel * _model;
		QLineEdit * _modNameEdit;
		QMap<common::SpineModules, QCheckBox *> _spineModuleCheckBoxes;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GENERALSPINESETTINGSWIDGET_H__ */
