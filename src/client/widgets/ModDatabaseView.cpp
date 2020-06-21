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
#include "InstallMode.h"
#include "SpineConfig.h"
#include "Uninstaller.h"

#include "common/MessageStructs.h"

#include "gui/WaitSpinner.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/DownloadQueue.h"
#include "utils/FileDownloader.h"
#include "utils/MultiFileDownloader.h"

#include "widgets/UpdateLanguage.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QDate>
#include <QDebug>
#include <QDirIterator>
#include <QGroupBox>
#include <QHeaderView>
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

using namespace spine;
using namespace spine::client;
using namespace spine::gui;
using namespace spine::utils;
using namespace spine::widgets;

namespace spine {
namespace widgets {
	
	struct InstalledMod {
		InstalledMod(const int i1, const int i2, const int i3, const int i4, const int i5) : id(i1), gothicVersion(common::GameType(i2)), majorVersion(i3), minorVersion(i4), patchVersion(i5) {
		}

		int32_t id;
		common::GameType gothicVersion;
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
		explicit SizeItem(const uint64_t size) : QStandardItem() {
			setSize(size);
		}

		void setSize(const uint64_t size) {
			QString sizeString;
			if (size == UINT64_MAX) {
				sizeString = "-";
			} else {
				QString unit = "B";
				double dSize = static_cast<double>(size);
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
			setData(quint64(size), Qt::UserRole);
		}
	};

