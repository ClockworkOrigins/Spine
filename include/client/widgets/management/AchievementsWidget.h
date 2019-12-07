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

#ifndef __SPINE_WIDGETS_MANAGEMENT_ACHIEVEMENTSWIDGET_H__
#define __SPINE_WIDGETS_MANAGEMENT_ACHIEVEMENTSWIDGET_H__

#include "ManagementCommon.h"

#include <QWidget>

class QVBoxLayout;

namespace spine {
namespace widgets {

	class AchievementWidget;

	class AchievementsWidget : public QWidget {
		Q_OBJECT

	public:
		AchievementsWidget(QWidget * par);

		void updateModList(QList<client::ManagementMod> modList);
		void selectedMod(int index);

	private slots:
		void updateAchievements();
		void addAchievement();

	private:
		QList<client::ManagementMod> _mods;
		int _modIndex;
		QVBoxLayout * _layout;
		QList<AchievementWidget *> _achievementEdits;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MANAGEMENT_ACHIEVEMENTSWIDGET_H__ */
