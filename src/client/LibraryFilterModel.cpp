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

#include "LibraryFilterModel.h"

#include "common/GameType.h"

#include "utils/Config.h"

#include <QSettings>
#include <QStandardItemModel>

using namespace spine;
using namespace spine::utils;

LibraryFilterModel::LibraryFilterModel(QObject * par) : QSortFilterProxyModel(par), _gameActive(true), _gothicActive(true), _gothic2Active(true), _gothicAndGothic2Active(true), _showHidden(false) {
	Config::IniParser->beginGroup("LIBRARYFILTER");
	_gameActive = Config::IniParser->value("Game", true).toBool();
	_gothicActive = Config::IniParser->value("Gothic", true).toBool();
	_gothic2Active = Config::IniParser->value("Gothic2", true).toBool();
	_gothicAndGothic2Active = Config::IniParser->value("GothicAndGothic2", true).toBool();
	_showHidden = Config::IniParser->value("ShowHidden", false).toBool();
	Config::IniParser->endGroup();
}

void LibraryFilterModel::gameChanged(int state) {
	_gameActive = state == Qt::Checked;
	Config::IniParser->setValue("LIBRARYFILTER/Game", _gameActive);
	invalidateFilter();
}

void LibraryFilterModel::gothicChanged(int state) {
	_gothicActive = state == Qt::Checked;
	Config::IniParser->setValue("LIBRARYFILTER/Gothic", _gothicActive);
	invalidateFilter();
}

void LibraryFilterModel::gothic2Changed(int state) {
	_gothic2Active = state == Qt::Checked;
	Config::IniParser->setValue("LIBRARYFILTER/Gothic2", _gothic2Active);
	invalidateFilter();
}

void LibraryFilterModel::gothicAndGothic2Changed(int state) {
	_gothicAndGothic2Active = state == Qt::Checked;
	Config::IniParser->setValue("LIBRARYFILTER/GothicAndGothic2", _gothicAndGothic2Active);
	invalidateFilter();
}

void LibraryFilterModel::showHidden(int state) {
	_showHidden = state == Qt::Checked;
	Config::IniParser->setValue("LIBRARYFILTER/ShowHidden", _showHidden);
	invalidateFilter();
}

bool LibraryFilterModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const {
	bool result = true;
	QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(sourceModel());

	const auto gameType = static_cast<common::GameType>(model->data(model->index(source_row, 0), GameRole).toInt());
	
	result = result && ((gameType == common::GameType::Gothic && _gothicActive) || (gameType == common::GameType::Gothic2 && _gothic2Active) || (gameType == common::GameType::GothicInGothic2 && _gothicAndGothic2Active) || (gameType == common::GameType::Gothic1And2 && (_gothicActive || _gothic2Active || _gothicAndGothic2Active)) || (gameType == common::GameType::Game && _gameActive));
	result = result && (_showHidden || !model->data(model->index(source_row, 0), HiddenRole).toBool());
	result = result && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	return result;
}