	class VersionItem : public QStandardItem {
	public:
		VersionItem(const uint8_t majorVersion, const uint8_t minorVersion, const uint8_t patchVersion) : QStandardItem(QString::number(int(majorVersion)) + "." + QString::number(int(minorVersion)) + "." + QString::number(int(patchVersion))) {
			QStandardItem::setData(quint64(majorVersion * 256 * 256 + minorVersion * 256 + patchVersion), Qt::UserRole);
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

ModDatabaseView::ModDatabaseView(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _mainWindow(mainWindow), _treeView(nullptr), _sourceModel(nullptr), _sortModel(nullptr), _gothicValid(false), _gothic2Valid(false), _waitSpinner(nullptr), _allowRenderer(false), _cached(false) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	_treeView = new QTreeView(this);
	_sourceModel = new QStandardItemModel(this);
	_sortModel = new DatabaseFilterModel(this);
	_sortModel->setSourceModel(_sourceModel);
	_sortModel->setSortRole(Qt::UserRole);
	_sortModel->setFilterRole(DatabaseRole::FilterRole);
	_sortModel->setFilterKeyColumn(DatabaseColumn::Name);
	_sortModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
	_treeView->setModel(_sortModel);
	_treeView->header()->setSortIndicatorShown(true);
	_treeView->header()->setStretchLastSection(true);
	_treeView->header()->setDefaultAlignment(Qt::AlignHCenter);
	_treeView->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("ID") << QApplication::tr("Name") << QApplication::tr("Author") << QApplication::tr("Type") << QApplication::tr("Game") << QApplication::tr("DevTime") << QApplication::tr("AvgTime") << QApplication::tr("ReleaseDate") << QApplication::tr("UpdateDate") << QApplication::tr("Version") << QApplication::tr("DownloadSize") << QString());
	_treeView->setAlternatingRowColors(true);
	_treeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	_treeView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	_treeView->setMinimumWidth(800);
	_treeView->setSortingEnabled(true);
	_treeView->sortByColumn(DatabaseColumn::Release);

	connect(_treeView->header(), &QHeaderView::sectionClicked, this, &ModDatabaseView::sortByColumn);

	connect(generalSettingsWidget, &GeneralSettingsWidget::languageChanged, this, &ModDatabaseView::changeLanguage);

	{
		QWidget * filterWidget = new QWidget(this);
		QHBoxLayout * hl = new QHBoxLayout();
		QLineEdit * le = new QLineEdit(filterWidget);
		le->setPlaceholderText(QApplication::tr("SearchPlaceholder"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(le, "SearchPlaceholder");

		connect(le, &QLineEdit::textChanged, this, &ModDatabaseView::changedFilterExpression);

		hl->addWidget(le);

		{
			QGroupBox * gb = new QGroupBox(QApplication::tr("Type"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "Type");

			QHBoxLayout * gbHl = new QHBoxLayout();

			{
				QGroupBox * gbGame = new QGroupBox(QApplication::tr("Game"), gb);
				UPDATELANGUAGESETTITLE(gbGame, "Game");
				
				QVBoxLayout * vbl = new QVBoxLayout();

				QCheckBox * cbFullVersion = new QCheckBox(QApplication::tr("FullVersion"), filterWidget);
				cbFullVersion->setChecked(_sortModel->isFullVersionsActive());
				UPDATELANGUAGESETTEXT(cbFullVersion, "FullVersion");
				connect(cbFullVersion, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::fullVersionsChanged);

				QCheckBox * cbDemo = new QCheckBox(QApplication::tr("Demo"), filterWidget);
				cbDemo->setChecked(_sortModel->isDemosActive());
				UPDATELANGUAGESETTEXT(cbDemo, "Demo");
				connect(cbDemo, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::demosChanged);

				QCheckBox * cbPlayTesting = new QCheckBox(QApplication::tr("PlayTesting"), filterWidget);
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
				QGroupBox * gbMod = new QGroupBox(QApplication::tr("Modification"), gb);
				UPDATELANGUAGESETTITLE(gbMod, "Modification");
				
				QVBoxLayout * vbl = new QVBoxLayout();

				QCheckBox * cbTotalConversion = new QCheckBox(QApplication::tr("TotalConversion"), filterWidget);
				cbTotalConversion->setChecked(_sortModel->isTotalConversionActive());
				UPDATELANGUAGESETTEXT(cbTotalConversion, "TotalConversion");
				connect(cbTotalConversion, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::totalConversionChanged);

				QCheckBox * cbEnhancement = new QCheckBox(QApplication::tr("Enhancement"), filterWidget);
				cbEnhancement->setChecked(_sortModel->isEnhancementActive());
				UPDATELANGUAGESETTEXT(cbEnhancement, "Enhancement");
				connect(cbEnhancement, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::enhancementChanged);

				QCheckBox * cbPatch = new QCheckBox(QApplication::tr("Patch"), filterWidget);
				cbPatch->setChecked(_sortModel->isPathActive());
				UPDATELANGUAGESETTEXT(cbPatch, "Patch");
				connect(cbPatch, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::patchChanged);

				QCheckBox * cbTool = new QCheckBox(QApplication::tr("Tool"), filterWidget);
				cbTool->setChecked(_sortModel->isToolActive());
				UPDATELANGUAGESETTEXT(cbTool, "Tool");
				connect(cbTool, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::toolChanged);

				QCheckBox * cbOriginal = new QCheckBox(QApplication::tr("Original"), filterWidget);
				cbOriginal->setChecked(_sortModel->isOriginalActive());
				UPDATELANGUAGESETTEXT(cbOriginal, "Original");
				connect(cbOriginal, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::originalChanged);

				QCheckBox * cbGMP = new QCheckBox(QApplication::tr("GothicMultiplayer"), filterWidget);
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
			QGroupBox * gb = new QGroupBox(QApplication::tr("Game"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "Game");

			QVBoxLayout * vbl = new QVBoxLayout();

			QCheckBox * cb1 = new QCheckBox(QApplication::tr("Gothic"), filterWidget);
			cb1->setChecked(_sortModel->isGothicActive());
			UPDATELANGUAGESETTEXT(cb1, "Gothic");
			connect(cb1, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gothicChanged);

			QCheckBox * cb2 = new QCheckBox(QApplication::tr("Gothic2"), filterWidget);
			cb2->setChecked(_sortModel->isGothic2Active());
			UPDATELANGUAGESETTEXT(cb2, "Gothic2");
			connect(cb2, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gothic2Changed);

			QCheckBox * cb3 = new QCheckBox(QApplication::tr("GothicAndGothic2"), filterWidget);
			cb3->setChecked(_sortModel->isGothicAndGothic2Active());
			UPDATELANGUAGESETTEXT(cb3, "GothicAndGothic2");
			connect(cb3, &QCheckBox::stateChanged, _sortModel, &DatabaseFilterModel::gothicAndGothic2Changed);

			QCheckBox * cb4 = new QCheckBox(QApplication::tr("Game"), filterWidget);
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
			QGroupBox * gb = new QGroupBox(QApplication::tr("DevTime"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "DevTime");

			QGridLayout * vbl = new QGridLayout();

			QSpinBox * sb1 = new QSpinBox(gb);
			sb1->setMinimum(0);
			sb1->setMaximum(1000);
			sb1->setValue(_sortModel->getMinDuration());
			QSpinBox * sb2 = new QSpinBox(gb);
			sb2->setMinimum(0);
			sb2->setMaximum(1000);
			sb2->setValue(_sortModel->getMaxDuration());

			connect(sb1, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), _sortModel, &DatabaseFilterModel::minDurationChanged);
			connect(sb2, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), _sortModel, &DatabaseFilterModel::maxDurationChanged);

			QLabel * l1 = new QLabel(QApplication::tr("MinDurationHours"), gb);
			UPDATELANGUAGESETTEXT(l1, "MinDurationHours");
			QLabel * l2 = new QLabel(QApplication::tr("MaxDurationHours"), gb);
			UPDATELANGUAGESETTEXT(l2, "MaxDurationHours");

			vbl->addWidget(l1, 0, 0);
			vbl->addWidget(sb1, 0, 1);
			vbl->addWidget(l2, 1, 0);
			vbl->addWidget(sb2, 1, 1);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}

		filterWidget->setLayout(hl);

		l->addWidget(filterWidget);
	}

	l->addWidget(_treeView, 2);

	setLayout(l);

	qRegisterMetaType<std::vector<common::Mod>>("std::vector<common::Mod>");
	qRegisterMetaType<std::vector<std::pair<int32_t, uint64_t>>>("std::vector<std::pair<int32_t, uint64_t>>");
	qRegisterMetaType<common::Mod>("common::Mod");
	qRegisterMetaType<common::UpdatePackageListMessage::Package>("common::UpdatePackageListMessage::Package");
	qRegisterMetaType<std::vector<common::UpdatePackageListMessage::Package>>("std::vector<common::UpdatePackageListMessage::Package>");
	qRegisterMetaType<std::vector<std::pair<std::string, std::string>>>("std::vector<std::pair<std::string, std::string>>");
	qRegisterMetaType<QSharedPointer<QList<QPair<QString, QString>>>>("QSharedPointer<QList<QPair<QString, QString>>>");

	connect(this, &ModDatabaseView::receivedModList, this, static_cast<void(ModDatabaseView::*)(std::vector<common::Mod>)>(&ModDatabaseView::updateModList));
	connect(this, &ModDatabaseView::receivedModFilesList, this, &ModDatabaseView::downloadModFiles);
	connect(_treeView, &QTreeView::clicked, this, &ModDatabaseView::selectedIndex);
	connect(_treeView, &QTreeView::doubleClicked, this, &ModDatabaseView::doubleClickedIndex);
	connect(this, &ModDatabaseView::receivedPackageList, this, &ModDatabaseView::updatePackageList);
	connect(this, &ModDatabaseView::receivedPackageFilesList, this, &ModDatabaseView::downloadPackageFiles);
	connect(this, &ModDatabaseView::triggerInstallMod, this, &ModDatabaseView::installMod);
	connect(this, &ModDatabaseView::triggerInstallPackage, this, &ModDatabaseView::installPackage);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS mods(ModID INT NOT NULL, GothicVersion INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS modfiles(ModID INT NOT NULL, File TEXT NOT NULL, Hash TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS patches(ModID INT NOT NULL, Name TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS packages(ModID INT NOT NULL, PackageID INT NOT NULL, File TEXT NOT NULL);", err);

	_sortModel->setRendererAllowed(true);

	removeInvalidDatabaseEntries();

	updateModList(-1, -1, InstallMode::None);
}

void ModDatabaseView::changeLanguage(QString language) {
	_cached = false;
	
	updateModList(-1, -1, InstallMode::None);
}

void ModDatabaseView::updateModList(int modID, int packageID, InstallMode mode) {
	if (!Config::OnlineMode) return;
	
	if (mode == InstallMode::Silent && modID != -1) {
		_installSilently.insert(modID);
	}
	if (!_cached) {
		_waitSpinner = new WaitSpinner(QApplication::tr("LoadingDatabase"), this);
		_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("ID") << QApplication::tr("Name") << QApplication::tr("Author") << QApplication::tr("Type") << QApplication::tr("Game") << QApplication::tr("DevTime") << QApplication::tr("AvgTime") << QApplication::tr("ReleaseDate") << QApplication::tr("UpdateDate") << QApplication::tr("Version") << QApplication::tr("DownloadSize") << QString());
	}
	QtConcurrent::run([this, modID, packageID]() {
		if (!_cached) {
			common::RequestAllModsMessage ramm;
			ramm.language = Config::Language.toStdString();
			ramm.username = Config::Username.toStdString();
			ramm.password = Config::Password.toStdString();
			std::string serialized = ramm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == err) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::UpdateAllModsMessage * uamm = dynamic_cast<common::UpdateAllModsMessage *>(m);
							if (uamm) {
								_cached = true;
								emit receivedModList(uamm->mods);
							}
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << static_cast<int>(err);
				}
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::UpdatePackageListMessage * uplm = dynamic_cast<common::UpdatePackageListMessage *>(m);
							if (uplm) {
								emit receivedPackageList(uplm->packages);
							}
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << static_cast<int>(err);
				}
			} else {
				qDebug() << "Error occurred: " << static_cast<int>(err);
			}
		}
		if (modID > 0 && packageID > 0) {
			emit triggerInstallPackage(modID, packageID);
		} else if (modID > 0) {
			emit triggerInstallMod(modID);
		}
	});
}

void ModDatabaseView::gothicValidationChanged(bool valid) {
	_gothicValid = valid;
}

void ModDatabaseView::gothic2ValidationChanged(bool valid) {
	_gothic2Valid = valid;
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

void ModDatabaseView::updateModList(std::vector<common::Mod> mods) {
	_sourceModel->removeRows(0, _sourceModel->rowCount());
	_parentMods.clear();
	int row = 0;
	Database::DBError err;
	std::vector<InstalledMod> ims = Database::queryAll<InstalledMod, int, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods;", err);
	QSet<int32_t> installedMods;
	for (InstalledMod im : ims) {
		installedMods.insert(im.id);
	}
	for (const common::Mod & mod : mods) {
		const QString modname = s2q(mod.name);
		QStandardItem * nameItem = new TextItem(modname);
		nameItem->setData(modname, DatabaseRole::FilterRole);
		nameItem->setEditable(false);
		{
			QFont f = nameItem->font();
			f.setUnderline(true);
			nameItem->setFont(f);
		}
		const QString teamname = s2q(mod.teamName);
		QStandardItem * teamItem = new TextItem(teamname);
		teamItem->setEditable(false);
		QString typeName;
		switch (mod.type) {
		case common::ModType::TOTALCONVERSION: {
			typeName = QApplication::tr("TotalConversion");
			break;
		}
		case common::ModType::ENHANCEMENT: {
			typeName = QApplication::tr("Enhancement");
			break;
		}
		case common::ModType::PATCH: {
			typeName = QApplication::tr("Patch");
			break;
		}
		case common::ModType::TOOL: {
			typeName = QApplication::tr("Tool");
			break;
		}
		case common::ModType::ORIGINAL: {
			typeName = QApplication::tr("Original");
			break;
		}
		case common::ModType::GMP: {
			typeName = QApplication::tr("GothicMultiplayer");
			break;
		}
		case common::ModType::FULLVERSION: {
			typeName = QApplication::tr("FullVersion");
			break;
		}
		case common::ModType::DEMO: {
			typeName = QApplication::tr("Demo");
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
		case common::GameType::Gothic: {
			gameName = QApplication::tr("Gothic");
			break;
		}
		case common::GameType::Gothic2: {
			gameName = QApplication::tr("Gothic2");
			break;
		}
		case common::GameType::GothicInGothic2: {
			gameName = QApplication::tr("GothicInGothic2");
			break;
		}
		case common::GameType::Gothic1And2: {
			gameName = QApplication::tr("GothicAndGothic2_2");
			break;
		}
		case common::GameType::Game: {
			gameName = QApplication::tr("Game");
			break;
		}
		default: {
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
		DateItem * releaseDateItem = new DateItem(date);
		releaseDateItem->setEditable(false);

		date = QDate(2000, 1, 1);
		date = date.addDays(std::max(mod.releaseDate, mod.updateDate));
		DateItem * updateDateItem = new DateItem(date);
		updateDateItem->setEditable(false);
		
		VersionItem * versionItem = new VersionItem(mod.majorVersion, mod.minorVersion, mod.patchVersion);
		versionItem->setEditable(false);
		SizeItem * sizeItem = new SizeItem(mod.downloadSize);
		sizeItem->setEditable(false);
		QStandardItem * buttonItem = nullptr;
		if (_downloadingList.contains(mod.id)) {
			buttonItem = new TextItem(QApplication::tr("InQueue"));
			buttonItem->setToolTip(QApplication::tr("InQueue"));
		} else if (installedMods.find(mod.id) == installedMods.end()) {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
		} else {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::trasho)));
			buttonItem->setToolTip(QApplication::tr("Uninstall"));
		}
		QFont f = buttonItem->font();
		f.setPointSize(13);
		f.setFamily("FontAwesome");
		buttonItem->setFont(f);
		buttonItem->setEditable(false);

		QStandardItem * idItem = new IntItem(mod.id);
		idItem->setEditable(false);

		_sourceModel->appendRow(QList<QStandardItem *>() << idItem << nameItem << teamItem << typeItem << gameItem << devTimeItem << avgTimeItem << releaseDateItem << updateDateItem << versionItem << sizeItem << buttonItem);
		for (int i = 0; i < _sourceModel->columnCount(); i++) {
			_sourceModel->setData(_sourceModel->index(row, i), Qt::AlignCenter, Qt::TextAlignmentRole);
		}
		if ((mod.gothic == common::GameType::Gothic && !_gothicValid) || (mod.gothic == common::GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == common::GameType::GothicInGothic2 && (!_gothicValid || !_gothic2Valid)) || (mod.gothic == common::GameType::Gothic1And2 && !_gothicValid && !_gothic2Valid) || Config::DOWNLOADDIR.isEmpty() || !QDir(Config::DOWNLOADDIR).exists()) {
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
			sizeItem->setEnabled(false);
			buttonItem->setEnabled(false);
		}
		_parentMods.insert(mod.id, _sourceModel->index(row, 0));
		row++;
	}
	_mods = mods;
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
			const common::Mod mod = _mods[_sortModel->mapToSource(index).row()];
			emit loadPage(mod.id);
		} else { // package
			const common::Mod mod = _mods[_sortModel->mapToSource(index.parent()).row()];
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

void ModDatabaseView::downloadModFiles(common::Mod mod, QSharedPointer<QList<QPair<QString, QString>>> fileList, QString fileserver) {
	const QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mod.id));
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		Q_UNUSED(b);
	}
	MultiFileDownloader * mfd = new MultiFileDownloader(this);
	for (const auto & p : *fileList) {
		QFileInfo fi(p.first);
		FileDownloader * fd = new FileDownloader(QUrl(fileserver + QString::number(mod.id) + "/" + p.first), dir.absolutePath() + "/" + fi.path(), fi.fileName(), p.second, mfd);
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
		for (; row < static_cast<int>(_mods.size()); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		TextItem * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
		buttonItem->setText(QApplication::tr("InQueue"));
		buttonItem->setToolTip(QApplication::tr("InQueue"));
	}

	connect(mfd, &MultiFileDownloader::downloadProgressPercent, [this, mod](qreal progress) {
		int row = 0;
		for (; row < static_cast<int>(_mods.size()); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		TextItem * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
		buttonItem->setText(QString("%1: %2%").arg(QApplication::tr("Downloading")).arg(static_cast<int>(progress * 100)));
		buttonItem->setToolTip(QApplication::tr("Downloading"));
	});

	connect(mfd, &MultiFileDownloader::downloadSucceeded, [this, mod, fileList]() {
		_downloadingList.removeAll(mod.id);
		
		int row = 0;
		for (; row < static_cast<int>(_mods.size()); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		TextItem * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
		buttonItem->setText(QChar(static_cast<int>(FontAwesome::trasho)));
		buttonItem->setToolTip(QApplication::tr("Uninstall"));
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO mods (ModID, GothicVersion, MajorVersion, MinorVersion, PatchVersion) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(static_cast<int>(mod.gothic)) + ", " + std::to_string(static_cast<int>(_mods[row].majorVersion)) + ", " + std::to_string(static_cast<int>(_mods[row].minorVersion)) + ", " + std::to_string(static_cast<int>(_mods[row].patchVersion)) + ");", err);
		for (const auto & p : *fileList) {
			QString fileName = p.first;
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mod.id) + ", '" + fileName.toStdString() + "', '" + q2s(p.second) + "');", err);
		}
		if (mod.type == common::ModType::PATCH || mod.type == common::ModType::TOOL) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO patches (ModID, Name) VALUES (" + std::to_string(mod.id) + ", '" + mod.name + "');", err);
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		const int currentDate = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now() - std::chrono::system_clock::time_point()).count();
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO installDates (ModID, InstallDate) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(currentDate) + ";", err);

		// enable systempack by default
		if (mod.type != common::ModType::PATCH && mod.type != common::ModType::TOOL) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(mod.gothic == common::GameType::Gothic ? 57 : 40) + ", 1);", err);
		}

