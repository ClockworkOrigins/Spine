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

#include "widgets/ModInfoPage.h"

#include <thread>

#include "Config.h"
#include "Database.h"
#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "common/SpineModules.h"

#include "utils/Compression.h"
#include "utils/Conversion.h"
#include "utils/Hashing.h"

#include "widgets/DownloadProgressDialog.h"
#include "widgets/FullscreenPreview.h"
#include "widgets/GeneralSettingsWidget.h"
#include "widgets/NewsWidget.h"
#include "widgets/RatingWidget.h"
#include "widgets/WaitSpinner.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QScrollArea>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <QtConcurrentRun>

namespace spine {
namespace widgets {

	ModInfoPage::ModInfoPage(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QSettings * iniParser, QWidget * par) : QWidget(par), _mainWindow(mainWindow), _modnameLabel(nullptr), _previewImageLabel(nullptr), _ratingWidget(nullptr), _rateWidget(nullptr), _thumbnailView(nullptr), _installButton(nullptr), _descriptionView(nullptr), _spineFeaturesView(nullptr), _thumbnailModel(nullptr), _spineFeatureModel(nullptr), _modID(-1), _screens(), _editInfoPageButton(nullptr), _descriptionEdit(nullptr), _featuresEdit(nullptr), _spineFeaturesEdit(nullptr), _addImageButton(nullptr), _deleteImageButton(nullptr), _moduleCheckBoxes(), _applyButton(nullptr), _generalSettingsWidget(generalSettingsWidget), _iniParser(iniParser), _waitSpinner(nullptr), _optionalPackageButtons(), _forceEdit(false) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QScrollArea * scrollArea = new QScrollArea(this);
		QWidget * widget = new QWidget(scrollArea);
		QVBoxLayout * scrollLayout = new QVBoxLayout();
		scrollLayout->setAlignment(Qt::AlignTop);
		widget->setLayout(scrollLayout);
		scrollArea->setWidget(widget);
		scrollArea->setWidgetResizable(true);
		scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		widget->setProperty("achievementEditor", true);

		_modnameLabel = new QLabel(widget);
		_modnameLabel->setProperty("modnameTitle", true);
		scrollLayout->addWidget(_modnameLabel, 0, Qt::AlignTop);
		_modnameLabel->hide();

		{
			QHBoxLayout * hl = new QHBoxLayout();
			hl->addStretch(25);
			_previewImageLabel = new QLabel(widget);
			hl->addWidget(_previewImageLabel, 50, Qt::AlignHCenter);
			_previewImageLabel->hide();
			_previewImageLabel->setFixedSize(640, 480);

			{
				QVBoxLayout * vl = new QVBoxLayout();
				_ratingWidget = new RatingWidget(this);
				vl->addWidget(_ratingWidget, 0, Qt::AlignTop | Qt::AlignRight);
				_ratingWidget->setEditable(false);
				_ratingWidget->setVisible(false);

				_rateWidget = new RatingWidget(this);
				vl->addWidget(_rateWidget, 0, Qt::AlignTop | Qt::AlignRight);
				_rateWidget->setEditable(true);
				_rateWidget->setVisible(false);

				vl->addStretch(1);
				hl->addLayout(vl, 25);
			}

			QVBoxLayout * vl = new QVBoxLayout();
			_addImageButton = new QPushButton(QIcon(":/svg/add.svg"), "", widget);
			connect(_addImageButton, SIGNAL(released()), this, SLOT(addImage()));
			_addImageButton->hide();

			vl->addWidget(_addImageButton, 0, Qt::AlignTop | Qt::AlignRight);

			_deleteImageButton = new QPushButton(QIcon(":/svg/remove.svg"), "", widget);
			connect(_deleteImageButton, SIGNAL(released()), this, SLOT(removeImage()));
			_deleteImageButton->hide();

			vl->addWidget(_deleteImageButton, 0, Qt::AlignTop | Qt::AlignRight);
			vl->addStretch(1);

			hl->addLayout(vl);
			hl->setStretchFactor(_previewImageLabel, 1);

			scrollLayout->addLayout(hl);
		}

