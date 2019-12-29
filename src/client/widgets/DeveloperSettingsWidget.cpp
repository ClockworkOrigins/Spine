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

#include "widgets/DeveloperSettingsWidget.h"

#include "DirValidator.h"

#include "common/GothicVersion.h"

#include "utils/Config.h"

#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

DeveloperSettingsWidget::DeveloperSettingsWidget(QWidget * par) : QWidget(par), _developerModeCheckbox(nullptr), _zSpyCheckbox(nullptr), _devPaths() {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QLabel * label = new QLabel(QApplication::tr("DeveloperModeInfo"), this);
		label->setWordWrap(true);
		UPDATELANGUAGESETTEXT(label, "DeveloperModeInfo");

		l->addWidget(label);
	}
	{
		const bool developerMode = Config::IniParser->value("DEVELOPER/Active", false).toBool();

		_developerModeCheckbox = new QCheckBox(QApplication::tr("ActivateDeveloperMode"), this);
		UPDATELANGUAGESETTEXT(_developerModeCheckbox, "ActivateDeveloperMode");
		_developerModeCheckbox->setChecked(developerMode);

		l->addWidget(_developerModeCheckbox);
	}
	{
		const bool zSpy = Config::IniParser->value("DEVELOPER/ZSpy", false).toBool();

		_zSpyCheckbox = new QCheckBox(QApplication::tr("ActivateZSpy"), this);
		UPDATELANGUAGESETTEXT(_developerModeCheckbox, "ActivateZSpy");
		_zSpyCheckbox->setChecked(zSpy);

		l->addWidget(_zSpyCheckbox);
	}
	for (int i = 0; i < 10; i++) {
		QGridLayout * hl = new QGridLayout();

		const QString path = Config::IniParser->value("DEVELOPER/Path_" + QString::number(i), "").toString();
		const common::GothicVersion gv = common::GothicVersion(Config::IniParser->value("DEVELOPER/Gothic_" + QString::number(i), int(common::GothicVersion::GOTHIC2)).toInt());

		QLabel * gothicPathLabel = new QLabel(QApplication::tr("DevPath").arg(i), this);
		UPDATELANGUAGESETTEXTARG(gothicPathLabel, "DevPath", i);
		QLineEdit * gothicPathLineEdit = new QLineEdit(path, this);
		gothicPathLineEdit->setValidator(new DirValidator());
		gothicPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		QPushButton * gothicPathPushButton = new QPushButton("...", this);
		QCheckBox * gothic1CheckBox = new QCheckBox(QApplication::tr("Gothic"), this);
		gothic1CheckBox->setChecked(gv == common::GothicVersion::GOTHIC);
		QCheckBox * gothic2CheckBox = new QCheckBox(QApplication::tr("Gothic2"), this);
		gothic2CheckBox->setChecked(gv == common::GothicVersion::GOTHIC2);
		hl->addWidget(gothicPathLabel, 0, 0);
		hl->addWidget(gothicPathLineEdit, 0, 1);
		hl->addWidget(gothicPathPushButton, 0, 2);
		hl->addWidget(gothic1CheckBox, 0, 3);
		hl->addWidget(gothic2CheckBox, 0, 4);
		connect(gothicPathPushButton, SIGNAL(clicked()), this, SLOT(openFileDialog()));
		connect(gothic1CheckBox, SIGNAL(stateChanged(int)), this, SLOT(changedGothic(int)));
		connect(gothic2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(changedGothic2(int)));

		DevPath dp {};
		dp.lineEdit = gothicPathLineEdit;
		dp.pushButton = gothicPathPushButton;
		dp.g1Box = gothic1CheckBox;
		dp.g2Box = gothic2CheckBox;
		_devPaths.append(dp);

		hl->setAlignment(Qt::AlignLeft);

		l->addLayout(hl);
	}

	setLayout(l);
}

DeveloperSettingsWidget::~DeveloperSettingsWidget() {
}

