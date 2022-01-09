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

#include "widgets/SubmitCompatibilityDialog.h"

#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/FileDownloader.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QPushButton>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QVBoxLayout>

using namespace spine::common;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

SubmitCompatibilityDialog::SubmitCompatibilityDialog() : SubmitCompatibilityDialog(-1, -1, GameType::Gothic2) {
}

SubmitCompatibilityDialog::SubmitCompatibilityDialog(int32_t modID, int32_t patchID, GameType gothicVersion) : QDialog(nullptr), _g1Button(nullptr), _g2Button(nullptr), _modView(nullptr), _patchView(nullptr), _compatibleButton(nullptr), _notCompatibleButton(nullptr), _modList(nullptr), _patchList(nullptr), _submitButton(nullptr), _showSubmitted(false), _modID(modID), _patchID(patchID), _gothicVersion(gothicVersion) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * gb = new QGroupBox(this);
		auto * hl = new QHBoxLayout();
		hl->setAlignment(Qt::AlignCenter);

		_g1Button = new QRadioButton(QApplication::tr("Gothic"), gb);
		_g2Button = new QRadioButton(QApplication::tr("Gothic2"), gb);
		_g1Button->setChecked(true);

		hl->addWidget(_g1Button);
		hl->addWidget(_g2Button);

		gb->setLayout(hl);

		l->addWidget(gb);

		connect(_g1Button, &QRadioButton::toggled, this, &SubmitCompatibilityDialog::updateView);
		connect(_g2Button, &QRadioButton::toggled, this, &SubmitCompatibilityDialog::updateView);
	}
	{
		auto * hl = new QHBoxLayout();

		_modView = new QListView(this);
		_patchView = new QListView(this);
		_modList = new QStandardItemModel(_modView);
		_patchList = new QStandardItemModel(_patchView);
		_modView->setModel(_modList);
		_patchView->setModel(_patchList);

		connect(_modView, &QListView::clicked, this, &SubmitCompatibilityDialog::selectIndex);
		connect(_patchView, &QListView::clicked, this, &SubmitCompatibilityDialog::selectPatchIndex);

		hl->addWidget(_modView);
		hl->addWidget(_patchView);

		l->addLayout(hl);
	}
	{
		auto * gb = new QGroupBox(this);
		auto * hl = new QHBoxLayout();
		hl->setAlignment(Qt::AlignCenter);

		_compatibleButton = new QRadioButton(QApplication::tr("Compatible"), this);
		_notCompatibleButton = new QRadioButton(QApplication::tr("NotCompatible"), this);
		_compatibleButton->setChecked(true);

		hl->addWidget(_compatibleButton);
		hl->addWidget(_notCompatibleButton);

		gb->setLayout(hl);

		l->addWidget(gb);
	}
	auto * showSubmitted = new QCheckBox(QApplication::tr("ShowAlreadySubmittedCompatibilities"), this);
	l->addWidget(showSubmitted);
	connect(showSubmitted, &QCheckBox::stateChanged, this, &SubmitCompatibilityDialog::changedHiddenState);

	auto * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Ok);
	b->setText(QApplication::tr("Submit"));
	_submitButton = b;

	connect(b, &QPushButton::released, this, &SubmitCompatibilityDialog::accepted);
	connect(b, &QPushButton::released, this, &SubmitCompatibilityDialog::accept);
	connect(b, &QPushButton::released, this, &SubmitCompatibilityDialog::hide);

	b = dbb->button(QDialogButtonBox::StandardButton::Cancel);
	b->setText(QApplication::tr("Cancel"));

	connect(b, &QPushButton::released, this, &SubmitCompatibilityDialog::rejected);
	connect(b, &QPushButton::released, this, &SubmitCompatibilityDialog::reject);
	connect(b, &QPushButton::released, this, &SubmitCompatibilityDialog::hide);

	connect(this, &SubmitCompatibilityDialog::receivedModList, this, &SubmitCompatibilityDialog::updateModList);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(QApplication::tr("SubmitCompatibility"));

	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "CREATE TABLE IF NOT EXISTS ownCompatibilityVotes(ModID INT NOT NULL, PatchID INT NOT NULL, Compatible INT NOT NULL, PRIMARY KEY(ModID, PatchID));", err);

	QtConcurrent::run([this]() {
		{
			QJsonObject json;
			json["Username"] = Config::Username;
			json["Password"] = Config::Password;

			Https::post(DATABASESERVER_PORT, "requestOwnCompatibilities", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & jsonData, int statusCode) {
				if (statusCode != 200) return;

				if (!jsonData.contains("Compatibilities")) return;

				Database::DBError dbErr;
				Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "DELETE FROM ownCompatibilityVotes;", dbErr);
				Database::open(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, dbErr);
				Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "BEGIN TRANSACTION;", dbErr);
				
				for (const auto jsonRef : jsonData["Compatibilities"].toArray()) {
					const auto jsonProj = jsonRef.toObject();

					const auto projectID = jsonProj["ProjectID"].toString().toInt();
					const auto patchID = jsonProj["PatchID"].toString().toInt();
					const auto compatible = jsonProj["Compatible"].toString().toInt();

					Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "INSERT INTO ownCompatibilityVotes (ModID, PatchID, Compatible) VALUES (" + std::to_string(projectID) + ", " + std::to_string(patchID) + ", " + std::to_string(compatible) + ");", dbErr);
				}
				
				Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "END TRANSACTION;", dbErr);
				Database::close(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, dbErr);
			});
		}

		QJsonObject json;
		json["Username"] = Config::Username;
		json["Password"] = Config::Password;
		json["Language"] = Config::Language;
		json["Simplified"] = 1;

		Https::postAsync(DATABASESERVER_PORT, "requestAllProjects", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & jsonData, int statusCode) {
			if (statusCode != 200) return;

			if (!jsonData.contains("Projects")) return;

			QList<Mod> projects;

			for (const auto jsonRef : jsonData["Projects"].toArray()) {
				const auto jsonProj = jsonRef.toObject();

				Mod project;
				project.id = jsonProj["ProjectID"].toString().toInt();
				project.name = q2s(jsonProj["Name"].toString());
				project.gothic = static_cast<GameType>(jsonProj["GameType"].toString().toInt());
				project.type = static_cast<ModType>(jsonProj["ModType"].toString().toInt());

				projects.push_back(project);
			}

			emit receivedModList(projects);
		});
	});
}

