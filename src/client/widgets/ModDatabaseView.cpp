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

#include "widgets/ModDatabaseView.h"

#include <utility>

#include "DatabaseFilterModel.h"
#include "FontAwesome.h"
#include "IconCache.h"
#include "InstallMode.h"
#include "SpineConfig.h"
#include "Uninstaller.h"

#include "common/MessageStructs.h"

#include "gui/DownloadQueueWidget.h"
#include "gui/OverlayMessageHandler.h"
#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/DownloadQueue.h"
#include "utils/FileDownloader.h"
#include "utils/ImageMerger.h"
#include "utils/LanguageConverter.h"
#include "utils/MultiFileDownloader.h"

#include "widgets/CenteredIconDelegate.h"
#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCheckBox>
#include <QDate>
#include <QDebug>
#include <QDirIterator>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QtConcurrentRun>
#include <QTreeView>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::common;
using namespace spine::gui;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

namespace spine {
namespace widgets {
namespace {
	QMap<int, QPixmap> languagePixmaps;
}
	
	struct InstalledMod {
		InstalledMod(const int i1, const int i2, const int i3, const int i4, const int i5) : id(i1), gothicVersion(static_cast<GameType>(i2)), majorVersion(static_cast<int8_t>(i3)), minorVersion(static_cast<int8_t>(i4)), patchVersion(static_cast<int8_t>(i5)) {
		}

		int32_t id;
		GameType gothicVersion;
		int8_t majorVersion;
		int8_t minorVersion;
		int8_t patchVersion;
	};

	struct InstalledPackage {
		InstalledPackage(const std::string & s1, const std::string & s2, std::string s3) : modID(std::stoi(s1)), packageID(std::stoi(s2)), file(std::move(s3)) {
		}

		int32_t modID;
		int32_t packageID;
		std::string file;
	};

	class TextItem : public QStandardItem {
	public:
		explicit TextItem(const QString text) : QStandardItem(text) {
			QStandardItem::setData(text, Qt::UserRole);
		}

		void setText(const QString text) {
			QStandardItem::setText(text);
			setData(text, Qt::UserRole);
		}
	};

	class DateItem : public QStandardItem {
	public:
		explicit DateItem(QDate date) : QStandardItem(date.toString("dd.MM.yyyy")) {
			QStandardItem::setData(date, Qt::UserRole);
		}
	};

	class PlayTimeItem : public QStandardItem {
	public:
		explicit PlayTimeItem(const int32_t playTime) : QStandardItem() {
			const QString timeString = utils::timeToString(playTime);
			setText(timeString);
			QStandardItem::setData(playTime, Qt::UserRole);
		}
	};

	class SizeItem : public QStandardItem {
	public:
		explicit SizeItem(const quint64 size) : QStandardItem() {
			setSize(size);
		}

		void setSize(const quint64 size) {
			QString sizeString;
			if (size == std::numeric_limits<uint64_t>::max()) {
				sizeString = "-";
			} else {
				QString unit = "B";
				auto dSize = static_cast<double>(size);
				while (dSize > 1024 && unit != "GB") {
					dSize /= 1024.0;
					if (unit == "B") {
						unit = "KB";
					} else if (unit == "KB") {
						unit = "MB";
					} else if (unit == "MB") {
						unit = "GB";
					}
				}
				sizeString = QString::number(dSize, 'f', 1) + " " + unit;
			}
			setText(sizeString);
			setData(size, Qt::UserRole);
		}
	};

	class VersionItem : public QStandardItem {
	public:
		VersionItem(const uint8_t majorVersion, const uint8_t minorVersion, const uint8_t patchVersion) : QStandardItem(QString::number(majorVersion) + "." + QString::number(minorVersion) + "." + QString::number(patchVersion)) {
			QStandardItem::setData(static_cast<quint64>(majorVersion * 256 * 256 + minorVersion * 256 + patchVersion), Qt::UserRole);
		}
	};

	class IntItem : public QStandardItem {
	public:
		IntItem(int i) : QStandardItem(QString::number(i)) {
			QStandardItem::setData(i, Qt::UserRole);
		}
	};
	
} /* namespace widgets */
} /* namespace spine */