		// notify server download was successful
		QtConcurrent::run([mod]() {
			common::DownloadSucceededMessage dsm;
			dsm.modID = mod.id;
			const std::string serialized = dsm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
				sock.writePacket(serialized);
			}
		});
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("InstallationSuccessful"), QApplication::tr("InstallationSuccessfulText").arg(s2q(mod.name)), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
		emit finishedInstallation(mod.id, -1, true);

		if (mod.type == common::ModType::GMP) {
			const bool gmpInstalled = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT  * FROM mods WHERE ModID = 228 LIMIT 1;", err) > 0;

			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(mod.id) + ", 228, 1);", err);

			if (!gmpInstalled) {
				emit triggerInstallMod(228);
			}
		}
	});
	
	connect(mfd, &MultiFileDownloader::downloadFailed, [this, mod, fileList, fileserver](DownloadError error) {
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
					downloadModFiles(mod, fileList, fileserver);
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
	});

	mfd->setSize(mod.downloadSize);
	DownloadQueue::getInstance()->add(mfd);
}

void ModDatabaseView::sortByColumn(int column) {
	_sortModel->sort(column, ((_sortModel->sortColumn() == column) ? ((_sortModel->sortOrder() == Qt::SortOrder::AscendingOrder) ? Qt::SortOrder::DescendingOrder : Qt::SortOrder::AscendingOrder) : Qt::SortOrder::AscendingOrder));
}

