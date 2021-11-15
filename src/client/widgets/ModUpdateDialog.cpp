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

#include "widgets/ModUpdateDialog.h"

#include "IconCache.h"
#include "SpineConfig.h"

#include "common/Language.h"
#include "common/ModVersion.h"

#include "gui/DownloadQueueWidget.h"
#include "gui/OverlayMessageHandler.h"
#include "gui/Spoiler.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/FileDownloader.h"
#include "utils/MultiFileDownloader.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QtConcurrentRun>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::common;
using namespace spine::gui;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

ModUpdateDialog::ModFile::ModFile(std::string i, std::string s1, std::string s2) : modID(std::stoi(i)), file(s2q(s1)), hash(s2q(s2)) {}

ModUpdateDialog::ModUpdateDialog(QMainWindow * mainWindow) : QDialog(nullptr), _mainWindow(mainWindow), _infoLabel(nullptr), _checkBoxLayout(nullptr), _running(false), _lastTimeRejected(false), _loginChecked(false), _spineUpdateChecked(false) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	_infoLabel = new QLabel(QApplication::tr("SearchingForModUpdates"), this);
	_infoLabel->setWordWrap(true);
	l->addWidget(_infoLabel);

	_checkBoxLayout = new QVBoxLayout();
	_checkBoxLayout->setAlignment(Qt::AlignTop);

	auto * scrollArea = new QScrollArea(this);
	auto * mainWidget = new QWidget(this);
	mainWidget->setLayout(_checkBoxLayout);
	scrollArea->setWidget(mainWidget);
	scrollArea->setWidgetResizable(true);
	mainWidget->setProperty("default", true);

	l->addWidget(scrollArea, 1);

	_dontShowAgain = new QCheckBox(QApplication::tr("DontShowAgain"), this);
	l->addWidget(_dontShowAgain);

	auto * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);
	dbb->hide();

	qRegisterMetaType<QList<ModUpdate>>("QList<utils::ModUpdate>");
	qRegisterMetaType<QList<QPair<QString, QString>>>("QList<QPair<QString, QString>>");

	connect(this, &ModUpdateDialog::receivedMods, dbb, &QDialogButtonBox::show);
	connect(this, &ModUpdateDialog::receivedMods, this, &ModUpdateDialog::updateModList);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
	b->setText(QApplication::tr("Apply"));

	connect(b, &QPushButton::released, this, &ModUpdateDialog::accepted);
	connect(b, &QPushButton::released, this, &ModUpdateDialog::accept);
	connect(b, &QPushButton::released, this, &ModUpdateDialog::hide);

	b = dbb->button(QDialogButtonBox::StandardButton::Discard);
	b->setText(QApplication::tr("Discard"));

	connect(b, &QPushButton::released, this, &ModUpdateDialog::rejected);
	connect(b, &QPushButton::released, this, &ModUpdateDialog::reject);
	connect(b, &QPushButton::released, this, &ModUpdateDialog::hide);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "CREATE TABLE IF NOT EXISTS updates (ModID INT PRIMARY KEY, Name TEXT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL);", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "DELETE FROM updates WHERE ModID = 0;", err);

	err.error = false;
	
	Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "INSERT INTO updates (ModID, Name, MajorVersion, MinorVersion, PatchVersion, SpineVersion) VALUES (0, 'Foo', 0, 0, 0, 0);", err);
	
	if (err.error) {
		Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "DROP TABLE updates;", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "CREATE TABLE IF NOT EXISTS updates (ModID INT PRIMARY KEY, Name TEXT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL);", err);
	}
}

void ModUpdateDialog::loginChanged() {
	_loginChecked = true;
	
	if (_lastTimeRejected) {
		_lastTimeRejected = false;
		return;
	}
	if (_running) {
		QTimer::singleShot(1000, [this] { loginChanged(); });
	} else {
		checkForUpdate();
	}
}