ModDatabaseView::ModDatabaseView(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _mainWindow(mainWindow), _treeView(nullptr), _sourceModel(nullptr), _sortModel(nullptr), _gothicValid(false), _gothic2Valid(false), _gothic3Valid(false), _waitSpinner(nullptr), _allowRenderer(false), _cached(false) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);
	
	setProperty("default", true);

	_treeView = new QTreeView(this);
	_sourceModel = new QStandardItemModel(this);
	_sortModel = new DatabaseFilterModel(this);
	_sortModel->setSourceModel(_sourceModel);
	_sortModel->setSortRole(Qt::UserRole);
	_sortModel->setFilterRole(DatabaseRole::FilterRole);
	_sortModel->setFilterKeyColumn(DatabaseColumn::Name);
	_sortModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
	_treeView->setModel(_sortModel);

	const QStringList headerLabels = { QApplication::tr("ID"), QApplication::tr("Name"), QApplication::tr("Author"), QApplication::tr("Type"), QApplication::tr("Game"), QApplication::tr("DevTime"), QApplication::tr("AvgTime"), QApplication::tr("ReleaseDate"), QApplication::tr("UpdateDate"), QApplication::tr("Version"), QApplication::tr("Languages"), QApplication::tr("DownloadSize"), QString() };

	_sourceModel->setHorizontalHeaderLabels(headerLabels);
	_treeView->header()->setSortIndicatorShown(true);
	_treeView->header()->setStretchLastSection(true);
	_treeView->header()->setDefaultAlignment(Qt::AlignHCenter);
	_treeView->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	_treeView->header()->setModel(_sourceModel);
	_treeView->setAlternatingRowColors(true);
	_treeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	_treeView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	_treeView->setMinimumWidth(800);
	_treeView->setSortingEnabled(true);
	_treeView->sortByColumn(DatabaseColumn::Release, Qt::DescendingOrder);
	_treeView->setItemDelegate(new CenteredIconDelegate(_treeView));

	connect(_treeView->header(), &QHeaderView::sectionClicked, this, &ModDatabaseView::sortByColumn);

	connect(generalSettingsWidget, &GeneralSettingsWidget::languageChanged, this, &ModDatabaseView::changeLanguage);

	const auto iconDE = IconCache::getInstance()->getOrLoadIcon(":/languages/de-DE.png");
	const auto iconEN = IconCache::getInstance()->getOrLoadIcon(":/languages/en-US.png");
	const auto iconPL = IconCache::getInstance()->getOrLoadIcon(":/languages/pl-PL.png");
	const auto iconRU = IconCache::getInstance()->getOrLoadIcon(":/languages/ru-RU.png");
	
	auto pmDE = iconDE.pixmap(iconDE.availableSizes().first());
	auto pmEN = iconEN.pixmap(iconEN.availableSizes().first());
	auto pmPL = iconPL.pixmap(iconPL.availableSizes().first());
	auto pmRU = iconRU.pixmap(iconRU.availableSizes().first());

	pmDE = pmDE.scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	pmEN = pmEN.scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	pmPL = pmPL.scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	pmRU = pmRU.scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	languagePixmaps.insert(German, pmDE);
	languagePixmaps.insert(English, pmEN);
	languagePixmaps.insert(Polish, pmPL);
	languagePixmaps.insert(Russian, pmRU);

	const QPixmap pmDEEN = ImageMerger::merge(pmDE, pmEN);
	const QPixmap pmDEPL = ImageMerger::merge(pmDE, pmPL);
	const QPixmap pmDERU = ImageMerger::merge(pmDE, pmRU);
	
	const QPixmap pmENPL = ImageMerger::merge(pmEN, pmPL);
	const QPixmap pmENRU = ImageMerger::merge(pmEN, pmRU);
	
	const QPixmap pmPLRU = ImageMerger::merge(pmPL, pmRU);

	languagePixmaps.insert(German | English, pmDEEN);
	languagePixmaps.insert(German | Polish, pmDEPL);
	languagePixmaps.insert(German | Russian, pmDERU);
	
	languagePixmaps.insert(English | Polish, pmENPL);
	languagePixmaps.insert(English | Russian, pmENRU);
	
	languagePixmaps.insert(Polish | Russian, pmPLRU);

	const QPixmap pmDEENPL = ImageMerger::merge(pmDE, pmEN, pmPL);
	const QPixmap pmDEENRU = ImageMerger::merge(pmDE, pmEN, pmRU);
	const QPixmap pmDEPLRU = ImageMerger::merge(pmDE, pmPL, pmRU);
	
	const QPixmap pmENPLRU = ImageMerger::merge(pmEN, pmPL, pmRU);
	
	languagePixmaps.insert(German | English | Polish, pmDEENPL);
	languagePixmaps.insert(German | English | Russian, pmDEENRU);
	languagePixmaps.insert(German | Polish | Russian, pmDEPLRU);
	
	languagePixmaps.insert(English | Polish | Russian, pmENPLRU);
	
	const QPixmap pmDEENPLRU = ImageMerger::merge(pmDE, pmEN, pmPL, pmRU);
	
	languagePixmaps.insert(German | English | Polish | Russian, pmDEENPLRU);

	{
		auto * filterWidget = new QWidget(this);
		auto * hl = new QHBoxLayout();
		auto * le = new QLineEdit(filterWidget);
		le->setPlaceholderText(QApplication::tr("SearchPlaceholder"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(le, "SearchPlaceholder");

		connect(le, &QLineEdit::textChanged, this, &ModDatabaseView::changedFilterExpression);

		hl->addWidget(le);

		{
			auto * gbLanguages = new QGroupBox(QApplication::tr("Languages"), filterWidget);
			UPDATELANGUAGESETTITLE(gbLanguages, "Languages");
			
			auto * gl = new QGridLayout();

			auto * cbGerman = new QCheckBox(QApplication::tr("German"), filterWidget);
			cbGerman->setChecked(_sortModel->isLanguageActive(German));
			UPDATELANGUAGESETTEXT(cbGerman, "German");
			connect(cbGerman, &QCheckBox::stateChanged, [this](int state) {
				_sortModel->languageChanged(German, state);
			});

			auto * cbEnglish = new QCheckBox(QApplication::tr("English"), filterWidget);
			cbEnglish->setChecked(_sortModel->isLanguageActive(English));
			UPDATELANGUAGESETTEXT(cbEnglish, "English");
			connect(cbEnglish, &QCheckBox::stateChanged, [this](int state) {
				_sortModel->languageChanged(English, state);
			});

			auto * cbPolish = new QCheckBox(QApplication::tr("Polish"), filterWidget);
			cbPolish->setChecked(_sortModel->isLanguageActive(Polish));
			UPDATELANGUAGESETTEXT(cbPolish, "Polish");
			connect(cbPolish, &QCheckBox::stateChanged, [this](int state) {
				_sortModel->languageChanged(Polish, state);
			});

			auto * cbRussian = new QCheckBox(QApplication::tr("Russian"), filterWidget);
			cbRussian->setChecked(_sortModel->isLanguageActive(Russian));
			UPDATELANGUAGESETTEXT(cbRussian, "Russian");
			connect(cbRussian, &QCheckBox::stateChanged, [this](int state) {
				_sortModel->languageChanged(Russian, state);
			});

			auto * lblGerman = new QLabel(gbLanguages);
			lblGerman->setPixmap(languagePixmaps[German]);

			auto * lblEnglish = new QLabel(gbLanguages);
			lblEnglish->setPixmap(languagePixmaps[English]);

			auto * lblPolish = new QLabel(gbLanguages);
			lblPolish->setPixmap(languagePixmaps[Polish]);

			auto * lblRussian = new QLabel(gbLanguages);
			lblRussian->setPixmap(languagePixmaps[Russian]);

			gl->addWidget(cbGerman, 0, 0);
			gl->addWidget(lblGerman, 0, 1);
			gl->addWidget(cbEnglish, 1, 0);
			gl->addWidget(lblEnglish, 1, 1);
			gl->addWidget(cbPolish, 2, 0);
			gl->addWidget(lblPolish, 2, 1);
			gl->addWidget(cbRussian, 3, 0);
			gl->addWidget(lblRussian, 3, 1);
			gl->setRowStretch(4, 1);

			gbLanguages->setLayout(gl);

			hl->addWidget(gbLanguages);
		}

		{
			auto * gb = new QGroupBox(QApplication::tr("Type"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "Type");

			auto * gbHl = new QHBoxLayout();

			{
				auto * gbGame = new QGroupBox(QApplication::tr("Game"), gb);
				UPDATELANGUAGESETTITLE(gbGame, "Game");
				
				auto * vbl = new QVBoxLayout();

				auto * cbFullVersion = new QCheckBox(QApplication::tr("FullVersion"), filterWidget);
				cbFullVersion->setChecked(_sortModel->isFullVersionsActive());
				UPDATELANGUAGESETTEXT(cbFullVersion, "FullVersion");
				connect(cbFullVersion, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::fullVersionsChanged);

				auto * cbDemo = new QCheckBox(QApplication::tr("Demo"), filterWidget);
				cbDemo->setChecked(_sortModel->isDemosActive());
				UPDATELANGUAGESETTEXT(cbDemo, "Demo");
				connect(cbDemo, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::demosChanged);

				auto * cbPlayTesting = new QCheckBox(QApplication::tr("PlayTesting"), filterWidget);
				cbPlayTesting->setChecked(_sortModel->isPlayTestingActive());
				UPDATELANGUAGESETTEXT(cbPlayTesting, "PlayTesting");
				connect(cbPlayTesting, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::playTestingChanged);

				vbl->addWidget(cbFullVersion);
				vbl->addWidget(cbDemo);
				vbl->addWidget(cbPlayTesting);

				vbl->addStretch(1);

				gbGame->setLayout(vbl);

				gbHl->addWidget(gbGame);
			}

			{
				auto * gbMod = new QGroupBox(QApplication::tr("Modification"), gb);
				UPDATELANGUAGESETTITLE(gbMod, "Modification");
				
				auto * vbl = new QVBoxLayout();

				auto * cbTotalConversion = new QCheckBox(QApplication::tr("TotalConversion"), filterWidget);
				cbTotalConversion->setChecked(_sortModel->isTotalConversionActive());
				UPDATELANGUAGESETTEXT(cbTotalConversion, "TotalConversion");
				connect(cbTotalConversion, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::totalConversionChanged);

				auto * cbEnhancement = new QCheckBox(QApplication::tr("Enhancement"), filterWidget);
				cbEnhancement->setChecked(_sortModel->isEnhancementActive());
				UPDATELANGUAGESETTEXT(cbEnhancement, "Enhancement");
				connect(cbEnhancement, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::enhancementChanged);

				auto * cbPatch = new QCheckBox(QApplication::tr("Patch"), filterWidget);
				cbPatch->setChecked(_sortModel->isPathActive());
				UPDATELANGUAGESETTEXT(cbPatch, "Patch");
				connect(cbPatch, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::patchChanged);

				auto * cbTool = new QCheckBox(QApplication::tr("Tool"), filterWidget);
				cbTool->setChecked(_sortModel->isToolActive());
				UPDATELANGUAGESETTEXT(cbTool, "Tool");
				connect(cbTool, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::toolChanged);

				auto * cbOriginal = new QCheckBox(QApplication::tr("Original"), filterWidget);
				cbOriginal->setChecked(_sortModel->isOriginalActive());
				UPDATELANGUAGESETTEXT(cbOriginal, "Original");
				connect(cbOriginal, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::originalChanged);

				auto * cbGMP = new QCheckBox(QApplication::tr("GothicMultiplayer"), filterWidget);
				cbGMP->setChecked(_sortModel->isGMPActive());
				UPDATELANGUAGESETTEXT(cbGMP, "GothicMultiplayer");
				connect(cbGMP, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gmpChanged);

				vbl->addWidget(cbTotalConversion);
				vbl->addWidget(cbEnhancement);
				vbl->addWidget(cbPatch);
				vbl->addWidget(cbTool);
				vbl->addWidget(cbOriginal);
				vbl->addWidget(cbGMP);

				vbl->addStretch(1);

				gbMod->setLayout(vbl);

				gbHl->addWidget(gbMod);
			}

			gb->setLayout(gbHl);

			hl->addWidget(gb);
		}

		{
			auto * gb = new QGroupBox(QApplication::tr("Game"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "Game");

			auto * vbl = new QVBoxLayout();

			auto * cb1 = new QCheckBox(QApplication::tr("Gothic"), filterWidget);
			cb1->setChecked(_sortModel->isGothicActive());
			UPDATELANGUAGESETTEXT(cb1, "Gothic");
			connect(cb1, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gothicChanged);

			auto * cb2 = new QCheckBox(QApplication::tr("Gothic2"), filterWidget);
			cb2->setChecked(_sortModel->isGothic2Active());
			UPDATELANGUAGESETTEXT(cb2, "Gothic2");
			connect(cb2, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gothic2Changed);

			auto * cb3 = new QCheckBox(QApplication::tr("GothicAndGothic2"), filterWidget);
			cb3->setChecked(_sortModel->isGothicAndGothic2Active());
			UPDATELANGUAGESETTEXT(cb3, "GothicAndGothic2");
			connect(cb3, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gothicAndGothic2Changed);

			auto * cb4 = new QCheckBox(QApplication::tr("Game"), filterWidget);
			cb4->setChecked(_sortModel->isGamesActive());
			UPDATELANGUAGESETTEXT(cb4, "Game");
			connect(cb4, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gamesChanged);

			vbl->addWidget(cb1);
			vbl->addWidget(cb2);
			vbl->addWidget(cb3);
			vbl->addWidget(cb4);

			vbl->addStretch(1);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}

		{
			auto * gb = new QGroupBox(QApplication::tr("General"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "General");

			auto * vbl = new QVBoxLayout();

			auto * cb1 = new QCheckBox(QApplication::tr("InstalledFilter"), filterWidget);
			cb1->setChecked(_sortModel->isInstalledProjectsActive());
			UPDATELANGUAGESETTEXT(cb1, "InstalledFilter");
			connect(cb1, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::installedProjectsChanged);

			auto * cb2 = new QCheckBox(QApplication::tr("PlayedFilter"), filterWidget);
			cb2->setChecked(_sortModel->isPlayedProjectsActive());
			UPDATELANGUAGESETTEXT(cb2, "PlayedFilter");
			connect(cb2, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::playedProjectsChanged);

			vbl->addWidget(cb1);
			vbl->addWidget(cb2);

			vbl->addStretch(1);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}

		{
			auto * gb = new QGroupBox(QApplication::tr("DevTime"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "DevTime");

			auto * gl = new QGridLayout();

			auto * sb1 = new QSpinBox(gb);
			sb1->setMinimum(0);
			sb1->setMaximum(1000);
			sb1->setValue(_sortModel->getMinDuration());
			auto * sb2 = new QSpinBox(gb);
			sb2->setMinimum(0);
			sb2->setMaximum(1000);
			sb2->setValue(_sortModel->getMaxDuration());

			connect(sb1, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), _sortModel, &DatabaseFilterModel::minDurationChanged);
			connect(sb2, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), _sortModel, &DatabaseFilterModel::maxDurationChanged);

			auto * l1 = new QLabel(QApplication::tr("MinDurationHours"), gb);
			UPDATELANGUAGESETTEXT(l1, "MinDurationHours");
			auto * l2 = new QLabel(QApplication::tr("MaxDurationHours"), gb);
			UPDATELANGUAGESETTEXT(l2, "MaxDurationHours");

			gl->addWidget(l1, 0, 0);
			gl->addWidget(sb1, 0, 1);
			gl->addWidget(l2, 1, 0);
			gl->addWidget(sb2, 1, 1);
			gl->setRowStretch(2, 1);

			gb->setLayout(gl);

			hl->addWidget(gb);
		}

		filterWidget->setLayout(hl);

		l->addWidget(filterWidget);
	}

	l->addWidget(_treeView, 2);

	setLayout(l);

	qRegisterMetaType<QList<Mod>>("QList<common::Mod>");
	qRegisterMetaType<QList<QPair<int32_t, uint64_t>>>("QList<QPair<int32_t, uint64_t>>");
	qRegisterMetaType<Mod>("common::Mod");
	qRegisterMetaType<Package>("common::Package");
	qRegisterMetaType<QList<Package>>("QList<common::Package>");
	qRegisterMetaType<QList<std::pair<std::string, std::string>>>("QList<std::pair<std::string, std::string>>");
	qRegisterMetaType<QSharedPointer<QList<QPair<QString, QString>>>>("QSharedPointer<QList<QPair<QString, QString>>>");
	qRegisterMetaType<QSet<int32_t>>("QSet<int32_t>");

	connect(this, &ModDatabaseView::receivedModList, this, static_cast<void(ModDatabaseView::*)(QList<Mod>)>(&ModDatabaseView::updateModList));
	connect(this, &ModDatabaseView::receivedModFilesList, this, &ModDatabaseView::downloadModFiles);
	connect(_treeView, &QTreeView::clicked, this, &ModDatabaseView::selectedIndex);
	connect(_treeView, &QTreeView::doubleClicked, this, &ModDatabaseView::doubleClickedIndex);
	connect(this, &ModDatabaseView::receivedPackageList, this, &ModDatabaseView::updatePackageList);
	connect(this, &ModDatabaseView::receivedPackageFilesList, this, &ModDatabaseView::downloadPackageFiles);
	connect(this, &ModDatabaseView::triggerInstallMod, this, &ModDatabaseView::installMod);
	connect(this, &ModDatabaseView::triggerInstallPackage, this, &ModDatabaseView::installPackage);
	connect(this, &ModDatabaseView::receivedPlayedProjects, _sortModel, &DatabaseFilterModel::setPlayedProjects);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS mods(ModID INT NOT NULL, GothicVersion INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS modfiles(ModID INT NOT NULL, File TEXT NOT NULL, Hash TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS patches(ModID INT NOT NULL, Name TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS packages(ModID INT NOT NULL, PackageID INT NOT NULL, File TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS languages(ProjectID INT PRIMARY KEY, Language INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS supportedLanguages(ProjectID INT PRIMARY KEY, Languages INT NOT NULL);", err);

	err.error = false;
	Database::queryAll<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT SpineVersion FROM mods LIMIT 1;", err);

	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "ALTER TABLE mods ADD SpineVersion INT NOT NULL DEFAULT 0;", err);
	}

	_sortModel->setRendererAllowed(true);

	updateDatabaseEntries();

	updateModList(-1, -1, InstallMode::None);
}

void ModDatabaseView::changeLanguage(QString) {
	_cached = false;
	
	updateModList(-1, -1, InstallMode::None);
}

void ModDatabaseView::updateModList(int modID, int packageID, InstallMode mode) {
	if (!Config::OnlineMode) return;
	
	if (mode == InstallMode::Silent && modID != -1) {
		_installSilently.insert(modID);
	}
	if (!_cached) {
		delete _waitSpinner;
		_waitSpinner = new WaitSpinner(QApplication::tr("LoadingDatabase"), this);
		_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("ID") << QApplication::tr("Name") << QApplication::tr("Author") << QApplication::tr("Type") << QApplication::tr("Game") << QApplication::tr("DevTime") << QApplication::tr("AvgTime") << QApplication::tr("ReleaseDate") << QApplication::tr("UpdateDate") << QApplication::tr("Version") << QApplication::tr("Languages") << QApplication::tr("DownloadSize") << QString());
	}

	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["Language"] = Config::Language;

	if (!_cached) {
		Https::postAsync(DATABASESERVER_PORT, "requestAllProjects", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, modID, packageID](const QJsonObject & data, int statusCode) {
			if (statusCode != 200) return;

			if (!data.contains("Projects")) return;

			QList<Mod> projects;

			for (const auto jsonRef : data["Projects"].toArray()) {
				const auto jsonProj = jsonRef.toObject();

				Mod project;
				project.id = jsonProj["ProjectID"].toString().toInt();
				project.name = q2s(jsonProj["Name"].toString());
				project.gothic = static_cast<GameType>(jsonProj["GameType"].toString().toInt());
				project.type = static_cast<ModType>(jsonProj["ModType"].toString().toInt());
				project.supportedLanguages = static_cast<uint16_t>(jsonProj["SupportedLanguages"].toString().toInt());
				project.teamID = jsonProj["TeamID"].toString().toInt();
				project.teamName = q2s(jsonProj["TeamName"].toString());
				project.releaseDate = jsonProj["ReleaseDate"].toString().toInt();
				project.majorVersion = static_cast<int8_t>(jsonProj["MajorVersion"].toString().toInt());
				project.minorVersion = static_cast<int8_t>(jsonProj["MinorVersion"].toString().toInt());
				project.patchVersion = static_cast<int8_t>(jsonProj["PatchVersion"].toString().toInt());
				project.spineVersion = static_cast<int8_t>(jsonProj["SpineVersion"].toString().toInt());
				project.devDuration = jsonProj["DevDuration"].toString().toInt();
				project.avgDuration = jsonProj["AvgDuration"].toString().toInt();
				project.downloadSize = jsonProj["DownloadSize"].toString().toLongLong();
				project.updateDate = jsonProj["UpdateDate"].toString().toInt();
				project.language = static_cast<Language>(jsonProj["Language"].toString().toInt());

				projects.push_back(project);
			}

			emit receivedModList(projects);

			QSet<int32_t> playedProjects;
			for (const auto jsonRef : data["PlayedProjects"].toArray()) {
				const auto jsonProj = jsonRef.toObject();

				playedProjects << jsonProj["ID"].toString().toInt();
			}

			emit receivedPlayedProjects(playedProjects);

			QList<Package> packages;

			for (const auto jsonRef : data["Packages"].toArray()) {
				const auto jsonProj = jsonRef.toObject();

				Package package;
				package.packageID = jsonProj["PackageID"].toString().toInt();
				package.modID = jsonProj["ProjectID"].toString().toInt();
				package.name = q2s(jsonProj["Name"].toString());
				package.downloadSize = jsonProj["DownloadSize"].toString().toLongLong();
				package.language = q2s(jsonProj["Language"].toString());

				packages.push_back(package);
			}

			emit receivedPackageList(packages);

			_cached = true;

			if (modID > 0 && packageID > 0) {
				emit triggerInstallPackage(modID, packageID);
			} else if (modID > 0) {
				emit triggerInstallMod(modID);
			}
		});
	} else {
		if (modID > 0 && packageID > 0) {
			emit triggerInstallPackage(modID, packageID);
		} else if (modID > 0) {
			emit triggerInstallMod(modID);
		}
	}
}

void ModDatabaseView::gothicValidationChanged(bool valid) {
	_gothicValid = valid;
}

void ModDatabaseView::gothic2ValidationChanged(bool valid) {
	_gothic2Valid = valid;
}

void ModDatabaseView::gothic3ValidationChanged(bool valid) {
	_gothic3Valid = valid;
}

void ModDatabaseView::loginChanged() {
	// check if a GMP mod is installed and GMP is not installed
	Database::DBError dbErr;
	const std::vector<int> gmpModInstalled = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 62 OR ModID = 117 OR ModID = 171 OR ModID = 172 OR ModID = 173 OR ModID = 218;", dbErr);
	const bool gmpInstalled = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = 228 LIMIT 1;", dbErr) > 0;

	if (!gmpModInstalled.empty() && !gmpInstalled) {
		updateModList(228, -1, InstallMode::Silent);
			
		for (int modID : gmpModInstalled) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(modID) + " AND PatchID = 228;", dbErr);
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(modID) + ", 228, 1);", dbErr);
		}
	}

	_cached = false;
	updateModList(-1, -1, InstallMode::None);
}

void ModDatabaseView::setGothicDirectory(QString dir) {
	_gothicDirectory = dir;
}

void ModDatabaseView::setGothic2Directory(QString dir) {
	_gothic2Directory = dir;
}

void ModDatabaseView::updateModList(QList<Mod> mods) {
	_sourceModel->removeRows(0, _sourceModel->rowCount());
	_parentMods.clear();
	int row = 0;
	Database::DBError err;
	std::vector<InstalledMod> ims = Database::queryAll<InstalledMod, int, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods;", err);
	QSet<int32_t> installedMods;
	for (InstalledMod im : ims) {
		installedMods.insert(im.id);
	}

	const QFontMetrics fm(_treeView->font());
	
	for (const Mod & mod : mods) {
		const QString modname = s2q(mod.name);
		QStandardItem * nameItem = new TextItem(fm.elidedText(modname, Qt::ElideRight, 300));
		nameItem->setData(modname, DatabaseRole::FilterRole);
		nameItem->setEditable(false);
		{
			QFont f = nameItem->font();
			f.setUnderline(true);
			nameItem->setFont(f);
		}
		const QString teamname = s2q(mod.teamName);
		QStandardItem * teamItem = new TextItem(fm.elidedText(teamname, Qt::ElideRight, 200));
		teamItem->setEditable(false);
		QString typeName;
		switch (mod.type) {
		case ModType::TOTALCONVERSION: {
			typeName = QApplication::tr("TotalConversion");
			break;
		}
		case ModType::ENHANCEMENT: {
			typeName = QApplication::tr("Enhancement");
			break;
		}
		case ModType::PATCH: {
			typeName = QApplication::tr("Patch");
			break;
		}
		case ModType::TOOL: {
			typeName = QApplication::tr("Tool");
			break;
		}
		case ModType::ORIGINAL: {
			typeName = QApplication::tr("Original");
			break;
		}
		case ModType::GMP: {
			typeName = QApplication::tr("GothicMultiplayer");
			break;
		}
		case ModType::FULLVERSION: {
			typeName = QApplication::tr("FullVersion");
			break;
		}
		case ModType::DEMO: {
			typeName = QApplication::tr("Demo");
			break;
		}
		case ModType::PLAYTESTING: {
			typeName = QApplication::tr("PlayTesting");
			break;
		}
		default: {
			break;
		}
		}
		QStandardItem * typeItem = new TextItem(typeName);
		typeItem->setEditable(false);
		QString gameName;
		switch (mod.gothic) {
		case GameType::Gothic: {
			gameName = QApplication::tr("Gothic");
			break;
		}
		case GameType::Gothic2: {
			gameName = QApplication::tr("Gothic2");
			break;
		}
		case GameType::GothicInGothic2: {
			gameName = QApplication::tr("GothicInGothic2");
			break;
		}
		case GameType::Gothic1And2: {
			gameName = QApplication::tr("GothicAndGothic2_2");
			break;
		}
		case GameType::Game: {
			gameName = QApplication::tr("Game");
			break;
		}
		}
		QStandardItem * gameItem = new TextItem(gameName);
		gameItem->setEditable(false);
		QStandardItem * devTimeItem = new PlayTimeItem(mod.devDuration);
		devTimeItem->setToolTip(QApplication::tr("DevTimeTooltip"));
		devTimeItem->setEditable(false);
		QStandardItem * avgTimeItem = new PlayTimeItem(mod.avgDuration);
		avgTimeItem->setToolTip(QApplication::tr("AvgTimeTooltip"));
		avgTimeItem->setEditable(false);
		QDate date(2000, 1, 1);
		date = date.addDays(mod.releaseDate);
		auto * releaseDateItem = new DateItem(date);
		releaseDateItem->setEditable(false);

		date = QDate(2000, 1, 1);
		date = date.addDays(std::max(mod.releaseDate, mod.updateDate));
		auto * updateDateItem = new DateItem(date);
		updateDateItem->setEditable(false);
		
		auto * versionItem = new VersionItem(mod.majorVersion, mod.minorVersion, mod.patchVersion);
		versionItem->setEditable(false);
		auto * sizeItem = new SizeItem(mod.downloadSize);
		sizeItem->setEditable(false);
		QStandardItem * buttonItem = nullptr;
		if (_downloadingList.contains(mod.id)) {
			buttonItem = new TextItem(QApplication::tr("InQueue"));
			buttonItem->setToolTip(QApplication::tr("InQueue"));
			buttonItem->setData(false, Installed);
		} else if (installedMods.find(mod.id) == installedMods.end()) {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
			buttonItem->setData(false, Installed);
		} else {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::trasho)));
			buttonItem->setToolTip(QApplication::tr("Uninstall"));
			buttonItem->setData(true, Installed);
		}
		QFont f = buttonItem->font();
		f.setPointSize(13);
		f.setFamily("FontAwesome");
		buttonItem->setFont(f);
		buttonItem->setEditable(false);

		QStandardItem * idItem = new IntItem(mod.id);
		idItem->setEditable(false);

		auto * languagesItem = new QStandardItem();
		languagesItem->setEditable(false);

		const auto pm = languagePixmaps[mod.supportedLanguages];
		
		languagesItem->setData(QVariant(pm), Qt::DecorationRole);
		languagesItem->setData(static_cast<qint32>(mod.supportedLanguages), LanguagesRole);

		_sourceModel->appendRow(QList<QStandardItem *>() << idItem << nameItem << teamItem << typeItem << gameItem << devTimeItem << avgTimeItem << releaseDateItem << updateDateItem << versionItem << languagesItem << sizeItem << buttonItem);
		for (int i = 0; i < _sourceModel->columnCount(); i++) {
			_sourceModel->setData(_sourceModel->index(row, i), Qt::AlignCenter, Qt::TextAlignmentRole);
		}
		if ((mod.gothic == GameType::Gothic && !_gothicValid) || (mod.gothic == GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == GameType::GothicInGothic2 && (!_gothicValid || !_gothic2Valid)) || (mod.gothic == GameType::Gothic1And2 && !_gothicValid && !_gothic2Valid) || Config::DOWNLOADDIR.isEmpty() || !QDir(Config::DOWNLOADDIR).exists()) {
			idItem->setEnabled(false);
			nameItem->setEnabled(false);
			teamItem->setEnabled(false);
			typeItem->setEnabled(false);
			gameItem->setEnabled(false);
			devTimeItem->setEnabled(false);
			avgTimeItem->setEnabled(false);
			releaseDateItem->setEnabled(false);
			updateDateItem->setEnabled(false);
			versionItem->setEnabled(false);
			languagesItem->setEnabled(false);
			sizeItem->setEnabled(false);
			buttonItem->setEnabled(false);
		}
		
		_parentMods.insert(mod.id, _sourceModel->index(row, 0));
		row++;
	}
	_mods = mods;

	QtConcurrent::run([mods]() {
		for (const auto & mod : mods) {
			Database::DBError err2;
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO supportedLanguages (ProjectID, Languages) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(mod.supportedLanguages) + ");", err2);

			if (err2.error) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE supportedLanguages SET Languges = " + std::to_string(mod.supportedLanguages) + " WHERE ProjectID = " + std::to_string(mod.id) + ";", err2);
			}
		}
	});
}

