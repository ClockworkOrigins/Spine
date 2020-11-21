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

#include "common/ProjectStats.h"

#include <QWidget>

class QLabel;

namespace spine {
namespace widgets {

	class ProfileModView : public QWidget {
		Q_OBJECT

	public:
		ProfileModView(common::ProjectStats ms, QString gothicDirectory, QString gothic2Directory, QWidget * par);

		bool isPatchOrTool() const;
		QSize sizeHint() const override;
		uint64_t getDuration() const;

	signals:
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);

	private slots:
		void prepareAchievementView();
		void prepareScoreView();

	private:
		int32_t _modID;
		QLabel * _nameLabel;
		bool _patchOrTool;
		uint64_t _duration;

		void paintEvent(QPaintEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
