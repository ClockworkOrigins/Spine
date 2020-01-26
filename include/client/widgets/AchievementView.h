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

#include "common/MessageStructs.h"

#include <QWidget>

class QLabel;

namespace spine {
namespace widgets {

	class AchievementView : public QWidget {
		Q_OBJECT

	public:
		AchievementView(int32_t modID, common::SendAllAchievementStatsMessage::AchievementStats as, QWidget * par);
		~AchievementView();

		void updateIcons();

	private:
		int32_t _modID;
		common::SendAllAchievementStatsMessage::AchievementStats _achievement;
		QLabel * _lockedIcon;
		QLabel * _unlockedIcon;
		
		QSize sizeHint() const override;
		void paintEvent(QPaintEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
