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
#include "DirectXVersionCheck.h"
#include "FontAwesome.h"
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
#include "utils/WindowsExtensions.h"

#include "widgets/UninstallDialog.h"
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
				double dSize = double(size);
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

ModDatabaseView::ModDatabaseView(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _mainWindow(mainWindow), _treeView(nullptr), _sourceModel(nullptr), _sortModel(nullptr), _mods(), _gothicValid(false), _gothic2Valid(false), _parentMods(), _gothicDirectory(), _gothic2Directory(), _packageIDIconMapping(), _waitSpinner(nullptr), _allowRenderer(false) {
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
	_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("ID") << QApplication::tr("Name") << QApplication::tr("Author") << QApplication::tr("Type") << QApplication::tr("Game") << QApplication::tr("DevTime") << QApplication::tr("AvgTime") << QApplication::tr("ReleaseDate") << QApplication::tr("Version") << QApplication::tr("DownloadSize") << QString());
	_treeView->setAlternatingRowColors(true);
	_treeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	_treeView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	_treeView->setMinimumWidth(800);
	_treeView->setSortingEnabled(true);
	_treeView->sortByColumn(DatabaseColumn::Release);

	connect(_treeView->header(), SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));

	connect(generalSettingsWidget, SIGNAL(languageChanged(QString)), this, SLOT(changeLanguage(QString)));

	{
		QWidget * filterWidget = new QWidget(this);
		QHBoxLayout * hl = new QHBoxLayout();
		QLineEdit * le = new QLineEdit(filterWidget);
		le->setPlaceholderText(QApplication::tr("SearchPlaceholder"));
		UPDATELANGUAGESETPLACEHOLDERTEXT(le, "SearchPlaceholder");

		connect(le, SIGNAL(textChanged(const QString &)), this, SLOT(changedFilterExpression(const QString &)));

		hl->addWidget(le);

		{
			QGroupBox * gb = new QGroupBox(QApplication::tr("Type"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "Type");

			QVBoxLayout * vbl = new QVBoxLayout();

			QCheckBox * cb1 = new QCheckBox(QApplication::tr("TotalConversion"), filterWidget);
			cb1->setChecked(_sortModel->isTotalConversionActive());
			UPDATELANGUAGESETTEXT(cb1, "TotalConversion");
			connect(cb1, SIGNAL(stateChanged(int)), _sortModel, SLOT(totalConversionChanged(int)));

			QCheckBox * cb2 = new QCheckBox(QApplication::tr("Enhancement"), filterWidget);
			cb2->setChecked(_sortModel->isEnhancementActive());
			UPDATELANGUAGESETTEXT(cb2, "Enhancement");
			connect(cb2, SIGNAL(stateChanged(int)), _sortModel, SLOT(enhancementChanged(int)));

			QCheckBox * cb3 = new QCheckBox(QApplication::tr("Patch"), filterWidget);
			cb3->setChecked(_sortModel->isPathActive());
			UPDATELANGUAGESETTEXT(cb3, "Patch");
			connect(cb3, SIGNAL(stateChanged(int)), _sortModel, SLOT(patchChanged(int)));

			QCheckBox * cb4 = new QCheckBox(QApplication::tr("Tool"), filterWidget);
			cb4->setChecked(_sortModel->isToolActive());
			UPDATELANGUAGESETTEXT(cb4, "Tool");
			connect(cb4, SIGNAL(stateChanged(int)), _sortModel, SLOT(toolChanged(int)));

			QCheckBox * cb5 = new QCheckBox(QApplication::tr("Original"), filterWidget);
			cb5->setChecked(_sortModel->isOriginalActive());
			UPDATELANGUAGESETTEXT(cb5, "Original");
			connect(cb5, SIGNAL(stateChanged(int)), _sortModel, SLOT(originalChanged(int)));

			QCheckBox * cb6 = new QCheckBox(QApplication::tr("GothicMultiplayer"), filterWidget);
			cb6->setChecked(_sortModel->isGMPActive());
			UPDATELANGUAGESETTEXT(cb6, "GothicMultiplayer");
			connect(cb6, SIGNAL(stateChanged(int)), _sortModel, SLOT(gmpChanged(int)));

			vbl->addWidget(cb1);
			vbl->addWidget(cb2);
			vbl->addWidget(cb3);
			vbl->addWidget(cb4);
			vbl->addWidget(cb5);
			vbl->addWidget(cb6);

			gb->setLayout(vbl);

			hl->addWidget(gb);
		}

		{
			QGroupBox * gb = new QGroupBox(QApplication::tr("Game"), filterWidget);
			UPDATELANGUAGESETTITLE(gb, "Game");

			QVBoxLayout * vbl = new QVBoxLayout();

			QCheckBox * cb1 = new QCheckBox(QApplication::tr("Gothic"), filterWidget);
			cb1->setChecked(_sortModel->isGothicActive());
			UPDATELANGUAGESETTEXT(cb1, "Gothic");
			connect(cb1, SIGNAL(stateChanged(int)), _sortModel, SLOT(gothicChanged(int)));

			QCheckBox * cb2 = new QCheckBox(QApplication::tr("Gothic2"), filterWidget);
			cb2->setChecked(_sortModel->isGothic2Active());
			UPDATELANGUAGESETTEXT(cb2, "Gothic2");
			connect(cb2, SIGNAL(stateChanged(int)), _sortModel, SLOT(gothic2Changed(int)));

			QCheckBox * cb3 = new QCheckBox(QApplication::tr("GothicAndGothic2"), filterWidget);
			cb3->setChecked(_sortModel->isGothicAndGothic2Active());
			UPDATELANGUAGESETTEXT(cb3, "GothicAndGothic2");
			connect(cb3, SIGNAL(stateChanged(int)), _sortModel, SLOT(gothicAndGothic2Changed(int)));
			cb3->hide();

			vbl->addWidget(cb1);
			vbl->addWidget(cb2);
			vbl->addWidget(cb3);

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

			connect(sb1, SIGNAL(valueChanged(int)), _sortModel, SLOT(minDurationChanged(int)));
			connect(sb2, SIGNAL(valueChanged(int)), _sortModel, SLOT(maxDurationChanged(int)));

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

	l->addWidget(_treeView);

	setLayout(l);

	qRegisterMetaType<std::vector<common::Mod>>("std::vector<common::Mod>");
	qRegisterMetaType<std::vector<std::pair<int32_t, uint64_t>>>("std::vector<std::pair<int32_t, uint64_t>>");
	qRegisterMetaType<common::Mod>("common::Mod");
	qRegisterMetaType<common::UpdatePackageListMessage::Package>("common::UpdatePackageListMessage::Package");
	qRegisterMetaType<std::vector<common::UpdatePackageListMessage::Package>>("std::vector<common::UpdatePackageListMessage::Package>");
	qRegisterMetaType<std::vector<std::pair<std::string, std::string>>>("std::vector<std::pair<std::string, std::string>>");

	connect(this, SIGNAL(receivedModList(std::vector<common::Mod>)), this, SLOT(updateModList(std::vector<common::Mod>)));
	connect(this, SIGNAL(receivedModFilesList(common::Mod, std::vector<std::pair<std::string, std::string>>, QString)), this, SLOT(downloadModFiles(common::Mod, std::vector<std::pair<std::string, std::string>>, QString)));
	connect(_treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectedIndex(const QModelIndex &)));
	connect(_treeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(doubleClickedIndex(const QModelIndex &)));
	connect(this, SIGNAL(receivedPackageList(std::vector<common::UpdatePackageListMessage::Package>)), this, SLOT(updatePackageList(std::vector<common::UpdatePackageListMessage::Package>)));
	connect(this, SIGNAL(receivedPackageFilesList(common::Mod, common::UpdatePackageListMessage::Package, std::vector<std::pair<std::string, std::string>>, QString)), this, SLOT(downloadPackageFiles(common::Mod, common::UpdatePackageListMessage::Package, std::vector<std::pair<std::string, std::string>>, QString)));
	connect(this, SIGNAL(triggerInstallMod(int)), this, SLOT(installMod(int)));
	connect(this, SIGNAL(triggerInstallPackage(int, int)), this, SLOT(installPackage(int, int)));

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS mods(ModID INT NOT NULL, GothicVersion INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS modfiles(ModID INT NOT NULL, File TEXT NOT NULL, Hash TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS patches(ModID INT NOT NULL, Name TEXT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "CREATE TABLE IF NOT EXISTS packages(ModID INT NOT NULL, PackageID INT NOT NULL, File TEXT NOT NULL);", err);

#ifdef Q_OS_WIN
	DWORD versionMajor = 0;
	DWORD versionMinor = 0;

	const HRESULT hr = GetDXVersion(&versionMajor, &versionMinor);
	if (SUCCEEDED(hr)) {
		LOGINFO("DirectX version: " << versionMajor << "." << versionMinor);
		_allowRenderer = versionMajor >= 11;
	} else {
		_allowRenderer = true;
	}
	_allowRenderer = true;
#else
	_allowRenderer = false;
#endif
	_sortModel->setRendererAllowed(_allowRenderer);

	removeInvalidDatabaseEntries();
}

void ModDatabaseView::changeLanguage(QString language) {
	updateModList(-1);
}

void ModDatabaseView::updateModList(int modID, int packageID) {
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage updateModList #1: " << getPRAMValue());
#endif
	_waitSpinner = new WaitSpinner(QApplication::tr("LoadingDatabase"), this);
	_sourceModel->setHorizontalHeaderLabels(QStringList() << QApplication::tr("ID") << QApplication::tr("Name") << QApplication::tr("Author") << QApplication::tr("Type") << QApplication::tr("Game") << QApplication::tr("DevTime") << QApplication::tr("AvgTime") << QApplication::tr("ReleaseDate") << QApplication::tr("Version") << QApplication::tr("DownloadSize") << QString());
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage updateModList #2: " << getPRAMValue());
#endif
	QtConcurrent::run([this, modID, packageID]() {
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage updateModList #3: " << getPRAMValue());
#endif
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
							emit receivedModList(uamm->mods);
						}
					}
					delete m;
				} catch (...) {
					return;
				}
			} else {
				qDebug() << "Error occurred: " << int(err);
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
				qDebug() << "Error occurred: " << int(err);
			}
		} else {
			qDebug() << "Error occurred: " << int(err);
		}
		if (modID > 0 && packageID > 0) {
			emit triggerInstallPackage(modID, packageID);
		} else if (modID > 0) {
			emit triggerInstallMod(modID);
		}
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage updateModList #4: " << getPRAMValue());
#endif
	});
}