void ModDatabaseView::selectedIndex(const QModelIndex & index) {
	if (index.column() == DatabaseColumn::Install) {
		if (!index.parent().isValid()) { // Mod has no parent, only packages have
			selectedModIndex(index);
		} else { // package
			selectedPackageIndex(index);
		}
	} else if (index.column() == DatabaseColumn::Name) {
		if (!index.parent().isValid()) { // Mod has no parent, only packages have
			const Mod mod = _mods[_sortModel->mapToSource(index).row()];
			emit loadPage(mod.id);
		} else { // package
			const Mod mod = _mods[_sortModel->mapToSource(index.parent()).row()];
			emit loadPage(mod.id);
		}
	}
}

void ModDatabaseView::doubleClickedIndex(const QModelIndex & index) {
	if (!index.parent().isValid()) { // Mod has no parent, only packages have
		selectedModIndex(index);
	} else { // package
		selectedPackageIndex(index);
	}
}

void ModDatabaseView::downloadModFiles(Mod mod, QSharedPointer<QList<QPair<QString, QString>>> fileList, QString fileserver, QString fallbackServer) {
	const QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mod.id));
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		Q_UNUSED(b)
	}
	auto * mfd = new MultiFileDownloader(this);
	for (const auto & p : *fileList) {
		QFileInfo fi(p.first);

		const auto relativePath = QString::number(mod.id) + "/" + p.first;
		
		auto * fd = new FileDownloader(QUrl(fileserver + relativePath), QUrl(fallbackServer + relativePath), dir.absolutePath() + "/" + fi.path(), fi.fileName(), p.second, mfd);
		mfd->addFileDownloader(fd);

		// zip workflow
		const auto suffix = fi.completeSuffix();
		
		if (suffix.compare("zip.z", Qt::CaseInsensitive) != 0) continue;

		// 1. if it is a zip, register new signal. FileDownloader will send signal after extracting the archive reporting the files with hashes it contained
		// 2. reported files need to be added to filelist and archive must be removed
		connect(fd, &FileDownloader::unzippedArchive, [fileList](QString archive, QList<QPair<QString, QString>> files) {
			for (auto it = fileList->begin(); it != fileList->end(); ++it) {
				if (it->first != archive) continue;
				
				fileList->erase(it);
				
				break;
			}

			fileList->append(files);
		});
	}

	{
		int row = 0;
		for (; row < _mods.size(); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		auto * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, Install));
		buttonItem->setText(QApplication::tr("InQueue"));
		buttonItem->setToolTip(QApplication::tr("InQueue"));
		buttonItem->setData(false, Installed);
	}

	connect(mfd, &MultiFileDownloader::downloadProgressPercent, [this, mod](qreal progress) {
		int row = 0;
		for (; row < _mods.size(); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		auto * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, Install));
		buttonItem->setText(QString("%1: %2%").arg(QApplication::tr("Downloading")).arg(static_cast<int>(progress * 100)));
		buttonItem->setToolTip(QApplication::tr("Downloading"));
		buttonItem->setData(false, Installed);
	});

	connect(mfd, &MultiFileDownloader::downloadSucceeded, [this, mod, fileList]() {
		_downloadingList.removeAll(mod.id);
		
		int row = 0;
		for (; row < _mods.size(); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		auto * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, Install));
		buttonItem->setText(QChar(static_cast<int>(FontAwesome::trasho)));
		buttonItem->setToolTip(QApplication::tr("Uninstall"));
		buttonItem->setData(true, Installed);
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO mods (ModID, GothicVersion, MajorVersion, MinorVersion, PatchVersion, SpineVersion) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(static_cast<int>(mod.gothic)) + ", " + std::to_string(_mods[row].majorVersion) + ", " + std::to_string(_mods[row].minorVersion) + ", " + std::to_string(_mods[row].patchVersion) + ", " + std::to_string(_mods[row].spineVersion) + ");", err);
		for (const auto & p : *fileList) {
			QString fileName = p.first;
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mod.id) + ", '" + fileName.toStdString() + "', '" + q2s(p.second) + "');", err);
		}
		if (mod.type == ModType::PATCH || mod.type == ModType::TOOL) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO patches (ModID, Name) VALUES (" + std::to_string(mod.id) + ", '" + mod.name + "');", err);
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		const int currentDate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO installDates (ModID, InstallDate) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(currentDate) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO languages (ProjectID, Language) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(mod.language) + ");", err);
		
		// enable systempack by default
		if (mod.type != ModType::PATCH && mod.type != ModType::TOOL) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(mod.gothic == GameType::Gothic ? 57 : 40) + ", 1);", err);
		}

		QJsonObject json;
		json["ID"] = mod.id;

		Https::postAsync(DATABASESERVER_PORT, "downloadSucceeded", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});

		if (!_installSilently.contains(mod.id)) {
			OverlayMessageHandler::getInstance()->showMessage(IconCache::getInstance()->getOrLoadIconAsImage(":/svg/download.svg"), QApplication::tr("InstallationSuccessfulText").arg(s2q(mod.name)));
		}
		
		emit finishedInstallation(mod.id, -1, true);

		if (mod.type == ModType::GMP) {
			const bool gmpInstalled = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT  * FROM mods WHERE ModID = 228 LIMIT 1;", err) > 0;

			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(mod.id) + ", 228, 1);", err);

			if (!gmpInstalled) {
				emit triggerInstallMod(228);
			}
		}
	});
	
	connect(mfd, &MultiFileDownloader::downloadFailed, [this, mod, fileList, fileserver, fallbackServer](DownloadError error) {
		static bool entered = false;

		if (entered) return;

		entered = true;
		
		bool paused = false;
		if (error == DownloadError::CanceledError) {
			QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("PauseDownload"), QApplication::tr("PauseDownloadDescription"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
			msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("PauseDownload"));
			msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
			paused = QMessageBox::StandardButton::Ok == msg.exec();
		}
		if (!paused) {
			QDir dir2(Config::DOWNLOADDIR + "/mods/" + QString::number(mod.id));
			
			QString errorText = QApplication::tr("InstallationUnsuccessfulText").arg(s2q(mod.name));
			if (error == DownloadError::DiskSpaceError) {
				errorText += "\n\n" + QApplication::tr("NotEnoughDiskSpace");
			} else if (error == DownloadError::NetworkError) {
				errorText += "\n\n" + QApplication::tr("NetworkError");
			} else if (error == DownloadError::HashError) {
				errorText += "\n\n" + QApplication::tr("HashError");
			}
			
			if (error == DownloadError::NetworkError) {
				QMessageBox msg(QMessageBox::Icon::Warning, QApplication::tr("InstallationUnsuccessful"), errorText, QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
				msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Retry"));
				msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
				if (QMessageBox::StandardButton::Ok == msg.exec()) {
					downloadModFiles(mod, fileList, fileserver, fallbackServer);
				} else {
					dir2.removeRecursively();
					emit finishedInstallation(mod.id, -1, false);
				}
			} else {
				QMessageBox msg(QMessageBox::Icon::Warning, QApplication::tr("InstallationUnsuccessful"), errorText, QMessageBox::StandardButton::Ok);
				msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				msg.exec();
				dir2.removeRecursively();
				emit finishedInstallation(mod.id, -1, false);
			}
		}

		entered = false;
	});

	mfd->setSize(mod.downloadSize);
	DownloadQueueWidget::getInstance()->addDownload(s2q(mod.name), mfd);
}

