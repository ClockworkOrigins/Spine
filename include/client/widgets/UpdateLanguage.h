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

#pragma once

#include "client/widgets/GeneralSettingsWidget.h"

#include "clockUtils/log/Log.h"

#define UPDATELANGUAGESETTEXT(targetWidget, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setText(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETTEXTARG(targetWidget, textIdentifier, argument) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setText(QApplication::tr(textIdentifier).arg(argument)); })
#define UPDATELANGUAGESETTEXTEXT(targetWidget, textIdentifier, concatText) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setText(QApplication::tr(textIdentifier) + concatText); })
#define UPDATELANGUAGESETTOOLTIP(targetWidget, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setToolTip(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETTABTEXT(targetWidget, index, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setTabText(index, QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETTITLE(targetWidget, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setTitle(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETWINDOWTITLE(targetWidget, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setWindowTitle(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETPLACEHOLDERTEXT(targetWidget, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setPlaceholderText(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETPLACEHOLDERTEXTEXT(targetWidget, textIdentifier, concatText) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setPlaceholderText(QApplication::tr(textIdentifier) + concatText); })
#define UPDATELANGUAGESETITEMTEXT(targetWidget, index, textIdentifier) QObject::connect(spine::widgets::GeneralSettingsWidget::getInstance(), &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setItemText(index, QApplication::tr(textIdentifier)); })