void ModUpdateDialog::spineUpToDate() {
	_spineUpdateChecked = true;
	
	if (_lastTimeRejected) {
		_lastTimeRejected = false;
		return;
	}
	if (_running) {
		QTimer::singleShot(1000, [this] { loginChanged(); });
	} else {
		checkForUpdate();
	}
}

void ModUpdateDialog::updateModList(QList<ModUpdate> updates, bool forceAccept) {
	if (updates.empty()) {
		_infoLabel->setText(QApplication::tr("NoModUpdates"));
		_running = false;
	} else {
		_infoLabel->setText(QApplication::tr("SelectModUpdates"));

		clear();
		
		int visibleCount = 0;
		for (const ModUpdate & u : updates) {
			Database::DBError err;
			Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "DELETE FROM updates WHERE ModID = " + std::to_string(u.modID) + " AND MajorVersion != " + std::to_string(static_cast<int>(u.majorVersion)) + " AND MinorVersion != " + std::to_string(static_cast<int>(u.minorVersion)) + " AND PatchVersion != " + std::to_string(static_cast<int>(u.patchVersion)) + " AND SpineVersion != " + std::to_string(static_cast<int>(u.spineVersion)) + " LIMIT 1;", err);
			auto result = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "SELECT ModID FROM updates WHERE ModID = " + std::to_string(u.modID) + " AND MajorVersion = " + std::to_string(static_cast<int>(u.majorVersion)) + " AND MinorVersion = " + std::to_string(static_cast<int>(u.minorVersion)) + " AND PatchVersion = " + std::to_string(static_cast<int>(u.patchVersion)) + " AND SpineVersion = " + std::to_string(static_cast<int>(u.spineVersion)) + " LIMIT 1;", err);

			if (!result.empty()) continue;

			QString title = QString("%1 (%2 => %3.%4.%5)").arg(u.name).arg(_oldVersions[u.modID]).arg(static_cast<int>(u.majorVersion)).arg(static_cast<int>(u.minorVersion)).arg(static_cast<int>(u.patchVersion));

			auto * hl = new QHBoxLayout();
			
			auto * cb = new QCheckBox(title, this);
			cb->setChecked(true); // default enabled
			_checkBoxes << cb;
			_updates.push_back(u);
			const bool b = hasChanges(u);
			cb->setVisible(b);
			visibleCount += b ? !_alreadyDisplayedProjects.contains(u.modID) : 0;

			_alreadyDisplayedProjects.insert(u.modID);

			auto * lbl = new QLabel(u.savegameCompatible ? "" : "!", this);
			lbl->setProperty("error", true);
			lbl->setProperty("bold", true);
			lbl->setVisible(b);

			if (!u.savegameCompatible) {
				lbl->setToolTip(QApplication::tr("SaveNotCompatibleTooltip"));
			}

			_widgets << lbl;

			hl->addWidget(cb, 1);
			hl->addWidget(lbl);
			
			_checkBoxLayout->addLayout(hl);
			_checkBoxLayouts << hl;

			const auto changelog = u.changelog;

			if (b && !changelog.isEmpty()) {
				auto * vl = new QVBoxLayout();
				auto * tb = new QTextBrowser(this);
				tb->setProperty("changelog", true);
				tb->setText(changelog);
				vl->addWidget(tb);

				auto * s = new Spoiler(QApplication::tr("Changelog"), this);
				s->setContentLayout(vl);

				_checkBoxLayout->addWidget(s);
				_widgets << s;
			}
		}
		if (!_updates.empty()) {
			if (visibleCount > 0 && !forceAccept) {
				exec();
			} else if (visibleCount == 0 && !_alreadyDisplayedProjects.isEmpty()) {
				_updates.clear();
				accept();
			} else {
				accept();
			}
		}
	}
}

void ModUpdateDialog::accept() {
	QList<ModUpdate> hides;
	
	for (int i = 0; i < _updates.size(); i++) {
		if (!_checkBoxes[i]->isChecked()) {
			hides.append(_updates[i]);
			continue;
		}

		updateProject(_updates[i]);
	}

	for (int i = 0; i < _updates.size(); i++) {
		if (!_checkBoxes[i]->isChecked()) continue;

		emit updateStarted(_updates[i].modID);
	}
	
	hideUpdates(hides);

	clear();

	QDialog::accept();
}

