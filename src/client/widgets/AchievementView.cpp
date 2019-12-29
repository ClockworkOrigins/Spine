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

#include "widgets/AchievementView.h"

#include "utils/Config.h"

#include "widgets/AchievementInfoView.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

AchievementView::AchievementView(int32_t modID, common::SendAllAchievementStatsMessage::AchievementStats as, QWidget * par) : QWidget(par) {
	QHBoxLayout * l = new QHBoxLayout();

	setObjectName("AchievementView");

	QString iconUnlocked = ":/Achievement_Unlocked.png";
	QString iconLocked = ":/Achievement_Locked.png";
	if (!as.iconUnlocked.empty()) {
		QString filename = QString::fromStdString(as.iconUnlocked);
		filename.chop(2);
		iconUnlocked = Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/" + filename;
	}
	if (!as.iconLocked.empty()) {
		QString filename = QString::fromStdString(as.iconLocked);
		filename.chop(2);
		iconLocked = Config::MODDIR + "/mods/" + QString::number(modID) + "/achievements/" + filename;
	}

	{
		QLabel * lbl = new QLabel(this);
		QPixmap achievementPixmap(iconUnlocked);
		if (achievementPixmap.isNull()) {
			achievementPixmap = QPixmap(":/Achievement_Unlocked.png");
		}
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		lbl->setFixedSize(64, 64);
		lbl->setPixmap(pixmap);

		lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		lbl->setProperty("score", true);

		l->addWidget(lbl, Qt::AlignLeft);
	}
	{
		AchievementInfoView * aiv = new AchievementInfoView(as, this);
		l->addWidget(aiv);
	}
	{
		QLabel * lbl = new QLabel(this);
		QPixmap achievementPixmap(as.unlocked ? iconUnlocked : iconLocked);
		if (!as.iconUnlocked.empty() && as.iconLocked.empty() && !as.unlocked) {
			const QPixmap achievementPixmapUnlocked(iconUnlocked);
			if (!achievementPixmapUnlocked.isNull()) {
				QImage img = achievementPixmapUnlocked.toImage();
				unsigned int * d = reinterpret_cast<unsigned int*>(img.bits());
				const int pixelCount = img.width() * img.height();

				// Convert each pixel to grayscale
				for (int i = 0; i < pixelCount; ++i) {
					const int val = qGray(*d);
					*d = qRgba(val, val, val, qAlpha(*d));
					++d;
				}
				achievementPixmap = QPixmap::fromImage(img);
			}
		}
		if (achievementPixmap.isNull()) {
			achievementPixmap = QPixmap(as.unlocked ? ":/Achievement_Unlocked.png" : ":/Achievement_Locked.png");
		}
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

AchievementView::~AchievementView() {
}

QSize AchievementView::sizeHint() const {
	return QSize(800, 80);
}

void AchievementView::paintEvent(QPaintEvent *) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
