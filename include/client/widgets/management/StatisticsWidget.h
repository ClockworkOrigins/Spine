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

#ifndef __SPINE_WIDGETS_MANAGEMENT_STATISTICSWIDGET_H__
#define __SPINE_WIDGETS_MANAGEMENT_STATISTICSWIDGET_H__

#include "ManagementCommon.h"

#include "common/MessageStructs.h"

#include <QModelIndex>
#include <QWidget>

class QLabel;
class QVBoxLayout;

namespace spine {
namespace widgets {

	class StatisticsWidget : public QWidget {
		Q_OBJECT

	public:
		StatisticsWidget(QWidget * par);
		~StatisticsWidget();

		void updateModList(QList<client::ManagementMod> modList);
		void selectedMod(int index);

	private:
		QList<client::ManagementMod> _mods;
		int _modIndex;
		QList<QLabel *> _labelList;
		QVBoxLayout * _downloadsLayout;
		QVBoxLayout * _playersLayout;
		QVBoxLayout * _playtimesLayout;
		QVBoxLayout * _achievementsLayout;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_MANAGEMENT_STATISTICSWIDGET_H__ */
