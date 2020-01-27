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

#include "common/MessageStructs.h"
#include "common/SpineModules.h"

#include <QItemSelection>
#include <QMap>
#include <QWidget>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListView;
class QMainWindow;
class QPushButton;
class QStandardItemModel;
class QTextBrowser;
class QTextEdit;
class QVBoxLayout;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace widgets {

	class GeneralSettingsWidget;
	class NewsWidget;
	class NewsWriterDialog;
	class RatingWidget;

	class ModInfoPage : public QWidget {
		Q_OBJECT

	public:
		ModInfoPage(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par);

	signals:
		void receivedPage(common::SendInfoPageMessage *);
		void tryInstallMod(int, int);
		void tryInstallPackage(int, int);
		void gotRandomMod(int32_t);
		void triggerModStart(int, QString);
		void openAchievementView(int32_t, QString);
		void openScoreView(int32_t, QString);

	public slots:
		void loginChanged();
		void loadPage(int32_t modID);
		void finishedInstallation(int modID, int packageID, bool success);
		void switchToEdit();
		void forceEditPage();
		void updateStarted(int modID);
		void updateFinished(int modID);

	private slots:
		void updatePage(common::SendInfoPageMessage * sipm);
		void installMod();
		void installPackage();
		void changePreviewImage(const QModelIndex & idx);
		void addImage();
		void removeImage();
		void submitChanges();
		void requestRandomMod();
		void startMod();
		void showFullscreen();
		void selectedSpineFeature(const QModelIndex & idx);
		void changedThumbnailSelection(QItemSelection selection);

	private:
		QMainWindow * _mainWindow;
		QLabel * _modnameLabel;
		QLabel * _previewImageLabel;
		RatingWidget * _ratingWidget;
		RatingWidget * _rateWidget;
		QListView * _thumbnailView;
		QPushButton * _installButton;
		QPushButton * _startButton;
		QVBoxLayout * _optionalPackageButtonsLayout;
		QTextBrowser * _descriptionView;
		QListView * _spineFeaturesView;
		QStandardItemModel * _thumbnailModel;
		QStandardItemModel * _spineFeatureModel;
		int32_t _modID;
		std::vector<std::pair<std::string, std::string>> _screens;
		QPushButton * _editInfoPageButton;
		QTextEdit * _descriptionEdit;
		QLineEdit * _featuresEdit;
		QGroupBox * _spineFeaturesEdit;
		QPushButton * _addImageButton;
		QPushButton * _deleteImageButton;
		QMap<common::SpineModules, QCheckBox *> _moduleCheckBoxes;
		QPushButton * _applyButton;
		GeneralSettingsWidget * _generalSettingsWidget;
		gui::WaitSpinner * _waitSpinner;
		QList<QPushButton *> _optionalPackageButtons;
		bool _forceEdit;
		QList<int> _runningUpdates;

		void mouseDoubleClickEvent(QMouseEvent * evt) override;
		void showEvent(QShowEvent * evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
