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

#ifndef __SPINE_WIDGETS_GOTHIC1INIPAGES_ENGINEPAGE_H__
#define __SPINE_WIDGETS_GOTHIC1INIPAGES_ENGINEPAGE_H__

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {
namespace g1 {

	class EnginePage : public QWidget {
		Q_OBJECT

	public:
		EnginePage(QSettings * iniParser, QWidget * par);
		~EnginePage();

		void reject();
		void accept();

	private:
		QSettings * _iniParser;
		QSpinBox * _zTexCacheOutTimeMSec;
		QSpinBox * _zTexCacheSizeMaxByte;
	};

} /* namespace g1 */
} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_GOTHIC1INIPAGES_ENGINEPAGE_H__ */
