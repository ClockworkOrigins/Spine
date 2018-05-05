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

#include <QFileInfo>
#include <QGuiApplication>
#include <QPixmap>

int main(int argc, char ** argv) {
	QGuiApplication app(argc, argv);
	argc--;
	argv++;
	while (argc) {
		QPixmap pixmap(argv[0]);
		QImage img = pixmap.scaled(QSize(64, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();
		QFileInfo fi(argv[0]);
		img.save(QString(argv[0]).replace(fi.suffix(), "png"));
		argc--;
		argv++;
	}
	return 0;
}
