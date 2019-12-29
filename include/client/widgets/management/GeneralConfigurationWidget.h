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

#pragma once

#include "ManagementCommon.h"

#include "widgets/management/IManagementWidget.h"

#include <QModelIndex>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QSpinBox;

namespace spine {
namespace gui {
	class WaitSpinner;
}
namespace client {
namespace widgets {

	class GeneralConfigurationWidget : public QWidget, public IManagementWidget {
		Q_OBJECT

	public:
		GeneralConfigurationWidget(QWidget * par);
		~GeneralConfigurationWidget();

		void updateModList(QList<ManagementMod> modList);
		void selectedMod(int index);
		void updateView() override;

	signals:
		void triggerInfoPage(int32_t);
		void removeSpinner();
		void loadedData(ManagementGeneralData);

	private slots:
		void updateMod();
		void openInfoPage();
		void updateData(ManagementGeneralData content);

	private:
		QList<ManagementMod> _mods;
		int _modIndex;
		QCheckBox * _enabledBox;
		QComboBox * _typeBox;
		QComboBox * _gothicVersionBox;
		QDateEdit * _releaseDateEdit;
		QSpinBox * _devDurationBox;
		gui::WaitSpinner * _waitSpinner;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
