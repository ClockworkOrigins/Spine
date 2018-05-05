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

#include "widgets/HiddenAchievementsView.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>

namespace spine {
namespace widgets {

	HiddenAchievementsView::HiddenAchievementsView(int32_t count, QWidget * par) : QWidget(par) {
		QHBoxLayout * l = new QHBoxLayout();

		setObjectName("AchievementView");

		const QString iconLocked = ":/Achievement_Locked.png";

		{
			QLabel * lbl = new QLabel("+" + QString::number(count), this);
			lbl->setFixedSize(64, 64);
			lbl->setAlignment(Qt::AlignCenter);

			lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			lbl->setProperty("hiddenAchievementCount", true);

			l->addWidget(lbl, Qt::AlignLeft);
		}
		{
			QLabel * lbl = new QLabel(QApplication::tr("HiddenAchievementsDescription"), this);
			lbl->setFixedSize(QSize(800 - 2 * 70, 70));
			lbl->setAlignment(Qt::AlignCenter);
			l->addWidget(lbl);
		}
		{
			QLabel * lbl = new QLabel(this);
			QPixmap achievementPixmap(iconLocked);
			const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
			lbl->setFixedSize(64, 64);
			lbl->setPixmap(pixmap);

			lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			lbl->setProperty("score", true);

			l->addWidget(lbl, Qt::AlignRight);
		}

		setLayout(l);

		setFixedSize(800, 70);
		setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		l->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

		setProperty("score", true);
	}

	HiddenAchievementsView::~HiddenAchievementsView() {
	}

	QSize HiddenAchievementsView::sizeHint() const {
		return QSize(800, 80);
	}

	void HiddenAchievementsView::paintEvent(QPaintEvent *) {
		QStyleOption opt;
		opt.init(this);
		QPainter p(this);
		style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	}

} /* namespace widgets */
} /* namespace spine */
