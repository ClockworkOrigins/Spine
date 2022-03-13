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

#include "widgets/SavegameDialog.h"

#include "SavegameManager.h"

#include "utils/Config.h"

#include "widgets/LocationSettingsWidget.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

SavegameDialog::SavegameDialog(LocationSettingsWidget * locationSettingsWidget, QWidget * par) : QDialog(par), _filterModel(nullptr), _model(nullptr), _savegameManager(new SavegameManager(this)), _savegameManager2(new SavegameManager(this)), _gothicDirectory(locationSettingsWidget->getGothicDirectory()), _gothic2Directory(locationSettingsWidget->getGothic2Directory()) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * hl = new QHBoxLayout();
		auto * g1Button = new QPushButton(QApplication::tr("Gothic1Save"), this);
		auto * g2Button = new QPushButton(QApplication::tr("Gothic2Save"), this);
		hl->addWidget(g1Button);
		hl->addWidget(g2Button);
		g1Button->setVisible(locationSettingsWidget->isGothicValid(true));

		connect(g1Button, &QPushButton::released, this, &SavegameDialog::openG1Save);
		connect(g2Button, &QPushButton::released, this, &SavegameDialog::openG2Save);

		l->addLayout(hl);
	}

	auto * filterEdit = new QLineEdit(this);
	l->addWidget(filterEdit);

	auto * tv = new QTableView(this);
	_model = new QStandardItemModel(tv);
	auto * filterModel = new QSortFilterProxyModel(tv);
	filterModel->setSourceModel(_model);
	filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	filterModel->setFilterKeyColumn(-1);
	tv->setModel(filterModel);
	l->addWidget(tv);
	tv->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	tv->horizontalHeader()->hide();
	tv->verticalHeader()->hide();
	connect(filterEdit, &QLineEdit::textChanged, filterModel, &QSortFilterProxyModel::setFilterFixedString);
	connect(_model, &QStandardItemModel::itemChanged, this, &SavegameDialog::itemChanged);

	auto * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Ok);
	b->setText(QApplication::tr("Save"));

	connect(b, &QPushButton::released, this, &SavegameDialog::save);
	connect(b, &QPushButton::released, this, &SavegameDialog::accepted);
	connect(b, &QPushButton::released, this, &SavegameDialog::accept);
	connect(b, &QPushButton::released, this, &SavegameDialog::hide);

	b = dbb->button(QDialogButtonBox::StandardButton::Cancel);
	b->setText(QApplication::tr("Cancel"));

	connect(b, &QPushButton::released, this, &SavegameDialog::rejected);
	connect(b, &QPushButton::released, this, &SavegameDialog::reject);
	connect(b, &QPushButton::released, this, &SavegameDialog::hide);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(QApplication::tr("SavegameEditor"));

	restoreSettings();
}

SavegameDialog::~SavegameDialog() {
	saveSettings();
}

void SavegameDialog::openG1Save() {
	openSave(_gothicDirectory);
}

void SavegameDialog::openG2Save() {
	openSave(_gothic2Directory);
}

void SavegameDialog::save() const {
	SavegameManager::save(_openedFile, _variables);
}

void SavegameDialog::itemChanged(QStandardItem * itm) {
	if (itm->model() != _model)
		return;

	_variables[itm->index().row()].changed = _variables[itm->index().row()].value != itm->data(Qt::DisplayRole).toInt();
	_variables[itm->index().row()].value = itm->data(Qt::DisplayRole).toInt();
}

void SavegameDialog::openSave(const QString & basePath) {
	const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectSave"), basePath, "SAVEDAT.SAV");
	if (path.isEmpty())
		return;

	auto replace = true;

	if (!_savegameManager->getVariables().isEmpty()) {
		QMessageBox resultMsg(QMessageBox::Icon::NoIcon, QApplication::tr("SavegameAlreadyLoaded"), QApplication::tr("SavegameAlreadyLoadedDescription"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::No);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Replace"));
		resultMsg.button(QMessageBox::StandardButton::No)->setText(QApplication::tr("Compare"));
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);

		replace = resultMsg.exec() == QMessageBox::StandardButton::Ok;
	}

	QList<Variable> variables;

	if (replace) {
		_openedFile = path;
		_savegameManager->load(path);
		variables = _savegameManager->getVariables();
	} else {
		variables = _savegameManager->getVariables();
		_savegameManager2->load(path);
		const auto diffVariables = _savegameManager2->getVariables();

		merge(variables, diffVariables);
	}

	updateView(variables, !replace);
}

void SavegameDialog::updateView(const QList<Variable> & variables, bool showDiff) {
	_variables = variables;
	_model->clear();

	for (const Variable & v : _variables) {
		auto * itmName = new QStandardItem(QString::fromStdString(v.name));
		itmName->setEditable(false);

		auto * itmValue = new QStandardItem();
		itmValue->setEditable(true);
		itmValue->setData(v.value, Qt::DisplayRole);

		QList<QStandardItem *> items;

		items << itmName << itmValue;

		if (showDiff) {
			auto * itmDiff = new QStandardItem();
			itmDiff->setEditable(false);
			itmDiff->setData(v.diffValue, Qt::DisplayRole);

			items << itmDiff;
		}

		_model->appendRow(items);
	}
}

void SavegameDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/SavegameDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/SavegameDialogGeometry");
	}
}

void SavegameDialog::saveSettings() const {
	Config::IniParser->setValue("WINDOWGEOMETRY/SavegameDialogGeometry", saveGeometry());
}

void SavegameDialog::merge(QList<Variable> & variables, const QList<Variable> & diffVariables) const {
	for (auto it = variables.begin(); it != variables.end(); ++it) {
		for (auto it2 = diffVariables.begin(); it2 != diffVariables.end(); ++it2) {
			if (it->name != it2->name)
				continue;

			it->diffValue = it2->value;

			break;
		}
	}
}
