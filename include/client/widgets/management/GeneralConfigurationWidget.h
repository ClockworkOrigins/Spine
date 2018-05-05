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

#ifndef __SPINE_WIDGETS_MANAGEMENT_GENERALCONFIGURATIONWIDGET_H__
#define __SPINE_WIDGETS_MANAGEMENT_GENERALCONFIGURATIONWIDGET_H__

#include "common/MessageStructs.h"

#include <QModelIndex>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QSpinBox;

namespace spine {
namespace widgets {

	class GeneralConfigurationWidget : public QWidget {
		Q_OBJECT

	public:
		GeneralConfigurationWidget(QWidget * par);
		~GeneralConfigurationWidget();

		void updateModList(std::vector<common::SendModManagementMessage::ModManagement> modList);
		void selectedMod(int index);

	signals:
		void triggerInfoPage(int32_t);

	private slots:
		void updateMod();
		void openInfoPage();

	private:
		std::vector<common::SendModManagementMessage::ModManagement> _mods;
		int _modIndex;
		QCheckBox * _enabledBox;
		QComboBox * _typeBox;
		QComboBox * _gothicVersionBox;
		QDateEdit * _releaseDateEdit;
		QSpinBox * _devDurationBox;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MANAGEMENT_GENERALCONFIGURATIONWIDGET_H__ */