void ModDatabaseView::gothicValidationChanged(bool valid) {
	_gothicValid = valid;
}

void ModDatabaseView::gothic2ValidationChanged(bool valid) {
	_gothic2Valid = valid;
}

void ModDatabaseView::loginChanged() {
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage ModDatabaseView::setUsername #1: " << getPRAMValue());
#endif
	// check if a GMP mod is installed and GMP is not installed
	Database::DBError dbErr;
	const std::vector<int> gmpModInstalled = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID FROM mods WHERE ModID = 62 OR ModID = 117 OR ModID = 171 OR ModID = 172 OR ModID = 173 OR ModID = 218;", dbErr);
	const bool gmpInstalled = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = 228 LIMIT 1;", dbErr) > 0;

	if (!gmpModInstalled.empty() && !gmpInstalled) {
		updateModList(228);
			
		for (int modID : gmpModInstalled) {
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "DELETE FROM patchConfigs WHERE ModID = " + std::to_string(modID) + " AND PatchID = 228;", dbErr);
			Database::execute(Config::BASEDIR.toStdString() + "/" + PATCHCONFIG_DATABASE, "INSERT INTO patchConfigs (ModID, PatchID, Enabled) VALUES (" + std::to_string(modID) + ", 228, 1);", dbErr);
		}
	} else if (isVisible()) {
		updateModList(-1);
	}
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage ModDatabaseView::setUsername #2: " << getPRAMValue());
#endif
}