void SubmitCompatibilityDialog::updateModList(QList<Mod> mods) {
	for (const Mod & mod : mods) {
		if (mod.gothic == GameType::Gothic && (mod.type == ModType::TOTALCONVERSION || mod.type == ModType::ENHANCEMENT || mod.type == ModType::ORIGINAL)) {
			_g1Mods.append(mod);
		} else if (mod.gothic == GameType::Gothic && (mod.type == ModType::PATCH || mod.type == ModType::TOOL)) {
			_g1Patches.append(mod);
		} else if (mod.gothic == GameType::Gothic2 && (mod.type == ModType::TOTALCONVERSION || mod.type == ModType::ENHANCEMENT || mod.type == ModType::ORIGINAL)) {
			_g2Mods.append(mod);
		} else if (mod.gothic == GameType::Gothic2 && (mod.type == ModType::PATCH || mod.type == ModType::TOOL)) {
			_g2Patches.append(mod);
		} else if (mod.gothic == GameType::Gothic1And2 && (mod.type == ModType::PATCH || mod.type == ModType::TOOL)) {
			_g1Patches.append(mod);
			_g2Patches.append(mod);
		}
	}
	const std::function<bool(const Mod & a, const Mod & b)> compareFunc = [](const Mod & a, const Mod & b) {
		return a.name < b.name;
	};
	std::sort(_g1Mods.begin(), _g1Mods.end(), compareFunc);
	std::sort(_g1Patches.begin(), _g1Patches.end(), compareFunc);
	std::sort(_g2Mods.begin(), _g2Mods.end(), compareFunc);
	std::sort(_g2Patches.begin(), _g2Patches.end(), compareFunc);

	if (_gothicVersion == GameType::Gothic) {
		_g1Button->setChecked(true);
		_g2Button->setChecked(false);
	} else if (_gothicVersion == GameType::Gothic2) {
		_g1Button->setChecked(false);
		_g2Button->setChecked(true);
	}

	updateView();
}

void SubmitCompatibilityDialog::updateView() {
	const GameType gv = _g1Button->isChecked() ? GameType::Gothic : GameType::Gothic2;
	_modList->clear();
	QList<Mod> tmpMods = (gv == GameType::Gothic) ? _g1Mods : _g2Mods;
	_currentPatches = (gv == GameType::Gothic) ? _g1Patches : _g2Patches;
	_currentMods.clear();
	for (const Mod & mod : tmpMods) {
		Database::DBError err;
		const int count = Database::queryCount(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "SELECT * FROM ownCompatibilityVotes WHERE ModID = " + std::to_string(mod.id) + ";", err);
		if (count < _currentPatches.size() || _showSubmitted) {
			_currentMods.append(mod);
		}
	}
	int row = 0;
	int counter = 0;
	for (const Mod & mod : _currentMods) {
		if (mod.id == _modID) {
			row = counter;
		}
		auto * itm = new QStandardItem(s2q(mod.name));
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
	for (const Mod & mod : _currentPatches) {
		Database::DBError err;
		const auto results = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "SELECT Compatible FROM ownCompatibilityVotes WHERE ModID = " + std::to_string(_currentMods[idx.row()].id) + " AND PatchID = " + std::to_string(mod.id) + " LIMIT 1;", err);
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
	for (const Mod & mod : _filteredPatches) {
		if (mod.id == _patchID) {
			row = counter;
		}
		auto * itm = new QStandardItem(s2q(mod.name));
		itm->setEditable(false);
		itm->setData(compatibles[counter++], Qt::UserRole);
		_patchList->appendRow(itm);
	}
	if (!_filteredPatches.empty()) {
		_patchView->setCurrentIndex(_patchList->index(row, 0));
	}
}

void SubmitCompatibilityDialog::selectPatchIndex(const QModelIndex & idx) {
	_compatibleButton->setChecked(idx.data(Qt::UserRole).toBool());
	_notCompatibleButton->setChecked(!idx.data(Qt::UserRole).toBool());
}

void SubmitCompatibilityDialog::accept() {
	std::string username = Config::Username.toStdString();
	std::string password = Config::Password.toStdString();
	const int32_t projectID = _currentMods[_modView->currentIndex().row()].id;
	const int32_t patchID = _filteredPatches[_patchView->currentIndex().row()].id;
	const bool compatible = _compatibleButton->isChecked();

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ProjectID"] = projectID;
	requestData["PatchID"] = patchID;
	requestData["Compatible"] = compatible ? 1 : 0;

	Https::postAsync(DATABASESERVER_PORT, "submitCompatibility", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {});

	Database::DBError dbErr;
	Database::execute(Config::BASEDIR.toStdString() + "/" + COMPATIBILITY_DATABASE, "INSERT INTO ownCompatibilityVotes (ModID, PatchID, Compatible) VALUES (" + std::to_string(projectID) + ", " + std::to_string(patchID) + ", " + std::to_string(compatible) + ");", dbErr);

	QDialog::accept();
}

void SubmitCompatibilityDialog::changedHiddenState(int state) {
	_showSubmitted = state == Qt::CheckState::Checked;
	updateView();
}
