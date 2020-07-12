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
// Copyright 2019 Clockwork Origins

#include "ImageMerger.h"

#include <QPixmap>

using namespace spine::utils;

QPixmap ImageMerger::merge(const QPixmap & a, const QPixmap & b) {
	QImage img(a.width() + 5 + b.width(), std::max(a.height(), b.height()), QImage::Format::Format_ARGB32);
	img.fill(Qt::transparent);

	QImage tmpImage = a.toImage();
	
	for (int i = 0; i < a.height(); i++) {
		for (int j = 0; j < a.width(); j++) {
			img.setPixel(j, i, tmpImage.pixel(j, i));
		}
	}
	
	tmpImage = b.toImage();
	for (int i = 0; i < b.height(); i++) {
		for (int j = 0; j < b.width(); j++) {
			img.setPixel(j + a.width() + 5, i, tmpImage.pixel(j, i));
		}
	}
	
	return QPixmap::fromImage(img);
}

QPixmap ImageMerger::merge(const QPixmap & a, const QPixmap & b, const QPixmap & c) {
	const auto ab = merge(a, b);

	return merge(ab, c);
}

QPixmap ImageMerger::merge(const QPixmap & a, const QPixmap & b, const QPixmap & c, const QPixmap & d) {
	const auto ab = merge(a, b);
	const auto cd = merge(c, d);

	return merge(ab, cd);
}