void ModDatabaseView::sortByColumn(int column) {
	_sortModel->sort(column, _sortModel->sortColumn() == column ? (_sortModel->sortOrder() == Qt::SortOrder::AscendingOrder ? Qt::SortOrder::DescendingOrder : Qt::SortOrder::AscendingOrder) : Qt::SortOrder::AscendingOrder);
}

void ModDatabaseView::changedFilterExpression(const QString & expression) {
	_sortModel->setFilterRegExp(expression);
}

void ModDatabaseView::updatePackageList(QList<Package> packages) {
	_packages.clear();
	_packageIDIconMapping.clear();
	Database::DBError err;
	std::vector<InstalledPackage> ips = Database::queryAll<InstalledPackage, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM packages;", err);
	QSet<int32_t> installedPackages;
	for (const InstalledPackage & im : ips) {
		installedPackages.insert(im.packageID);
	}

	const QFontMetrics fm(_treeView->font());
	
	for (const Package & package : packages) {
		if (_parentMods.find(package.modID) == _parentMods.end()) { // hidden parent or bug, don't crash in this case
			continue;
		}
		const QString packageName = s2q(package.name);
		QStandardItem * nameItem = new TextItem(fm.elidedText(packageName, Qt::ElideRight, 300));
		nameItem->setData(s2q(_mods[_parentMods[package.modID].row()].name), DatabaseRole::FilterRole);
		nameItem->setData(package.packageID, DatabaseRole::PackageIDRole);
		nameItem->setEditable(false);
		{
			QFont f = nameItem->font();
			f.setUnderline(true);
			nameItem->setFont(f);
		}
		auto * sizeItem = new SizeItem(package.downloadSize);
		TextItem * buttonItem = nullptr;
		if (_downloadingPackageList.contains(package.packageID)) {
			buttonItem = new TextItem(QApplication::tr("InQueue"));
			buttonItem->setToolTip(QApplication::tr("InQueue"));
			buttonItem->setData(false, Installed);
		} else if (installedPackages.find(package.packageID) == installedPackages.end()) {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
			buttonItem->setData(false, Installed);
		} else {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::trasho)));
			buttonItem->setToolTip(QApplication::tr("Uninstall"));
			buttonItem->setData(true, Installed);
		}
		_packageIDIconMapping.insert(package.packageID, buttonItem);
		QFont f = buttonItem->font();
		f.setFamily("FontAwesome");
		f.setPointSize(13);
		buttonItem->setFont(f);
		buttonItem->setEditable(false);
		QStandardItem * par = _sourceModel->item(_parentMods[package.modID].row());
		for (int i = 0; i < std::numeric_limits<int>::max(); i++) {
			if (par->child(i) == nullptr) {
				par->setChild(i, DatabaseColumn::Name, nameItem);
				par->setChild(i, DatabaseColumn::Size, sizeItem);
				par->setChild(i, DatabaseColumn::Install, buttonItem);
				nameItem->setTextAlignment(Qt::AlignCenter);
				sizeItem->setTextAlignment(Qt::AlignCenter);
				buttonItem->setTextAlignment(Qt::AlignCenter);
				
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::ModID, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Author, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::AvgDuration, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::DevDuration, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Game, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Release, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Update, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Type, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Version, itm);
				}
				{
					auto * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Languages, itm);
				}
				break;
			}
		}
		_packages[package.modID].push_back(package);
	}
	delete _waitSpinner;
	_waitSpinner = nullptr;
}

