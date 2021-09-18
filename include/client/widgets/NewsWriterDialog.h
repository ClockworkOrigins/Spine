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

#include <QDialog>
#include <QMap>

#include "common/MessageStructs.h"
#include "common/Mod.h"

class QCheckBox;
class QComboBox;
class QDateEdit;
class QGridLayout;
class QLineEdit;
class QTextEdit;

namespace spine {
namespace widgets {

	class NewsWidget;

	class NewsWriterDialog : public QDialog {
		Q_OBJECT

	public:
		NewsWriterDialog(QWidget * par);

	signals:
		void receivedModList(std::vector<common::Mod>);
		void refresh();

	private slots:
		void changedNews();
		void accept() override;
		void updateModList(std::vector<common::Mod> mods);
		void addImage();
		void changedLanguage();

	private:
		NewsWidget * _newsPreviewWidget;
		QLineEdit * _titleEdit;
		QDateEdit * _dateEdit;
		QTextEdit * _bodyEdit;
		QGridLayout * _modListLayout;
		QLineEdit * _imageReferencesEdit;
		QList<QCheckBox *> _mods;
		QComboBox * _languageBox;

		typedef struct {
			QString title;
			QString body;
		} NewsEntry;

		QMap<common::Language, NewsEntry> _newsEntries;

		void showEvent(QShowEvent * evt) override;
		void requestMods();
	};

} /* namespace widgets */
} /* namespace spine */
