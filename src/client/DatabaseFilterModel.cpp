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

#include "DatabaseFilterModel.h"

#include "utils/Config.h"

#include <QApplication>
#include <QSettings>
#include <QStandardItemModel>

using namespace spine;
using namespace spine::utils;

DatabaseFilterModel::DatabaseFilterModel(QObject * par) : QSortFilterProxyModel(par), _gamesActive(true), _demosActive(true), _fullVersionsActive(true), _playTestingActive(true), _gothicActive(true), _gothic2Active(true), _gothicAndGothic2Active(true), _totalConversionActive(true), _enhancementActive(true), _patchActive(true), _toolActive(true), _originalActive(true), _gmpActive(true), _minDuration(0), _maxDuration(1000), _rendererAllowed(false), _languages(0), _installedProjectsActive(true), _playedProjectsActive(true) {
	Config::IniParser->beginGroup("DATABASEFILTER");
	_gamesActive = Config::IniParser->value("Games", true).toBool();
	_demosActive = Config::IniParser->value("Demos", true).toBool();
	_fullVersionsActive = Config::IniParser->value("FullVersions", true).toBool();
	_playTestingActive = Config::IniParser->value("PlayTesting", true).toBool();
	_gothicActive = Config::IniParser->value("Gothic", true).toBool();
	_gothic2Active = Config::IniParser->value("Gothic2", true).toBool();
	_gothicAndGothic2Active = Config::IniParser->value("GothicAndGothic2", true).toBool();
	_totalConversionActive = Config::IniParser->value("TotalConversion", true).toBool();
	_enhancementActive = Config::IniParser->value("Enhancement", true).toBool();
	_patchActive = Config::IniParser->value("Patch", true).toBool();
	_toolActive = Config::IniParser->value("Tool", true).toBool();
	_originalActive = Config::IniParser->value("Original", true).toBool();
	_gmpActive = Config::IniParser->value("GMP", true).toBool();
	_minDuration = Config::IniParser->value("MinDuration", 0).toInt();
	_maxDuration = Config::IniParser->value("MaxDuration", 1000).toInt();
	_languages = Config::IniParser->value("Languages", common::Language::German | common::Language::English | common::Language::Polish | common::Language::Russian).toInt();
	_installedProjectsActive = Config::IniParser->value("InstalledProjects", true).toBool();
	_playedProjectsActive = Config::IniParser->value("PlayedProjects", true).toBool();
	Config::IniParser->endGroup();
}

void DatabaseFilterModel::setPlayedProjects(const QSet<int32_t> & playedProjects) {
	_playedProjects = playedProjects;
	invalidateFilter();
}

void DatabaseFilterModel::gamesChanged(int state) {
	_gamesActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Games", _gamesActive);
	invalidateFilter();
}

void DatabaseFilterModel::demosChanged(int state) {
	_demosActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Demos", _demosActive);
	invalidateFilter();
}

void DatabaseFilterModel::fullVersionsChanged(int state) {
	_fullVersionsActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/FullVersions", _fullVersionsActive);
	invalidateFilter();
}

void DatabaseFilterModel::playTestingChanged(int state) {
	_playTestingActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/PlayTesting", _playTestingActive);
	invalidateFilter();
}

void DatabaseFilterModel::gothicChanged(int state) {
	_gothicActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Gothic", _gothicActive);
	invalidateFilter();
}

void DatabaseFilterModel::gothic2Changed(int state) {
	_gothic2Active = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Gothic2", _gothic2Active);
	invalidateFilter();
}

void DatabaseFilterModel::gothicAndGothic2Changed(int state) {
	_gothicAndGothic2Active = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/GothicAndGothic2", _gothicAndGothic2Active);
	invalidateFilter();
}

void DatabaseFilterModel::totalConversionChanged(int state) {
	_totalConversionActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/TotalConversion", _totalConversionActive);
	invalidateFilter();
}

void DatabaseFilterModel::enhancementChanged(int state) {
	_enhancementActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Enhancement", _enhancementActive);
	invalidateFilter();
}

void DatabaseFilterModel::patchChanged(int state) {
	_patchActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Patch", _patchActive);
	invalidateFilter();
}

void DatabaseFilterModel::toolChanged(int state) {
	_toolActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Tool", _toolActive);
	invalidateFilter();
}

void DatabaseFilterModel::originalChanged(int state) {
	_originalActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/Original", _originalActive);
	invalidateFilter();
}