void ModDatabaseView::downloadPackageFiles(Mod mod, Package package, QSharedPointer<QList<QPair<QString, QString>>> fileList, QString fileserver, QString fallbackServer) {
	const QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mod.id));
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		Q_UNUSED(b)
	}
	auto * mfd = new MultiFileDownloader(this);
	for (const auto & p : *fileList) {
		QFileInfo fi(p.first);

		const auto relativePath = QString::number(mod.id) + "/" + p.first;
		
		auto * fd = new FileDownloader(QUrl(fileserver + relativePath), QUrl(fallbackServer + relativePath), dir.absolutePath() + "/" + fi.path(), fi.fileName(), p.second, mfd);
		mfd->addFileDownloader(fd);

		// zip workflow
		const auto suffix = fi.completeSuffix();

		if (suffix.compare("zip.z", Qt::CaseInsensitive) != 0) continue;

		// 1. if it is a zip, register new signal. FileDownloader will send signal after extracting the archive reporting the files with hashes it contained
		// 2. reported files need to be added to filelist and archive must be removed
		connect(fd, &FileDownloader::unzippedArchive, [fileList](QString archive, QList<QPair<QString, QString>> files) {
			for (auto it = fileList->begin(); it != fileList->end(); ++it) {
				if (it->first != archive) continue;

				fileList->erase(it);

				break;
			}

			fileList->append(files);
		});
	}

	{
		TextItem * buttonItem = _packageIDIconMapping[package.packageID];
		buttonItem->setText(QApplication::tr("InQueue"));
		buttonItem->setToolTip(QApplication::tr("InQueue"));
	}

	connect(mfd, &MultiFileDownloader::downloadProgressPercent, [this, package](qreal progress) {
		TextItem * buttonItem = _packageIDIconMapping[package.packageID];
		buttonItem->setText(QString("%1: %2%").arg(QApplication::tr("Downloading")).arg(static_cast<int>(progress * 100)));
		buttonItem->setToolTip(QApplication::tr("Downloading"));
	});

	connect(mfd, &MultiFileDownloader::downloadSucceeded, [this, package, fileList, mod]() {
		_downloadingPackageList.removeAll(package.packageID);
		
		TextItem * buttonItem = _packageIDIconMapping[package.packageID];
		buttonItem->setText(QChar(static_cast<int>(FontAwesome::trasho)));
		buttonItem->setToolTip(QApplication::tr("Uninstall"));
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		for (const auto & p : *fileList) {
			QString fileName = p.first;
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mod.id) + ", '" + fileName.toStdString() + "', '" + q2s(p.second) + "');", err);
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO packages (ModID, PackageID, File) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(package.packageID) + ", '" + fileName.toStdString() + "');", err);
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		
		// notify server download was successful
		QJsonObject json;
		json["ID"] = package.packageID;

		Https::postAsync(DATABASESERVER_PORT, "packageDownloadSucceeded", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});

		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("InstallationSuccessful"), QApplication::tr("InstallationSuccessfulText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
		emit finishedInstallation(mod.id, package.packageID, true);
	});

	connect(mfd, &MultiFileDownloader::downloadFailed, [this, package, mod](DownloadError error) {
		QString errorText = QApplication::tr("InstallationUnsuccessfulText").arg(s2q(package.name));
		if (error == DownloadError::DiskSpaceError) {
			errorText += "\n\n" + QApplication::tr("NotEnoughDiskSpace");
		}
		QMessageBox msg(QMessageBox::Icon::Warning, QApplication::tr("InstallationUnsuccessful"), errorText, QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
		emit finishedInstallation(mod.id, package.packageID, false);
	});
	
	DownloadQueueWidget::getInstance()->addDownload(s2q(package.name), mfd);
}

