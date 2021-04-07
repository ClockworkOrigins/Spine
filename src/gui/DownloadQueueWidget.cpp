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

#include "gui/DownloadQueueWidget.h"

#include "DownloadEntryWidget.h"

#include "utils/DownloadQueue.h"
#include "utils/MultiFileDownloader.h"

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

using namespace spine::gui;
using namespace spine::utils;

DownloadQueueWidget * DownloadQueueWidget::instance = nullptr;

DownloadQueueWidget::DownloadQueueWidget(QWidget * par) : QWidget(par) {
	instance = this;
	
	_entryLayout = new QVBoxLayout();
	_entryLayout->setAlignment(Qt::AlignTop);
	
	_titleLabel = new QLabel(QApplication::tr("Downloads"), this);
	_titleLabel->setProperty("newsTitle", true);

	_entryLayout->addWidget(_titleLabel, 0, Qt::AlignHCenter);

	setLayout(_entryLayout);
}

DownloadQueueWidget * DownloadQueueWidget::getInstance() {
	return instance;
}

void DownloadQueueWidget::addDownload(const QString & name, MultiFileDownloader * mfd) {
	auto * dew = new DownloadEntryWidget(name, this);

	connect(mfd, &MultiFileDownloader::totalBytes, dew, &DownloadEntryWidget::setTotalBytes);
	connect(mfd, &MultiFileDownloader::downloadProgress, dew, &DownloadEntryWidget::setCurrentBytes);
	connect(mfd, &MultiFileDownloader::downloadSucceeded, dew, &DownloadEntryWidget::finished);
	connect(mfd, &MultiFileDownloader::downloadFailed, dew, &DownloadEntryWidget::aborted);
	connect(mfd, &MultiFileDownloader::startedDownload, dew, &DownloadEntryWidget::started);
	connect(dew, &DownloadEntryWidget::canceled, DownloadQueue::getInstance(), [mfd]() {
		DownloadQueue::getInstance()->cancel(mfd);
	});

	_entryLayout->addWidget(dew);

	_downloadEntries.append(dew);

	DownloadQueue::getInstance()->add(mfd);
}

void DownloadQueueWidget::setTitle(QString text) {
	_titleLabel->setText(text);
}

void DownloadQueueWidget::remove() {
	auto * entryWidget = dynamic_cast<DownloadEntryWidget *>(sender());
	_downloadEntries.removeAll(entryWidget);
}