void DeveloperSettingsWidget::saveSettings() {
	Config::IniParser->setValue("DEVELOPER/Active", _developerModeCheckbox->isChecked());
	Config::IniParser->setValue("DEVELOPER/ZSpy", _zSpyCheckbox->isChecked());
	for (int i = 0; i < _devPaths.size(); i++) {
		Config::IniParser->setValue("DEVELOPER/Path_" + QString::number(i), _devPaths[i].lineEdit->text());
		Config::IniParser->setValue("DEVELOPER/Gothic_" + QString::number(i), _devPaths[i].g1Box->isChecked() ? int(common::GothicVersion::GOTHIC) : int(common::GothicVersion::GOTHIC2));
	}
	emit developerModeChanged(_developerModeCheckbox->isChecked());
	emit zSpyChanged(_zSpyCheckbox->isChecked());
}

void DeveloperSettingsWidget::rejectSettings() {
	{
		const bool developerMode = Config::IniParser->value("DEVELOPER/Active", false).toBool();
		_developerModeCheckbox->setChecked(developerMode);
	}
	{
		const bool zSpy = Config::IniParser->value("DEVELOPER/ZSpy", false).toBool();
		_zSpyCheckbox->setChecked(zSpy);
	}
	for (int i = 0; i < _devPaths.size(); i++) {
		const QString path = Config::IniParser->value("DEVELOPER/Path_" + QString::number(i), "").toString();
		const common::GothicVersion gv = common::GothicVersion(Config::IniParser->value("DEVELOPER/Gothic_" + QString::number(i), int(common::GothicVersion::GOTHIC2)).toInt());

		_devPaths[i].lineEdit->setText(path);
		_devPaths[i].g1Box->setChecked(gv == common::GothicVersion::GOTHIC);
		_devPaths[i].g2Box->setChecked(gv == common::GothicVersion::GOTHIC2);
	}
}

bool DeveloperSettingsWidget::isDeveloperModeActive() const {
	return _developerModeCheckbox->isChecked();
}

bool DeveloperSettingsWidget::isZSpyActive() const {
	return _zSpyCheckbox->isChecked();
}

QString DeveloperSettingsWidget::getPath(int id) const {
	return _devPaths[id].lineEdit->text();
}

common::GothicVersion DeveloperSettingsWidget::getGothicVersion(int id) const {
	return _devPaths[id].g1Box->isChecked() ? common::GothicVersion::GOTHIC : common::GothicVersion::GOTHIC2;
}

void DeveloperSettingsWidget::changedDeveloperMode() {
	_developerModeCheckbox->setChecked(!_developerModeCheckbox->isChecked());
	Config::IniParser->setValue("DEVELOPER/Active", _developerModeCheckbox->isChecked());
	emit developerModeChanged(_developerModeCheckbox->isChecked());
}

void DeveloperSettingsWidget::openFileDialog() {
	QPushButton * pb = qobject_cast<QPushButton *>(sender());
	QLineEdit * le = nullptr;
	for (DevPath dp : _devPaths) {
		if (pb == dp.pushButton) {
			le = dp.lineEdit;
			break;
		}
	}
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothicDir"), le->text());
	if (!path.isEmpty()) {
		le->setText(path);
	}
}

void DeveloperSettingsWidget::changedGothic(int checkState) {
	QCheckBox * cb = qobject_cast<QCheckBox *>(sender());
	for (DevPath dp : _devPaths) {
		if (cb == dp.g1Box) {
			dp.g1Box->blockSignals(true);
			dp.g2Box->blockSignals(true);
			dp.g1Box->setChecked(checkState == Qt::Checked);
			dp.g2Box->setChecked(checkState != Qt::Checked);
			dp.g1Box->blockSignals(false);
			dp.g2Box->blockSignals(false);
			break;
		}
	}
}

void DeveloperSettingsWidget::changedGothic2(int checkState) {
	QCheckBox * cb = qobject_cast<QCheckBox *>(sender());
	for (DevPath dp : _devPaths) {
		if (cb == dp.g1Box) {
			dp.g1Box->blockSignals(true);
			dp.g2Box->blockSignals(true);
			dp.g1Box->setChecked(checkState != Qt::Checked);
			dp.g2Box->setChecked(checkState == Qt::Checked);
			dp.g1Box->blockSignals(false);
			dp.g2Box->blockSignals(false);
			break;
		}
	}
}
