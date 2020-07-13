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

#include "common/ModUpdate.h"

#include <QDialog>
#include <QMap>

class QCheckBox;
class QLabel;
class QMainWindow;
class QVBoxLayout;

namespace spine {
namespace widgets {

	class ModUpdateDialog : public QDialog {
		Q_OBJECT

	public:
		ModUpdateDialog(QMainWindow * mainWindow);

	signals:
		void receivedMods(std::vector<common::ModUpdate>, bool);
		void updateStarted(int);
		void updatedMod(int);

	public slots:
		void loginChanged();
		void checkForUpdate();
		void checkForUpdate(int32_t modID, bool forceAccept);

	private slots:
		void updateModList(std::vector<common::ModUpdate> updates, bool);
		void accept() override;
		void reject() override;

	private:
		struct ModFile {
			int32_t packageID = -1;
			int32_t modID = -1;
			std::string file;
			std::string hash;
			QString fileserver;

			ModFile() {}
			
			ModFile(std::string i, std::string s1, std::string s2) : packageID(-1), modID(std::stoi(i)), file(s1), hash(s2) {}
			
			ModFile(int i, std::string s1, std::string s2, int32_t p, QString fs) : packageID(p), modID(int32_t(i)), file(s1), hash(s2), fileserver(fs) {}
		};

		QMainWindow * _mainWindow;
		QLabel * _infoLabel;
		QVBoxLayout * _checkBoxLayout;
		std::vector<common::ModUpdate> _updates;
		std::vector<QCheckBox *> _checkBoxes;
		QCheckBox * _dontShowAgain;
		bool _running;
		bool _lastTimeRejected;
		QMap<int32_t, QString> _oldVersions;

		void hideUpdates(QList<common::ModUpdate> hides) const;
		bool hasChanges(common::ModUpdate mu) const;

		void unzippedArchive(QString archive, QList<QPair<QString, QString>> files, ModFile mf, QSharedPointer<QList<ModFile>> installFiles, QSharedPointer<QList<ModFile>> newFiles, QSharedPointer<QList<ModFile>> removeFiles);
	};

} /* namespace widgets */
} /* namespace spine */
