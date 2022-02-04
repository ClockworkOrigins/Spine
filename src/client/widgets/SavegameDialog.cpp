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
#include <QPushButton>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

SavegameDialog::SavegameDialog(LocationSettingsWidget * locationSettingsWidget, QWidget * par) : QDialog(par), _filterModel(nullptr), _model(nullptr), _savegameManager(new SavegameManager(this)), _gothicDirectory(locationSettingsWidget->getGothicDirectory()), _gothic2Directory(locationSettingsWidget->getGothic2Directory()) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QHBoxLayout * hl = new QHBoxLayout();
		QPushButton * g1Button = new QPushButton(QApplication::tr("Gothic1Save"), this);
		QPushButton * g2Button = new QPushButton(QApplication::tr("Gothic2Save"), this);
		hl->addWidget(g1Button);
		hl->addWidget(g2Button);
		g1Button->setVisible(locationSettingsWidget->isGothicValid(true));

		connect(g1Button, &QPushButton::released, this, &SavegameDialog::openG1Save);
		connect(g2Button, &QPushButton::released, this, &SavegameDialog::openG2Save);

		l->addLayout(hl);
	}

	QLineEdit * filterEdit = new QLineEdit(this);
	l->addWidget(filterEdit);

	QTableView * tv = new QTableView(this);
	_model = new QStandardItemModel(tv);
	QSortFilterProxyModel * filterModel = new QSortFilterProxyModel(tv);
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

	QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, Qt::Orientation::Horizontal, this);
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
	const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectSave"), _gothicDirectory, "SAVEDAT.SAV");
	if (!path.isEmpty()) {
		_openedFile = path;
		_savegameManager->load(path);
		updateView();
	}
}

void SavegameDialog::openG2Save() {
	const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectSave"), _gothic2Directory, "SAVEDAT.SAV");
	if (!path.isEmpty()) {
		_openedFile = path;
		_savegameManager->load(path);
		updateView();
	}
}

void SavegameDialog::save() {
	_savegameManager->save(_openedFile, _variables);
}

void SavegameDialog::itemChanged(QStandardItem * itm) {
	if (itm->model() == _model) {
		_variables[itm->index().row()].changed = _variables[itm->index().row()].value != itm->data(Qt::DisplayRole).toInt();
		_variables[itm->index().row()].value = itm->data(Qt::DisplayRole).toInt();
	}
}

void SavegameDialog::updateView() {
	_variables = _savegameManager->getVariables();
	_model->clear();
	for (const Variable & v : _variables) {
		QStandardItem * itmName = new QStandardItem(QString::fromStdString(v.name));
		itmName->setEditable(false);
		QStandardItem * itmValue = new QStandardItem();
		itmValue->setEditable(true);
		itmValue->setData(v.value, Qt::DisplayRole);
		_model->appendRow(QList<QStandardItem *>() << itmName << itmValue);
	}
}

void SavegameDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/SavegameDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/SavegameDialogGeometry");
	}
}

void SavegameDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/SavegameDialogGeometry", saveGeometry());
}
