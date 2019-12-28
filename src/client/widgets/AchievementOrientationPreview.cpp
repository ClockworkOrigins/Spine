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

#include "widgets/AchievementOrientationPreview.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>

namespace spine {
namespace widgets {

	AchievementOrientationPreview::AchievementOrientationPreview(int type, QWidget * parent) : QDialog(parent, Qt::Popup), _type(type) {
		setWindowState(windowState() | Qt::WindowState::WindowFullScreen);
	}

	AchievementOrientationPreview::~AchievementOrientationPreview() {
	}

	void AchievementOrientationPreview::paintEvent(QPaintEvent *) {
		QPainter p(this);
		p.setRenderHints(QPainter::HighQualityAntialiasing);
		QColor backgroundColor;
		backgroundColor.setNamedColor("#000000");

		{
			QPainterPath path;
			path.addRect(QApplication::desktop()->screenGeometry());
			p.fillPath(path, QBrush(backgroundColor));
			p.drawPath(path);
		}

		{
			QRect area;
			if (_type == 0) {
				area = QRect(0, 0, 400, 80);
			} else if (_type == 1) {
				area = QRect(QApplication::desktop()->screenGeometry().width() - 400, 0, 400, 80);
			} else if (_type == 2) {
				area = QRect(0, QApplication::desktop()->screenGeometry().height() - 80, 400, 80);
			} else if (_type == 3) {
				area = QRect(QApplication::desktop()->screenGeometry().width() - 400, QApplication::desktop()->screenGeometry().height() - 80, 400, 80);
			}
			QPainterPath path;
			path.addRect(area);
			backgroundColor.setNamedColor("#800000");
			p.fillPath(path, QBrush(backgroundColor));
			p.drawPath(path);
		}
	}

	void AchievementOrientationPreview::mouseDoubleClickEvent(QMouseEvent *) {
		accept();
	}

	void AchievementOrientationPreview::mousePressEvent(QMouseEvent *) {
		accept();
	}

} /* namespace widgets */
} /* namespace spine */