		_thumbnailView = new QListView(widget);
		_thumbnailView->setIconSize(QSize(300, 100));
		_thumbnailView->setViewMode(QListView::IconMode);
		_thumbnailView->setUniformItemSizes(true);
		scrollLayout->addWidget(_thumbnailView, 0, Qt::AlignCenter);
		_thumbnailView->hide();
		_thumbnailView->setFixedHeight(140);
		_thumbnailView->setSpacing(5);
		_thumbnailView->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
		_thumbnailView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
		_thumbnailView->setWrapping(false);
		connect(_thumbnailView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(changePreviewImage(const QModelIndex &)));
		connect(_thumbnailView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(changePreviewImage(const QModelIndex &)));
		connect(_thumbnailView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(showFullscreen()));
		connect(_thumbnailView, SIGNAL(activated(const QModelIndex &)), this, SLOT(showFullscreen()));

		_thumbnailModel = new QStandardItemModel(_thumbnailView);
		_thumbnailView->setModel(_thumbnailModel);
		connect(_thumbnailView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(changedThumbnailSelection(QItemSelection)));

		_installButton = new QPushButton(QIcon(":/svg/download.svg"), QApplication::tr("Install"), widget);
		scrollLayout->addWidget(_installButton, 0, Qt::AlignLeft);
		_installButton->hide();
		UPDATELANGUAGESETTEXT(_installButton, "Install");
		connect(_installButton, SIGNAL(released()), this, SLOT(installMod()));

		_startButton = new QPushButton(QApplication::tr("StartMod"), widget);
		scrollLayout->addWidget(_startButton, 0, Qt::AlignLeft);
		_startButton->hide();
		UPDATELANGUAGESETTEXT(_startButton, "StartMod");
		connect(_startButton, SIGNAL(released()), this, SLOT(startMod()));

		_optionalPackageButtonsLayout = new QVBoxLayout();
		scrollLayout->addLayout(_optionalPackageButtonsLayout);

		QHBoxLayout * hl = new QHBoxLayout();
		hl->setAlignment(Qt::AlignTop);

		_descriptionView = new QTextBrowser(widget);
		_descriptionView->setOpenExternalLinks(true);
		hl->addWidget(_descriptionView);
		_descriptionView->hide();

