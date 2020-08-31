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

#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;

namespace spine {
namespace utils {
	class MultiFileDownloader;
} /* namespace utils */
namespace gui {

	class DownloadEntryWidget;
	
	class DownloadQueueWidget : public QWidget {
		Q_OBJECT

	public:
		DownloadQueueWidget(QWidget * par);

		static DownloadQueueWidget * getInstance();

		void addDownload(const QString & name, utils::MultiFileDownloader * mfd);

	public slots:
		void setTitle(QString text);

	private slots:
		void remove();

	private:
		static DownloadQueueWidget * instance;
		
		QList<DownloadEntryWidget *> _downloadEntries;

		QVBoxLayout * _entryLayout;

		QLabel * _titleLabel;
	};

} /* namespace gui */
} /* namespace spine */
