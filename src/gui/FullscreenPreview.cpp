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

#include "FullscreenPreview.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPixmap>

using namespace spine::gui;

FullscreenPreview::FullscreenPreview(QString imagePath, QWidget * parent) : QDialog(parent, Qt::Popup) {
	imagePath.chop(2);
	const QPixmap preview(imagePath);
	QSize targetSize = preview.size();
	if (targetSize.width() > QApplication::desktop()->screenGeometry().size().width() || targetSize.height() > QApplication::desktop()->screenGeometry().size().height()) {
		targetSize = QApplication::desktop()->screenGeometry().size();
	}
	_pixmap = preview.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	setWindowState(windowState() | Qt::WindowState::WindowFullScreen);
}

void FullscreenPreview::paintEvent(QPaintEvent *) {
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
	p.drawPixmap((QApplication::desktop()->screenGeometry().width() - _pixmap.width()) / 2, (QApplication::desktop()->screenGeometry().height() - _pixmap.height()) / 2, _pixmap);
}

void FullscreenPreview::mouseDoubleClickEvent(QMouseEvent *) {
	accept();
}

void FullscreenPreview::mousePressEvent(QMouseEvent *) {
	accept();
}
