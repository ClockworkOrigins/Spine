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

#include "ManagementCommon.h"

#include "widgets/management/IManagementWidget.h"

#include <QFutureWatcher>
#include <QWidget>

class QVBoxLayout;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class AchievementWidget;

	class AchievementsWidget : public QWidget, public IManagementWidget {
		Q_OBJECT

	public:
		AchievementsWidget(QWidget * par);
		~AchievementsWidget();

		void updateModList(QList<ManagementMod> modList);
		void selectedMod(int index);
		void updateView() override;

	signals:
		void removeSpinner();
		void loadedAchievements(QList<ManagementAchievement>);

	private slots:
		void updateAchievements();
		void addAchievement();
		void updateAchievementViews(QList<ManagementAchievement> achievementList);

	private:
		QList<client::ManagementMod> _mods;
		int _modIndex;
		QVBoxLayout * _layout;
		QList<AchievementWidget *> _achievementEdits;
		gui::WaitSpinner * _waitSpinner;

		QFutureWatcher<void> _futureWatcher;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
