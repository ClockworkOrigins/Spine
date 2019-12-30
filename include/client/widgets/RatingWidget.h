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

#include <array>
#include <cstdint>

#include <QWidget>

class QSvgWidget;

namespace spine {
namespace widgets {

	class RatingWidget : public QWidget {
		Q_OBJECT

	public:
		enum class RatingMode {
			Overall,
			User
		};
		
		RatingWidget(RatingMode mode, QWidget * par);

		void setEditable(bool editable);

	signals:
		void receivedRating(int32_t, int32_t, int32_t, bool);

	public slots:
		void loginChanged();
		void setModID(int32_t modID);
		void setVisible(bool visible) override;
		void setModName(QString name);

	private slots:
		void updateRating(int32_t modID, int32_t sum, int32_t count, bool allowedToRate);

	private:
		std::array<QSvgWidget *, 5> _svgs;
		int32_t _modID;
		QString _modname;
		bool _allowedToRate;
		bool _editable;
		bool _visible;
		RatingMode _mode;

		void mousePressEvent(QMouseEvent * evt) override;
		void requestRating();
	};

} /* namespace widgets */
} /* namespace spine */
