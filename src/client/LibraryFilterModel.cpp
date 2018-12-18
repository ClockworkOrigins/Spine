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

#include "LibraryFilterModel.h"

#include "common/GothicVersion.h"

#include <QSettings>
#include <QStandardItemModel>

namespace spine {

	LibraryFilterModel::LibraryFilterModel(QSettings * iniParser, QObject * par) : QSortFilterProxyModel(par), _iniParser(iniParser), _gothicActive(true), _gothic2Active(true), _gothicAndGothic2Active(true), _showHidden(false) {
		_iniParser->beginGroup("LIBRARYFILTER");
		_gothicActive = _iniParser->value("Gothic", true).toBool();
		_gothic2Active = _iniParser->value("Gothic2", true).toBool();
		_gothicAndGothic2Active = _iniParser->value("GothicAndGothic2", true).toBool();
		_showHidden = _iniParser->value("ShowHidden", false).toBool();
		_iniParser->endGroup();
	}

	void LibraryFilterModel::gothicChanged(int state) {
		_gothicActive = state == Qt::Checked;
		_iniParser->setValue("LIBRARYFILTER/Gothic", _gothicActive);
		invalidateFilter();
	}

	void LibraryFilterModel::gothic2Changed(int state) {
		_gothic2Active = state == Qt::Checked;
		_iniParser->setValue("LIBRARYFILTER/Gothic2", _gothic2Active);
		invalidateFilter();
	}

	void LibraryFilterModel::gothicAndGothic2Changed(int state) {
		_gothicAndGothic2Active = state == Qt::Checked;
		_iniParser->setValue("LIBRARYFILTER/GothicAndGothic2", _gothicAndGothic2Active);
		invalidateFilter();
	}

	void LibraryFilterModel::showHidden(int state) {
		_showHidden = state == Qt::Checked;
		_iniParser->setValue("LIBRARYFILTER/ShowHidden", _showHidden);
		invalidateFilter();
	}

	bool LibraryFilterModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const {
		bool result = true;
		QStandardItemModel * model = dynamic_cast<QStandardItemModel *>(sourceModel());
		result = result && ((common::GothicVersion(model->data(model->index(source_row, 0), GothicRole).toInt()) == common::GothicVersion::GOTHIC && _gothicActive) || (common::GothicVersion(model->data(model->index(source_row, 0), GothicRole).toInt()) == common::GothicVersion::GOTHIC2 && _gothic2Active) || (common::GothicVersion(model->data(model->index(source_row, 0), GothicRole).toInt()) == common::GothicVersion::GOTHICINGOTHIC2 && _gothicAndGothic2Active) || (common::GothicVersion(model->data(model->index(source_row, 0), GothicRole).toInt()) == common::GothicVersion::Gothic1And2 && (_gothicActive || _gothic2Active || _gothicAndGothic2Active)));
		result = result && (_showHidden || !model->data(model->index(source_row, 0), HiddenRole).toBool());
		result = result && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		return result;
	}

} /* namespace spine */