void ModUpdateDialog::reject() {
	QList<ModUpdate> list;
	for (const auto & mu : _updates) {
		list << mu;
	}
	hideUpdates(list);
	_lastTimeRejected = true;
	_running = false;

	clear();
	
	QDialog::reject();
}

void ModUpdateDialog::checkForUpdate() {
	if (!_loginChecked) return;

	if (!_spineUpdateChecked) return;
	
	_running = true;
	QtConcurrent::run([this]() {
		Database::DBError err;

		struct ProjectLanguage {
			int id;
			int language;
		};
		
		auto m = Database::queryAll<ModVersion, int, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods;", err);
		auto pl = Database::queryAll<ProjectLanguage, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ProjectID, Language FROM languages;", err);
		for (ModVersion & mv : m) {
			_oldVersions.insert(mv.modID, QString("%1.%2.%3").arg(static_cast<int>(mv.majorVersion)).arg(static_cast<int>(mv.minorVersion)).arg(static_cast<int>(mv.patchVersion)));

			const auto it = std::find_if(pl.begin(), pl.end(), [mv](const ProjectLanguage & p) {
				return p.id == mv.modID;
			});

			if (it == pl.end()) continue;

			mv.language = it->language;
		}
		requestUpdates(m, false);
	});
}

void ModUpdateDialog::checkForUpdate(int32_t modID, bool forceAccept) {
	_running = true;
	QtConcurrent::run([this, modID, forceAccept]() {
		QList<ModVersion> m = { ModVersion(modID, 0, 0, 0, 0) };
		Database::DBError err;
		auto m2 = Database::queryAll<ModVersion, int, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods WHERE ModID = " + std::to_string(modID) + " LIMIT 1;", err);
		auto cl = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT Language FROM languages WHERE ProjectID = " + std::to_string(modID) + " LIMIT 1;", err);
		
		_oldVersions.insert(modID, QString("%1.%2.%3").arg(static_cast<int>(m2.empty() ? m[0].majorVersion : m2[0].majorVersion)).arg(static_cast<int>(m2.empty() ? m[0].minorVersion : m2[0].minorVersion)).arg(static_cast<int>(m2.empty() ? m[0].patchVersion : m2[0].patchVersion)));
		m[0].language = cl.empty() ? English : cl[0];

		requestUpdates(m, forceAccept);
	});
}

void ModUpdateDialog::hideUpdates(QList<ModUpdate> hides) const {
	if (!_dontShowAgain->isChecked()) return;

	for (const ModUpdate & mu : hides) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "INSERT INTO updates (ModID, Name, MajorVersion, MinorVersion, PatchVersion, SpineVersion) VALUES (" + std::to_string(mu.modID) + ", '" + q2s(mu.name) + "', " + std::to_string(static_cast<int>(mu.majorVersion)) + ", " + std::to_string(static_cast<int>(mu.minorVersion)) + "," + std::to_string(static_cast<int>(mu.patchVersion)) + "," + std::to_string(static_cast<int>(mu.spineVersion)) + ");", err);
	}
}