void DatabaseFilterModel::gmpChanged(int state) {
	_gmpActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/GMP", _gmpActive);
	invalidateFilter();
}

void DatabaseFilterModel::minDurationChanged(int minDuration) {
	_minDuration = minDuration;
	Config::IniParser->setValue("DATABASEFILTER/MinDuration", _minDuration);
	invalidateFilter();
}

void DatabaseFilterModel::maxDurationChanged(int maxDuration) {
	_maxDuration = maxDuration;
	Config::IniParser->setValue("DATABASEFILTER/MaxDuration", _maxDuration);
	invalidateFilter();
}

void DatabaseFilterModel::languageChanged(common::Language language, int state) {
	if (state == Qt::Checked) {
		_languages |= language;
	} else {
		_languages &= ~language;
	}
	Config::IniParser->setValue("DATABASEFILTER/Languages", _languages);
	invalidateFilter();
}

void DatabaseFilterModel::installedProjectsChanged(int state) {
	_installedProjectsActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/InstalledProjects", _installedProjectsActive);
	invalidateFilter();
}

void DatabaseFilterModel::playedProjectsChanged(int state) {
	_playedProjectsActive = state == Qt::Checked;
	Config::IniParser->setValue("DATABASEFILTER/PlayedProjects", _playedProjectsActive);
	invalidateFilter();
}

void DatabaseFilterModel::setFilter(QString filter) {
	_filter = filter;
	invalidateFilter();
}

bool DatabaseFilterModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const {
	bool result = true;
	const auto * model = dynamic_cast<QStandardItemModel *>(sourceModel());
	if (!source_parent.isValid() && !model->item(source_row, DatabaseColumn::Name)->data(PackageIDRole).isValid()) {
		const auto projectID = model->item(source_row, DatabaseColumn::ModID)->text().toInt();
		const auto typeText = model->item(source_row, DatabaseColumn::Type)->text();
		const auto gameText = model->item(source_row, DatabaseColumn::Game)->text();
		const int devDuration = model->item(source_row, DatabaseColumn::DevDuration)->data(Qt::UserRole).toInt();
		const int languages = model->item(source_row, DatabaseColumn::Languages)->data(LanguagesRole).toInt();
		const bool installed = model->item(source_row, DatabaseColumn::Install)->data(Installed).toBool();

		const auto keywordsStr = model->item(source_row, DatabaseColumn::Name)->data(KeywordsRole).toString();
		const auto keywords = keywordsStr.split(";", Qt::SkipEmptyParts);

		result = result && ((typeText == QApplication::tr("TotalConversion") && _totalConversionActive) || (typeText == QApplication::tr("Enhancement") && _enhancementActive) || (typeText == QApplication::tr("Patch") && _patchActive) || (typeText == QApplication::tr("Tool") && _toolActive) || (typeText == QApplication::tr("Original") && _originalActive) || (typeText == QApplication::tr("GothicMultiplayer") && _gmpActive) || (typeText == QApplication::tr("FullVersion") && _gamesActive) || (typeText == QApplication::tr("Demo") && _demosActive) || (typeText == QApplication::tr("PlayTesting") && _playTestingActive));
		result = result && ((gameText == QApplication::tr("Gothic") && _gothicActive) || (gameText == QApplication::tr("Gothic2") && _gothic2Active) || (gameText == QApplication::tr("GothicAndGothic2_2") && _gothicAndGothic2Active) || (gameText == QApplication::tr("Game") && _gamesActive));
		result = result && (typeText == QApplication::tr("Patch") || typeText == QApplication::tr("Tool") || (devDuration / 60 >= _minDuration && devDuration / 60 <= _maxDuration));
		result = result && (!installed || _installedProjectsActive);
		result = result && (_playedProjectsActive || !_playedProjects.contains(projectID));

		if (!_filter.isEmpty()) {
			auto found = false;
			for (const auto & keyword : keywords) {
				if (!keyword.contains(_filter, Qt::CaseInsensitive))
					continue;

				found = true;
			}

			result = result && found;
		}

		bool languageMatch = false;
		for (int i = 1; i < common::Language::Count; i *= 2) {
			if (!(_languages & i && languages & i)) continue;
			
			languageMatch = true;
			break;
		}
		
		result = result && languageMatch;
		result = result && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	}
	result = result && (_rendererAllowed || !model->item(source_row, DatabaseColumn::Name)->text().contains("D3D11")); // check here to also filter D3D11 as package for another modification (e.g. GRM)
	return result;
}
