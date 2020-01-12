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

#include <QColor>
#include <QMap>
#include <QProcess>
#include <QString>

class QCheckBox;
class QGridLayout;
class QGroupBox;
class QNetworkAccessManager;
class QSlider;

namespace spine {
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
		void installMod(int);
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

		void createWidget() override;
		virtual common::GameType getGothicVersion() const = 0;

		virtual QString getExecutable() const = 0;

		virtual void startViaSteam(QStringList arguments) = 0;

		virtual bool canBeStartedWithSteam() const = 0;

	private:
		QPushButton * _startSpacerButton;
		QLabel * _adminInfoLabel;
		QLabel * _nameLabel;
		QLabel * _versionLabel;
		QLabel * _teamLabel;
		QLabel * _contactLabel;
		QLabel * _homepageLabel;
		QGroupBox * _patchGroup;
		QGridLayout * _patchLayout;
		QGroupBox * _pdfGroup;
		QVBoxLayout * _pdfLayout;

		QCheckBox * _compileScripts;
		QCheckBox * _startupWindowed;
		QCheckBox * _convertTextures;
		QCheckBox *_noSound;
		QCheckBox * _noMusic;
		QLabel * _zSpyLabel;
		QSlider * _zSpyLevel;
		
		QStringList _copiedFiles;
		QStringList _skippedFiles;
		QString _lastBaseDir;

		bool _developerModeActive;

		QList<QCheckBox *> _patchList;
		QList<QLabel *> _pdfList;

		QMap<QCheckBox *, int32_t> _checkboxPatchIDMapping;

		QList<std::tuple<QString, QString, QString>> _gothicIniBackup;
		QList<std::tuple<QString, QString, QString>> _systempackIniBackup;

		int _patchCounter;

		bool _hideIncompatible;

		QNetworkAccessManager * _networkAccessManager;

		Qt::WindowStates _oldWindowState;

		QColor _splashTextColor;

		int _gmpCounterBackup;

		QStringList _systempackPreLoads;
		QStringList _unionPlugins;

		bool _zSpyActivated;

		void start() override;
		
		void setDeveloperMode(bool enabled) override;

		void restoreSettings() override;
		void saveSettings() override;

		void removeEmptyDirs() const;

		void updateModStats() override;
		
		bool prepareModStart(QString * usedExecutable, QStringList * backgroundExecutables, bool * newGMP, QSet<QString> * dependencies);

		void updateView(int modID, const QString & iniFile) override;
		void removeModFiles();

		void checkToolCfg(QString path, QStringList * backgroundExecutables, bool * newGMP);

		bool isAllowedSymlinkSuffix(QString suffix) const;
		
		void collectDependencies(int modID, QSet<QString> * dependencies, QSet<QString> * forbidden);
		void prepareForNinja();
		void updatePlugins(int modID);

		bool linkOrCopyFile(QString sourcePath, QString destinationPath);
		bool canSkipFile(const QString & filename) const;

		void emitSplashMessage(QString message);

		QString getOverallSavePath() const override;

		void syncAdditionalTimes(int duration) override;

		void setZSpyActivated(bool enabled) override;
	};

} /* namespace launcher */
} /* namespace spine */