bool ModUpdateDialog::hasChanges(ModUpdate mu) const {
	Database::DBError err;
	auto m = Database::queryAll<ModFile, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, File, Hash FROM modfiles WHERE ModID = " + std::to_string(mu.modID) + ";", err);
	auto packageIDs = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT DISTINCT PackageID FROM packages WHERE ModID = " + std::to_string(mu.modID) + ";", err);

	for (auto it = mu.packageFiles.begin(); it != mu.packageFiles.end();) {
		bool found = false;
		for (int i : packageIDs) {
			if (it->first == i) {
				found = true;
				break;
			}
		}
		if (found) {
			++it;
		} else {
			it = mu.packageFiles.erase(it);
		}
	}
	bool b = mu.files.size() + mu.packageFiles.size() != m.size();
	if (!b) {
		while (!b && !mu.files.empty()) {
			bool found = false;
			for (auto it = m.begin(); it != m.end(); ++it) {
				if ((it->file == mu.files[0].first || it->file + ".z" == mu.files[0].first) && it->hash == mu.files[0].second) {
					mu.files.erase(mu.files.begin());
					m.erase(it);
					found = true;
					break;
				}
			}
			if (!found) {
				b = true;
			}
		}
		while (!b && !mu.packageFiles.empty()) {
			bool found = false;
			for (auto it = m.begin(); it != m.end(); ++it) {
				for (auto it2 = mu.packageFiles.begin(); it2 != mu.packageFiles.end(); ++it2) {
					for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3) {
						if ((it->file == it3->first || it->file + ".z" == it3->first) && it->hash == it3->second) {
							it2->second.erase(it3);
							if (it2->second.empty()) {
								mu.packageFiles.erase(it2);
							}
							m.erase(it);
							found = true;
							break;
						}
					}
					if (found) {
						break;
					}
				}
				if (found) {
					break;
				}
			}
			if (!found) {
				b = true;
			}
		}
	}
	return b;
}

void ModUpdateDialog::unzippedArchive(QString archive, QList<QPair<QString, QString>> files, ModFile mf, QSharedPointer<QList<ModFile>> installFiles, QSharedPointer<QList<ModFile>> newFiles, QSharedPointer<QList<ModFile>> removeFiles) {
	for (auto it = installFiles->begin(); it != installFiles->end(); ++it) {
		if (it->modID != mf.modID) continue;
		
		if (it->file != archive) continue;
		
		installFiles->erase(it);
		
		break;
	}
	
	for (auto it = newFiles->begin(); it != newFiles->end(); ++it) {
		if (it->modID != mf.modID) continue;
		
		if (it->file != archive) continue;
		
		newFiles->erase(it);
		
		break;
	}

	for (const auto & p : files) {
		bool found = false;
		
		for (auto it = removeFiles->begin(); it != removeFiles->end(); ++it) {
			if (it->modID != mf.modID) continue;

			if (it->file != p.first) continue;

			installFiles->append(*it);
			removeFiles->erase(it);

			found = true;

			break;
		}

		if (!found) {
			ModFile f;
			
			f.modID = mf.modID;
			f.packageID = mf.packageID;
			f.file = p.first;
			f.hash = p.second;
			f.fileserver = mf.fileserver;
			f.fallbackFileserver = mf.fallbackFileserver;
			
			newFiles->append(f);
		}
	}
}

