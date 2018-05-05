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

#ifndef __SPINE_WIDGETS_ACHIEVEMENTVIEW_H__
#define __SPINE_WIDGETS_ACHIEVEMENTVIEW_H__

#include "common/MessageStructs.h"

#include <QWidget>

namespace spine {
namespace widgets {

	class AchievementView : public QWidget {
		Q_OBJECT

	public:
		AchievementView(int32_t modID, common::SendAllAchievementStatsMessage::AchievementStats as, QWidget * par);
		~AchievementView();

	private:
		QSize sizeHint() const override;
		void paintEvent(QPaintEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_ACHIEVEMENTVIEW_H__ */