void ModDatabaseView::changedFilterExpression(const QString & expression) {
	_sortModel->setFilterRegExp(expression);
}

void ModDatabaseView::updatePackageList(std::vector<common::UpdatePackageListMessage::Package> packages) {
	_packages.clear();
	_packageIDIconMapping.clear();
	Database::DBError err;
	std::vector<InstalledPackage> ips = Database::queryAll<InstalledPackage, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM packages;", err);
	QSet<int32_t> installedPackages;
	for (const InstalledPackage & im : ips) {
		installedPackages.insert(im.packageID);
	}
	for (const common::UpdatePackageListMessage::Package & package : packages) {
		if (_parentMods.find(package.modID) == _parentMods.end()) { // hidden parent or bug, don't crash in this case
			continue;
		}
		const QString packageName = s2q(package.name);
		QStandardItem * nameItem = new TextItem(packageName);
		nameItem->setData(s2q(_mods[_parentMods[package.modID].row()].name), DatabaseRole::FilterRole);
		nameItem->setData(package.packageID, DatabaseRole::PackageIDRole);
		nameItem->setEditable(false);
		{
			QFont f = nameItem->font();
			f.setUnderline(true);
			nameItem->setFont(f);
		}
		SizeItem * sizeItem = new SizeItem(package.downloadSize);
		TextItem * buttonItem = nullptr;
		if (_downloadingPackageList.contains(package.packageID)) {
			buttonItem = new TextItem(QApplication::tr("InQueue"));
			buttonItem->setToolTip(QApplication::tr("InQueue"));
		} else if (installedPackages.find(package.packageID) == installedPackages.end()) {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
		} else {
			buttonItem = new TextItem(QChar(static_cast<int>(FontAwesome::trasho)));
			buttonItem->setToolTip(QApplication::tr("Uninstall"));
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
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::ModID, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Author, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::AvgDuration, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::DevDuration, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Game, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Release, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Update, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Type, itm);
				}
				{
					QStandardItem * itm = new QStandardItem();
					itm->setEditable(false);
					par->setChild(i, DatabaseColumn::Version, itm);
				}
				break;
			}
		}
		_packages[package.modID].push_back(package);
	}
	delete _waitSpinner;
	_waitSpinner = nullptr;
}

