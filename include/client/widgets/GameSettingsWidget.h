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

#ifndef __SPINE_WIDGETS_GAMESETTINGSWIDGET_H__
#define __SPINE_WIDGETS_GAMESETTINGSWIDGET_H__

#include <QWidget>

class QCheckBox;
class QSettings;

namespace spine {
namespace widgets {

	class GameSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		static int downloadRate;

		GameSettingsWidget(QSettings * iniParser, QWidget * par);
		~GameSettingsWidget();

		void saveSettings();
		void rejectSettings();

		bool getShowAchievements() const;

	signals:
		void showAchievementsChanged(bool);

	private:
		QSettings * _iniParser;
		QCheckBox * _showAchievementsCheckBox;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GAMESETTINGSWIDGET_H__ */
