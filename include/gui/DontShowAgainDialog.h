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
// Copyright 2022 Clockwork Origins

#pragma once

#include <QDialog>

class QCheckBox;

namespace spine {
namespace gui {

	class DontShowAgainDialog : public QDialog {
		Q_OBJECT

	public:
		DontShowAgainDialog(QString title, QString text, const QString & settingsKey, QWidget * par);
		~DontShowAgainDialog();

        bool canShow() const;

	private:
        QString _settingsKey;
		QCheckBox * _dontShowAgainBox;
	};

} /* namespace gui */
} /* namespace spine */