void ModUpdateDialog::requestUpdates(const QList<ModVersion> & m, bool forceAccept) {
	if (m.empty()) {
		emit receivedMods(QList<ModUpdate>(), forceAccept);
		
		return;
	}
	
	QJsonObject json;
	json["Language"] = Config::Language;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;

	QJsonArray jsonArr;

	for (const auto & mv : m) {
		QJsonObject j;
		j["ProjectID"] = mv.modID;
		j["Language"] = mv.language;
		j["VersionMajor"] = static_cast<int>(mv.majorVersion);
		j["VersionMinor"] = static_cast<int>(mv.minorVersion);
		j["VersionPatch"] = static_cast<int>(mv.patchVersion);
		j["VersionSpine"] = static_cast<int>(mv.spineVersion);

		jsonArr << j;
	}

	json["Projects"] = jsonArr;

	Https::postAsync(DATABASESERVER_PORT, "projectVersionCheck", QJsonDocument(json).toJson(QJsonDocument::Compact), [this, forceAccept](const QJsonObject & jsonData, int responseCode) {
		if (responseCode != 200) {
			emit receivedMods(QList<ModUpdate>(), forceAccept);
			return;
		}

		QList<ModUpdate> updates;

		if (jsonData.contains("Updates")) {
			for (const auto jsonRef : jsonData["Updates"].toArray()) {
				const auto j = jsonRef.toObject();
				
				ModUpdate mu;
				mu.modID = j["ProjectID"].toString().toInt();
				mu.name = decodeString(j["Name"].toString());
				mu.majorVersion = static_cast<int8_t>(j["VersionMajor"].toString().toInt());
				mu.minorVersion = static_cast<int8_t>(j["VersionMinor"].toString().toInt());
				mu.patchVersion = static_cast<int8_t>(j["VersionPatch"].toString().toInt());
				mu.spineVersion = static_cast<int8_t>(j["VersionSpine"].toString().toInt());
				mu.fileserver = j["Fileserver"].toString();
				mu.fallbackFileserver = j["FallbackFileserver"].toString();

				if (mu.fallbackFileserver.isEmpty()) {
					mu.fallbackFileserver = mu.fileserver;
				}
				mu.gothicVersion = static_cast<GameType>(j["Type"].toString().toInt());
				mu.savegameCompatible = j["SavegameCompatible"].toString().toInt();
				mu.changelog = decodeString(j["Changelog"].toString()).replace("&quot;", "\"");
				mu.modID = j["ProjectID"].toString().toInt();

				if (j.contains("Files")) {
					for (const auto jsonRef2 : j["Files"].toArray()) {
						const auto j2 = jsonRef2.toObject();

						const auto file = j2["File"].toString();
						const auto hash = j2["Hash"].toString();

						mu.files << qMakePair(file, hash);
					}
				}

				if (j.contains("Packages")) {
					for (const auto jsonRef2 : j["Packages"].toArray()) {
						const auto j2 = jsonRef2.toObject();

						if (!j2.contains("Files")) continue;

						const auto packageID = j2["PackageID"].toString().toInt();

						QList<QPair<QString, QString>> files;
						
						for (const auto jsonRef3 : j2["Files"].toArray()) {
							const auto j3 = jsonRef3.toObject();

							const auto file = j3["File"].toString();
							const auto hash = j3["Hash"].toString();

							files << qMakePair(file, hash);
						}
						
						mu.packageFiles << qMakePair(packageID, files);
					}
				}

				updates.push_back(mu);
			}
		}

		emit receivedMods(updates, forceAccept);
	});
}

