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

#ifndef __SPINE_WIDGETS_DEVELOPERSETTINGSWIDGET_H__
#define __SPINE_WIDGETS_DEVELOPERSETTINGSWIDGET_H__

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;
class QSettings;

namespace spine {
namespace common {
	enum class GothicVersion;
} /* namespace common */
namespace widgets {

	class DeveloperSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		DeveloperSettingsWidget(QSettings * iniParser, QWidget * par);
		~DeveloperSettingsWidget();

		void saveSettings();
		void rejectSettings();

		bool isDeveloperModeActive() const;
		bool isZSpyActive() const;

		QString getPath(int id) const;
		common::GothicVersion getGothicVersion(int id) const;

	signals:
		void developerModeChanged(bool);
		void zSpyChanged(bool);

	public slots:
		void changedDeveloperMode();

	private slots:
		void openFileDialog();
		void changedGothic(int checkState);
		void changedGothic2(int checkState);

	private:
		struct DevPath {
			QLineEdit * lineEdit;
			QPushButton * pushButton;
			QCheckBox * g1Box;
			QCheckBox * g2Box;
		};
		QSettings * _iniParser;
		QCheckBox * _developerModeCheckbox;
		QCheckBox * _zSpyCheckbox;
		QList<DevPath> _devPaths;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_DEVELOPERSETTINGSWIDGET_H__ */
