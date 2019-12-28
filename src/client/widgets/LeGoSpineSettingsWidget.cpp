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

#include "widgets/LeGoSpineSettingsWidget.h"

#include "UpdateLanguage.h"

#include "models/SpineEditorModel.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	LeGoSpineSettingsWidget::LeGoSpineSettingsWidget(models::SpineEditorModel * model, QWidget * par) : QWidget(par), _model(model), _legoModuleCheckBoxes(), _achievementsEnabled(false), _gamepadEnabled(false) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QGroupBox * modulesBox = new QGroupBox(QApplication::tr("Modules"), this);
		UPDATELANGUAGESETTITLE(modulesBox, "Modules");
		l->addWidget(modulesBox);

		{
			QGridLayout * gl = new QGridLayout();
			modulesBox->setLayout(gl);

			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoPrintSModule"), modulesBox);
				gl->addWidget(cb, 0, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::PrintS, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoHookEngineModule"), modulesBox);
				gl->addWidget(cb, 0, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::HookEngine, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoAIFunctionModule"), modulesBox);
				gl->addWidget(cb, 0, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::AI_Function, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoTrialogeModule"), modulesBox);
				gl->addWidget(cb, 1, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Trialoge, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoDialoggesturesModule"), modulesBox);
				gl->addWidget(cb, 1, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Dialoggestures, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoFrameFunctionsModule"), modulesBox);
				gl->addWidget(cb, 1, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::FrameFunctions, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoCursorModule"), modulesBox);
				gl->addWidget(cb, 2, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Cursor, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoFocusnamesModule"), modulesBox);
				gl->addWidget(cb, 2, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Focusnames, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoRandomModule"), modulesBox);
				gl->addWidget(cb, 2, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Random, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoBloodsplatsModule"), modulesBox);
				gl->addWidget(cb, 3, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Bloodsplats, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoSavesModule"), modulesBox);
				gl->addWidget(cb, 3, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Saves, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoPermMemModule"), modulesBox);
				gl->addWidget(cb, 3, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::PermMem, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoAnim8Module"), modulesBox);
				gl->addWidget(cb, 4, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Anim8, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoViewModule"), modulesBox);
				gl->addWidget(cb, 4, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::View, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoInterfaceModule"), modulesBox);
				gl->addWidget(cb, 4, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Interface, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoBarsModule"), modulesBox);
				gl->addWidget(cb, 5, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Bars, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoButtonsModule"), modulesBox);
				gl->addWidget(cb, 5, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Buttons, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoTimerModule"), modulesBox);
				gl->addWidget(cb, 5, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Timer, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoEventHandlerModule"), modulesBox);
				gl->addWidget(cb, 6, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::EventHandler, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoGameStateModule"), modulesBox);
				gl->addWidget(cb, 6, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Gamestate, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoSpriteModule"), modulesBox);
				gl->addWidget(cb, 6, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Sprite, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoNamesModule"), modulesBox);
				gl->addWidget(cb, 7, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Names, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoConsoleCommandsModule"), modulesBox);
				gl->addWidget(cb, 7, 1);
				_legoModuleCheckBoxes.insert(models::LeGoModules::ConsoleCommands, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoBuffsModule"), modulesBox);
				gl->addWidget(cb, 7, 2);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Buffs, cb);
			}
			{
				QCheckBox * cb = new QCheckBox(QApplication::tr("LeGoRenderModule"), modulesBox);
				gl->addWidget(cb, 8, 0);
				_legoModuleCheckBoxes.insert(models::LeGoModules::Render, cb);
			}
		}

		setLayout(l);
	}

	LeGoSpineSettingsWidget::~LeGoSpineSettingsWidget() {
	}

	void LeGoSpineSettingsWidget::save() {
		int32_t modules = 0;
		if (_legoModuleCheckBoxes[models::LeGoModules::PrintS]->isChecked()) {
			modules |= models::LeGoModules::PrintS;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::HookEngine]->isChecked()) {
			modules |= models::LeGoModules::HookEngine;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::AI_Function]->isChecked()) {
			modules |= models::LeGoModules::AI_Function;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Trialoge]->isChecked()) {
			modules |= models::LeGoModules::Trialoge;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Dialoggestures]->isChecked()) {
			modules |= models::LeGoModules::Dialoggestures;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::FrameFunctions]->isChecked()) {
			modules |= models::LeGoModules::FrameFunctions;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Cursor]->isChecked()) {
			modules |= models::LeGoModules::Cursor;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Focusnames]->isChecked()) {
			modules |= models::LeGoModules::Focusnames;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Random]->isChecked()) {
			modules |= models::LeGoModules::Random;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Bloodsplats]->isChecked()) {
			modules |= models::LeGoModules::Bloodsplats;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Saves]->isChecked()) {
			modules |= models::LeGoModules::Saves;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::PermMem]->isChecked()) {
			modules |= models::LeGoModules::PermMem;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Anim8]->isChecked()) {
			modules |= models::LeGoModules::Anim8;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::View]->isChecked()) {
			modules |= models::LeGoModules::View;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Interface]->isChecked()) {
			modules |= models::LeGoModules::Interface;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Bars]->isChecked()) {
			modules |= models::LeGoModules::Bars;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Buttons]->isChecked()) {
			modules |= models::LeGoModules::Buttons;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Timer]->isChecked()) {
			modules |= models::LeGoModules::Timer;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::EventHandler]->isChecked()) {
			modules |= models::LeGoModules::EventHandler;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Gamestate]->isChecked()) {
			modules |= models::LeGoModules::Gamestate;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Sprite]->isChecked()) {
			modules |= models::LeGoModules::Sprite;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Names]->isChecked()) {
			modules |= models::LeGoModules::Names;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::ConsoleCommands]->isChecked()) {
			modules |= models::LeGoModules::ConsoleCommands;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Buffs]->isChecked()) {
			modules |= models::LeGoModules::Buffs;
		}
		if (_legoModuleCheckBoxes[models::LeGoModules::Render]->isChecked()) {
			modules |= models::LeGoModules::Render;
		}
		_model->setLeGoModules(modules);
	}

	void LeGoSpineSettingsWidget::achievementStateChanged(int state) {
		_achievementsEnabled = state == Qt::Checked;
		if (_achievementsEnabled) {
			_legoModuleCheckBoxes[models::LeGoModules::FrameFunctions]->setChecked(_achievementsEnabled || _gamepadEnabled);
			_legoModuleCheckBoxes[models::LeGoModules::View]->setChecked(_achievementsEnabled);
			_legoModuleCheckBoxes[models::LeGoModules::Interface]->setChecked(_achievementsEnabled);
		}
		_legoModuleCheckBoxes[models::LeGoModules::FrameFunctions]->setDisabled(_achievementsEnabled || _gamepadEnabled);
		_legoModuleCheckBoxes[models::LeGoModules::View]->setDisabled(_achievementsEnabled);
		_legoModuleCheckBoxes[models::LeGoModules::Interface]->setDisabled(_achievementsEnabled);
	}

	void LeGoSpineSettingsWidget::gamepadStateChanged(int state) {
		_gamepadEnabled = state == Qt::Checked;
		if (_gamepadEnabled) {
			_legoModuleCheckBoxes[models::LeGoModules::FrameFunctions]->setChecked(_achievementsEnabled || _gamepadEnabled);
		}
		_legoModuleCheckBoxes[models::LeGoModules::FrameFunctions]->setDisabled(_achievementsEnabled || _gamepadEnabled);
	}

	void LeGoSpineSettingsWidget::updateFromModel() {
		const int32_t modules = _model->getLeGoModules();
		_legoModuleCheckBoxes[models::LeGoModules::PrintS]->setChecked(modules & models::LeGoModules::PrintS);
		_legoModuleCheckBoxes[models::LeGoModules::HookEngine]->setChecked(modules & models::LeGoModules::HookEngine);
		_legoModuleCheckBoxes[models::LeGoModules::AI_Function]->setChecked(modules & models::LeGoModules::AI_Function);
		_legoModuleCheckBoxes[models::LeGoModules::Trialoge]->setChecked(modules & models::LeGoModules::Trialoge);
		_legoModuleCheckBoxes[models::LeGoModules::Dialoggestures]->setChecked(modules & models::LeGoModules::Dialoggestures);
		_legoModuleCheckBoxes[models::LeGoModules::FrameFunctions]->setChecked(modules & models::LeGoModules::FrameFunctions);
		_legoModuleCheckBoxes[models::LeGoModules::Cursor]->setChecked(modules & models::LeGoModules::Cursor);
		_legoModuleCheckBoxes[models::LeGoModules::Focusnames]->setChecked(modules & models::LeGoModules::Focusnames);
		_legoModuleCheckBoxes[models::LeGoModules::Random]->setChecked(modules & models::LeGoModules::Random);
		_legoModuleCheckBoxes[models::LeGoModules::Bloodsplats]->setChecked(modules & models::LeGoModules::Bloodsplats);
		_legoModuleCheckBoxes[models::LeGoModules::Saves]->setChecked(modules & models::LeGoModules::Saves);
		_legoModuleCheckBoxes[models::LeGoModules::PermMem]->setChecked(modules & models::LeGoModules::PermMem);
		_legoModuleCheckBoxes[models::LeGoModules::Anim8]->setChecked(modules & models::LeGoModules::Anim8);
		_legoModuleCheckBoxes[models::LeGoModules::View]->setChecked(modules & models::LeGoModules::View);
		_legoModuleCheckBoxes[models::LeGoModules::Interface]->setChecked(modules & models::LeGoModules::Interface);
		_legoModuleCheckBoxes[models::LeGoModules::Bars]->setChecked(modules & models::LeGoModules::Bars);
		_legoModuleCheckBoxes[models::LeGoModules::Buttons]->setChecked(modules & models::LeGoModules::Buttons);
		_legoModuleCheckBoxes[models::LeGoModules::Timer]->setChecked(modules & models::LeGoModules::Timer);
		_legoModuleCheckBoxes[models::LeGoModules::EventHandler]->setChecked(modules & models::LeGoModules::EventHandler);
		_legoModuleCheckBoxes[models::LeGoModules::Gamestate]->setChecked(modules & models::LeGoModules::Gamestate);
		_legoModuleCheckBoxes[models::LeGoModules::Sprite]->setChecked(modules & models::LeGoModules::Sprite);
		_legoModuleCheckBoxes[models::LeGoModules::Names]->setChecked(modules & models::LeGoModules::Names);
		_legoModuleCheckBoxes[models::LeGoModules::ConsoleCommands]->setChecked(modules & models::LeGoModules::ConsoleCommands);
		_legoModuleCheckBoxes[models::LeGoModules::Buffs]->setChecked(modules & models::LeGoModules::Buffs);
		_legoModuleCheckBoxes[models::LeGoModules::Render]->setChecked(modules & models::LeGoModules::Render);
	}

} /* namespace widgets */
} /* namespace spine */
