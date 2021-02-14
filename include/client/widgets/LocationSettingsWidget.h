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

#pragma once

#include <atomic>

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QSettings;

namespace spine {
namespace widgets {

	class GeneralSettingsWidget;

	class LocationSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		LocationSettingsWidget(bool temporary, QWidget * par);

		static LocationSettingsWidget * getInstance();

		void saveSettings();
		void rejectSettings();

		QString getGothicDirectory() const;
		QString getGothic2Directory() const;
		QString getGothic3Directory() const;
		QString getScreenshotDirectory() const;

		bool isGothicValid(bool restored) const;
		bool isGothic2Valid(bool restored) const;
		bool isGothic3Valid(bool restored) const;

		bool startGothicWithSteam() const;
		bool startGothic2WithSteam() const;
		bool startGothic3WithSteam() const;

	signals:
		void pathChanged();
		void validGothic(bool);
		void validGothic2(bool);
		void validGothic3(bool);
		void downloadPathChanged();
		void foundGothic(QString);
		void foundGothic2(QString);
		void foundGothic3(QString);
		void screenshotDirectoryChanged(QString);
		void finishedSearch();
		void finishedFolder(QString);

	public slots:
		void setGothicDirectory(QString path);
		void setGothic2Directory(QString path);
		void setGothic3Directory(QString path);

	private slots:
		void openGothicFileDialog();
		void openGothic2FileDialog();
		void openGothic3FileDialog();
		void openDownloadFileDialog();
		void searchGothic();
		void openScreenshotDirectoryFileDialog();

	private:
		QLineEdit * _gothicPathLineEdit;
		QLineEdit * _gothic2PathLineEdit;
		QLineEdit * _gothic3PathLineEdit;
		QLineEdit * _downloadPathLineEdit;
		QLineEdit * _screenshotPathLineEdit;
		std::atomic<int> _futureCounter;
		bool _cancelSearch;

		QCheckBox * _gothicSteam;
		QCheckBox * _gothic2Steam;
		QCheckBox * _gothic3Steam;

		static LocationSettingsWidget * instance;

		typedef struct {
			bool gothicFound;
			bool gothic2Found;
			bool gothic3Found;
		} SearchConfig;

		bool isGothicValid(QString path, QString executable, bool restored) const;
		bool isGothic3Valid(QString path, QString executable, bool restored) const;
		void searchGothicAsync(bool searchG1, bool searchG2, bool searchG3);
		void checkPartition(QString partition, QString filter, SearchConfig * searchConfig, bool recursve = false);
	};

} /* namespace widgets */
} /* namespace spine */
