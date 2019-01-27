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

#ifndef __SPINE_WIDGETS_LOCATIONSETTINGSWIDGET_H__
#define __SPINE_WIDGETS_LOCATIONSETTINGSWIDGET_H__

#include <atomic>

#include <QWidget>

class QLineEdit;
class QSettings;

namespace spine {
namespace widgets {

	class GeneralSettingsWidget;

	class LocationSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		LocationSettingsWidget(QSettings * iniParser, GeneralSettingsWidget * generalSettingsWidget, bool temporary, QWidget * par);

		void saveSettings();
		void rejectSettings();

		QString getGothicDirectory() const;
		QString getGothic2Directory() const;
		QString getScreenshotDirectory() const;

		bool isGothicValid(bool restored) const;
		bool isGothic2Valid(bool restored) const;

	signals:
		void pathChanged();
		void validGothic(bool);
		void validGothic2(bool);
		void downloadPathChanged();
		void foundGothic(QString);
		void foundGothic2(QString);
		void screenshotDirectoryChanged(QString);
		void finishedSearch();
		void finishedFolder(QString);

	public slots:
		void setGothicDirectory(QString path);
		void setGothic2Directory(QString path);

	private slots:
		void openGothicFileDialog();
		void openGothic2FileDialog();
		void openDownloadFileDialog();
		void searchGothic();
		void openScreenshotDirectoryFileDialog();

	private:
		QSettings * _iniParser;
		QLineEdit * _gothicPathLineEdit;
		QLineEdit * _gothic2PathLineEdit;
		QLineEdit * _downloadPathLineEdit;
		QLineEdit * _screenshotPathLineEdit;
		std::atomic<int> _futureCounter;
		bool _cancelSearch;

		bool isGothicValid(QString path, QString executable, bool restored) const;
		void searchGothicAsync(bool searchG1, bool searchG2);
		void checkPartition(QString partition, QString filter, bool * gothicFound, bool * gothic2Found, bool recursve = false);
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_LOCATIONSETTINGSWIDGET_H__ */
