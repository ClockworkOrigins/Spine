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

#ifndef __SPINE_WIDGETS_ACHIEVEMENTINFOVIEW_H__
#define __SPINE_WIDGETS_ACHIEVEMENTINFOVIEW_H__

#include "common/MessageStructs.h"

#include <QWidget>

namespace spine {
namespace widgets {

	class AchievementInfoView : public QWidget {
		Q_OBJECT

	public:
		AchievementInfoView(common::SendAllAchievementStatsMessage::AchievementStats as, QWidget * par);
		~AchievementInfoView();

	private:
		QString _name;
		QString _description;
		double _progress;
		int _currentProgress;
		int _maxProgress;
		bool _unlocked;

		QSize sizeHint() const override;
		void paintEvent(QPaintEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_ACHIEVEMENTINFOVIEW_H__ */
