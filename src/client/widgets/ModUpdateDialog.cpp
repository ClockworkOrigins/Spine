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

#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/DownloadQueue.h"
#include "utils/FileDownloader.h"
#include "utils/MultiFileDownloader.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QtConcurrentRun>
#include <QTimer>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

ModUpdateDialog::ModUpdateDialog(QMainWindow * mainWindow) : QDialog(nullptr), _mainWindow(mainWindow), _infoLabel(nullptr), _checkBoxLayout(nullptr), _updates(), _checkBoxes(), _running(false), _lastTimeRejected(false), _oldVersions() {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	_infoLabel = new QLabel(QApplication::tr("SearchingForModUpdates"), this);
	_infoLabel->setWordWrap(true);
	l->addWidget(_infoLabel);

	_checkBoxLayout = new QVBoxLayout();
	l->addLayout(_checkBoxLayout);

	_dontShowAgain = new QCheckBox(QApplication::tr("DontShowAgain"), this);
	l->addWidget(_dontShowAgain);

	QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);
	dbb->hide();

	qRegisterMetaType<std::vector<common::ModUpdate>>("std::vector<common::ModUpdate>");

	connect(this, SIGNAL(receivedMods(std::vector<common::ModUpdate>)), dbb, SLOT(show()));
	connect(this, SIGNAL(receivedMods(std::vector<common::ModUpdate>)), this, SLOT(updateModList(std::vector<common::ModUpdate>)));

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
	b->setText(QApplication::tr("Apply"));

	connect(b, SIGNAL(clicked()), this, SIGNAL(accepted()));
	connect(b, SIGNAL(clicked()), this, SLOT(accept()));
	connect(b, SIGNAL(clicked()), this, SLOT(hide()));

	b = dbb->button(QDialogButtonBox::StandardButton::Discard);
	b->setText(QApplication::tr("Discard"));

	connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
	connect(b, SIGNAL(clicked()), this, SLOT(reject()));
	connect(b, SIGNAL(clicked()), this, SLOT(hide()));

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "CREATE TABLE IF NOT EXISTS updates (ModID INT PRIMARY KEY, Name TEXT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL);", err);
}

ModUpdateDialog::~ModUpdateDialog() {
}

int ModUpdateDialog::exec() {
	return QDialog::exec();
}

void ModUpdateDialog::loginChanged() {
	if (_lastTimeRejected) {
		_lastTimeRejected = false;
		return;
	}
	if (_running) {
		QTimer::singleShot(1000, std::bind(&ModUpdateDialog::loginChanged, this));
	} else {
		checkForUpdate();
	}
}

void ModUpdateDialog::updateModList(std::vector<common::ModUpdate> updates) {
	if (updates.empty()) {
		_infoLabel->setText(QApplication::tr("NoModUpdates"));
		_running = false;
	} else {
		_infoLabel->setText(QApplication::tr("SelectModUpdates"));
		_updates.clear();
		for (QCheckBox * cb : _checkBoxes) {
			_checkBoxLayout->removeWidget(cb);
			cb->deleteLater();
		}
		_checkBoxes.clear();
		int visibleCount = 0;
		for (const common::ModUpdate & u : updates) {
			Database::DBError err;
			Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "DELETE FROM updates WHERE ModID = " + std::to_string(u.modID) + " AND MajorVersion != " + std::to_string(int(u.majorVersion)) + " AND MinorVersion != " + std::to_string(int(u.minorVersion)) + " AND PatchVersion != " + std::to_string(int(u.patchVersion)) + " LIMIT 1;", err);
			std::vector<int> result = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "SELECT ModID FROM updates WHERE ModID = " + std::to_string(u.modID) + " AND MajorVersion = " + std::to_string(int(u.majorVersion)) + " AND MinorVersion = " + std::to_string(int(u.minorVersion)) + " AND PatchVersion = " + std::to_string(int(u.patchVersion)) + " LIMIT 1;", err);
			if (result.empty()) {
				QCheckBox * cb = new QCheckBox(s2q(u.name) + " (" + _oldVersions[u.modID] + " => " + QString("%1.%2.%3").arg(int(u.majorVersion)).arg(int(u.minorVersion)).arg(int(u.patchVersion)) + ")", this);
				cb->setChecked(true); // default enabled
				_checkBoxes.push_back(cb);
				_checkBoxLayout->addWidget(cb);
				_updates.push_back(u);
				const bool b = hasChanges(u);
				cb->setVisible(b);
				visibleCount += b ? 1 : 0;
			}
		}
		if (!_updates.empty()) {
			if (visibleCount > 0) {
				exec();
			} else {
				accept();
			}
		}
	}
}