void ModUpdateDialog::updateProject(ModUpdate mu) {
	QSharedPointer<QList<ModFile>> installFiles(new QList<ModFile>());
	QSharedPointer<QList<ModFile>> removeFiles(new QList<ModFile>());
	QSharedPointer<QList<ModFile>> newFiles(new QList<ModFile>());
	
	Database::DBError err;
	auto m = Database::queryAll<ModFile, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, File, Hash FROM modfiles WHERE ModID = " + std::to_string(mu.modID) + ";", err);
	auto p = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT DISTINCT PackageID FROM packages WHERE ModID = " + std::to_string(mu.modID) + ";", err);
	QList<ModFile> updateFiles;
	for (const auto & pr : mu.files) {
		updateFiles << ModFile(mu.modID, pr.first, pr.second, -1, mu.fileserver, mu.fallbackFileserver);
	}
	for (int packageID : p) {
		for (int j = 0; j < mu.packageFiles.size(); j++) {
			if (mu.packageFiles[j].first == packageID) {
				for (const auto & pr : mu.packageFiles[j].second) {
					updateFiles << ModFile(mu.modID, pr.first, pr.second, mu.packageFiles[j].first, mu.fileserver, mu.fallbackFileserver);
				}
				mu.files.append(mu.packageFiles[j].second);
				break;
			}
		}
	}
	QSet<int> newPackages;
	for (int j = 0; j < mu.packageFiles.size(); j++) {
		// check if package isn't already installed
		bool found = false;
		for (int packageID : p) {
			if (packageID == mu.packageFiles[j].first) {
				found = true;
				break;
			}
		}
		// in case the package wasn't installed, check if at least a file of it already exists
		if (!found) {
			bool add = false;
			for (const auto & pr : mu.packageFiles[j].second) {
				for (const auto & mf : m) {
					if (mf.file == pr.first) {
						newPackages.insert(mu.packageFiles[j].first);
						add = true;
						break;
					}
				}
				if (add) {
					break;
				}
			}
			if (add) {
				for (const auto & pr : mu.packageFiles[j].second) {
					updateFiles << ModFile(mu.modID, pr.first, pr.second, mu.packageFiles[j].first, mu.fileserver, mu.fallbackFileserver);
				}
			}
		}
	}
	// check all existing files for updates
	for (const ModFile & mf : m) {
		// compare with all files in the update list
		bool found = false;
		for (const ModFile & filePair : updateFiles) {
			// if file name is the same check if hash differs
			if (filePair.file == mf.file || filePair.file == mf.file + ".z") {
				found = true;
				if (filePair.hash != mf.hash) {
					installFiles->push_back(filePair);
				}
				break;
			}
		}
		// file was not found in new update, so it has to be deleted
		if (!found) {
			removeFiles->push_back(mf);
		}
	}
	// check for new files
	for (const auto & pr : updateFiles) {
		bool found = false;
		for (const ModFile & mf : m) {
			if (pr.file == mf.file || pr.file == mf.file + ".z") {
				found = true;
				break;
			}
		}
		// file was not found in local copy, so it has to be installed
		if (!found) {
			newFiles->push_back(pr);
		}
	}

	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE patches SET Name = '" + q2s(mu.name) + "' WHERE ModID = " + std::to_string(mu.modID) + ";", err);
	Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE mods SET GothicVersion = " + std::to_string(static_cast<int>(mu.gothicVersion)) + " WHERE ModID = " + std::to_string(mu.modID) + ";", err);

	auto * mfd = new MultiFileDownloader(this);
	for (const ModFile & mf : *installFiles) {
		QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mf.modID));
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b)
		}
		QFileInfo fi(mf.file);

		const auto relativePath = QString::number(mf.modID) + "/" + mf.file;
		
		auto * fd = new FileDownloader(QUrl(mf.fileserver + relativePath), QUrl(mf.fallbackFileserver + relativePath), dir.absolutePath() + "/" + fi.path(), fi.fileName(), mf.hash, mfd);
		mfd->addFileDownloader(fd);

		// zip workflow
		const auto suffix = fi.completeSuffix();

		if (suffix.compare("zip.z", Qt::CaseInsensitive) != 0) continue;

		// 1. if it is a zip, register new signal. FileDownloader will send signal after extracting the archive reporting the files with hashes it contained
		// 2. reported files need to be added to filelist and archive must be removed
		connect(fd, &FileDownloader::unzippedArchive, this, [this, mf, installFiles, newFiles, removeFiles](const QString & archive, const QList<QPair<QString, QString>> & files) {
			unzippedArchive(archive, files, mf, installFiles, newFiles, removeFiles);
			});
	}
	for (const ModFile & mf : *newFiles) {
		QDir dir(Config::DOWNLOADDIR + "/mods/" + QString::number(mf.modID));
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b)
		}
		QFileInfo fi(mf.file);

		const auto relativePath = QString::number(mf.modID) + "/" + mf.file;
		
		auto * fd = new FileDownloader(QUrl(mf.fileserver + relativePath), QUrl(mf.fallbackFileserver + relativePath), dir.absolutePath() + "/" + fi.path(), fi.fileName(), mf.hash, mfd);
		mfd->addFileDownloader(fd);

		// zip workflow
		const auto suffix = fi.completeSuffix();

		if (suffix.compare("zip.z", Qt::CaseInsensitive) != 0) continue;

		// 1. if it is a zip, register new signal. FileDownloader will send signal after extracting the archive reporting the files with hashes it contained
		// 2. reported files need to be added to filelist and archive must be removed
		connect(fd, &FileDownloader::unzippedArchive, this, [this, mf, installFiles, newFiles, removeFiles](const QString & archive, const QList<QPair<QString, QString>> & files) {
			unzippedArchive(archive, files, mf, installFiles, newFiles, removeFiles);
		});
	}

	connect(mfd, &MultiFileDownloader::downloadSucceeded, this, [this, installFiles, newFiles, removeFiles, mu]() {
		bool success = true;
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		for (ModFile mf : *installFiles) {
			QString fileName = mf.file;
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			mf.file = fileName;
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE modfiles SET Hash = '" + q2s(mf.hash) + "' WHERE ModID = " + std::to_string(mf.modID) + " AND File = '" + q2s(mf.file) + "';", err);
			success = success && !err.error;
		}
		for (ModFile mf : *newFiles) {
			QString fileName = mf.file;
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			mf.file = fileName;
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mf.modID) + ", '" + q2s(mf.file) + "', '" + q2s(mf.hash) + "');", err);
			success = success && !err.error;
			if (success && mf.packageID != -1) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO packages (ModID, PackageID, File) VALUES (" + std::to_string(mf.modID) + ", " + std::to_string(mf.packageID) + ", '" + q2s(mf.file) + "');", err);
				success = success && !err.error;
			}
		}
		for (ModFile mf : *removeFiles) {
			QString fileName = mf.file;
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			mf.file = fileName;
			QFile(Config::DOWNLOADDIR + "/mods/" + QString::number(mf.modID) + "/" + fileName).remove();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM modfiles WHERE ModID = " + std::to_string(mf.modID) + " AND File = '" + q2s(mf.file) + "';", err);
			success = success && !err.error;
			if (success && mf.packageID != -1) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM packages WHERE ModID = " + std::to_string(mf.modID) + " AND PackageID = " + std::to_string(mf.packageID) + " AND File = '" + q2s(mf.file) + "';", err);
				success = success && !err.error;
			}
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		success = success && !err.error;

		if (success) {
			QJsonArray jsonArr;

			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE mods SET MajorVersion = " + std::to_string(static_cast<int>(mu.majorVersion)) + ", MinorVersion = " + std::to_string(static_cast<int>(mu.minorVersion)) + ", PatchVersion = " + std::to_string(static_cast<int>(mu.patchVersion)) + ", SpineVersion = " + std::to_string(static_cast<int>(mu.spineVersion)) + " WHERE ModID = " + std::to_string(mu.modID) + ";", err);
			success = success && !err.error;
			if (success) {
				jsonArr << mu.modID;

				emit updatedMod(mu.modID);
			}

			if (!jsonArr.empty()) {
				QJsonObject json;
				json["IDs"] = jsonArr;

				Https::postAsync(DATABASESERVER_PORT, "updateSucceeded", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});
			}
		}
		if (success) {
			if (!installFiles->empty() || !newFiles->empty() || !removeFiles->empty()) {
				OverlayMessageHandler::getInstance()->showMessage(IconCache::getInstance()->getOrLoadIconAsImage(":/svg/download.svg"), QApplication::tr("UpdateSuccessful"));
			}
		} else {
			OverlayMessageHandler::getInstance()->showMessage(IconCache::getInstance()->getOrLoadIconAsImage(":/svg/download.svg"), QApplication::tr("UpdateUnsuccessful"));
		}
		_running = false;
	});

	connect(mfd, &MultiFileDownloader::downloadFailed, this, [this]() {
		OverlayMessageHandler::getInstance()->showMessage(IconCache::getInstance()->getOrLoadIconAsImage(":/svg/download.svg"), QApplication::tr("UpdateUnsuccessful"));

		_running = false;
	});

	DownloadQueueWidget::getInstance()->addDownload(QApplication::tr("PatchingProject").arg(mu.name), mfd);
}

void ModUpdateDialog::clear() {
	_updates.clear();
	for (QCheckBox * cb : _checkBoxes) {
		delete cb;
	}
	for (QWidget * w : _widgets) {
		delete w;
	}
	for (QHBoxLayout * hl : _checkBoxLayouts) {
		delete hl;
	}
	_checkBoxes.clear();
	_checkBoxLayouts.clear();
	_widgets.clear();
}