void ModDatabaseView::installMod(int modID) {
	const QModelIndex idx = _parentMods.value(modID);
	selectedModIndex(idx);
}

void ModDatabaseView::installPackage(int modID, int packageID) {
	const QModelIndex idx = _parentMods.value(modID);
	int row = 0;
	QModelIndex packageIdx;
	while ((packageIdx = idx.child(row++, DatabaseColumn::Name)).isValid()) {
		if (packageIdx.data(DatabaseRole::PackageIDRole).toInt() == packageID) {
			break;
		}
	}
	if (packageIdx.isValid()) {
		selectedPackageIndex(packageIdx);
	}
}

void ModDatabaseView::resizeEvent(QResizeEvent *) {
	int columnCount = _sourceModel->columnCount();
	const int width = _treeView->width();
	if (width < 1000) {
		_treeView->hideColumn(DatabaseColumn::Author);
		_treeView->hideColumn(DatabaseColumn::AvgDuration);
		_treeView->hideColumn(DatabaseColumn::Game);
		_treeView->hideColumn(DatabaseColumn::Version);
		columnCount -= 3;
	} else if (width < 1300) {
		_treeView->hideColumn(DatabaseColumn::Author);
		_treeView->hideColumn(DatabaseColumn::AvgDuration);
		_treeView->showColumn(DatabaseColumn::Game);
		_treeView->hideColumn(DatabaseColumn::Version);
		columnCount -= 2;
	} else if (width < 1600) {
		_treeView->showColumn(DatabaseColumn::Author);
		_treeView->hideColumn(DatabaseColumn::AvgDuration);
		_treeView->showColumn(DatabaseColumn::Game);
		_treeView->hideColumn(DatabaseColumn::Version);
		columnCount -= 1;
	} else if (width > 1600) {
		_treeView->showColumn(DatabaseColumn::Version);
		_treeView->showColumn(DatabaseColumn::Author);
		_treeView->showColumn(DatabaseColumn::AvgDuration);
		_treeView->showColumn(DatabaseColumn::Game);
	}
	for (int i = 0; i < _sourceModel->columnCount() - 1; i++) {
		_treeView->setColumnWidth(i, _treeView->width() / columnCount);
	}
}

