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

#ifndef __SPINE_WIDGETS_GAMEPADSPINESETTINGSWIDGET_H__
#define __SPINE_WIDGETS_GAMEPADSPINESETTINGSWIDGET_H__

#include <QWidget>

class QCheckBox;
class QLabel;
class QVBoxLayout;

namespace spine {
namespace models {
	class SpineEditorModel;
} /* namespace models */
namespace widgets {

	class GamepadSpineSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		GamepadSpineSettingsWidget(models::SpineEditorModel * model, QWidget * par);
		~GamepadSpineSettingsWidget();

		void save();

	private slots:
		void updateFromModel();

	private:
		models::SpineEditorModel * _model;
		QLabel * _earthquakeVibrationLabel;
		QCheckBox * _earthquakeVibrationCheckBox;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GAMEPADSPINESETTINGSWIDGET_H__ */
