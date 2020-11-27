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
// Copyright 2020 Clockwork Origins

#pragma once

#include <array>
#include <cstdint>

#include <QWidget>

class QPushButton;
class QSvgWidget;

namespace spine {
namespace widgets {

	class ReviewWidget : public QWidget {
		Q_OBJECT

	public:
		ReviewWidget(const QString & reviewer, const QString & review, uint64_t playTime, uint64_t playTimeAtReview, uint64_t date, int rating, int32_t projectID, QWidget * par);

	private:
        std::array<QSvgWidget *, 5> _svgs;
	};

} /* namespace widgets */
} /* namespace spine */