qint64 ModDatabaseView::getDownloadSize(Mod mod) const {
	qint64 size = 0;
	for (int i = 0; i < _mods.size(); i++) {
		if (_mods[i].id == mod.id) {
			size = _sourceModel->item(i, DatabaseColumn::Size)->data(Qt::UserRole).toLongLong();
			break;
		}
	}
	return size;
}

void ModDatabaseView::selectedModIndex(const QModelIndex & index) {
	if (!index.isValid()) return;

	Mod mod;
	if (index.model() == _sortModel) {
		mod = _mods[_sortModel->mapToSource(index).row()];
	} else {
		mod = _mods[index.row()];
	}
	if ((mod.gothic == GameType::Gothic && !_gothicValid) || (mod.gothic == GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == GameType::Gothic1And2 && !_gothic2Valid && !_gothicValid)) {
		emit finishedInstallation(mod.id, -1, false);
		return;
	}
	
	if (_downloadingList.contains(mod.id)) return;

	Database::DBError err;
	std::vector<InstalledMod> ims = Database::queryAll<InstalledMod, int, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods;", err);
	QSet<int32_t> installedMods;
	for (InstalledMod im : ims) {
		installedMods.insert(im.id);
	}
	if (installedMods.find(mod.id) == installedMods.end()) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyWantToInstall"), QApplication::tr("ReallyWantToInstallText").arg(s2q(mod.name)), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (_installSilently.contains(mod.id) || QMessageBox::StandardButton::Ok == msg.exec()) {
			_downloadingList.append(mod.id);

			QJsonObject json;
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Language"] = LanguageConverter::convert(mod.language);
			json["ProjectID"] = mod.id;

			Https::postAsync(DATABASESERVER_PORT, "requestProjectFiles", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, mod](const QJsonObject & data, int statusCode) {
				if (statusCode != 200) return;

				if (!data.contains("Files")) return;

				QSharedPointer<QList<QPair<QString, QString>>> fileList(new QList<QPair<QString, QString>>());

				for (const auto jsonRef : data["Files"].toArray()) {
					const auto jsonFile = jsonRef.toObject();

					const auto path = jsonFile["File"].toString();
					const auto hash = jsonFile["Hash"].toString();
					
					fileList->append(qMakePair(path, hash));
				}

				const auto fileserver = data["Fileserver"].toString();
				auto fallbackFileserver = data["FallbackFileserver"].toString();

				if (fallbackFileserver.isEmpty()) {
					fallbackFileserver = fileserver;
				}

				emit receivedModFilesList(mod, fileList, fileserver, fallbackFileserver);
			});
		}
	} else {
		const bool uninstalled = Uninstaller::uninstall(mod.id, s2q(mod.name), mod.gothic == GameType::Gothic ? _gothicDirectory : _gothic2Directory);
		if (uninstalled) {
			int row = 0;
			for (; row < _mods.size(); row++) {
				if (_mods[row].id == mod.id) {
					break;
				}
			}
			auto * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
			buttonItem->setText(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
			buttonItem->setData(false, Installed);
		}
	}
}