void ModDatabaseView::downloadPackageFiles(common::Mod mod, common::UpdatePackageListMessage::Package package, QSharedPointer<QList<QPair<QString, QString>>> fileList, QString fileserver) {
	const QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mod.id));
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		Q_UNUSED(b);
	}
	MultiFileDownloader * mfd = new MultiFileDownloader(this);
	for (const auto & p : *fileList) {
		QFileInfo fi(p.first);
		FileDownloader * fd = new FileDownloader(QUrl(fileserver + QString::number(mod.id) + "/" + p.first), dir.absolutePath() + "/" + fi.path(), fi.fileName(), p.second, mfd);
		mfd->addFileDownloader(fd);
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
		QtConcurrent::run([package]() {
			common::PackageDownloadSucceededMessage pdsm;
			pdsm.packageID = package.packageID;
			const std::string serialized = pdsm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
				sock.writePacket(serialized);
			}
		});
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

	DownloadQueue::getInstance()->add(mfd);
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
		_treeView->hideColumn(DatabaseColumn::ModID);
		_treeView->hideColumn(DatabaseColumn::Author);
		_treeView->hideColumn(DatabaseColumn::AvgDuration);
		_treeView->hideColumn(DatabaseColumn::Game);
		_treeView->hideColumn(DatabaseColumn::Version);
		columnCount -= 3;
	} else if (width < 1300) {
		_treeView->hideColumn(DatabaseColumn::ModID);
		_treeView->hideColumn(DatabaseColumn::Author);
		_treeView->hideColumn(DatabaseColumn::AvgDuration);
		_treeView->showColumn(DatabaseColumn::Game);
		_treeView->hideColumn(DatabaseColumn::Version);
		columnCount -= 2;
	} else if (width < 1600) {
		_treeView->hideColumn(DatabaseColumn::ModID);
		_treeView->showColumn(DatabaseColumn::Author);
		_treeView->hideColumn(DatabaseColumn::AvgDuration);
		_treeView->showColumn(DatabaseColumn::Game);
		_treeView->hideColumn(DatabaseColumn::Version);
		columnCount -= 1;
	} else if (width > 1600) {
		_treeView->showColumn(DatabaseColumn::ModID);
		_treeView->showColumn(DatabaseColumn::Version);
		_treeView->showColumn(DatabaseColumn::Author);
		_treeView->showColumn(DatabaseColumn::AvgDuration);
		_treeView->showColumn(DatabaseColumn::Game);
	}
	for (int i = 0; i < _sourceModel->columnCount() - 1; i++) {
		_treeView->setColumnWidth(i, _treeView->width() / columnCount);
	}
}

