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
// Copyright 2021 Clockwork Origins

#pragma once

#include "launcher/ILauncher.h"

#include <QProcess>

namespace spine {
namespace client {
	enum class InstallMode;
}
namespace launcher {

	class Gothic3Launcher : public ILauncher {
		Q_OBJECT

	public:
		Gothic3Launcher();

		void setDirectory(const QString & directory);

	signals:
		void installMod(int, int, client::InstallMode);

	private slots:
		void finishedGame(int exitCode, QProcess::ExitStatus exitStatus);

	private:
		QLabel * _nameLabel = nullptr;

		Qt::WindowStates _oldWindowState;

		QString _directory;
		
		bool supportsGame(common::GameType gameType) const override;
		bool supportsModAndIni(int32_t gameID, const QString & iniFile) const override;

		void start() override;
		
		void updateModStats() override;
		
		void updateView(int gameID, const QString & configFile) override;

		QString getOverallSavePath() const override;

		void updateModel(QStandardItemModel * model) override;

		void finishedInstallation(int gameID, int packageID, bool success) override;
		
		void createWidget() override;

		void parseGame(int gameID, int gameType);

		void updatedProject(int projectID) override;

		void startViaSteam(QStringList arguments);

		bool canBeStartedWithSteam() const;
	};

} /* namespace launcher */
} /* namespace spine */