void ModDatabaseView::selectedPackageIndex(const QModelIndex & index) {
	if (!index.isValid()) return;

	Mod mod = _mods[index.model() == _sortModel ? _sortModel->mapToSource(index.parent()).row() : index.parent().row()];
	if ((mod.gothic == GameType::Gothic && !_gothicValid) || (mod.gothic == GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == GameType::Gothic1And2 && !_gothicValid && !_gothic2Valid)) {
		emit finishedInstallation(mod.id, -1, false);
		return;
	}
	Database::DBError err;
	std::vector<InstalledMod> ims = Database::queryAll<InstalledMod, int, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods;", err);
	QSet<int32_t> installedMods;
	for (const InstalledMod & im : ims) {
		installedMods.insert(im.id);
	}
	std::vector<InstalledPackage> ips = Database::queryAll<InstalledPackage, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM packages;", err);
	QSet<int32_t> installedPackages;
	for (const InstalledPackage & ip : ips) {
		installedPackages.insert(ip.packageID);
	}
	if (installedMods.find(mod.id) == installedMods.end()) {
		emit finishedInstallation(mod.id, -1, false);
		return;
	}
	Package package = _packages[mod.id][(index.model() == _sortModel) ? _sortModel->mapToSource(index).row() : index.row()];

	if (_downloadingPackageList.contains(package.packageID)) return;
	
	if (installedPackages.find(package.packageID) == installedPackages.end()) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyWantToInstall"), QApplication::tr("ReallyWantToInstallText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == msg.exec()) {
			_downloadingPackageList.append(package.packageID);

			QJsonObject json;
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;
			json["Language"] = s2q(package.language);
			json["PackageID"] = package.packageID;

			Https::postAsync(DATABASESERVER_PORT, "requestPackageFiles", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, mod, package](const QJsonObject & data, int statusCode) {
				if (statusCode != 200) return;

				if (!data.contains("Files")) return;

				QSharedPointer<QList<QPair<QString, QString>>> fileList(new QList<QPair<QString, QString>>());

				for (const auto jsonRef : data["Files"].toArray()) {
					const auto jsonFile = jsonRef.toObject();

					const auto path = jsonFile["File"].toString();
					const auto hash = jsonFile["Hash"].toString();

					fileList->append(qMakePair(path, hash));
				}

				const auto fileserver = data["Fileserver"].toString();
				auto fallbackFileserver = data["FallbackFileserver"].toString();

				if (fallbackFileserver.isEmpty()) {
					fallbackFileserver = fileserver;
				}

				emit receivedPackageFilesList(mod, package, fileList, fileserver, fallbackFileserver);
			});
		}
	} else {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyWantToUninstall"), QApplication::tr("ReallyWantToUninstallText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == msg.exec()) {
			QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mod.id));
			const auto files = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File FROM packages WHERE PackageID = " + std::to_string(package.packageID) + ";", err);
			for (const std::string & s : files) {
				QFile(dir.absolutePath() + "/" + QString::fromStdString(s)).remove();
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM modfiles WHERE ModID = " + std::to_string(mod.id) + " AND File = '" + s + "';", err);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM packages WHERE PackageID = " + std::to_string(package.packageID) + ";", err);

			TextItem * buttonItem = _packageIDIconMapping[package.packageID];
			buttonItem->setText(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
			QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("UninstallationSuccessful"), QApplication::tr("UninstallationSuccessfulText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok);
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.exec();
		}
	}
}

void ModDatabaseView::updateDatabaseEntries() {
	// Free Aiming: 223 and 227. 227 is now obsolete
	if (isInstalled(223) && isInstalled(227)) {
		Uninstaller::uninstall(227);
	}
	// Workaround Helper: 225 and 234. 234 is now obsolete
	if (isInstalled(225) && isInstalled(234)) {
		Uninstaller::uninstall(234);
	}
	
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM modfiles WHERE File = 'D3D11.zip';", err);

	const auto patches = Database::queryAll<std::vector<int>, int, int>(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "SELECT ModID, PatchID FROM patchConfigs;", err);

	QMap<QPair<int, int>, int> patchCount;
	
	for (const auto & patch : patches) {
		const int modID = patch[0];
		const int patchID = patch[1];

		patchCount[qMakePair(modID, patchID)]++;
	}

	for (auto it = patchCount.begin(); it != patchCount.end(); ++it) {
		for (int i = 1; i < it.value(); i++) {
			const int modID = it.key().first;
			const int patchID = it.key().second;
			
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(modID) + " AND PatchID = " + std::to_string(patchID) + ";", err);
		}
	}

	const auto projects = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods;", err);

	for (const auto & project : projects) {
		const auto count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ProjectID FROM languages WHERE ProjectID = " + std::to_string(project) + " LIMIT 1;", err);

		if (count == 1) continue;
		
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO languages (ProjectID, Language) VALUES (" + std::to_string(project) + ", " + std::to_string(LanguageConverter::convert(Config::Language)) + ");", err);
	}
}

bool ModDatabaseView::isInstalled(int modID) {
	Database::DBError err;
	const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(modID) + ";", err);

	return count == 1;
}
