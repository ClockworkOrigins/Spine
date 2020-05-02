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
// Copyright 2019 Clockwork Origins

#pragma once

#include "launcher/ILauncher.h"

#include <QMap>
#include <QPixmap>
#include <QProcess>
#include <QString>

class QCheckBox;
class QGridLayout;
class QGroupBox;
class QNetworkAccessManager;
class QSlider;

namespace spine {
namespace client {
	enum class InstallMode;
}
namespace launcher {

	class Gothic1And2Launcher : public ILauncher {
		Q_OBJECT
		Q_INTERFACES(spine::launcher::ILauncher)

	public:
		void init() override;

		bool supportsGame(common::GameType gothic) const override = 0;
		bool supportsModAndIni(int32_t modID, const QString & iniFile) const override;

		virtual void setDirectory(const QString & directory);

		void setHideIncompatible(bool enabled);

	signals:
		void installMod(int, int, client::InstallMode);
		void receivedCompatibilityList(int, std::vector<int32_t>, std::vector<int32_t>);
		void changeSplashMessage(QString, int, QColor);

	protected slots:
		void finishedMod(int exitCode, QProcess::ExitStatus exitStatus);

	private slots:
		void startSpacer();
		void finishedSpacer();
		void updateCompatibilityList(int modID, std::vector<int32_t> incompatiblePatches, std::vector<int32_t> forbiddenPatches);
		void changedPatchState();
		void errorOccurred(QProcess::ProcessError error);

	protected:
		QString _directory;

		bool _developerModeActive = false;

		void createWidget() override;
		virtual common::GameType getGothicVersion() const = 0;

		virtual QString getExecutable() const = 0;

		virtual void startViaSteam(QStringList arguments) = 0;

		virtual bool canBeStartedWithSteam() const = 0;

		virtual QPixmap getDefaultIcon() const = 0;

		virtual void modFinished() {}

	private:
		QPushButton * _startSpacerButton = nullptr;
		QLabel * _adminInfoLabel = nullptr;
		QLabel * _nameLabel = nullptr;
		QLabel * _versionLabel = nullptr;
		QLabel * _teamLabel = nullptr;
		QLabel * _contactLabel = nullptr;
		QLabel * _homepageLabel = nullptr;
		QGroupBox * _patchGroup = nullptr;
		QGridLayout * _patchLayout = nullptr;
		QGroupBox * _pdfGroup = nullptr;
		QVBoxLayout * _pdfLayout = nullptr;

		QCheckBox * _compileScripts = nullptr;
		QCheckBox * _startupWindowed = nullptr;
		QCheckBox * _convertTextures = nullptr;
		QCheckBox *_noSound = nullptr;
		QCheckBox * _noMusic = nullptr;
		QLabel * _zSpyLabel = nullptr;
		QSlider * _zSpyLevel = nullptr;
		
		QStringList _copiedFiles;
		QStringList _skippedFiles;
		QString _lastBaseDir;

		QList<QCheckBox *> _patchList;
		QList<QLabel *> _pdfList;

		QMap<QCheckBox *, int32_t> _checkboxPatchIDMapping;

		QList<std::tuple<QString, QString, QString>> _gothicIniBackup;
		QList<std::tuple<QString, QString, QString>> _systempackIniBackup;

		int _patchCounter = 0;

		bool _hideIncompatible = true;

		QNetworkAccessManager * _networkAccessManager = nullptr;

		Qt::WindowStates _oldWindowState;

		QColor _splashTextColor;

		int _gmpCounterBackup = 0;

		QStringList _systempackPreLoads;
		QStringList _unionPlugins;

		bool _zSpyActivated = false;
		
		QMap<QString, std::tuple<QString, int32_t>> _parsedInis;

		bool _running;

		void start() override;
		
		void setDeveloperMode(bool enabled) override;

		void restoreSettings() override;
		void saveSettings() override;

		void removeEmptyDirs() const;

		void updateModStats() override;
		
		bool prepareModStart(QString * usedExecutable, QStringList * backgroundExecutables, bool * newGMP, QSet<QString> * dependencies, bool * renderer);

		void updateView(int modID, const QString & iniFile) override;
		void removeModFiles();

		void checkToolCfg(QString path, QStringList * backgroundExecutables, bool * newGMP);
		
		void collectDependencies(int modID, QSet<QString> * dependencies, QSet<QString> * forbidden);
		void prepareForNinja();
		void updatePlugins(int modID);

		bool linkOrCopyFolder(QString sourcePath, QString destinationPath);
		bool canSkipFile(const QString & filename) const;

		void emitSplashMessage(QString message);

		QString getOverallSavePath() const override;

		void syncAdditionalTimes(int duration) override;

		void setZSpyActivated(bool enabled) override;
		
		void parseMods();
		void parseMods(QString baseDir);
		void parseInstalledMods();
		void parseMod(QString folder);
		void parseIni(QString file);

		void updateModel(QStandardItemModel * model) override;

		void finishedInstallation(int modID, int packageID, bool success) override;
	};

} /* namespace launcher */
} /* namespace spine */