		_descriptionEdit = new QTextEdit(widget);
		hl->addWidget(_descriptionEdit);
		_descriptionEdit->hide();
		_descriptionEdit->setPlaceholderText(QApplication::tr("InfoPageDescriptionPlaceholder"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_descriptionEdit, "InfoPageDescriptionPlaceholder");

		_spineFeaturesView = new QListView(widget);
		hl->addWidget(_spineFeaturesView, 0, Qt::AlignTop);
		_spineFeaturesView->hide();

		_spineFeaturesEdit = new QGroupBox(QApplication::tr("SpineFeatures"), widget);
		hl->addWidget(_spineFeaturesEdit);
		_spineFeaturesEdit->hide();
		UPDATELANGUAGESETTITLE(_spineFeaturesEdit, "SpineFeatures");
		{
			QVBoxLayout * vl = new QVBoxLayout();
			vl->setAlignment(Qt::AlignTop);
			_spineFeaturesEdit->setLayout(vl);

			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("AchievementModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::Achievements, cb);
				vl->addWidget(cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("ScoresModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::Scores, cb);
				vl->addWidget(cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("MultiplayerModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::Multiplayer, cb);
				vl->addWidget(cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("OverallSaveModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::OverallSave, cb);
				vl->addWidget(cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("GamepadModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::Gamepad, cb);
				vl->addWidget(cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("FriendsModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::Friends, cb);
				vl->addWidget(cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("StatisticsModule"), _spineFeaturesEdit);
				_moduleCheckBoxes.insert(common::SpineModules::Statistics, cb);
				vl->addWidget(cb);
			}
		}

		hl->setStretchFactor(_descriptionView, 1);
		hl->setStretchFactor(_descriptionEdit, 1);

		_spineFeatureModel = new QStandardItemModel(_spineFeaturesView);
		_spineFeaturesView->setModel(_spineFeatureModel);
		_spineFeaturesView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
		_spineFeaturesView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
		_spineFeaturesView->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
		connect(_spineFeaturesView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectedSpineFeature(const QModelIndex &)));

		scrollLayout->addLayout(hl);

		_featuresEdit = new QLineEdit(widget);
		scrollLayout->addWidget(_featuresEdit);
		_featuresEdit->hide();
		_featuresEdit->setPlaceholderText(QApplication::tr("FeaturesEditPlaceholder"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(_featuresEdit, "FeaturesEditPlaceholder");

		l->addWidget(scrollArea);

		_editInfoPageButton = new QPushButton(QIcon(":/svg/edit.svg"), "", this);
		_editInfoPageButton->setToolTip(QApplication::tr("EditInfoPageTooltip"));
		UPDATELANGUAGESETTOOLTIP(_editInfoPageButton, "EditInfoPageTooltip");
		l->addWidget(_editInfoPageButton, 0, Qt::AlignBottom | Qt::AlignRight);
		_editInfoPageButton->hide();
		connect(_editInfoPageButton, SIGNAL(released()), this, SLOT(switchToEdit()));

		_applyButton = new QPushButton(QApplication::tr("Apply"), this);
		UPDATELANGUAGESETTEXT(_applyButton, "Apply");
		l->addWidget(_applyButton, 0, Qt::AlignBottom | Qt::AlignRight);
		_applyButton->hide();
		connect(_applyButton, SIGNAL(released()), this, SLOT(submitChanges()));

		scrollLayout->addStretch(1);

		setLayout(l);

		qRegisterMetaType<int32_t>("int32_t");
		connect(this, SIGNAL(receivedPage(common::SendInfoPageMessage *)), this, SLOT(updatePage(common::SendInfoPageMessage *)));
		connect(this, SIGNAL(gotRandomMod(int32_t)), this, SLOT(loadPage(int32_t)));
	}

	void ModInfoPage::loginChanged() {
		_rateWidget->loginChanged();
	}

	void ModInfoPage::loadPage(int32_t modID) {
		if (modID == -1) {
			return;
		}
		delete _waitSpinner;
		_waitSpinner = new WaitSpinner(QApplication::tr("LoadingPage"), this);
		_modID = modID;
		std::thread([this, modID]() {
			common::RequestInfoPageMessage ripm;
			ripm.modID = modID;
			ripm.language = Config::Language.toStdString();
			ripm.username = Config::Username.toStdString();
			ripm.password = Config::Password.toStdString();
			std::string serialized = ripm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == err) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendInfoPageMessage * sipm = dynamic_cast<common::SendInfoPageMessage *>(m);
							if (sipm) {
								emit receivedPage(sipm);
							}
						}
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << int(err);
				}
			} else {
				qDebug() << "Error occurred: " << int(err);
			}
		}).detach();
	}

	void ModInfoPage::finishedInstallation(int modID, int, bool success) {
		if (_modID != modID || !success) {
			return;
		}
		Database::DBError err;
		const bool installed = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err) > 0;
		_installButton->setVisible(!installed);
		_startButton->setVisible(installed);

		for (QPushButton * pb : _optionalPackageButtons) {
			const bool packageInstalled = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT PackageID FROM packages WHERE ModID = " + std::to_string(_modID) + " AND PackageID = " + std::to_string(pb->property("packageid").toInt()) + " LIMIT 1;", err) > 0;
			pb->setVisible(installed && !packageInstalled);
		}
	}

	void ModInfoPage::updatePage(common::SendInfoPageMessage * sipm) {
		delete _waitSpinner;
		_waitSpinner = nullptr;
		_modnameLabel->setText(s2q(sipm->modname) + QString(" (%1.%2.%3)").arg(sipm->majorVersion).arg(sipm->minorVersion).arg(sipm->patchVersion));
		_modnameLabel->setVisible(!sipm->modname.empty());

		for (QPushButton * pb : _optionalPackageButtons) {
			pb->deleteLater();
		}
		_optionalPackageButtons.clear();

		_ratingWidget->setModID(_modID);
		_rateWidget->setModID(_modID);
		_ratingWidget->setModName(s2q(sipm->modname));
		_rateWidget->setModName(s2q(sipm->modname));
		_ratingWidget->setVisible(true);
		_rateWidget->setVisible(true);

		_thumbnailModel->clear();
		_screens.clear();
		int minWidth = INT_MAX;
		int maxWidth = 0;
		if (!sipm->screenshots.empty()) {
			QList<QPair<QString, QString>> images;
			for (const auto & p : sipm->screenshots) {
				QString filename = QString::fromStdString(p.first);
				filename.chop(2); // .z
				if (!QFile(Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + filename).exists()) {
					images.append(QPair<QString, QString>(QString::fromStdString(p.first), QString::fromStdString(p.second)));
				}
			}
			if (!images.empty()) {
				MultiFileDownloader * mfd = new MultiFileDownloader(this);
				connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
				connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
				for (const auto & p : images) {
					QString filename = p.first;
					filename.chop(2); // every image is compressed, so it has a .z at the end
					if (!QFile(Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + filename).exists()) {
						QFileInfo fi(p.first);
						FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/mods/" + QString::number(_modID) + "/screens/" + p.first), Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + fi.path(), fi.fileName(), p.second, mfd);
						mfd->addFileDownloader(fd);
					}
				}
				DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
				progressDlg.setCancelButton(nullptr);
				progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				progressDlg.exec();
			}
			QString filename = QString::fromStdString(sipm->screenshots[0].first);
			filename.chop(2);
			const QPixmap preview(Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + filename);
			_previewImageLabel->setPixmap(preview.scaled(QSize(640, 480), Qt::KeepAspectRatio, Qt::SmoothTransformation));
			for (const auto & p : sipm->screenshots) {
				QString fn = QString::fromStdString(p.first);
				fn.chop(2); // .z
				QStandardItem * itm = new QStandardItem();
				QPixmap thumb(Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + fn);
				QPixmap scaledThumb = thumb.scaled(QSize(300, 100), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				itm->setIcon(scaledThumb);
				minWidth = std::min(minWidth, scaledThumb.width());
				maxWidth = std::max(maxWidth, scaledThumb.width());
				_thumbnailModel->appendRow(itm);
			}
			_screens = sipm->screenshots;
		} else {
			_previewImageLabel->setPixmap(QPixmap());
		}
		_previewImageLabel->setVisible(!sipm->screenshots.empty());
		if (minWidth != INT_MAX) {
			_thumbnailView->setFixedWidth(std::min(width() - 50, int(sipm->screenshots.size()) * (maxWidth + 15)));
		}
		_thumbnailView->setVisible(!sipm->screenshots.empty());

		QString infoText = s2q(sipm->description);

		if (!sipm->features.empty()) {
			infoText.append("<br><br><strong>" + QApplication::tr("Features") + ":</strong><br><ul>");
			for (const std::string & f : sipm->features) {
				infoText.append("<li> " + s2q(f));
			}
			infoText.append("</ul>");
		}
		_descriptionView->setHtml(infoText.replace("&apos;", "'"));
		_descriptionView->setVisible(!infoText.isEmpty());
		_descriptionView->setMinimumHeight(static_cast<int>(_descriptionView->document()->size().height() + 75));
		_descriptionView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
		_descriptionEdit->setPlainText(s2q(sipm->description));

		QString featuresText;
		for (const auto & s : sipm->features) {
			featuresText.append(s2q(s) + ";");
		}
		_featuresEdit->setText(featuresText.replace("&apos;", "'"));

		_spineFeatureModel->clear();
		if (sipm->spineFeatures & common::SpineModules::Achievements) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("AchievementModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::Achievements), Qt::UserRole);
		}
		if (sipm->spineFeatures & common::SpineModules::Scores) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("ScoresModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::Scores), Qt::UserRole);
		}
		if (sipm->spineFeatures & common::SpineModules::Multiplayer) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("MultiplayerModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::Multiplayer), Qt::UserRole);
		}
		if (sipm->spineFeatures & common::SpineModules::OverallSave) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("OverallSaveModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::OverallSave), Qt::UserRole);
		}
		if (sipm->spineFeatures & common::SpineModules::Gamepad) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("GamepadModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::Gamepad), Qt::UserRole);
		}
		if (sipm->spineFeatures & common::SpineModules::Friends) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("FriendsModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::Friends), Qt::UserRole);
		}
		if (sipm->spineFeatures & common::SpineModules::Statistics) {
			QStandardItem * itm = new QStandardItem(QApplication::tr("StatisticsModule"));
			_spineFeatureModel->appendRow(itm);
			itm->setData(int(common::SpineModules::Statistics), Qt::UserRole);
		}
		_spineFeaturesView->setVisible(_spineFeatureModel->rowCount());
		_moduleCheckBoxes[common::SpineModules::Achievements]->setChecked(sipm->spineFeatures & common::SpineModules::Achievements);
		_moduleCheckBoxes[common::SpineModules::Scores]->setChecked(sipm->spineFeatures & common::SpineModules::Scores);
		_moduleCheckBoxes[common::SpineModules::Multiplayer]->setChecked(sipm->spineFeatures & common::SpineModules::Multiplayer);
		_moduleCheckBoxes[common::SpineModules::OverallSave]->setChecked(sipm->spineFeatures & common::SpineModules::OverallSave);
		_moduleCheckBoxes[common::SpineModules::Gamepad]->setChecked(sipm->spineFeatures & common::SpineModules::Gamepad);
		_moduleCheckBoxes[common::SpineModules::Friends]->setChecked(sipm->spineFeatures & common::SpineModules::Friends);
		_moduleCheckBoxes[common::SpineModules::Statistics]->setChecked(sipm->spineFeatures & common::SpineModules::Statistics);

		_editInfoPageButton->setVisible(sipm->editRights);

		_addImageButton->hide();
		_deleteImageButton->hide();
		_descriptionEdit->hide();
		_featuresEdit->hide();
		_spineFeaturesEdit->hide();
		_applyButton->hide();

		Database::DBError err;
		const bool installed = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(_modID) + " LIMIT 1;", err) > 0;
		_installButton->setVisible(!installed && sipm->installAllowed);
		_installButton->setProperty("modid", int(_modID));

		const QDirIterator it(Config::MODDIR + "/mods/" + QString::number(_modID) + "/System", QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
		_startButton->setVisible(installed && it.hasNext());

		for (const auto & p : sipm->optionalPackages) {
			const bool packageInstalled = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT PackageID FROM packages WHERE ModID = " + std::to_string(_modID) + " AND PackageID = " + std::to_string(p.first) + " LIMIT 1;", err) > 0;
			QPushButton * pb = new QPushButton(QIcon(":/svg/download.svg"), s2q(p.second), this);
			pb->setVisible(!packageInstalled && installed && sipm->installAllowed);
			pb->setProperty("packageid", int(p.first));
			_optionalPackageButtonsLayout->addWidget(pb, 0, Qt::AlignLeft);
			_optionalPackageButtons.append(pb);
			connect(pb, SIGNAL(released()), this, SLOT(installPackage()));
		}

		delete sipm;

		if (_forceEdit) {
			switchToEdit();
		}
	}

	void ModInfoPage::installMod() {
		const int32_t modID = sender()->property("modid").toInt();
		emit tryInstallMod(modID, -1);
	}

	void ModInfoPage::installPackage() {
		const int32_t packageID = sender()->property("packageid").toInt();
		emit tryInstallPackage(_modID, packageID);
	}

	void ModInfoPage::changePreviewImage(const QModelIndex & idx) {
		QString filename = QString::fromStdString(_screens[idx.row()].first);
		if (!_screens[idx.row()].second.empty()) { // downloaded screens: relative path
			filename.chop(2);
			const QPixmap preview(Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + filename);
			_previewImageLabel->setPixmap(preview.scaled(QSize(640, 480), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		} else {
			const QPixmap preview(filename);
			_previewImageLabel->setPixmap(preview.scaled(QSize(640, 480), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
	}

	void ModInfoPage::switchToEdit() {
		_addImageButton->show();
		_deleteImageButton->show();
		_descriptionView->hide();
		_descriptionEdit->show();
		_spineFeaturesView->hide();
		_spineFeaturesEdit->show();
		_featuresEdit->show();
		_installButton->hide();
		_editInfoPageButton->hide();
		_ratingWidget->setVisible(false);
		_rateWidget->setVisible(false);
		_applyButton->show();
		_previewImageLabel->show();
		_thumbnailView->show();
		_addImageButton->setEnabled(_screens.size() < 10 || Config::Username == "Bonne");

		_forceEdit = false;
	}

	void ModInfoPage::forceEditPage() {
		_forceEdit = true;
	}

	void ModInfoPage::addImage() {
		if (_screens.size() == 10 && Config::Username != "Bonne") {
			return;
		}
		const QString folder = _iniParser->value("PATH/Images", ".").toString();
		const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectImage"), folder, "Images (*.png *.jpg)");
		if (path.isEmpty()) {
			return;
		}
		_iniParser->setValue("PATH/Images", QFileInfo(path).absolutePath());
		QStandardItem * itm = new QStandardItem();
		const QPixmap thumb(path);
		itm->setIcon(thumb.scaled(QSize(300, 100), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		_thumbnailModel->appendRow(itm);
		_screens.emplace_back(path.toStdString(), "");
		_addImageButton->setEnabled(_screens.size() < 10 || Config::Username == "Bonne");
	}

	void ModInfoPage::removeImage() {
		if (!_thumbnailView->currentIndex().isValid()) {
			return;
		}
		const int row = _thumbnailView->currentIndex().row();
		_thumbnailModel->removeRow(row);
		_screens.erase(_screens.begin() + row);
	}

	void ModInfoPage::submitChanges() {
		WaitSpinner spinner(QApplication::tr("SubmittingPage"), this);

		QFutureWatcher<void> watcher(this);
		QEventLoop loop;
		connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
		int32_t modID = _modID;
		std::string language = Config::Language.toStdString();
		std::string description = q2s(_descriptionEdit->toPlainText());
		std::vector<std::string> features;
		for (const QString & s : _featuresEdit->text().split(";", QString::SkipEmptyParts)) {
			features.push_back(q2s(s));
		}
		int32_t modules = 0;
		if (_moduleCheckBoxes[common::SpineModules::Achievements]->isChecked()) {
			modules |= common::SpineModules::Achievements;
		}
		if (_moduleCheckBoxes[common::SpineModules::Scores]->isChecked()) {
			modules |= common::SpineModules::Scores;
		}
		if (_moduleCheckBoxes[common::SpineModules::Multiplayer]->isChecked()) {
			modules |= common::SpineModules::Multiplayer;
		}
		if (_moduleCheckBoxes[common::SpineModules::OverallSave]->isChecked()) {
			modules |= common::SpineModules::OverallSave;
		}
		if (_moduleCheckBoxes[common::SpineModules::Gamepad]->isChecked()) {
			modules |= common::SpineModules::Gamepad;
		}
		if (_moduleCheckBoxes[common::SpineModules::Friends]->isChecked()) {
			modules |= common::SpineModules::Friends;
		}
		if (_moduleCheckBoxes[common::SpineModules::Statistics]->isChecked()) {
			modules |= common::SpineModules::Statistics;
		}
		const QFuture<void> future = QtConcurrent::run([this, modID, language, description, features, modules]() {
			common::SubmitInfoPageMessage sipm;
			sipm.modID = modID;
			sipm.language = language;
			sipm.description = description;
			sipm.features = features;
			sipm.spineFeatures = modules;
			for (auto p : _screens) {
				if (p.second.empty()) {
					// new screenshot
					// 1. calculate hash
					QString hashSum;
					const bool b = utils::Hashing::hash(QString::fromStdString(p.first), hashSum);
					if (b) {
						p.second = hashSum.toStdString();
					}
					// 2. compress
					utils::Compression::compress(s2q(p.first), false);
					p.first += ".z";
					// 3. add image to message
					{
						QFile f(QString::fromStdString(p.first));
						if (f.open(QIODevice::ReadOnly)) {
							QByteArray byteArr = f.readAll();
							std::vector<uint8_t> buffer(byteArr.length());
							memcpy(&buffer[0], byteArr.data(), byteArr.length());
							sipm.imageFiles.emplace_back(QFileInfo(QString::fromStdString(p.first)).fileName().toStdString(), buffer);
						}
						f.close();
						f.remove();
					}
					sipm.screenshots.emplace_back(QFileInfo(QString::fromStdString(p.first)).fileName().toStdString(), p.second);
				} else {
					sipm.screenshots.push_back(p);
				}
			}
			std::string serialized = sipm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == err) {
				sock.writePacket(serialized);
				sock.receivePacket(serialized); // blocks until ack arrives or error happens
			} else {
				qDebug() << "Error occurred: " << int(err);
			}
		});
		watcher.setFuture(future);
		loop.exec();
		loadPage(_modID);
	}

	void ModInfoPage::requestRandomMod() {
		std::thread([this]() {
			common::RequestRandomModMessage rrmm;
			rrmm.language = Config::Language.toStdString();
			std::string serialized = rrmm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == err) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendRandomModMessage * srmm = dynamic_cast<common::SendRandomModMessage *>(m);
							if (srmm) {
								emit gotRandomMod(srmm->modID);
							}
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << int(err);
				}
			} else {
				qDebug() << "Error occurred: " << int(err);
			}
		}).detach();
	}

	void ModInfoPage::startMod() {
		QDirIterator it(Config::MODDIR + "/mods/" + QString::number(_modID) + "/System", QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
		if (it.hasNext()) {
			it.next();
			emit triggerModStart(_modID, it.filePath());
		}
	}

	void ModInfoPage::showFullscreen() {
		QItemSelectionModel * selectionModel = _thumbnailView->selectionModel();
		const QModelIndexList list = selectionModel->selectedRows();
		const int row = list.isEmpty() ? 0 : selectionModel->selectedRows().front().row();
		FullscreenPreview fp(Config::MODDIR + "/mods/" + QString::number(_modID) + "/screens/" + QString::fromStdString(_screens[row].first), this);
		fp.exec();
	}

	void ModInfoPage::selectedSpineFeature(const QModelIndex & idx) {
		if (idx.data(Qt::UserRole).toInt() == common::SpineModules::Achievements) {
			emit openAchievementView(_modID, _modnameLabel->text());
		} else if (idx.data(Qt::UserRole).toInt() == common::SpineModules::Scores) {
			emit openScoreView(_modID, _modnameLabel->text());
		}
	}

	void ModInfoPage::changedThumbnailSelection(QItemSelection selection) {
		if (!selection.empty()) {
			changePreviewImage(selection.indexes().first());
		}
	}

	void ModInfoPage::mouseDoubleClickEvent(QMouseEvent * evt) {
		QWidget * child = childAt(evt->pos());
		if (child == _previewImageLabel) {
			showFullscreen();
		}
	}

	void ModInfoPage::showEvent(QShowEvent * evt) {
		QWidget::showEvent(evt);
		static bool firstOpen = true;
		if (firstOpen) {
			firstOpen = false;
			if (_modID == -1) {
				requestRandomMod();
			}
		}
	}

} /* namespace widgets */
} /* namespace spine */
