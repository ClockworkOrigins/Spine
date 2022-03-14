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

#include "translator/GothicParser.h"

#include <iostream>

#include "common/TranslationModel.h"

#include "utils/Conversion.h"

#include <QDirIterator>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QtConcurrentMap>

const QSet<QString> IGNORE_FILES = { "EngineClasses", "PrintDebug.d", "PrintPlus.d", "BodyStates.d", "Ikarus_Const", "Ikarus_Doc", "Ikarus.d", "LeGo/", "_intern/Constants.d", "Camera.d", "Menu.d", "Music.d", "ParticleFX.d", "SFX.d", "VisualFX.d", "CamInst.d", "MusicInst.d", "PfxInst.d", "/PFX/", "PfxInstEngine.d", "PfxInstMagic.d", "/Camera/", "/Music/", "/SFX/", "/VisualFX/", "SfxInst.d", "SfxInstSpeech.d", "VisualFxInst.d", "Menu_Defines.d", "/GFA/" };
const QSet<QString> IGNORE_FUNCTIONS = {
	"AI_PlayAni", "AI_Teleport", "AI_GotoFP", "AI_GotoWP", "PrintDebugNpc", "BLOOD_EMITTER", "BLOOD_TEXTURE", "Hlp_StrCmp", "Mdl_ApplyOverlayMds", "Mdl_RemoveOverlayMDS", "Mdl_StartFaceAni", "AI_StartState",
	"Npc_StopAni", "AI_PlayAniBS", "Wld_PlayEffect", "Npc_GetDistToWP", "Npc_IsOnFP", "Npc_PlayAni", "Wld_IsFPAvailable", "Mdl_ApplyRandomAni", "Mdl_ApplyRandomAniFreq", "Wld_IsNextFPAvailable", "AI_GotoNextFP",
	"Wld_SendUnTrigger", "Wld_SendTrigger", "PrintDebug", "MEM_SearchVobByName", "MEM_GetMenuItemByString", "Mdl_SetVisual", "Mdl_SetVisualBody", "HookEngine", "MEM_Info", "LoadLibrary", "GetProcAddress",
	"Snd_Play3D", "Wld_StopEffect", "B_SetNpcVisual", "AI_UseMob", "Wld_IsMobAvailable", "Wld_InsertNpc", "Wld_SetObjectRoutine", "Wld_InsertItem", "Wld_AssignRoomToGuild", "B_StartOtherRoutine", "Wld_SetMobRoutine",
	"Snd_Play", "B_SetLevelchange", "MEM_GetGothOpt", "MEM_SetGothOpt", "B_Say", "onSelAction", "onSelAction_s", "userString", "Update_ChoiceBox", "Npc_ExchangeRoutine", "AI_StopFX", "Mob_HasItems", "MEM_Error",
	"Spine_OverallSaveGetInt", "Spine_OverallSaveGetString", "Spine_OverallSaveSetInt", "Spine_OverallSaveSetString", "Spine_UpdateStatistic", "MEM_WriteString", "MEM_SendToSpy", "Mob_CreateItems"
};
const QList<QRegularExpression> IGNORE_REGEX = { QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*wp[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*visual[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*scemeName[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*visual_change[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*effect[ \t]*="), QRegularExpression("[ \t]*TA_[a-zA-Z_0-9]+[ \t]*\\([ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*\""), QRegularExpression("items\\[[0-9]+\\]"), QRegularExpression("musictheme[ \t]*="), QRegularExpression("backPic[ \t]*="), QRegularExpression("onChgSetOption[ \t]*="), QRegularExpression("onChgSetOptionSection[ \t]*="), QRegularExpression("hideIfOptionSectionSet[ \t]*="), QRegularExpression("hideIfOptionSet[ \t]*="), QRegularExpression("fontName[ \t]*="), QRegularExpression("BIP01 ") };

using namespace spine::common;
using namespace spine::translation;

GothicParser::GothicParser(QObject * par) : QObject(par), _currentProgress(0), _maxProgress(0) {
}

void GothicParser::parseTexts(QString path, TranslationModel * model) {
	// 1. determine all files
	_currentProgress = 0;
	QStringList filesToParse;

	QDirIterator it(path, QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QString filePath = it.filePath();
		bool found = false;
		for (const QString & s : IGNORE_FILES) {
			if (filePath.contains(s, Qt::CaseInsensitive)) {
				found = true;
				break;
			}
		}
		if (!found) {
			filesToParse.append(it.filePath());
		}
	}
	_maxProgress = filesToParse.size();

	// 2. Parse them asynchronously
	QEventLoop loop;
	auto * watcher = new QFutureWatcher<void>();
	connect(watcher, &QFutureWatcher<void>::finished, this, &GothicParser::finished);
	connect(watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
	connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
	const QFuture<void> future = QtConcurrent::map(filesToParse, [this, model](QString filePath) {
		parseFile(filePath, model);
		++_currentProgress;
		emit updatedProgress(_currentProgress, _maxProgress);
	});
	watcher->setFuture(future);
	loop.exec();
	std::cout << model->getDialogTextCount() << " Dialog Texts" << std::endl;

	int count = 0;
	for (const auto & d : model->getDialogs()) {
		for (const auto & d2 : d) {
			count += s2q(d2).split(' ').count();
		}
	}
	std::cout << "Words: " << count << std::endl;

	model->postProcess();
	std::cout << model->getNames().size() << " Names" << std::endl;
	std::cout << model->getTexts().size() << " Texts" << std::endl;
	std::cout << model->getDialogs().size() << " Dialogs" << std::endl;
	std::cout << model->getDialogTextCount() << " Dialog Texts" << std::endl;
	std::cout << (model->getNames().size() + model->getTexts().size() + model->getDialogTextCount()) << " Overall" << std::endl;
}

void GothicParser::parseFile(QString filePath, TranslationModel * model) {
	//static QMutex l;
	//QMutexLocker lg(&l);
	QFile f(filePath);
	if (f.open(QIODevice::ReadOnly)) {
		QTextStream ts(&f);
		bool starComment = false;
		bool svm = false;
		bool inFunction = false;
		int bracketCount = 0;
		QStringList dialogsInFunction;
		while (!ts.atEnd()) {
			QString line = ts.readLine();
			bool comment = false;
			bracketCount += line.count("{");
			bracketCount -= line.count("}");
			// normal text
			while (!comment && !line.isEmpty()) {
				bool found = false;
				for (const QString & s : IGNORE_FUNCTIONS) {
					if (line.contains(s, Qt::CaseInsensitive)) {
						found = true;
						break;
					}
				}
				if (found) {
					break;
				}
				for (const QRegularExpression & re : IGNORE_REGEX) {
					if (line.contains(re)) {
						found = true;
						break;
					}
				}
				if (found) {
					break;
				}
				if (!starComment && line.contains(QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*name[ \t]*=[ \t]*\"[^\"]+\"[ \t]*;"))) {
					// Name
					parseName(line, model);
					break;
				}
				if (line.contains(QRegularExpression("instance[ \t]+SVM_[0-9]+[ \t]*\\(C_SVM\\)", QRegularExpression::PatternOption::CaseInsensitiveOption))) {
					svm = true;
				}
				if (line.contains(QRegularExpression("func[ \t]+[A-Za-z]+[ \t]+[A-Za-z0-9_]+[ \t]*\\(", QRegularExpression::PatternOption::CaseInsensitiveOption))) {
					inFunction = true;
					QString currentFunction = QRegularExpression("func[ \t]+[A-Za-z]+[ \t]+([A-Za-z0-9_]+)[ \t]*\\(", QRegularExpression::PatternOption::CaseInsensitiveOption).match(line).captured(1);
					dialogsInFunction.clear();
				}
				if (!starComment && inFunction && line.contains(QRegularExpression("[ \t]*AI_Output[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*\"[^\"]+[ \t]*\"\\);[ \t]\\/\\/", QRegularExpression::PatternOption::CaseInsensitiveOption))) {
					QString dialogText = QRegularExpression("[ \t]*AI_Output[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*\"[^\"]+[ \t]*\"\\);[ \t]\\/\\/", QRegularExpression::PatternOption::CaseInsensitiveOption).match(line).captured(0);
					dialogText = line.remove(dialogText);
					dialogsInFunction.append(dialogText);
				}
				bool started = false;
				bool atEnd = false;
				bool jumpToCommentForSVM = false;
				QString text;
				for (int i = 0; i < line.size(); i++) {
					if (line.at(i) == '\"' && !starComment) {
						if (!started) {
							if (svm && !jumpToCommentForSVM) {
								jumpToCommentForSVM = true;
							} else if (!svm) {
								started = true;
							}
						} else if (!jumpToCommentForSVM) {
							line = line.mid(i + 1);
							break;
						}
					} else if (line.at(i) == '/' && i + 1 < line.size() && line.at(i + 1) == '/' && !started && !starComment) {
						if (jumpToCommentForSVM) {
							started = true;
							i++;
						} else {
							comment = true;
							break;
						}
					} else if (line.at(i) == '/' && i + 1 < line.size() && line.at(i + 1) == '*' && !started && !starComment) {
						starComment = true;
						i++;
					} else if (line.at(i) == '*' && i + 1 < line.size() && line.at(i + 1) == '/' && !started && starComment) {
						starComment = false;
						line = line.mid(i + 1);
						break;
					} else if (line.at(i) == '}' && i + 1 < line.size() && line.at(i + 1) == ';' && svm && !starComment) {
						svm = false;
						line = line.mid(i + 1);
						break;
					} else if (line.at(i) == '}' && i + 1 < line.size() && line.at(i + 1) == ';' && inFunction && !starComment && bracketCount == 0) {
						inFunction = false;
						if (!dialogsInFunction.isEmpty()) {
							std::vector<std::string> dialogs;
							for (const QString & s : dialogsInFunction) {
								dialogs.push_back(q2s(s));
							}
							model->addDialog(dialogs);
						}
						line = line.mid(i + 1);
						break;
					} else {
						if (started) {
							text += line.at(i);
						}
					}
					if (i == line.size() - 1) {
						atEnd = true;
					}
				}
				if (!text.isEmpty() && !text.trimmed().isEmpty() && text.contains(QRegularExpression("[a-zA-Z]+")) && !text.startsWith("$") && !text.endsWith(".TGA", Qt::CaseInsensitive) && !text.endsWith(".BIK", Qt::CaseInsensitive) && !text.endsWith(".ZEN", Qt::CaseInsensitive) && !text.endsWith(".3DS", Qt::CaseInsensitive) && !text.endsWith(".ASC", Qt::CaseInsensitive) && !text.endsWith(".WAV", Qt::CaseInsensitive) && (text.contains(" ") || text.count("_") < 2)) {
					model->addText(q2s(text.trimmed()));
				}
				// if we parsed the whole line and didn't close /* start next line
				if (atEnd) {
					break;
				}
			}
		}
	}
}

void GothicParser::parseName(QString line, TranslationModel * model) {
	bool started = false;
	QString name;
	for (int i = 0; i < line.size(); i++) {
		if (line.at(i) == '\"') {
			if (!started) {
				started = true;
			} else {
				line = line.mid(i + 1);
				break;
			}
		} else {
			if (started) {
				name += line.at(i);
			}
		}
	}
	if (!name.isEmpty() && name.contains(QRegularExpression("[a-zA-Z]+")) && !name.startsWith("$") && !name.endsWith(".TGA", Qt::CaseInsensitive)) {
		model->addName(q2s(name.trimmed()));
	}
}