void ModDatabaseView::setGothicDirectory(QString dir) {
	_gothicDirectory = dir;
}

void ModDatabaseView::setGothic2Directory(QString dir) {
	_gothic2Directory = dir;
}

void ModDatabaseView::updateModList(std::vector<common::Mod> mods) {
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage updateModList #10: " << getPRAMValue());
#endif
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
			gameName = QApplication::tr("GothicAndGothic2");
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
		VersionItem * versionItem = new VersionItem(mod.majorVersion, mod.minorVersion, mod.patchVersion);
		versionItem->setEditable(false);
		SizeItem * sizeItem = new SizeItem(mod.downloadSize);
		sizeItem->setEditable(false);
		QStandardItem * buttonItem = nullptr;
		if (installedMods.find(mod.id) == installedMods.end()) {
			buttonItem = new TextItem(QChar(int(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
		} else {
			buttonItem = new TextItem(QChar(int(FontAwesome::trasho)));
			buttonItem->setToolTip(QApplication::tr("Uninstall"));
		}
		QFont f = buttonItem->font();
		f.setPointSize(13);
		f.setFamily("FontAwesome");
		buttonItem->setFont(f);
		buttonItem->setEditable(false);

		QStandardItem * idItem = new IntItem(mod.id);
		idItem->setEditable(false);

		_sourceModel->appendRow(QList<QStandardItem *>() << idItem << nameItem << teamItem << typeItem << gameItem << devTimeItem << avgTimeItem << releaseDateItem << versionItem << sizeItem << buttonItem);
		for (int i = 0; i < _sourceModel->columnCount(); i++) {
			_sourceModel->setData(_sourceModel->index(row, i), Qt::AlignCenter, Qt::TextAlignmentRole);
		}
		if ((mod.gothic == common::GameType::Gothic && !_gothicValid) || (mod.gothic == common::GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == common::GameType::GothicInGothic2 && (!_gothicValid || !_gothic2Valid)) || (mod.gothic == common::GameType::Gothic1And2 && !_gothicValid && !_gothic2Valid) || Config::MODDIR.isEmpty() || !QDir(Config::MODDIR).exists()) {
			idItem->setEnabled(false);
			nameItem->setEnabled(false);
			teamItem->setEnabled(false);
			typeItem->setEnabled(false);
			gameItem->setEnabled(false);
			devTimeItem->setEnabled(false);
			avgTimeItem->setEnabled(false);
			releaseDateItem->setEnabled(false);
			versionItem->setEnabled(false);
			sizeItem->setEnabled(false);
			buttonItem->setEnabled(false);
		}
		_parentMods.insert(mod.id, _sourceModel->index(row, 0));
		row++;
	}
	_mods = mods;
#ifdef Q_OS_WIN
	LOGINFO("Memory Usage updateModList #11: " << getPRAMValue());
#endif
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

void ModDatabaseView::downloadModFiles(common::Mod mod, std::vector<std::pair<std::string, std::string>> fileList, QString fileserver) {
	const QDir dir(Config::MODDIR + "/mods/" + QString::number(mod.id));
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		Q_UNUSED(b);
	}
	MultiFileDownloader * mfd = new MultiFileDownloader(this);
	for (const auto & p : fileList) {
		QFileInfo fi(QString::fromStdString(p.first));
		FileDownloader * fd = new FileDownloader(QUrl(fileserver + QString::number(mod.id) + "/" + QString::fromStdString(p.first)), dir.absolutePath() + "/" + fi.path(), fi.fileName(), QString::fromStdString(p.second), mfd);
		mfd->addFileDownloader(fd);
	}

	connect(mfd, &MultiFileDownloader::downloadSucceeded, [this, mod, fileList]() {
		int row = 0;
		for (; row < int(_mods.size()); row++) {
			if (_mods[row].id == mod.id) {
				break;
			}
		}
		TextItem * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
		buttonItem->setText(QChar(int(FontAwesome::trasho)));
		buttonItem->setToolTip(QApplication::tr("Uninstall"));
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO mods (ModID, GothicVersion, MajorVersion, MinorVersion, PatchVersion) VALUES (" + std::to_string(mod.id) + ", " + std::to_string(int(mod.gothic)) + ", " + std::to_string(int(_mods[row].majorVersion)) + ", " + std::to_string(int(_mods[row].minorVersion)) + ", " + std::to_string(int(_mods[row].patchVersion)) + ");", err);
		for (const auto p : fileList) {
			QString fileName = QString::fromStdString(p.first);
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mod.id) + ", '" + fileName.toStdString() + "', '" + p.second + "');", err);
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
		finishedInstallation(mod.id, -1, true);

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
			QDir dir2(Config::MODDIR + "/mods/" + QString::number(mod.id));
			
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
					finishedInstallation(mod.id, -1, false);
				}
			} else {
				QMessageBox msg(QMessageBox::Icon::Warning, QApplication::tr("InstallationUnsuccessful"), errorText, QMessageBox::StandardButton::Ok);
				msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				msg.exec();
				dir2.removeRecursively();
				finishedInstallation(mod.id, -1, false);
			}
		}
	});

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
		if (installedPackages.find(package.packageID) == installedPackages.end()) {
			buttonItem = new TextItem(QChar(int(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
		} else {
			buttonItem = new TextItem(QChar(int(FontAwesome::trasho)));
			buttonItem->setToolTip(QApplication::tr("Uninstall"));
		}
		_packageIDIconMapping.insert(package.packageID, buttonItem);
		QFont f = buttonItem->font();
		f.setPointSize(13);
		buttonItem->setFont(f);
		buttonItem->setEditable(false);
		QStandardItem * par = _sourceModel->item(_parentMods[package.modID].row());
		for (int i = 0; i < INT_MAX; i++) {
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

void ModDatabaseView::downloadPackageFiles(common::Mod mod, common::UpdatePackageListMessage::Package package, std::vector<std::pair<std::string, std::string>> fileList, QString fileserver) {
	QDir dir(Config::MODDIR + "/mods/" + QString::number(mod.id));
	if (!dir.exists()) {
		bool b = dir.mkpath(dir.absolutePath());
		Q_UNUSED(b);
	}
	MultiFileDownloader * mfd = new MultiFileDownloader(this);
	for (const auto & p : fileList) {
		QFileInfo fi(QString::fromStdString(p.first));
		FileDownloader * fd = new FileDownloader(QUrl(fileserver + QString::number(mod.id) + "/" + QString::fromStdString(p.first)), dir.absolutePath() + "/" + fi.path(), fi.fileName(), QString::fromStdString(p.second), mfd);
		mfd->addFileDownloader(fd);
	}

	connect(mfd, &MultiFileDownloader::downloadSucceeded, [this, package, fileList, mod]() {
		TextItem * buttonItem = _packageIDIconMapping[package.packageID];
		buttonItem->setText(QChar(int(FontAwesome::trasho)));
		buttonItem->setToolTip(QApplication::tr("Uninstall"));
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		for (const auto & p : fileList) {
			QString fileName = QString::fromStdString(p.first);
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mod.id) + ", '" + fileName.toStdString() + "', '" + p.second + "');", err);
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
		finishedInstallation(mod.id, package.packageID, true);
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
			finishedInstallation(mod.id, package.packageID, false);
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
			size = _sourceModel->item(int(i), DatabaseColumn::Size)->data(Qt::UserRole).toLongLong();
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
		finishedInstallation(mod.id, -1, false);
		return;
	}
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
		if (mod.id == 40 || mod.id == 57 || mod.id == 37 || mod.id == 116 || mod.id == 36 || QMessageBox::StandardButton::Ok == msg.exec()) {
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
								emit receivedModFilesList(mod, lmfm->fileList, s2q(lmfm->fileserver));
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
			for (; row < int(_mods.size()); row++) {
				if (_mods[row].id == mod.id) {
					break;
				}
			}
			TextItem * buttonItem = dynamic_cast<TextItem *>(_sourceModel->item(row, DatabaseColumn::Install));
			buttonItem->setText(QChar(int(FontAwesome::downloado)));
			buttonItem->setToolTip(QApplication::tr("Install"));
		}
	}
}

void ModDatabaseView::selectedPackageIndex(const QModelIndex & index) {
	if (!index.isValid()) return;

	common::Mod mod = _mods[(index.model() == _sortModel) ? _sortModel->mapToSource(index.parent()).row() : index.parent().row()];
	if ((mod.gothic == common::GameType::Gothic && !_gothicValid) || (mod.gothic == common::GameType::Gothic2 && !_gothic2Valid) || (mod.gothic == common::GameType::Gothic1And2 && !_gothicValid && !_gothic2Valid)) {
		finishedInstallation(mod.id, -1, false);
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
		finishedInstallation(mod.id, -1, false);
		return;
	}
	common::UpdatePackageListMessage::Package package = _packages[mod.id][(index.model() == _sortModel) ? _sortModel->mapToSource(index).row() : index.row()];
	if (installedPackages.find(package.packageID) == installedPackages.end()) {
		qDebug() << "Install pressed";
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyWantToInstall"), QApplication::tr("ReallyWantToInstallText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == msg.exec()) {
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
								emit receivedPackageFilesList(mod, package, lmfm->fileList, s2q(lmfm->fileserver));
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
		qDebug() << "Uninstall pressed";
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ReallyWantToUninstall"), QApplication::tr("ReallyWantToUninstallText").arg(s2q(package.name)), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == msg.exec()) {
			QDir dir(Config::MODDIR + "/mods/" + QString::number(mod.id));
			const auto files = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT File FROM packages WHERE PackageID = " + std::to_string(package.packageID) + ";", err);
			for (const std::string & s : files) {
				QFile(dir.absolutePath() + "/" + QString::fromStdString(s)).remove();
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM modfiles WHERE ModID = " + std::to_string(mod.id) + " AND File = '" + s + "';", err);
			}
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM packages WHERE PackageID = " + std::to_string(package.packageID) + ";", err);

			TextItem * buttonItem = _packageIDIconMapping[package.packageID];
			buttonItem->setText(QChar(int(FontAwesome::downloado)));
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
}

bool ModDatabaseView::isInstalled(int modID) {
	Database::DBError err;
	const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(modID) + ";", err);

	return count == 1;
}
