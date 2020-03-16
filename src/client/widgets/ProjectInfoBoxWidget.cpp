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
// Copyright 2208 Clockwork Origins

#include "widgets/ProjectInfoBoxWidget.h"

#include "common/MessageStructs.h"

#include "utils/Config.h"

#include <QApplication>
#include <QDate>
#include <QHBoxLayout>
#include <QLabel>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

ProjectInfoBoxWidget::ProjectInfoBoxWidget(QWidget * par) : QWidget(par) {
	QVBoxLayout * l = new QVBoxLayout();

	_releaseDateLabel = new QLabel(this);
	_releaseDateLabel->setProperty("small", true);
	l->addWidget(_releaseDateLabel, 0, Qt::AlignLeft);
	
	setLayout(l);

	setFixedSize(300, 300);
}

void ProjectInfoBoxWidget::update(common::SendInfoPageMessage * sipm) {
	QDate date(2000, 1, 1);
	date = date.addDays(sipm->releaseDate);
	_releaseDateLabel->setText(QString("%1: %2").arg(QApplication::tr("ReleaseDate").toUpper()).arg(date.toString("dd.MM.yyyy")));
}
