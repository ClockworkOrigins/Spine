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

#ifdef WITH_TRANSLATOR

#include "translator/TranslationApplier.h"

#include "utils/Conversion.h"

#include <QtConcurrent>

#include "translator/api/TranslatorAPI.h"
#include "translator/common/MessageStructs.h"

const QSet<QString> IGNORE_FILES = { "EngineClasses", "PrintDebug.d", "PrintPlus.d", "BodyStates.d", "Ikarus_Const", "Ikarus_Doc", "Ikarus.d", "LeGo/", "_intern/Constants.d", "Camera.d", "Menu.d", "Music.d", "ParticleFX.d", "SFX.d", "VisualFX.d", "CamInst.d", "MusicInst.d", "PfxInst.d", "/PFX/", "PfxInstEngine.d", "PfxInstMagic.d", "/Camera/", "/Music/", "/SFX/", "/VisualFX/", "SfxInst.d", "SfxInstSpeech.d", "VisualFxInst.d", "Menu_Defines.d", ".src", "/GFA/" };
const QSet<QString> IGNORE_FUNCTIONS = {
	"AI_PlayAni", "AI_Teleport", "AI_GotoFP", "AI_GotoWP", "PrintDebugNpc", "BLOOD_EMITTER", "BLOOD_TEXTURE", "Hlp_StrCmp", "Mdl_ApplyOverlayMds", "Mdl_RemoveOverlayMDS", "Mdl_StartFaceAni", "AI_StartState",
	"Npc_StopAni", "AI_PlayAniBS", "Wld_PlayEffect", "Npc_GetDistToWP", "Npc_IsOnFP", "Npc_PlayAni", "Wld_IsFPAvailable", "Mdl_ApplyRandomAni", "Mdl_ApplyRandomAniFreq", "Wld_IsNextFPAvailable", "AI_GotoNextFP",
	"Wld_SendUnTrigger", "Wld_SendTrigger", "PrintDebug", "MEM_SearchVobByName", "MEM_GetMenuItemByString", "Mdl_SetVisual", "Mdl_SetVisualBody", "HookEngine", "MEM_Info", "LoadLibrary", "GetProcAddress",
	"Snd_Play3D", "Wld_StopEffect", "B_SetNpcVisual", "AI_UseMob", "Wld_IsMobAvailable", "Wld_InsertNpc", "Wld_SetObjectRoutine", "Wld_InsertItem", "Wld_AssignRoomToGuild", "B_StartOtherRoutine", "Wld_SetMobRoutine",
	"Snd_Play", "B_SetLevelchange", "MEM_GetGothOpt", "MEM_SetGothOpt", "B_Say", "onSelAction", "onSelAction_s", "userString", "Update_ChoiceBox", "Npc_ExchangeRoutine", "AI_StopFX", "Mob_HasItems", "MEM_Error",
	"Spine_OverallSaveGetInt", "Spine_OverallSaveGetString", "Spine_OverallSaveSetInt", "Spine_OverallSaveSetString", "Spine_UpdateStatistic", "MEM_WriteString", "MEM_SendToSpy", "Mob_CreateItems"
};
const QSet<QRegularExpression> IGNORE_REGEX = { QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*wp[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*visual[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*scemeName[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*visual_change[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*effect[ \t]*="), QRegularExpression("[ \t]*TA_[a-zA-Z_0-9]+[ \t]*\\([ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*\""), QRegularExpression("items\\[[0-9]+\\]"), QRegularExpression("musictheme[ \t]*="), QRegularExpression("backPic[ \t]*="), QRegularExpression("onChgSetOption[ \t]*="), QRegularExpression("onChgSetOptionSection[ \t]*="), QRegularExpression("hideIfOptionSectionSet[ \t]*="), QRegularExpression("hideIfOptionSet[ \t]*="), QRegularExpression("fontName[ \t]*="), QRegularExpression("BIP01 ") };

namespace spine {
namespace translator {

	TranslationApplier::TranslationApplier(uint32_t requestID, QObject * par) : QObject(par), _requestID(requestID), _currentProgress(0), _maxProgress(0) {
	}

	void TranslationApplier::parseTexts(QString path) {
		const QFuture<::translator::common::SendTranslationDownloadMessage *> f = QtConcurrent::run(::translator::api::TranslatorAPI::requestTranslationDownload, _requestID);

		QString translatedPath = QFileInfo(path + "/../translated/").absolutePath();

		// 1. determine all files
		_currentProgress = 0;
		QStringList filesToParse;

		QDirIterator dirIt(path, QStringList() << "*.d" << "*.src", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (dirIt.hasNext()) {
			dirIt.next();
			QString filePath = dirIt.filePath();
			bool found = false;
			for (const QString & s : IGNORE_FILES) {
				if (filePath.contains(s, Qt::CaseInsensitive)) {
					found = true;
					break;
				}
			}
			if (!found) {
				filesToParse.append(dirIt.filePath());
			} else {
				QString oldPath = filePath;
				const QString newPath = QFileInfo(oldPath.replace(path, translatedPath)).absoluteFilePath();
				if (QFileInfo::exists(newPath)) {
					QFile::remove(newPath);
				}
				bool b = QDir().mkpath(QFileInfo(newPath).absolutePath());
				Q_UNUSED(b);
				const bool copied = QFile::copy(filePath, newPath);
				Q_ASSERT(copied);
			}
		}
		_maxProgress = filesToParse.size();
		emit updateMaxProgress(_maxProgress);

		QMap<QString, QString> names;
		QMap<QString, QString> texts;
		QList<QPair<QStringList, QStringList>> dialogs;
		::translator::common::SendTranslationDownloadMessage * stdm = f.result();
		if (stdm == nullptr) {
			emit updatedCurrentProgress(_maxProgress);
			return;
		}
		for (auto it = stdm->names.cbegin(); it != stdm->names.cend(); ++it) {
			names.insert(s2q(it->first).trimmed(), s2q(it->second).trimmed());
		}
		for (auto it = stdm->texts.cbegin(); it != stdm->texts.cend(); ++it) {
			texts.insert(s2q(it->first).trimmed(), s2q(it->second).trimmed());
		}
		for (auto it = stdm->dialogs.cbegin(); it != stdm->dialogs.cend(); ++it) {
			Q_ASSERT(it->first.size() == it->second.size());
			QStringList original;
			QStringList translated;
			for (size_t i = 0; i < it->first.size(); i++) {
				original << s2q(it->first[i]).trimmed();
				translated << s2q(it->second[i]).trimmed();
			}
			dialogs.append(qMakePair(original, translated));
		}
		delete stdm;

		// 2. Parse them asynchronously
		QEventLoop loop;
		QFutureWatcher<void> * watcher = new QFutureWatcher<void>();
		connect(watcher, &QFutureWatcher<void>::finished, this, &TranslationApplier::finished);
		connect(watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
		connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
		const QFuture<void> future = QtConcurrent::map(filesToParse, [this, path, translatedPath, names, texts, dialogs](QString filePath) {
			parseFile(filePath, path, translatedPath, names, texts, dialogs);
			++_currentProgress;
			emit updatedCurrentProgress(_currentProgress);
		});
		watcher->setFuture(future);
		loop.exec();
	}

	void TranslationApplier::parseFile(QString filePath, QString basePath, QString translatedPath, QMap<QString, QString> names, QMap<QString, QString> texts, QList<QPair<QStringList, QStringList>> dialogs) {
		QString newPath = filePath;
		newPath = newPath.replace(basePath, translatedPath);
		const QString absolutePath = QFileInfo(newPath).absolutePath();
		if (!QDir().exists(absolutePath)) {
			bool b = QDir().mkpath(absolutePath);
			Q_UNUSED(b);
		}
		QFile inFile(filePath);
		QFile outFile(newPath);
		if (inFile.open(QIODevice::ReadOnly) && outFile.open(QIODevice::WriteOnly)) {
			QTextStream ts(&inFile);
			QTextStream outStream(&outFile);
			QString newFileContent;
			bool starComment = false;
			bool svm = false;
			bool inFunction = false;
			int bracketCount = 0;
			QStringList dialogsInFunction;
			while (!ts.atEnd()) {
				QString line = ts.readLine();
				if (line.isEmpty()) {
					newFileContent += '\n';
				}
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
						newFileContent += line + "\n";
						break;
					}
					for (const QRegularExpression & re : IGNORE_REGEX) {
						if (line.contains(re)) {
							found = true;
							break;
						}
					}
					if (found) {
						newFileContent += line + "\n";
						break;
					}
					if (!starComment && line.contains(QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*name[ \t]*=[ \t]*\"[^\"]+\"[ \t]*;"))) {
						// Name
						parseName(line, names, newFileContent);
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
						newFileContent += dialogText;
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
									newFileContent += line.left(i + 1);
								}
							} else if (!jumpToCommentForSVM) {
								line = line.mid(i + 1);
								if (text.isEmpty()) {
									newFileContent += "\"";
								}
								break;
							}
						} else if (line.at(i) == '/' && i + 1 < line.size() && line.at(i + 1) == '/' && !started && !starComment) {
							if (jumpToCommentForSVM) {
								started = true;
								newFileContent += line.left(i + 2);
								i++;
							} else {
								comment = true;
								newFileContent += line + "\n";
								break;
							}
						} else if (line.at(i) == '/' && i + 1 < line.size() && line.at(i + 1) == '*' && !started && !starComment) {
							starComment = true;
							i++;
						} else if (line.at(i) == '*' && i + 1 < line.size() && line.at(i + 1) == '/' && !started && starComment) {
							starComment = false;
							newFileContent += line.left(i + 1);
							line = line.mid(i + 1);
							break;
						} else if (line.at(i) == '}' && i + 1 < line.size() && line.at(i + 1) == ';' && svm && !starComment) {
							svm = false;
							newFileContent += line.left(i + 1);
							line = line.mid(i + 1);
							break;
						} else if (line.at(i) == '}' && i + 1 < line.size() && line.at(i + 1) == ';' && inFunction && !starComment && bracketCount == 0) {
							inFunction = false;
							if (!dialogsInFunction.isEmpty()) {
								for (auto p : dialogs) {
									if (p.first == dialogsInFunction) {
										for (int j = 0; j < dialogsInFunction.size(); j++) {
											newFileContent = newFileContent.replace("//" + p.first[j], "//" + p.second[j]);
										}
										break;
									}
								}
							}
							newFileContent += line.left(i + 1);
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
						if (texts.contains(text.trimmed())) {
							if (text.endsWith("\"")) {
								text.chop(1);
							}
							QString translatedText = texts[text.trimmed()];
							if (translatedText.endsWith("\"")) {
								translatedText.chop(1);
							}
							if (translatedText.startsWith("\"")) {
								translatedText = translatedText.mid(1);
							}
							translatedText = translatedText.replace("\"", "'");
							newFileContent += text.replace(text.trimmed(), translatedText);
							if (!jumpToCommentForSVM) {
								newFileContent += "\"";
							}
						} else {
							newFileContent += text + "\"";
						}
					} else {
						newFileContent += text;
						if (!text.isEmpty()) {
							newFileContent += "\"";
						}
					}
					// if we parsed the whole line and didn't close /* start next line
					if (atEnd || line.isEmpty()) {
						if (!jumpToCommentForSVM) {
							newFileContent += line;
						}
						newFileContent += "\n";
						break;
					}
				}
			}
			outStream << newFileContent;
		}
	}

	void TranslationApplier::parseName(QString line, QMap<QString, QString> names, QString & newFileContent) {
		bool started = false;
		QString name;
		for (int i = 0; i < line.size(); i++) {
			if (line.at(i) == '\"') {
				if (!started) {
					newFileContent += line.left(i + 1);
					started = true;
				} else {
					line = line.mid(i);
					break;
				}
			} else {
				if (started) {
					name += line.at(i);
				}
			}
		}
		if (!name.isEmpty() && name.contains(QRegularExpression("[a-zA-Z]+")) && !name.startsWith("$") && !name.endsWith(".TGA", Qt::CaseInsensitive)) {
			if (names.contains(name.trimmed())) {
				QString translatedName = names[name.trimmed()];
				translatedName.replace("\"", "'");
				newFileContent += name.replace(name.trimmed(), translatedName);
			} else {
				newFileContent += name;
			}
		}
		newFileContent += line + "\n";
	}

} /* namespace translator */
} /* namespace spine */

#endif /* WITH_TRANSLATOR */
