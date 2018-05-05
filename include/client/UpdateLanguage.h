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

#ifndef __SPINE_UPDATELANGUAGE_H__
#define __SPINE_UPDATELANGUAGE_H__

#include "clockUtils/log/Log.h"

#define UPDATELANGUAGESETTEXT(generalSettingsWidget, targetWidget, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setText(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETTEXTARG(generalSettingsWidget, targetWidget, textIdentifier, argument) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setText(QApplication::tr(textIdentifier).arg(argument)); })
#define UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, targetWidget, textIdentifier, concatText) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setText(QApplication::tr(textIdentifier) + concatText); })
#define UPDATELANGUAGESETTOOLTIP(generalSettingsWidget, targetWidget, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setToolTip(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETTABTEXT(generalSettingsWidget, targetWidget, index, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setTabText(index, QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETTITLE(generalSettingsWidget, targetWidget, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setTitle(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETWINDOWTITLE(generalSettingsWidget, targetWidget, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setWindowTitle(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETPLACEHOLDERTEXT(generalSettingsWidget, targetWidget, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setPlaceholderText(QApplication::tr(textIdentifier)); })
#define UPDATELANGUAGESETPLACEHOLDERTEXTEXT(generalSettingsWidget, targetWidget, textIdentifier, concatText) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setPlaceholderText(QApplication::tr(textIdentifier) + concatText); })
#define UPDATELANGUAGESETITEMTEXT(generalSettingsWidget, targetWidget, index, textIdentifier) connect(generalSettingsWidget, &spine::widgets::GeneralSettingsWidget::languageChanged, [=]() { LOGINFO(__FILE__ << ":" << __LINE__); targetWidget->setItemText(index, QApplication::tr(textIdentifier)); })

#endif /* __SPINE_UPDATELANGUAGE_H__ */
