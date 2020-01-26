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

AchievementView::AchievementView(int32_t modID, common::SendAllAchievementStatsMessage::AchievementStats as, QWidget * par) : QWidget(par), _modID(modID), _achievement(as) {
	QHBoxLayout * l = new QHBoxLayout();

	setObjectName("AchievementView");
	
	{
		_unlockedIcon = new QLabel(this);
		const QPixmap achievementPixmap(":/Achievement_Unlocked.png");
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		_unlockedIcon->setFixedSize(64, 64);
		_unlockedIcon->setPixmap(pixmap);

		_unlockedIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		_unlockedIcon->setProperty("score", true);

		l->addWidget(_unlockedIcon, Qt::AlignLeft);
	}
	{
		AchievementInfoView * aiv = new AchievementInfoView(as, this);
		l->addWidget(aiv);
	}
	{
		_lockedIcon = new QLabel(this);
		const QPixmap achievementPixmap = QPixmap(as.unlocked ? ":/Achievement_Unlocked.png" : ":/Achievement_Locked.png");
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		_lockedIcon->setFixedSize(64, 64);
		_lockedIcon->setPixmap(pixmap);

		_lockedIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		_lockedIcon->setProperty("score", true);

		l->addWidget(_lockedIcon, Qt::AlignRight);
	}

	setLayout(l);

	setFixedSize(800, 70);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	l->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

	setProperty("score", true);

	updateIcons();
}

AchievementView::~AchievementView() {
}

void AchievementView::updateIcons() {
	if (_achievement.iconUnlocked.empty() && _achievement.iconLocked.empty()) return;
	
	QString iconUnlocked = ":/Achievement_Unlocked.png";
	QString iconLocked = ":/Achievement_Locked.png";
	
	if (!_achievement.iconUnlocked.empty()) {
		QString filename = QString::fromStdString(_achievement.iconUnlocked);
		filename.chop(2);
		iconUnlocked = Config::MODDIR + "/mods/" + QString::number(_modID) + "/achievements/" + filename;
	}
	if (!_achievement.iconLocked.empty()) {
		QString filename = QString::fromStdString(_achievement.iconLocked);
		filename.chop(2);
		iconLocked = Config::MODDIR + "/mods/" + QString::number(_modID) + "/achievements/" + filename;
	}

	{
		QPixmap achievementPixmap(iconUnlocked);
		if (achievementPixmap.isNull()) {
			achievementPixmap = QPixmap(":/Achievement_Unlocked.png");
		}
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		_unlockedIcon->setPixmap(pixmap);
	}
	{
		QPixmap achievementPixmap(_achievement.unlocked ? iconUnlocked : iconLocked);
		if (!_achievement.iconUnlocked.empty() && _achievement.iconLocked.empty() && !_achievement.unlocked) {
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
			achievementPixmap = QPixmap(_achievement.unlocked ? ":/Achievement_Unlocked.png" : ":/Achievement_Locked.png");
		}
		const QPixmap pixmap = achievementPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		_lockedIcon->setPixmap(pixmap);
	}
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
