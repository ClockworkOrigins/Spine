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

#include <QWidget>

class QCheckBox;
class QComboBox;
class QSettings;
class QSpinBox;

namespace spine {
namespace widgets {

	class GeneralSettingsWidget : public QWidget {
		Q_OBJECT

	public:
		static bool skipExitCheckbox;

		explicit GeneralSettingsWidget(QWidget * par);

		static GeneralSettingsWidget * getInstance();

		void saveSettings();
		void rejectSettings();

		bool getHideIncompatible() const;

		QString getLanguage() const;

	signals:
		void languageChanged(QString);
		void changedHideIncompatible(bool);
		void resetModUpdates();

	private slots:
		void changedStyle(QString styleName);
		void reactivateModUpdates();

	private:
		QComboBox * _languageComboBox;
		QComboBox * _styleComboBox;
		QCheckBox * _hideIncompatibleCheckBox;
		QCheckBox * _extendedLoggingCheckBox;
		QCheckBox * _betaModeCheckBox;
		QCheckBox * _skipExitCheckBox;
		QCheckBox * _developerModeCheckBox;

		bool _devModeActive;

		static GeneralSettingsWidget * instance;
	};

} /* namespace widgets */
} /* namespace spine */
