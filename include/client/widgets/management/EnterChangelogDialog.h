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
// Copyright 2020 Clockwork Origins

#pragma once

#include <QDialog>
#include <QMap>

class QCheckBox;
class QComboBox;
class QTextEdit;

namespace spine {
namespace client {
namespace widgets {

	class EnterChangelogDialog : public QDialog {
	public:
		EnterChangelogDialog(QWidget * par);

		bool isSavegameCompatible() const;
		QMap<QString, QString> getChangelogs() const;

	private:
		QCheckBox * _savegameCompatibleBox;
		QComboBox * _languageBox;
		QTextEdit * _textEdit;

		QString _currentLanguage;
		QMap<QString, QString> _changelogs;
	};

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
