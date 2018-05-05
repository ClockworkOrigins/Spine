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

#include "models/SpineEditorModel.h"

#include "common/GothicVersion.h"
#include "common/SpineModules.h"

#include <QDebug>
#include <QDirIterator>
#include <QProcessEnvironment>
#include <QTextStream>

namespace spine {
namespace models {

	SpineEditorModel::SpineEditorModel(QObject * par) : QObject(par), _path(), _modules(0), _achievementOrientation(AchievementOrientation::BottomRight), _achievementDisplayDuration(5000), _achievements(), _scores(), _gothicVersion(common::GothicVersion::GOTHIC2), _earthquakeVibration(true), _legoModules(0), _modName("MODNAME") {
	}

	void SpineEditorModel::setPath(QString path) {
		_path = path;
	}

	QString SpineEditorModel::getPath() const {
		return _path;
	}

	void SpineEditorModel::setGothicVersion(common::GothicVersion gothicVersion) {
		_gothicVersion = gothicVersion;
	}

	common::GothicVersion SpineEditorModel::getGothicVersion() const {
		return _gothicVersion;
	}

	void SpineEditorModel::load() {
		_achievements.clear();
		_scores.clear();
		{
			QDirIterator it(_path + "/_work/data/Scripts/Content/", QStringList() << "Startup.d", QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) {
				it.next();
				QFile f(it.filePath());
				if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream ts(&f);
					while (!ts.atEnd()) {
						QString line = ts.readLine();
						if (line.contains("Spine_Init", Qt::CaseSensitivity::CaseInsensitive)) {
							int32_t modules = 0;
							line = line.replace(QRegExp("[ \t]"), "");
							if (line.contains("SPINE_ALL", Qt::CaseSensitivity::CaseInsensitive)) {
								modules |= common::SpineModules::All;
								if (line.contains("&~SPINE_MODULE_GETCURRENTUSERNAME", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::GetCurrentUsername;
								}
								if (line.contains("&~SPINE_MODULE_ACHIEVEMENTS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::Achievements;
								}
								if (line.contains("&~SPINE_MODULE_SCORES", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::Scores;
								}
								if (line.contains("&~SPINE_MODULE_MULTIPLAYER", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::Multiplayer;
								}
								if (line.contains("&~SPINE_MODULE_OVERALLSAVE", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::OverallSave;
								}
								if (line.contains("&~SPINE_MODULE_GAMEPAD", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::Gamepad;
								}
								if (line.contains("&~SPINE_MODULE_FRIENDS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::Friends;
								}
								if (line.contains("&~SPINE_MODULE_STATISTICS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~common::SpineModules::Statistics;
								}
							} else {
								if (line.contains("SPINE_MODULE_GETCURRENTUSERNAME", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::GetCurrentUsername;
								}
								if (line.contains("SPINE_MODULE_ACHIEVEMENTS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::Achievements;
								}
								if (line.contains("SPINE_MODULE_SCORES", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::Scores;
								}
								if (line.contains("SPINE_MODULE_MULTIPLAYER", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::Multiplayer;
								}
								if (line.contains("SPINE_MODULE_OVERALLSAVE", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::OverallSave;
								}
								if (line.contains("SPINE_MODULE_GAMEPAD", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::Gamepad;
								}
								if (line.contains("SPINE_MODULE_FRIENDS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::Friends;
								}
								if (line.contains("SPINE_MODULE_STATISTICS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= common::SpineModules::Statistics;
								}
							}
							setModules(modules);
						} else if (line.contains("LeGo_Init", Qt::CaseSensitivity::CaseInsensitive)) {
							int32_t modules = 0;
							line = line.replace(QRegExp("[ \t]"), "");
							if (line.contains("LEGO_ALL", Qt::CaseSensitivity::CaseInsensitive)) {
								modules |= LeGoModules::All;
								if (line.contains("&~LeGo_PrintS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::PrintS;
								}
								if (line.contains("&~LeGo_HookEngine", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::HookEngine;
								}
								if (line.contains("&~LeGo_AI_Function", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::AI_Function;
								}
								if (line.contains("&~LeGo_Trialoge", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Trialoge;
								}
								if (line.contains("&~LeGo_Dialoggestures", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Dialoggestures;
								}
								if (line.contains("&~LeGo_FrameFunctions", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::FrameFunctions;
								}
								if (line.contains("&~LeGo_Cursor", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Cursor;
								}
								if (line.contains("&~LeGo_Focusnames", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Focusnames;
								}
								if (line.contains("&~LeGo_Random", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Random;
								}
								if (line.contains("&~LeGo_Bloodsplats", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Bloodsplats;
								}
								if (line.contains("&~LeGo_Saves", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Saves;
								}
								if (line.contains("&~LeGo_PermMem", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::PermMem;
								}
								if (line.contains("&~LeGo_Anim8", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Anim8;
								}
								if (line.contains("&~LeGo_View", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::View;
								}
								if (line.contains("&~LeGo_Interface", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Interface;
								}
								if (line.contains("&~LeGo_Bars", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Bars;
								}
								if (line.contains("&~LeGo_Buttons", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Buttons;
								}
								if (line.contains("&~LeGo_Timer", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Timer;
								}
								if (line.contains("&~LeGo_EventHandler", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::EventHandler;
								}
								if (line.contains("&~LeGo_Gamestate", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Gamestate;
								}
								if (line.contains("&~LeGo_Sprite", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Sprite;
								}
								if (line.contains("&~LeGo_Names", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Names;
								}
								if (line.contains("&~LeGo_ConsoleCommands", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::ConsoleCommands;
								}
								if (line.contains("&~LeGo_Buffs", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Buffs;
								}
								if (line.contains("&~LeGo_Render", Qt::CaseSensitivity::CaseInsensitive)) {
									modules &= ~LeGoModules::Render;
								}
							} else {
								if (line.contains("LeGo_PrintS", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::PrintS;
								}
								if (line.contains("LeGo_HookEngine", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::HookEngine;
								}
								if (line.contains("LeGo_AI_Function", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::AI_Function;
								}
								if (line.contains("LeGo_Trialoge", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Trialoge;
								}
								if (line.contains("LeGo_Dialoggestures", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Dialoggestures;
								}
								if (line.contains("LeGo_FrameFunctions", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::FrameFunctions;
								}
								if (line.contains("LeGo_Cursor", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Cursor;
								}
								if (line.contains("LeGo_Focusnames", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Focusnames;
								}
								if (line.contains("LeGo_Random", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Random;
								}
								if (line.contains("LeGo_Bloodsplats", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Bloodsplats;
								}
								if (line.contains("LeGo_Saves", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Saves;
								}
								if (line.contains("LeGo_PermMem", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::PermMem;
								}
								if (line.contains("LeGo_Anim8", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Anim8;
								}
								if (line.contains("LeGo_View", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::View;
								}
								if (line.contains("LeGo_Interface", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Interface;
								}
								if (line.contains("LeGo_Bars", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Bars;
								}
								if (line.contains("LeGo_Buttons", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Buttons;
								}
								if (line.contains("LeGo_Timer", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Timer;
								}
								if (line.contains("LeGo_EventHandler", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::EventHandler;
								}
								if (line.contains("LeGo_Gamestate", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Gamestate;
								}
								if (line.contains("LeGo_Sprite", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Sprite;
								}
								if (line.contains("LeGo_Names", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Names;
								}
								if (line.contains("LeGo_ConsoleCommands", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::ConsoleCommands;
								}
								if (line.contains("LeGo_Buffs", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Buffs;
								}
								if (line.contains("LeGo_Render", Qt::CaseSensitivity::CaseInsensitive)) {
									modules |= LeGoModules::Render;
								}
							}
							setLeGoModules(modules);
						}
					}
				}
			}
		}
		{
			QDirIterator it(_path + "/_work/data/Scripts/Content/", QStringList() << "Spine_UserConstants.d", QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) {
				it.next();
				QFile f(it.filePath());
				if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream ts(&f);
					int achievementCounter = 0;
					while (!ts.atEnd()) {
						QString line = ts.readLine();
						if (line.contains("SPINE_ACHIEVEMENTORIENTATION", Qt::CaseSensitivity::CaseInsensitive)) {
							if (line.contains("SPINE_TOPLEFT", Qt::CaseSensitivity::CaseInsensitive)) {
								_achievementOrientation = AchievementOrientation::TopLeft;
							}
							if (line.contains("SPINE_TOPRIGHT", Qt::CaseSensitivity::CaseInsensitive)) {
								_achievementOrientation = AchievementOrientation::TopRight;
							}
							if (line.contains("SPINE_BOTTOMLEFT", Qt::CaseSensitivity::CaseInsensitive)) {
								_achievementOrientation = AchievementOrientation::BottomLeft;
							}
							if (line.contains("SPINE_BOTTOMRIGHT", Qt::CaseSensitivity::CaseInsensitive)) {
								_achievementOrientation = AchievementOrientation::BottomRight;
							}
						} else if (line.contains("SPINE_ACHIEVEMENT_DISPLAY_TIME", Qt::CaseSensitivity::CaseInsensitive)) {
							line = line.replace(QRegExp("[ \t]"), "");
							line = line.split("=").back(); // 5000;//blafoo
							line = line.split(";").front(); // 5000
							_achievementDisplayDuration = line.toInt();
						} else if (line.contains("SPINE_ACHIEVEMENT_NAMES", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[\t\n]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList achievementNames = line.split("\",");
							if (achievementCounter < achievementNames.size()) {
								for (int i = achievementCounter; i < achievementNames.size(); i++) {
									_achievements.append(AchievementModel());
								}
								achievementCounter = achievementNames.size();
							}
							for (int i = 0; i < achievementCounter; i++) {
								_achievements[i].name = achievementNames[i].replace("\"", "").replace("&quot;", "\"");
							}
						} else if (line.contains("SPINE_ACHIEVEMENT_TEXTURES", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[ \t\n\"]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList achievementImages = line.split(",");
							if (achievementCounter < achievementImages.size()) {
								for (int i = achievementCounter; i < achievementImages.size(); i++) {
									_achievements.append(AchievementModel());
								}
								achievementCounter = achievementImages.size();
							}
							for (int i = 0; i < achievementImages.size(); i++) {
								QDirIterator itImage(_path + "/_work/data/Textures/", QStringList() << achievementImages[i], QDir::Files, QDirIterator::Subdirectories);
								_achievements[i].unlockedImage = (itImage.hasNext()) ? itImage.next() : "";
							}
						} else if (line.contains("SPINE_ACHIEVEMENT_DESCRIPTIONS", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[\t\n]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList achievementDescriptions = line.split("\",");
							if (achievementCounter < achievementDescriptions.size()) {
								for (int i = achievementCounter; i < achievementDescriptions.size(); i++) {
									_achievements.append(AchievementModel());
								}
								achievementCounter = achievementDescriptions.size();
							}
							for (int i = 0; i < achievementCounter; i++) {
								_achievements[i].description = achievementDescriptions[i].replace("\"", "").replace("&quot;", "\"");
							}
						} else if (line.contains("SPINE_ACHIEVEMENT_LOCKED", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[ \t\n\"]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList achievementImages = line.split(",");
							if (achievementCounter < achievementImages.size()) {
								for (int i = achievementCounter; i < achievementImages.size(); i++) {
									_achievements.append(AchievementModel());
								}
								achievementCounter = achievementImages.size();
							}
							for (int i = 0; i < achievementCounter; i++) {
								QDirIterator itImage(_path + "/_work/data/Textures/", QStringList() << achievementImages[i], QDir::Files, QDirIterator::Subdirectories);
								_achievements[i].lockedImage = (itImage.hasNext()) ? itImage.next() : "";
							}
						} else if (line.contains("SPINE_ACHIEVEMENT_HIDDEN", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[ \t\n\"]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList achievementHidden = line.split(",");
							if (achievementCounter < achievementHidden.size()) {
								for (int i = achievementCounter; i < achievementHidden.size(); i++) {
									_achievements.append(AchievementModel());
								}
								achievementCounter = achievementHidden.size();
							}
							for (int i = 0; i < achievementCounter; i++) {
								_achievements[i].hidden = achievementHidden[i].compare("TRUE", Qt::CaseSensitivity::CaseInsensitive) == 0;
							}
						} else if (line.contains("SPINE_ACHIEVEMENT_PROGRESS", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[ \t\n\"]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList achievementProgress = line.split(",");
							if (achievementCounter < achievementProgress.size()) {
								for (int i = achievementCounter; i < achievementProgress.size(); i++) {
									_achievements.append(AchievementModel());
								}
								achievementCounter = achievementProgress.size();
							}
							for (int i = 0; i < achievementCounter; i++) {
								_achievements[i].maxProgress = achievementProgress[i].toInt();
							}
						} else if (line.contains("MAX_ACHIEVEMENTS", Qt::CaseSensitivity::CaseInsensitive)) {
							line = line.replace(QRegExp("[ \t]"), "");
							line = line.split("=").back(); // 7;//blafoo
							line = line.split(";").front(); // 7
							achievementCounter = line.toInt();
							_achievements.clear();
							for (int i = 0; i < achievementCounter; i++) {
								_achievements.append(AchievementModel());
							}
						} else if (line.contains("SPINE_SCORE_NAMES", Qt::CaseSensitivity::CaseInsensitive)) {
							while (!line.contains("};")) {
								line += ts.readLine();
							}
							line = line.replace(QRegExp("[ \t\n]"), "");
							line = line.split("{").back(); // 7;//blafoo
							line = line.split("}").front(); // 7
							QStringList scoreNames = line.split("\",");
							for (int i = 0; i < scoreNames.size(); i++) {
								ScoreModel sm;
								sm.name = scoreNames[i].replace("\"", "").replace("&quot;", "\"");
								_scores.append(sm);
							}
						} else if (line.contains("SPINE_EARTHQUAKE_VIBRATION", Qt::CaseSensitivity::CaseInsensitive)) {
							line = line.replace(QRegExp("[ \t]"), "");
							line = line.split("=").back(); // TRUE;//blafoo
							line = line.split(";").front(); // TRUE
							_earthquakeVibration = line.toUpper() == "TRUE";
						} else if (line.contains("SPINE_MODNAME", Qt::CaseSensitivity::CaseInsensitive)) {
							line = line.replace(QRegExp("[ \t]"), "");
							line = line.split("=").back(); // "MODNAME";//blafoo
							line = line.split(";").front(); // "MODNAME"
							_modName = line;
							_modName.remove("\""); // MODNAME
						}
					}
				}
			}
		}
		emit changed();
	}

	void SpineEditorModel::save() {
		{ // save all user constants
			QDirIterator it(_path + "/_work/data/Scripts/Content/", QStringList() << "Spine_UserConstants.d", QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) {
				it.next();
				QFile f(it.filePath());
				if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
					QTextStream ts(&f);
					ts << "const int SPINE_ACHIEVEMENTORIENTATION = ";
					switch (_achievementOrientation) {
					case AchievementOrientation::TopLeft: {
						ts << "SPINE_TOPLEFT";
						break;
					}
					case AchievementOrientation::TopRight: {
						ts << "SPINE_TOPRIGHT";
						break;
					}
					case AchievementOrientation::BottomLeft: {
						ts << "SPINE_BOTTOMLEFT";
						break;
					}
					case AchievementOrientation::BottomRight: {
						ts << "SPINE_BOTTOMRIGHT";
						break;
					}
					default: {
						break;
					}
					}
					ts << ";\n";
					ts << "const int SPINE_ACHIEVEMENT_DISPLAY_TIME = " << _achievementDisplayDuration << ";\n";
					ts << "\n";
					ts << "const int SPINE_EARTHQUAKE_VIBRATION = " << (_earthquakeVibration ? "TRUE" : "FALSE") << ";\n";
					ts << "\n";
					ts << "const int SPINE_MODNAME = \"" << _modName << "\";\n";
					ts << "\n";

					while (_achievements.size() < 3) {
						AchievementModel am;
						am.name = "PLACEHOLDER" + QString::number(_achievements.size());
						am.unlockedImage = "SPINE_ACHIEVEMENT_DEFAULT.TGA";
					}

					for (int i = 0; i < _achievements.size(); i++) {
						ts << "const int SPINE_ACHIEVEMENT_" << (i + 1) << " = " << i << ";\n";
					}
					ts << "\n";

					ts << "const int MAX_ACHIEVEMENTS = " << _achievements.size() << ";\n";
					ts << "\n";

					ts << "const string SPINE_ACHIEVEMENT_NAMES[MAX_ACHIEVEMENTS] = {\n";
					for (int i = 0; i < _achievements.size(); i++) {
						ts << "\t\"" << _achievements[i].name.replace("\"", "&quot;").toUtf8() << "\"";
						if (i < _achievements.size() - 1) {
							ts << ",";
						}
						ts << "\n";
					}
					ts << "};\n";
					ts << "\n";

					ts << "const string SPINE_ACHIEVEMENT_DESCRIPTIONS[MAX_ACHIEVEMENTS] = {\n";
					for (int i = 0; i < _achievements.size(); i++) {
						ts << "\t\"" << _achievements[i].description.replace("\"", "&quot;").toUtf8() << "\"";
						if (i < _achievements.size() - 1) {
							ts << ",";
						}
						ts << "\n";
					}
					ts << "};\n";
					ts << "\n";

					ts << "const string SPINE_ACHIEVEMENT_TEXTURES[MAX_ACHIEVEMENTS] = {\n";
					for (int i = 0; i < _achievements.size(); i++) {
						ts << "\t\"" << _achievements[i].unlockedImage << "\"";
						if (i < _achievements.size() - 1) {
							ts << ",";
						}
						ts << "\n";
					}
					ts << "};\n";
					ts << "\n";

					ts << "const string SPINE_ACHIEVEMENT_LOCKED[MAX_ACHIEVEMENTS] = {\n";
					for (int i = 0; i < _achievements.size(); i++) {
						ts << "\t\"" << _achievements[i].lockedImage << "\"";
						if (i < _achievements.size() - 1) {
							ts << ",";
						}
						ts << "\n";
					}
					ts << "};\n";
					ts << "\n";

					ts << "const string SPINE_ACHIEVEMENT_HIDDEN[MAX_ACHIEVEMENTS] = {\n";
					for (int i = 0; i < _achievements.size(); i++) {
						ts << "\t\"" << ((_achievements[i].hidden) ? "TRUE" : "FALSE") << "\"";
						if (i < _achievements.size() - 1) {
							ts << ",";
						}
						ts << "\n";
					}
					ts << "};\n";
					ts << "\n";

					ts << "const string SPINE_ACHIEVEMENT_PROGRESS[MAX_ACHIEVEMENTS] = {\n";
					for (int i = 0; i < _achievements.size(); i++) {
						ts << "\t\"" << _achievements[i].maxProgress << "\"";
						if (i < _achievements.size() - 1) {
							ts << ",";
						}
						ts << "\n";
					}
					ts << "};\n";

					if (!_scores.isEmpty()) {
						if (_scores.size() < 2) {
							_scores.push_back(ScoreModel());
						}
						ts << "\n";

						for (int i = 0; i < _scores.size(); i++) {
							ts << "const int SPINE_SCORE_" << (i + 1) << " = " << i << ";\n";
						}
						ts << "\n";

						ts << "const string SPINE_SCORE_NAMES[" << _scores.size() << "] = {\n";
						for (int i = 0; i < _scores.size(); i++) {
							ts << "\t\"" << _scores[i].name.replace("\"", "&quot;").toUtf8() << "\"";
							if (i < _scores.size() - 1) {
								ts << ",";
							}
							ts << "\n";
						}
						ts << "};\n";
					}
				}
			}
		}
		{
			QDirIterator it(_path + "/_work/data/Scripts/Content/", QStringList() << "Startup.d", QDir::Files, QDirIterator::Subdirectories);
			if (it.hasNext()) {
				it.next();
				QFile f(it.filePath());
				if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QFile outFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Startup.d");
					if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
						QTextStream outStream(&outFile);
						QTextStream ts(&f);
						bool inInitGlobal = false;
						bool foundSpine = false;
						bool foundLeGo = false;
						int counter = 0;
						bool canAbort = true;
						while (!ts.atEnd()) {
							QString line = ts.readLine();
							QString tmpLine = line;
							if (tmpLine.replace(QRegExp("[ \t]"), "").contains("FUNCVOIDINIT_GLOBAL()", Qt::CaseInsensitive)) {
								inInitGlobal = true;
								canAbort = false;
							}
							if (inInitGlobal) {
								counter += line.count("{");
								if (counter > 0) {
									canAbort = true;
								}
								counter -= line.count("}");
								if (counter == 0 && canAbort) {
									inInitGlobal = false;
									if (!foundLeGo) {
										outStream << getLeGoInitString() << "\n";
									}
									if (!foundSpine) {
										outStream << getSpineInitString() << "\n";
									}
									outStream << line << "\n";
									continue;
								}
								if (line.contains("LeGo_Init", Qt::CaseInsensitive)) {
									// override
									if (foundLeGo) {
										continue; // handle just once
									}
									outStream << getLeGoInitString() << "\n";
									foundLeGo = true;
								} else if (line.contains("Spine_Init", Qt::CaseInsensitive)) {
									// override
									if (foundSpine) {
										continue; // handle just once
									}
									outStream << getSpineInitString() << "\n";
									foundSpine = true;
								} else {
									outStream << line << "\n";
								}
							} else {
								outStream << line << "\n";
							}
						}
					}
				}
				f.close();
				QFile(it.filePath()).remove();
				QFile(QProcessEnvironment::systemEnvironment().value("TMP", ".") + "/Startup.d").rename(it.filePath());
			}
		}
	}

	void SpineEditorModel::setModules(int32_t modules) {
		_modules = modules;
	}

	int32_t SpineEditorModel::getModules() const {
		return _modules;
	}

	void SpineEditorModel::setAchievementOrientation(AchievementOrientation achievementOrientation) {
		_achievementOrientation = achievementOrientation;
	}

	AchievementOrientation SpineEditorModel::getAchievementOrientation() const {
		return _achievementOrientation;
	}

	void SpineEditorModel::setAchievementDisplayDuration(int duration) {
		_achievementDisplayDuration = duration;
	}

	int SpineEditorModel::getAchievementDisplayDuration() const {
		return _achievementDisplayDuration;
	}

	void SpineEditorModel::setAchievements(QList<AchievementModel> achievements) {
		_achievements = achievements;
	}

	QList<AchievementModel> SpineEditorModel::getAchievements() const {
		return _achievements;
	}

	void SpineEditorModel::setScores(QList<ScoreModel> scores) {
		_scores = scores;
	}

	QList<ScoreModel> SpineEditorModel::getScores() const {
		return _scores;
	}

	void SpineEditorModel::setEarthquakeVibration(bool enabled) {
		_earthquakeVibration = enabled;
	}

	bool SpineEditorModel::getEarthquakeVibration() const {
		return _earthquakeVibration;
	}

	void SpineEditorModel::setLeGoModules(int32_t modules) {
		_legoModules = modules;
	}

	int32_t SpineEditorModel::getLeGoModules() const {
		return _legoModules;
	}

	void SpineEditorModel::setModName(QString modName) {
		_modName = modName;
	}

	QString SpineEditorModel::getModName() const {
		return _modName;
	}

	QString SpineEditorModel::getSpineInitString() const {
		QString modules;
		if (_modules & common::SpineModules::GetCurrentUsername) {
			modules += "SPINE_MODULE_GETCURRENTUSERNAME | ";
		}
		if (_modules & common::SpineModules::Achievements) {
			modules += "SPINE_MODULE_ACHIEVEMENTS | ";
		}
		if (_modules & common::SpineModules::Scores) {
			modules += "SPINE_MODULE_SCORES | ";
		}
		if (_modules & common::SpineModules::Multiplayer) {
			modules += "SPINE_MODULE_MULTIPLAYER | ";
		}
		if (_modules & common::SpineModules::OverallSave) {
			modules += "SPINE_MODULE_OVERALLSAVE | ";
		}
		if (_modules & common::SpineModules::Gamepad) {
			modules += "SPINE_MODULE_GAMEPAD | ";
		}
		if (_modules & common::SpineModules::Friends) {
			modules += "SPINE_MODULE_FRIENDS | ";
		}
		if (_modules & common::SpineModules::Statistics) {
			modules += "SPINE_MODULE_STATISTICS | ";
		}
		modules.chop(3);
		if (modules.isEmpty()) {
			modules = "0";
		}
		return "\tSpine_Init(" + modules + ");";
	}

	QString SpineEditorModel::getLeGoInitString() const {
		QString modules;
		if (_legoModules & LeGoModules::PrintS) {
			modules += "LeGo_PrintS | ";
		}
		if (_legoModules & LeGoModules::HookEngine) {
			modules += "LeGo_HookEngine | ";
		}
		if (_legoModules & LeGoModules::AI_Function) {
			modules += "LeGo_AI_Function | ";
		}
		if (_legoModules & LeGoModules::Trialoge) {
			modules += "LeGo_Trialoge | ";
		}
		if (_legoModules & LeGoModules::Dialoggestures) {
			modules += "LeGo_Dialoggestures | ";
		}
		if (_legoModules & LeGoModules::FrameFunctions) {
			modules += "LeGo_FrameFunctions | ";
		}
		if (_legoModules & LeGoModules::Cursor) {
			modules += "LeGo_Cursor | ";
		}
		if (_legoModules & LeGoModules::Focusnames) {
			modules += "LeGo_Focusnames | ";
		}
		if (_legoModules & LeGoModules::Random) {
			modules += "LeGo_Random | ";
		}
		if (_legoModules & LeGoModules::Bloodsplats) {
			modules += "LeGo_Bloodsplats | ";
		}
		if (_legoModules & LeGoModules::Saves) {
			modules += "LeGo_Saves | ";
		}
		if (_legoModules & LeGoModules::PermMem) {
			modules += "LeGo_PermMem | ";
		}
		if (_legoModules & LeGoModules::Anim8) {
			modules += "LeGo_Anim8 | ";
		}
		if (_legoModules & LeGoModules::View) {
			modules += "LeGo_View | ";
		}
		if (_legoModules & LeGoModules::Interface) {
			modules += "LeGo_Interface | ";
		}
		if (_legoModules & LeGoModules::Bars) {
			modules += "LeGo_Bars | ";
		}
		if (_legoModules & LeGoModules::Buttons) {
			modules += "LeGo_Buttons | ";
		}
		if (_legoModules & LeGoModules::Timer) {
			modules += "LeGo_Timer | ";
		}
		if (_legoModules & LeGoModules::EventHandler) {
			modules += "LeGo_EventHandler | ";
		}
		if (_legoModules & LeGoModules::Gamestate) {
			modules += "LeGo_Gamestate | ";
		}
		if (_legoModules & LeGoModules::Sprite) {
			modules += "LeGo_Sprite | ";
		}
		if (_legoModules & LeGoModules::Names) {
			modules += "LeGo_Names | ";
		}
		if (_legoModules & LeGoModules::ConsoleCommands) {
			modules += "LeGo_ConsoleCommands | ";
		}
		if (_legoModules & LeGoModules::Buffs) {
			modules += "LeGo_Buffs | ";
		}
		if (_legoModules & LeGoModules::Render) {
			modules += "LeGo_Render | ";
		}
		modules.chop(3);
		if (modules.isEmpty()) {
			modules = "0";
		}
		return "\tLeGo_Init(" + modules + ");";
	}

} /* namespace models */
} /* namespace spine */