void ModUpdateDialog::accept() {
	QList<common::ModUpdate> hides;
	std::vector<ModFile> installFiles;
	std::vector<ModFile> removeFiles;
	std::vector<ModFile> newFiles;
	for (size_t i = 0; i < _updates.size(); i++) {
		if (!_checkBoxes[i]->isChecked()) {
			hides.append(_updates[i]);
			continue;
		}
		Database::DBError err;
		std::vector<ModFile> m = Database::queryAll<ModFile, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, File, Hash FROM modfiles WHERE ModID = " + std::to_string(_updates[i].modID) + ";", err);
		std::vector<int> p = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT DISTINCT PackageID FROM packages WHERE ModID = " + std::to_string(_updates[i].modID) + ";", err);
		std::vector<ModFile> updateFiles;
		for (const auto & pr : _updates[i].files) {
			updateFiles.emplace_back(_updates[i].modID, pr.first, pr.second, -1, s2q(_updates[i].fileserver));
		}
		for (int packageID : p) {
			for (size_t j = 0; j < _updates[i].packageFiles.size(); j++) {
				if (_updates[i].packageFiles[j].first == packageID) {
					for (const auto & pr : _updates[i].packageFiles[j].second) {
						updateFiles.emplace_back(_updates[i].modID, pr.first, pr.second, _updates[i].packageFiles[j].first, s2q(_updates[i].fileserver));
					}
					_updates[i].files.insert(_updates[i].files.end(), _updates[i].packageFiles[j].second.begin(), _updates[i].packageFiles[j].second.end());
					break;
				}
			}
		}
		QSet<int> newPackages;
		for (size_t j = 0; j < _updates[i].packageFiles.size(); j++) {
			// check if package isn't already installed
			bool found = false;
			for (int packageID : p) {
				if (packageID == _updates[i].packageFiles[j].first) {
					found = true;
					break;
				}
			}
			// in case the package wasn't installed, check if at least a file of it already exists
			if (!found) {
				bool add = false;
				for (const auto & pr : _updates[i].packageFiles[j].second) {
					for (const auto & mf : m) {
						if (mf.file == pr.first) {
							newPackages.insert(_updates[i].packageFiles[j].first);
							add = true;
							break;
						}
					}
					if (add) {
						break;
					}
				}
				if (add) {
					for (const auto & pr : _updates[i].packageFiles[j].second) {
						updateFiles.emplace_back(_updates[i].modID, pr.first, pr.second, _updates[i].packageFiles[j].first, s2q(_updates[i].fileserver));
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
						installFiles.push_back(filePair);
					}
					break;
				}
			}
			// file was not found in new update, so it has to be deleted
			if (!found) {
				removeFiles.push_back(mf);
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
				newFiles.push_back(pr);
			}
		}

		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE patches SET Name = '" + _updates[i].name + "' WHERE ModID = " + std::to_string(_updates[i].modID) + ";", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE mods SET GothicVersion = " + std::to_string(int(_updates[i].gothicVersion)) + " WHERE ModID = " + std::to_string(_updates[i].modID) + ";", err);
	}

	MultiFileDownloader * mfd = new MultiFileDownloader(this);
	for (const ModFile & mf : installFiles) {
		QDir dir(Config::MODDIR + "/mods/" + QString::number(mf.modID));
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b);
		}
		QFileInfo fi(QString::fromStdString(mf.file));
		FileDownloader * fd = new FileDownloader(QUrl(mf.fileserver + QString::number(mf.modID) + "/" + QString::fromStdString(mf.file)), dir.absolutePath() + "/" + fi.path(), fi.fileName(), QString::fromStdString(mf.hash), mfd);
		mfd->addFileDownloader(fd);
	}
	for (const ModFile & mf : newFiles) {
		QDir dir(Config::MODDIR + "/mods/" + QString::number(mf.modID));
		if (!dir.exists()) {
			bool b = dir.mkpath(dir.absolutePath());
			Q_UNUSED(b);
		}
		QFileInfo fi(QString::fromStdString(mf.file));
		FileDownloader * fd = new FileDownloader(QUrl(mf.fileserver + QString::number(mf.modID) + "/" + QString::fromStdString(mf.file)), dir.absolutePath() + "/" + fi.path(), fi.fileName(), QString::fromStdString(mf.hash), mfd);
		mfd->addFileDownloader(fd);
	}

	for (size_t i = 0; i < _updates.size(); i++) {
		if (!_checkBoxes[i]->isChecked()) continue;

		emit updateStarted(_updates[i].modID);
	}

	connect(mfd, &MultiFileDownloader::downloadSucceeded, [this, installFiles, newFiles, removeFiles, hides]() {
		bool success = true;
		Database::DBError err;
		Database::open(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "BEGIN TRANSACTION;", err);
		for (ModFile mf : installFiles) {
			QString fileName = QString::fromStdString(mf.file);
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			mf.file = fileName.toStdString();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE modfiles SET Hash = '" + mf.hash + "' WHERE ModID = " + std::to_string(mf.modID) + " AND File = '" + mf.file + "';", err);
			success = success && !err.error;
		}
		for (ModFile mf : newFiles) {
			QString fileName = QString::fromStdString(mf.file);
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			mf.file = fileName.toStdString();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + std::to_string(mf.modID) + ", '" + mf.file + "', '" + mf.hash + "');", err);
			success = success && !err.error;
			if (success && mf.packageID != -1) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "INSERT INTO packages (ModID, PackageID, File) VALUES (" + std::to_string(mf.modID) + ", " + std::to_string(mf.packageID) + ", '" + mf.file + "');", err);
				success = success && !err.error;
			}
		}
		for (ModFile mf : removeFiles) {
			QString fileName = QString::fromStdString(mf.file);
			QFileInfo fi(fileName);
			if (fi.suffix() == "z") {
				fileName = fileName.mid(0, fileName.size() - 2);
			}
			mf.file = fileName.toStdString();
			QFile(Config::MODDIR + "/mods/" + QString::number(mf.modID) + "/" + fileName).remove();
			Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM modfiles WHERE ModID = " + std::to_string(mf.modID) + " AND File = '" + mf.file + "';", err);
			success = success && !err.error;
			if (success && mf.packageID != -1) {
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "DELETE FROM packages WHERE ModID = " + std::to_string(mf.modID) + " AND PackageID = " + std::to_string(mf.packageID) + " AND File = '" + mf.file + "';", err);
				success = success && !err.error;
			}
		}
		Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "END TRANSACTION;", err);
		Database::close(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, err);
		success = success && !err.error;

		if (success) {
			common::UpdateSucceededMessage usm;
			clockUtils::sockets::TcpSocket sock;
			sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			for (size_t i = 0; i < _updates.size(); i++) {
				if (!_checkBoxes[i]->isChecked()) {
					continue;
				}
				Database::execute(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "UPDATE mods SET MajorVersion = " + std::to_string(int(_updates[i].majorVersion)) + ", MinorVersion = " + std::to_string(int(_updates[i].minorVersion)) + ", PatchVersion = " + std::to_string(int(_updates[i].patchVersion)) + " WHERE ModID = " + std::to_string(_updates[i].modID) + ";", err);
				success = success && !err.error;
				if (success) {
					usm.modID = _updates[i].modID;
					const std::string serialized = usm.SerializePublic();
					sock.writePacket(serialized);

					emit updatedMod(_updates[i].modID);
				}
			}
		}
		if (success) {
			if (!installFiles.empty() || !newFiles.empty() || !removeFiles.empty()) {
				QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("UpdateSuccessful"), QApplication::tr("UpdateSuccessfulText"), QMessageBox::StandardButton::Ok);
				msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				msg.exec();
			}
		} else {
			QMessageBox msg(QMessageBox::Icon::Warning, QApplication::tr("UpdateUnsuccessful"), QApplication::tr("UpdateUnsuccessfulText"), QMessageBox::StandardButton::Ok);
			msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			msg.exec();
		}
		_running = false;
		hideUpdates(hides);
	});

	connect(mfd, &MultiFileDownloader::downloadFailed, [this, hides]() {
		QMessageBox msg(QMessageBox::Icon::Warning, QApplication::tr("UpdateUnsuccessful"), QApplication::tr("UpdateUnsuccessfulText"), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.exec();

		_running = false;
		hideUpdates(hides);
	});

	DownloadQueue::getInstance()->add(mfd);
	
	QDialog::accept();
}

