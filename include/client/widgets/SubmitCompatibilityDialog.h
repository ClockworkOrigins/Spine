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

#ifndef __SPINE_WIDGETS_SUBMITCOMPATIBILITYDIALOG_H__
#define __SPINE_WIDGETS_SUBMITCOMPATIBILITYDIALOG_H__

#include "common/Mod.h"

#include <QDialog>

class QListView;
class QRadioButton;
class QStandardItemModel;

namespace spine {
namespace widgets {

	class SubmitCompatibilityDialog : public QDialog {
		Q_OBJECT

	public:
		SubmitCompatibilityDialog();
		SubmitCompatibilityDialog(int32_t modID, int32_t patchID, common::GothicVersion gothicVersion);
		~SubmitCompatibilityDialog();

	signals:
		void receivedModList(std::vector<common::Mod>);

	private slots:
		void updateModList(std::vector<common::Mod> mods);
		void updateView();
		void selectIndex(const QModelIndex & idx);
		void selectPatchIndex(const QModelIndex & idx);
		void accept() override;
		void changedHiddenState(int state);

	private:
		QRadioButton * _g1Button;
		QRadioButton * _g2Button;
		QListView * _modView;
		QListView * _patchView;
		QRadioButton * _compatibleButton;
		QRadioButton * _notCompatibleButton;
		QStandardItemModel * _modList;
		QStandardItemModel * _patchList;
		QPushButton * _submitButton;
		QList<common::Mod> _g1Mods;
		QList<common::Mod> _g1Patches;
		QList<common::Mod> _g2Mods;
		QList<common::Mod> _g2Patches;
		QList<common::Mod> _currentMods;
		QList<common::Mod> _currentPatches;
		QList<common::Mod> _filteredPatches;

		bool _showSubmitted;

		int32_t _modID;
		int32_t _patchID;
		common::GothicVersion _gothicVersion;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_SUBMITCOMPATIBILITYDIALOG_H__ */
