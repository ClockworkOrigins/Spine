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

#include "widgets/GeneralSpineSettingsWidget.h"

#include "Config.h"
#include "UpdateLanguage.h"

#include "common/SpineModules.h"

#include "models/SpineEditorModel.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	GeneralSpineSettingsWidget::GeneralSpineSettingsWidget(GeneralSettingsWidget * generalSettingsWidget, models::SpineEditorModel * model, QWidget * par) : QWidget(par), _model(model), _spineModuleCheckBoxes() {
		QVBoxLayout * vl = new QVBoxLayout();
		vl->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();
			QLabel * l = new QLabel(QApplication::tr("Name"), this);
			_modNameEdit = new QLineEdit(this);
			
			hl->addWidget(l);
			hl->addWidget(_modNameEdit);
			hl->addStretch(1);

			vl->addLayout(hl);
		}

		QGroupBox * modulesBox = new QGroupBox(QApplication::tr("Modules"), this);
		UPDATELANGUAGESETTITLE(generalSettingsWidget, modulesBox, "Modules");
		vl->addWidget(modulesBox);

		{
			QGridLayout * gl = new QGridLayout();
			modulesBox->setLayout(gl);

			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("UsernameModule"), modulesBox);
				gl->addWidget(cb, 0, 0);
				_spineModuleCheckBoxes.insert(common::SpineModules::GetCurrentUsername, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("AchievementModule"), modulesBox);
				gl->addWidget(cb, 0, 1);
				_spineModuleCheckBoxes.insert(common::SpineModules::Achievements, cb);
				connect(cb, SIGNAL(stateChanged(int)), this, SIGNAL(changedAchievementState(int)));
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("ScoresModule"), modulesBox);
				gl->addWidget(cb, 0, 2);
				_spineModuleCheckBoxes.insert(common::SpineModules::Scores, cb);
				connect(cb, SIGNAL(stateChanged(int)), this, SIGNAL(changedScoreState(int)));
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("MultiplayerModule"), modulesBox);
				gl->addWidget(cb, 1, 0);
				_spineModuleCheckBoxes.insert(common::SpineModules::Multiplayer, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("OverallSaveModule"), modulesBox);
				gl->addWidget(cb, 1, 1);
				_spineModuleCheckBoxes.insert(common::SpineModules::OverallSave, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("GamepadModule"), modulesBox);
				gl->addWidget(cb, 1, 2);
				_spineModuleCheckBoxes.insert(common::SpineModules::Gamepad, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("FriendsModule"), modulesBox);
				gl->addWidget(cb, 2, 0);
				_spineModuleCheckBoxes.insert(common::SpineModules::Friends, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("StatisticsModule"), modulesBox);
				gl->addWidget(cb, 2, 1);
				_spineModuleCheckBoxes.insert(common::SpineModules::Statistics, cb);
			}
		}

		setLayout(vl);
	}

	GeneralSpineSettingsWidget::~GeneralSpineSettingsWidget() {
	}

	void GeneralSpineSettingsWidget::save() {
		_model->setModName(_modNameEdit->text());
		int32_t modules = 0;
		if (_spineModuleCheckBoxes[common::SpineModules::GetCurrentUsername]->isChecked()) {
			modules |= common::SpineModules::GetCurrentUsername;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::Achievements]->isChecked()) {
			modules |= common::SpineModules::Achievements;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::Scores]->isChecked()) {
			modules |= common::SpineModules::Scores;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::Multiplayer]->isChecked()) {
			modules |= common::SpineModules::Multiplayer;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::OverallSave]->isChecked()) {
			modules |= common::SpineModules::OverallSave;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::Gamepad]->isChecked()) {
			modules |= common::SpineModules::Gamepad;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::Friends]->isChecked()) {
			modules |= common::SpineModules::Friends;
		}
		if (_spineModuleCheckBoxes[common::SpineModules::Statistics]->isChecked()) {
			modules |= common::SpineModules::Statistics;
		}
		_model->setModules(modules);
	}

	void GeneralSpineSettingsWidget::updateFromModel() {
		_modNameEdit->setText(_model->getModName());

		const int32_t modules = _model->getModules();
		_spineModuleCheckBoxes[common::SpineModules::GetCurrentUsername]->setChecked(modules & common::SpineModules::GetCurrentUsername);
		_spineModuleCheckBoxes[common::SpineModules::Achievements]->setChecked(modules & common::SpineModules::Achievements);
		_spineModuleCheckBoxes[common::SpineModules::Scores]->setChecked(modules & common::SpineModules::Scores);
		_spineModuleCheckBoxes[common::SpineModules::Multiplayer]->setChecked(modules & common::SpineModules::Multiplayer);
		_spineModuleCheckBoxes[common::SpineModules::OverallSave]->setChecked(modules & common::SpineModules::OverallSave);
		_spineModuleCheckBoxes[common::SpineModules::Gamepad]->setChecked(modules & common::SpineModules::Gamepad);
		_spineModuleCheckBoxes[common::SpineModules::Friends]->setChecked(modules & common::SpineModules::Friends);
		_spineModuleCheckBoxes[common::SpineModules::Statistics]->setChecked(modules & common::SpineModules::Statistics);
	}

} /* namespace widgets */
} /* namespace spine */