void ModUpdateDialog::reject() {
	hideUpdates(QList<common::ModUpdate>::fromVector(QVector<common::ModUpdate>::fromStdVector(_updates)));
	_lastTimeRejected = true;
	_running = false;
	QDialog::reject();
}

void ModUpdateDialog::checkForUpdate() {
	_running = true;
	QtConcurrent::run([this]() {
		Database::DBError err;
		std::vector<common::ModVersion> m = Database::queryAll<common::ModVersion, int, int, int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, MajorVersion, MinorVersion, PatchVersion FROM mods;", err);
		for (common::ModVersion mv : m) {
			_oldVersions.insert(mv.modID, QString("%1.%2.%3").arg(int(mv.majorVersion)).arg(int(mv.minorVersion)).arg(int(mv.patchVersion)));
		}
		common::ModVersionCheckMessage mvcm;
		mvcm.modVersions = m;
		mvcm.language = Config::Language.toStdString();
		mvcm.username = Config::Username.toStdString();
		mvcm.password = Config::Password.toStdString();
		std::string serialized = mvcm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
			sock.writePacket(serialized);
			if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
				try {
					common::Message * msg = common::Message::DeserializePublic(serialized);
					if (msg) {
						const common::SendModsToUpdateMessage * smtum = dynamic_cast<common::SendModsToUpdateMessage *>(msg);
						if (smtum) {
							emit receivedMods(smtum->updates);
						} else {
							qDebug() << int(msg->type);
							emit receivedMods(std::vector<common::ModUpdate>());
						}
					} else {
						emit receivedMods(std::vector<common::ModUpdate>());
					}
					delete msg;
				} catch (...) {
					emit receivedMods(std::vector<common::ModUpdate>());
					return;
				}
			}
		}
	});
}

