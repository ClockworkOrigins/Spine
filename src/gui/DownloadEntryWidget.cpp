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

#include "gui/DownloadEntryWidget.h"

#include "utils/Conversion.h"

#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStyleOption>

using namespace spine::gui;
using namespace spine::utils;

DownloadEntryWidget::DownloadEntryWidget(QString name, QWidget * par) : QWidget(par) {
	setObjectName("DownloadEntryWidget");
	
	auto * l = new QGridLayout();

	auto * lbl = new QLabel(name, this);
	
	l->addWidget(lbl, 0, 0);

	_progressBar = new QProgressBar(this);
	_progressBar->setAlignment(Qt::AlignCenter);

	l->addWidget(_progressBar, 0, 1);

	_downloadedBytes = new QLabel(this);

	l->addWidget(_downloadedBytes, 0, 2);

	_removeButton = new QPushButton("x", this);
	_removeButton->hide();

	l->addWidget(_removeButton, 0, 3);

	_cancelButton = new QPushButton("x", this);
	_cancelButton->show();

	l->addWidget(_cancelButton, 0, 4);

	l->setSpacing(5);

	l->setColumnStretch(0, 30);
	l->setColumnStretch(1, 50);
	l->setColumnStretch(2, 15);
	l->setColumnStretch(3, 5);
	l->setColumnStretch(4, 5);

	setLayout(l);

	connect(_removeButton, &QPushButton::released, this, &DownloadEntryWidget::removed);
	connect(_removeButton, &QPushButton::released, this, &QObject::deleteLater);

	connect(_cancelButton, &QPushButton::released, this, &DownloadEntryWidget::canceled);
	connect(_cancelButton, &QPushButton::released, _removeButton, &QWidget::show);
	connect(_cancelButton, &QPushButton::released, _removeButton, &QWidget::hide);

	setProperty("DownloadEntryWidget", true);

	setTotalBytes(0);
	setCurrentBytes(0);
}

void DownloadEntryWidget::setTotalBytes(qint64 bytes) {
	_progressBar->setMaximum(static_cast<int>(bytes / 1024 + 1));
	_maximumValue = byteToString(bytes);
	_downloadedBytes->setText(QApplication::tr("DownloadedBytes").arg(_currentValue, _maximumValue));
}

void DownloadEntryWidget::setCurrentBytes(qint64 bytes) {
	_progressBar->setValue(static_cast<int>(bytes / 1024));
	_currentValue = byteToString(bytes);
	_downloadedBytes->setText(QApplication::tr("DownloadedBytes").arg(_currentValue, _maximumValue));
}

void DownloadEntryWidget::started() {
	_removeButton->hide();
	_cancelButton->show();	
}

void DownloadEntryWidget::aborted() {
	_removeButton->show();
	_cancelButton->hide();
	_progressBar->setValue(0);
}

void DownloadEntryWidget::finished() {
	_removeButton->show();
	_cancelButton->hide();
	_progressBar->setValue(_progressBar->maximum());
}

void DownloadEntryWidget::paintEvent(QPaintEvent *) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
