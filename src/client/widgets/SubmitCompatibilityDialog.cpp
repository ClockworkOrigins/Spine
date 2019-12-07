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

#include "widgets/SubmitCompatibilityDialog.h"

#include <thread>

#include "Config.h"
#include "utils/Conversion.h"
#include "Database.h"
#include "FileDownloader.h"
#include "SpineConfig.h"

#include "common/MessageStructs.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QGroupBox>
#include <QListView>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	SubmitCompatibilityDialog::SubmitCompatibilityDialog(QString language, QString username, QString password) : SubmitCompatibilityDialog(language, username, password, -1, -1, common::GothicVersion::GOTHIC2) {
	}

	SubmitCompatibilityDialog::SubmitCompatibilityDialog(QString language, QString username, QString password, int32_t modID, int32_t patchID, common::GothicVersion gothicVersion) : QDialog(nullptr), _language(language), _username(username), _password(password), _g1Button(nullptr), _g2Button(nullptr), _modView(nullptr), _patchView(nullptr), _compatibleButton(nullptr), _notCompatibleButton(nullptr), _modList(nullptr), _patchList(nullptr), _submitButton(nullptr), _g1Mods(), _g1Patches(), _g2Mods(), _g2Patches(), _currentPatches(), _filteredPatches(), _showSubmitted(false), _modID(modID), _patchID(patchID), _gothicVersion(gothicVersion) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QGroupBox * gb = new QGroupBox(this);
			QHBoxLayout * hl = new QHBoxLayout();
			hl->setAlignment(Qt::AlignCenter);

			_g1Button = new QRadioButton(QApplication::tr("Gothic"), gb);
			_g2Button = new QRadioButton(QApplication::tr("Gothic2"), gb);
			_g1Button->setChecked(true);

			hl->addWidget(_g1Button);
			hl->addWidget(_g2Button);

			gb->setLayout(hl);

			l->addWidget(gb);

			connect(_g1Button, SIGNAL(toggled(bool)), this, SLOT(updateView()));
			connect(_g2Button, SIGNAL(toggled(bool)), this, SLOT(updateView()));
		}
		{
			QHBoxLayout * hl = new QHBoxLayout();

			_modView = new QListView(this);
			_patchView = new QListView(this);
			_modList = new QStandardItemModel(_modView);
			_patchList = new QStandardItemModel(_patchView);
			_modView->setModel(_modList);
			_patchView->setModel(_patchList);

			connect(_modView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectIndex(const QModelIndex &)));
			connect(_patchView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectPatchIndex(const QModelIndex &)));

			hl->addWidget(_modView);
			hl->addWidget(_patchView);

			l->addLayout(hl);
		}
		{
			QGroupBox * gb = new QGroupBox(this);
			QHBoxLayout * hl = new QHBoxLayout();
			hl->setAlignment(Qt::AlignCenter);

			_compatibleButton = new QRadioButton(QApplication::tr("Compatible"), this);
			_notCompatibleButton = new QRadioButton(QApplication::tr("NotCompatible"), this);
			_compatibleButton->setChecked(true);

			hl->addWidget(_compatibleButton);
			hl->addWidget(_notCompatibleButton);

			gb->setLayout(hl);

			l->addWidget(gb);
		}
		QCheckBox * showSubmitted = new QCheckBox(QApplication::tr("ShowAlreadySubmittedCompatibilities"), this);
		l->addWidget(showSubmitted);
		connect(showSubmitted, SIGNAL(stateChanged(int)), this, SLOT(changedHiddenState(int)));

		QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
		l->addWidget(dbb);

		setLayout(l);

		QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Ok);
		b->setText(QApplication::tr("Submit"));
		_submitButton = b;

		connect(b, SIGNAL(clicked()), this, SIGNAL(accepted()));
		connect(b, SIGNAL(clicked()), this, SLOT(accept()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		b = dbb->button(QDialogButtonBox::StandardButton::Cancel);
		b->setText(QApplication::tr("Cancel"));

		connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
		connect(b, SIGNAL(clicked()), this, SLOT(reject()));
		connect(b, SIGNAL(clicked()), this, SLOT(hide()));

		connect(this, SIGNAL(receivedModList(std::vector<common::Mod>)), this, SLOT(updateModList(std::vector<common::Mod>)));

		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		setWindowTitle(QApplication::tr("SubmitCompatibility"));

		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "CREATE TABLE IF NOT EXISTS ownCompatibilityVotes(ModID INT NOT NULL, PatchID INT NOT NULL, Compatible INT NOT NULL, PRIMARY KEY(ModID, PatchID));", err);

		std::thread([this]() {
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				{
					common::RequestOwnCompatibilitiesMessage rocm;
					rocm.username = _username.toStdString();
					rocm.password = _password.toStdString();
					std::string serialized = rocm.SerializePublic();
					sock.writePacket(serialized);
					if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
						try {
							common::Message * m = common::Message::DeserializePublic(serialized);
							if (m) {
								common::SendOwnCompatibilitiesMessage * socm = dynamic_cast<common::SendOwnCompatibilitiesMessage *>(m);
								if (socm) {
									Database::DBError dbErr;
									Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "DELETE FROM ownCompatibilityVotes;", dbErr);
									Database::open(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, dbErr);
									Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "BEGIN TRANSACTION;", dbErr);
									for (common::SendOwnCompatibilitiesMessage::Compatibility c : socm->compatibilities) {
										Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "INSERT INTO ownCompatibilityVotes (ModID, PatchID, Compatible) VALUES (" + std::to_string(c.modID) + ", " + std::to_string(c.patchID) + ", " + std::to_string(c.compatible) + ");", dbErr);
									}
									Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "END TRANSACTION;", dbErr);
									Database::close(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, dbErr);
								}
							}
							delete m;
						} catch (...) {
							return;
						}
					} else {
						qDebug() << "Error occurred: " << int(cErr);
						return;
					}
				}
				{
					common::RequestAllModsMessage ramm;
					ramm.language = _language.toStdString();
					ramm.username = _username.toStdString();
					ramm.password = _password.toStdString();
					std::string serialized = ramm.SerializePublic();
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
						qDebug() << "Error occurred: " << int(cErr);
					}
				}
			} else {
				qDebug() << "Error occurred: " << int(cErr);
			}
		}).detach();
	}

	SubmitCompatibilityDialog::~SubmitCompatibilityDialog() {
	}

	void SubmitCompatibilityDialog::updateModList(std::vector<common::Mod> mods) {
		for (const common::Mod & mod : mods) {
			if (mod.gothic == common::GothicVersion::GOTHIC && (mod.type == common::ModType::TOTALCONVERSION || mod.type == common::ModType::ENHANCEMENT || mod.type == common::ModType::ORIGINAL)) {
				_g1Mods.append(mod);
			} else if (mod.gothic == common::GothicVersion::GOTHIC && (mod.type == common::ModType::PATCH || mod.type == common::ModType::TOOL)) {
				_g1Patches.append(mod);
			} else if (mod.gothic == common::GothicVersion::GOTHIC2 && (mod.type == common::ModType::TOTALCONVERSION || mod.type == common::ModType::ENHANCEMENT || mod.type == common::ModType::ORIGINAL)) {
				_g2Mods.append(mod);
			} else if (mod.gothic == common::GothicVersion::GOTHIC2 && (mod.type == common::ModType::PATCH || mod.type == common::ModType::TOOL)) {
				_g2Patches.append(mod);
			} else if (mod.gothic == common::GothicVersion::Gothic1And2 && (mod.type == common::ModType::PATCH || mod.type == common::ModType::TOOL)) {
				_g1Patches.append(mod);
				_g2Patches.append(mod);
			}
		}
		const std::function<bool(const common::Mod & a, const common::Mod & b)> compareFunc = [](const common::Mod & a, const common::Mod & b) {
			return a.name < b.name;
		};
		std::sort(_g1Mods.begin(), _g1Mods.end(), compareFunc);
		std::sort(_g1Patches.begin(), _g1Patches.end(), compareFunc);
		std::sort(_g2Mods.begin(), _g2Mods.end(), compareFunc);
		std::sort(_g2Patches.begin(), _g2Patches.end(), compareFunc);

		if (_gothicVersion == common::GothicVersion::GOTHIC) {
			_g1Button->setChecked(true);
			_g2Button->setChecked(false);
		} else if (_gothicVersion == common::GothicVersion::GOTHIC2) {
			_g1Button->setChecked(false);
			_g2Button->setChecked(true);
		}

		updateView();
	}

	void SubmitCompatibilityDialog::updateView() {
		const common::GothicVersion gv = _g1Button->isChecked() ? common::GothicVersion::GOTHIC : common::GothicVersion::GOTHIC2;
		_modList->clear();
		QList<common::Mod> tmpMods = (gv == common::GothicVersion::GOTHIC) ? _g1Mods : _g2Mods;
		_currentPatches = (gv == common::GothicVersion::GOTHIC) ? _g1Patches : _g2Patches;
		_currentMods.clear();
		for (const common::Mod & mod : tmpMods) {
			Database::DBError err;
			const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "SELECT * FROM ownCompatibilityVotes WHERE ModID = " + std::to_string(mod.id) + ";", err);
			if (count < _currentPatches.size() || _showSubmitted) {
				_currentMods.append(mod);
			}
		}
		int row = 0;
		int counter = 0;
		for (const common::Mod & mod : _currentMods) {
			if (mod.id == _modID) {
				row = counter;
			}
			QStandardItem * itm = new QStandardItem(s2q(mod.name));
			itm->setEditable(false);
			_modList->appendRow(itm);
			counter++;
		}
		if (!_currentMods.empty()) {
			_modView->setCurrentIndex(_modList->index(row, 0));
			selectIndex(_modList->index(row, 0));
		}
	}

	void SubmitCompatibilityDialog::selectIndex(const QModelIndex & idx) {
		_filteredPatches.clear();
		_patchList->clear();
		QList<bool> compatibles;
		for (const common::Mod & mod : _currentPatches) {
			Database::DBError err;
			const std::vector<int> results = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "SELECT Compatible FROM ownCompatibilityVotes WHERE ModID = " + std::to_string(_currentMods[idx.row()].id) + " AND PatchID = " + std::to_string(mod.id) + " LIMIT 1;", err);
			if (results.empty() || _showSubmitted) {
				if (results.empty()) {
					compatibles.append(true);
				} else {
					compatibles.append(results[0] == 1);
				}
				_filteredPatches.append(mod);
			}
		}
		int counter = 0;
		int row = 0;
		for (const common::Mod & mod : _filteredPatches) {
			if (mod.id == _patchID) {
				row = counter;
			}
			QStandardItem * itm = new QStandardItem(s2q(mod.name));
			itm->setEditable(false);
			itm->setData(compatibles[counter++], Qt::UserRole);
			_patchList->appendRow(itm);
		}
		if (!_filteredPatches.empty()) {
			_patchView->setCurrentIndex(_modList->index(row, 0));
		}
	}

	void SubmitCompatibilityDialog::selectPatchIndex(const QModelIndex & idx) {
		_compatibleButton->setChecked(idx.data(Qt::UserRole).toBool());
		_notCompatibleButton->setChecked(!idx.data(Qt::UserRole).toBool());
	}

	void SubmitCompatibilityDialog::accept() {
		std::string username = _username.toStdString();
		std::string password = _password.toStdString();
		int32_t modID = _currentMods[_modView->currentIndex().row()].id;
		int32_t patchID = _filteredPatches[_patchView->currentIndex().row()].id;
		bool compatible = _compatibleButton->isChecked();
		std::thread([username, password, modID, patchID, compatible]() {
			common::SubmitCompatibilityMessage scm;
			scm.username = username;
			scm.password = password;
			scm.modID = modID;
			scm.patchID = patchID;
			scm.compatible = compatible;
			const std::string serialized = scm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == err) {
				sock.writePacket(serialized);
			} else {
				qDebug() << "Error occurred: " << int(err);
			}
			Database::DBError dbErr;
			Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "INSERT INTO ownCompatibilityVotes (ModID, PatchID, Compatible) VALUES (" + std::to_string(modID) + ", " + std::to_string(patchID) + ", " + std::to_string(compatible) + ");", dbErr);
			Q_ASSERT(!dbErr.error);
		}).detach();

		QDialog::accept();
	}

	void SubmitCompatibilityDialog::changedHiddenState(int state) {
		_showSubmitted = state == Qt::CheckState::Checked;
		updateView();
	}

} /* namespace widgets */
} /* namespace spine */