void ModUpdateDialog::checkForUpdate(int32_t modID) {
	_running = true;
	QtConcurrent::run([this, modID]() {
		std::vector<common::ModVersion> m = { common::ModVersion(modID, 0, 0, 0) };
		for (common::ModVersion mv : m) {
			_oldVersions.insert(mv.modID, QString("%1.%2.%3").arg(int(mv.majorVersion)).arg(int(mv.minorVersion)).arg(int(mv.patchVersion)));
		}
		common::ModVersionCheckMessage mvcm;
		mvcm.modVersions = m;
		mvcm.language = Config::Language.toStdString();
		mvcm.username = Config::Username.toStdString();
		mvcm.password = Config::Password.toStdString();
		std::string serialized = mvcm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000)) {
			sock.writePacket(serialized);
			if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
				try {
					common::Message * msg = common::Message::DeserializePublic(serialized);
					if (msg) {
						common::SendModsToUpdateMessage * smtum = dynamic_cast<common::SendModsToUpdateMessage *>(msg);
						if (smtum) {
							emit receivedMods(smtum->updates);
						} else {
							qDebug() << int(msg->type);
							emit receivedMods(std::vector<common::ModUpdate>());
						}
					} else {
						emit receivedMods(std::vector<common::ModUpdate>());
					}
					delete msg;
				} catch (...) {
					emit receivedMods(std::vector<common::ModUpdate>());
					return;
				}
			}
		}
	});
}

void ModUpdateDialog::hideUpdates(QList<common::ModUpdate> hides) const {
	if (!_dontShowAgain->isChecked()) {
		return;
	}
	for (const common::ModUpdate & mu : hides) {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "INSERT INTO updates (ModID, Name, MajorVersion, MinorVersion, PatchVersion) VALUES (" + std::to_string(mu.modID) + ", '" + mu.name + "', " + std::to_string(int(mu.majorVersion)) + ", " + std::to_string(int(mu.minorVersion)) + "," + std::to_string(int(mu.patchVersion)) + ");", err);
	}
}

bool ModUpdateDialog::hasChanges(common::ModUpdate mu) const {
	Database::DBError err;
	std::vector<ModFile> m = Database::queryAll<ModFile, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, File, Hash FROM modfiles WHERE ModID = " + std::to_string(mu.modID) + ";", err);
	std::vector<int> packageIDs = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT DISTINCT PackageID FROM packages WHERE ModID = " + std::to_string(mu.modID) + ";", err);

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
