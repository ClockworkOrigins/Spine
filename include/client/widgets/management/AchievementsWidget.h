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

#include "widgets/management/IManagementWidget.h"

#include <QWidget>

class QVBoxLayout;

namespace spine {
namespace widgets {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class AchievementWidget;

	class AchievementsWidget : public QWidget, public IManagementWidget {
		Q_OBJECT

	public:
		AchievementsWidget(const QString & username, const QString & password, QWidget * par);

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
		QString _username;
		QString _password;
		spine::widgets::WaitSpinner * _waitSpinner;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
