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

#ifndef __SPINE_WIDGETS_DOWNLOADPROGRESSDIALOG_H__
#define __SPINE_WIDGETS_DOWNLOADPROGRESSDIALOG_H__

#include <QProgressDialog>

class QMainWindow;
class QWinTaskbarProgress;

namespace spine {
	class MultiFileDownloader;
	enum class DownloadError;
namespace widgets {

	class GeneralSettingsWidget;

	class DownloadProgressDialog : public QProgressDialog {
		Q_OBJECT

	public:
		DownloadProgressDialog(MultiFileDownloader * downloader, QString labelText, qint64 min, qint64 max, qint64 maxSize, QMainWindow * mainWindow);
		~DownloadProgressDialog();

		bool hasDownloadSucceeded() const;
		DownloadError getLastError() const;

	public slots:
		int exec() override;

	private slots:
		void setValue(qint64 value);
		void setMaximum(qint64 max);
		void downloadSucceeded();
		void downloadFailed(DownloadError err);
		void downloadFile(QString fileName);

	private:
		bool _downloadSuccessful;
		bool _finished;
		MultiFileDownloader * _downloader;
		QWinTaskbarProgress * _taskbarProgress;
		QString _labelText;
		QString _currentValue;
		QString _maximumValue;
		qint64 _maxSize;
		QString _currentFileName;
		DownloadError _lastError;

		void closeEvent(QCloseEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_DOWNLOADPROGRESSDIALOG_H__ */