qint64 ModDatabaseView::getDownloadSize(common::Mod mod) const {
	qint64 size = 0;
	for (size_t i = 0; i < _mods.size(); i++) {
		if (_mods[i].id == mod.id) {
			size = _sourceModel->item(static_cast<int>(i), DatabaseColumn::Size)->data(Qt::UserRole).toLongLong();
			break;
		}
	}
	return size;
}

void ModDatabaseView::selectedModIndex(const QModelIndex & index) {
	if (!index.isValid()) return;

	common::Mod mod;
	if (index.model() == _sortModel) {
		mod = _mods[_sortModel->mapToSource(index).row()];
	} else {
		mod = _mods[index.row()];
	}
	if ((mod.gothic == common::GameType::Gothic && !_gothicValid) || (mod.gothic == common::GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == common::GameType::Gothic1And2 && !_gothic2Valid && !_gothicValid)) {
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
			
			// step 1: request all necessary files from server
			QtConcurrent::run([this, mod]() {
				common::RequestModFilesMessage rmfm;
				rmfm.modID = mod.id;
				rmfm.language = Config::Language.toStdString();
				std::string serialized = rmfm.SerializePublic();
				clockUtils::sockets::TcpSocket sock;
				if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
					sock.writePacket(serialized);
					if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
						try {
							common::Message * m = common::Message::DeserializePublic(serialized);
							if (m) {
								common::ListModFilesMessage * lmfm = dynamic_cast<common::ListModFilesMessage *>(m);

								QSharedPointer<QList<QPair<QString, QString>>> fileList(new QList<QPair<QString, QString>>());

								for (const auto & p : lmfm->fileList) {
									fileList->append(qMakePair(s2q(p.first), s2q(p.second)));
								}
								
								emit receivedModFilesList(mod, fileList, s2q(lmfm->fileserver));
							}
							delete m;
						} catch (...) {
							return;
						}
					}
				}
			});
		}
	} else {
		const bool uninstalled = client::Uninstaller::uninstall(mod.id, s2q(mod.name), mod.gothic == common::GameType::Gothic ? _gothicDirectory : _gothic2Directory);
		if (uninstalled) {
			int row = 0;
			for (; row < static_cast<int>(_mods.size()); row++) {
				if (_mods[row].id == mod.id) {
					break;
				}
			}
			TextItem * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
			buttonItem->setText(QChar(static_cast<int>(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
		}
	}
}

void ModDatabaseView::selectedPackageIndex(const QModelIndex & index) {
	if (!index.isValid()) return;

	common::Mod mod = _mods[index.model() == _sortModel ? _sortModel->mapToSource(index.parent()).row() : index.parent().row()];
	if ((mod.gothic == common::GameType::Gothic && !_gothicValid) || (mod.gothic == common::GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == common::GameType::Gothic1And2 && !_gothicValid && !_gothic2Valid)) {
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
	common::UpdatePackageListMessage::Package package = _packages[mod.id][(index.model() == _sortModel) ? _sortModel->mapToSource(index).row() : index.row()];

	if (_downloadingPackageList.contains(package.packageID)) return;
	
	if (installedPackages.find(package.packageID) == installedPackages.end()) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyWantToInstall"), QApplication::tr("ReallyWantToInstallText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == msg.exec()) {
			_downloadingPackageList.append(package.packageID);
			
			// step 1: request all necessary files from server
			QtConcurrent::run([this, mod, package]() {
				common::RequestPackageFilesMessage rpfm;
				rpfm.packageID = package.packageID;
				rpfm.language = Config::Language.toStdString();
				std::string serialized = rpfm.SerializePublic();
				clockUtils::sockets::TcpSocket sock;
				if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
					sock.writePacket(serialized);
					if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
						try {
							common::Message * m = common::Message::DeserializePublic(serialized);
							if (m) {
								common::ListModFilesMessage * lmfm = dynamic_cast<common::ListModFilesMessage *>(m);

								QSharedPointer<QList<QPair<QString, QString>>> fileList(new QList<QPair<QString, QString>>());

								for (const auto & p : lmfm->fileList) {
									fileList->append(qMakePair(s2q(p.first), s2q(p.second)));
								}
								
								emit receivedPackageFilesList(mod, package, fileList, s2q(lmfm->fileserver));
							}
							delete m;
						} catch (...) {
							return;
						}
					}
				}
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

void ModDatabaseView::removeInvalidDatabaseEntries() {
	// Free Aiming: 223 and 227. 227 is now obsolete
	if (isInstalled(223) && isInstalled(227)) {
		client::Uninstaller::uninstall(227);
	}
	// Workaround Helper: 225 and 234. 234 is now obsolete
	if (isInstalled(225) && isInstalled(234)) {
		client::Uninstaller::uninstall(234);
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
			
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(modID) + " AND PatchID = " + std::to_string(patchID) + " LIMIT 1;", err);
		}
	}
}

bool ModDatabaseView::isInstalled(int modID) {
	Database::DBError err;
	const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(modID) + ";", err);

	return count == 1;
}
