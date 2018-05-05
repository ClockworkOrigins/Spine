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

#include <cstdint>
#include <iostream>

#include "../../include/client/Conversion.h"

#include "translator/api/TranslatorAPI.h"
#include "translator/common/MessageStructs.h"

#include <QCache>
#include <QDirIterator>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

const QSet<QString> IGNORE_FILES = { "EngineClasses", "PrintDebug.d", "BodyStates.d", "Ikarus_Const", "Ikarus_Doc", "Ikarus.d", "LeGo/", "_intern/Constants.d", "Camera.d", "Menu.d", "Music.d", "ParticleFX.d", "SFX.d", "VisualFX.d", "CamInst.d", "MusicInst.d", "PfxInst.d", "/PFX/", "PfxInstEngine.d", "PfxInstMagic.d", "/Camera/", "/Music/", "/SFX/", "/VisualFX/", "SfxInst.d", "SfxInstSpeech.d", "VisualFxInst.d", "Menu_Defines.d", "/GFA/" };
const QSet<QString> IGNORE_FUNCTIONS = {
	"AI_PlayAni", "AI_Teleport", "AI_GotoFP", "AI_GotoWP", "PrintDebugNpc", "BLOOD_EMITTER", "BLOOD_TEXTURE", "Hlp_StrCmp", "Mdl_ApplyOverlayMds", "Mdl_RemoveOverlayMDS", "Mdl_StartFaceAni", "AI_StartState",
	"Npc_StopAni", "AI_PlayAniBS", "Wld_PlayEffect", "Npc_GetDistToWP", "Npc_IsOnFP", "Npc_PlayAni", "Wld_IsFPAvailable", "Mdl_ApplyRandomAni", "Mdl_ApplyRandomAniFreq", "Wld_IsNextFPAvailable", "AI_GotoNextFP",
	"Wld_SendUnTrigger", "Wld_SendTrigger", "PrintDebug", "MEM_SearchVobByName", "MEM_GetMenuItemByString", "Mdl_SetVisual", "Mdl_SetVisualBody", "HookEngine", "MEM_Info", "LoadLibrary", "GetProcAddress",
	"Snd_Play3D", "Wld_StopEffect", "B_SetNpcVisual", "AI_UseMob", "Wld_IsMobAvailable", "Wld_InsertNpc", "Wld_SetObjectRoutine", "Wld_InsertItem", "Wld_AssignRoomToGuild", "B_StartOtherRoutine", "Wld_SetMobRoutine",
	"Snd_Play", "B_SetLevelchange", "MEM_GetGothOpt", "MEM_SetGothOpt", "B_Say", "onSelAction", "onSelAction_s", "userString", "Update_ChoiceBox", "Npc_ExchangeRoutine", "AI_StopFX", "Mob_HasItems", "MEM_Error",
	"Spine_OverallSaveGetInt", "Spine_OverallSaveGetString", "Spine_OverallSaveSetInt", "Spine_OverallSaveSetString", "Spine_UpdateStatistic", "MEM_WriteString", "MEM_SendToSpy", "Mob_CreateItems"
};
const QList<QRegularExpression> IGNORE_REGEX = { QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*wp[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*visual[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*scemeName[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*visual_change[ \t]*="), QRegularExpression("[ \t]*[a-zA-Z_0-9]*[.]*effect[ \t]*="), QRegularExpression("[ \t]*TA_[a-zA-Z_0-9]+[ \t]*\\([ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*[0-9]+[ \t]*,[ \t]*\""), QRegularExpression("items\\[[0-9]+\\]"), QRegularExpression("musictheme[ \t]*="), QRegularExpression("backPic[ \t]*="), QRegularExpression("onChgSetOption[ \t]*="), QRegularExpression("onChgSetOptionSection[ \t]*="), QRegularExpression("hideIfOptionSectionSet[ \t]*="), QRegularExpression("hideIfOptionSet[ \t]*="), QRegularExpression("fontName[ \t]*="), QRegularExpression("BIP01 ") };

const std::string TRANSLATOR_NAME = "Translator";

QString convertStringForRegex(const QString & src) {
	return QRegularExpression::escape(src);
}

void findAndTranslateName(const QString & sourceDir, const QString & translatedDir, const std::string & destinationLanguage, uint32_t id, const QString & name);
void findAndTranslateText(const QString & sourceDir, const QString & translatedDir, const std::string & destinationLanguage, uint32_t id, const QString & searchText);
void findAndTranslateDialog(const QString & sourceDir, const QString & translatedDir, const std::string & destinationLanguage, uint32_t id, const QStringList & dialog);

static int failed = 0;
static int succeeded = 0;

int main(int argc, char ** argv) {
	argc--;
	argv++;
	// CMD arguments:
	// 1. ProjectName
	// 2. SourceLanguage
	// 3. DestinationLanguge
	// 4. SourceDir
	// 5. TranslatedDir
	if (argc != 5) {
		return EXIT_FAILURE;
	}

	const std::string projectName = argv[0];
	const std::string sourceLanguage = argv[1];
	const std::string destinationLanguage = argv[2];
	const QString sourceDir(argv[3]);
	const QString translatedDir(argv[4]);

	while (true) {
		translator::common::SendTextToTranslateMessage * sttm = translator::api::TranslatorAPI::requestTextToTranslate(TRANSLATOR_NAME, projectName, sourceLanguage, destinationLanguage);
		if (!sttm || (sttm->name.empty() && sttm->text.empty() && sttm->dialog.empty())) {
			break;
		}
		if (!sttm->name.empty()) {
			findAndTranslateName(sourceDir, translatedDir, destinationLanguage, sttm->id, s2q(sttm->name));
		} else if (!sttm->text.empty()) {
			findAndTranslateText(sourceDir, translatedDir, destinationLanguage, sttm->id, s2q(sttm->text));
		} else if (!sttm->dialog.empty()) {
			QStringList dialog;
			for (const std::string & s : sttm->dialog) {
				dialog << s2q(s);
			}
			findAndTranslateDialog(sourceDir, translatedDir, destinationLanguage, sttm->id, dialog);
		}
		delete sttm;
	}
	std::cout << "Could translate: " << succeeded << std::endl;
	std::cout << "Couldn't translate: " << failed << std::endl;
	return 0;
}

bool parseName(QString line, const QString & searchedName, QString & foundName) {
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
		if (name.trimmed() == searchedName) {
			foundName = name;
			return true;
		}
	}
	return false;
}

void findAndTranslateName(const QString & sourceDir, const QString & translatedDir, const std::string & destinationLanguage, const uint32_t id, const QString & name) {
	static QCache<QString, QString> fits;
	QList<QString> cachePaths = fits.keys();
	auto cacheIt = cachePaths.constBegin();
	QDirIterator it(sourceDir, QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (cacheIt != cachePaths.constEnd() || it.hasNext()) {
		QString filePath;
		QString fileName;
		if (cacheIt != cachePaths.constEnd()) {
			filePath = *cacheIt;
			fileName = QFileInfo(filePath).fileName();
			++cacheIt;
		} else {
			it.next();
			filePath = it.filePath();
			fileName = it.fileName();
		}
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
						QString foundName;
						if (parseName(line, name, foundName)) {
							// found the correct file
							f.close();
							f.open(QIODevice::ReadOnly);
							QTextStream fileStream(&f);
							QString fileContent = fileStream.readAll();
							QRegularExpression regExPrototype(QString("PROTOTYPE[ \t]+([\\S]+)[\\s]*\\([^}]*[ \t]*[a-zA-Z_0-9]*[.]*name[ \t]*=[ \t]*\"%1\"[ \t]*;[^}]*};").arg(convertStringForRegex(foundName)), QRegularExpression::CaseInsensitiveOption);
							QRegularExpression regExInstance(QString("INSTANCE[ \t]+([\\S]+)[\\s]*\\([^}]*[ \t]*[a-zA-Z_0-9]*[.]*name[ \t]*=[ \t]*\"%1\"[ \t]*;[^}]*};").arg(convertStringForRegex(foundName)), QRegularExpression::CaseInsensitiveOption);
							QString prototype;
							QString instance;
							if (fileContent.contains(regExPrototype)) {
								QRegularExpressionMatch match = regExPrototype.match(fileContent);
								prototype = match.captured(1);
							} else if (fileContent.contains(regExInstance)) {
								QRegularExpressionMatch match = regExInstance.match(fileContent);
								instance = match.captured(1);
							}
							if (prototype.isEmpty() && instance.isEmpty()) { // shouldn't happen
								std::cout << "No regex matches for Name \"" << q2s(foundName) << "\" in file " << q2s(filePath) << std::endl;
								failed++;
								return;
							}
							QDirIterator translatedIt(translatedDir, QStringList() << fileName, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
							if (!translatedIt.hasNext()) {
								std::cout << "File not found for translated search: " << q2s(fileName) << std::endl;
							}
							while (translatedIt.hasNext()) {
								translatedIt.next();
								const QString translatedFilePath = translatedIt.filePath();
								QFile translatedFile(translatedFilePath);
								translatedFile.open(QIODevice::ReadOnly);
								QTextStream translatedFileStream(&translatedFile);
								QString translatedFileContent = translatedFileStream.readAll();
								QRegularExpression regExPrototypeTranslated(QString("PROTOTYPE[ \t]+%1[\\s]*\\([^}]*[ \t]*[a-zA-Z_0-9]*[.]*name[ \t]*=[ \t]*\"([^\"]+)\"[ \t]*;[^}]*};").arg(prototype), QRegularExpression::CaseInsensitiveOption);
								QRegularExpression regExInstanceTranslated(QString("INSTANCE[ \t]+%1[\\s]*\\([^}]*[ \t]*[a-zA-Z_0-9]*[.]*name[ \t]*=[ \t]*\"([^\"]+)\"[ \t]*;[^}]*};").arg(instance), QRegularExpression::CaseInsensitiveOption);

								QString translatedName;
								if (translatedFileContent.contains(regExPrototypeTranslated)) {
									QRegularExpressionMatch match = regExPrototypeTranslated.match(translatedFileContent);
									translatedName = match.captured(1);
								} else if (translatedFileContent.contains(regExInstanceTranslated)) {
									QRegularExpressionMatch match = regExInstanceTranslated.match(translatedFileContent);
									translatedName = match.captured(1);
								}
								translatedName = translatedName.trimmed();
								if (translatedName.isEmpty()) { // shouldn't happen
									std::cout << "No regex matches for Transdlated Name \"" << q2s(foundName) << "\" in file " << q2s(translatedIt.filePath()) << std::endl;
									failed++;
									return;
								}
								std::cout << "Translated Name \"" << q2s(foundName) << "\" in file " << q2s(filePath) << " to \"" << q2s(translatedName) << "\"" << std::endl;
								translator::api::TranslatorAPI::sendTranslationDraft(TRANSLATOR_NAME, destinationLanguage, q2s(translatedName), "", {}, id);
								fits.insert(filePath, new QString(filePath));
								succeeded++;
							}
							return;
						}
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
								for (QString s : dialogsInFunction) {
									dialogs.push_back(q2s(s));
								}
								//model->addDialog(dialogs);
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
					if (!text.isEmpty() && text.contains(QRegularExpression("[a-zA-Z]+")) && !text.startsWith("$") && !text.endsWith(".TGA", Qt::CaseInsensitive) && !text.endsWith(".BIK", Qt::CaseInsensitive) && !text.endsWith(".ZEN", Qt::CaseInsensitive) && !text.endsWith(".3DS", Qt::CaseInsensitive) && !text.endsWith(".ASC", Qt::CaseInsensitive)) {
						//model->addText(q2s(text.trimmed()));
					}
					// if we parsed the whole line and didn't close /* start next line
					if (atEnd) {
						break;
					}
				}
			}
		}
	}
	failed++;
}

void findAndTranslateText(const QString & sourceDir, const QString & translatedDir, const std::string & destinationLanguage, const uint32_t id, const QString & searchText) {
	static QCache<QString, QString> fits;
	QList<QString> cachePaths = fits.keys();
	auto cacheIt = cachePaths.constBegin();
	QDirIterator it(sourceDir, QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (cacheIt != cachePaths.constEnd() || it.hasNext()) {
		QString filePath;
		QString fileName;
		if (cacheIt != cachePaths.constEnd()) {
			filePath = *cacheIt;
			fileName = QFileInfo(filePath).fileName();
			++cacheIt;
		} else {
			it.next();
			filePath = it.filePath();
			fileName = it.fileName();
		}
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
						// not important here
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
								for (QString s : dialogsInFunction) {
									dialogs.push_back(q2s(s));
								}
								//model->addDialog(dialogs);
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
					if (!text.isEmpty() && text.contains(QRegularExpression("[a-zA-Z]+")) && !text.startsWith("$") && !text.endsWith(".TGA", Qt::CaseInsensitive) && !text.endsWith(".BIK", Qt::CaseInsensitive) && !text.endsWith(".ZEN", Qt::CaseInsensitive) && !text.endsWith(".3DS", Qt::CaseInsensitive) && !text.endsWith(".ASC", Qt::CaseInsensitive)) {
						if (text.trimmed() == searchText) {
							f.close();
							f.open(QIODevice::ReadOnly);
							QTextStream fileStream(&f);
							QString fileContent = fileStream.readAll();
							const QRegularExpression regExConstString(QString(R"(CONST[\s]+STRING[\s]+([\S]+)[\s]+=[\s]+"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExSVM(QString("INSTANCE[\\s]*([\\S]*)[\\s]*\\(C_SVM\\)[^}]*\\\n[\\s]*([\\S]+)[\\s]*=[\\s]*\"([\\S]+)\"[\\s]*;[\\s]*//%1[^}]*};").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExDescriptionAndText(QString(R"(INSTANCE[\s]*([\S]*)[\s]*\([^}]*[\s]+([\S]+)[\s]*=[\s]*"%1"[^}]*};)").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExDescriptionAndTextPrototype(QString(R"(PROTOTYPE[\s]*([\S]*)[\s]*\([^}]*[\s]+([\S]+)[\s]*=[\s]*"%1"[^}]*};)").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExAddChoice(QString("Info_AddChoice[\\s]*\\([\\s]*([^,\\\n \\\t]+)[\\s]*,[^\\\n\"]*\"%1\"[^\\\n]*,[\\s]*([\\S]+)[\\s]*\\);").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExDocPrintLine(QString(R"(Doc_PrintLine[s]*[\s]*\([^,]*,[^,]*,[^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExPrintScreen(QString(R"(PrintScreen[\s]*\([^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExPrint(QString(R"(Print[\s]*\([^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regAIOutput(QString(R"(AI_Output[\s]*\([^,]+,[^,]+,[^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regNpcExchangeRoutine(QString(R"(Npc_ExchangeRoutine[\s]*\([^,]+,[^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regStopFX(QString(R"(AI_StopFX[\s]*\([^,]+,[^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regMobHasItems(QString(R"(Mob_HasItems[\s]*\([^"]*"%1"[^,]*,)").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regStringAssignment(QString("\\\n[ \t]*[\\S]+[ \t]*=[^\"\\\n]*\"%1\"").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExLogEntry(QString(R"(B_LogEntry[\s]*\([^,]+,[^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							const QRegularExpression regExLogAddEntry(QString(R"(Log_AddEntry[\s]*\([^,]+,[^"]*"%1")").arg(convertStringForRegex(text)), QRegularExpression::CaseInsensitiveOption);
							QString constString;
							QString svmInstance;
							QString svmVariable;
							QString svmString;
							QString descriptionOrTextInstance;
							QString descriptionOrTextPrototype;
							QString descriptionOrTextVariable;
							QString choiceBase;
							QString choiceChild;
							int printLine = -1;
							int printLineCount = 0;
							int printScreenLine = -1;
							int printScreenLineCount = 0;
							int printNormalLine = -1;
							int printNormalLineCount = 0;
							bool isPassthrough = false;
							int assignmentLine = -1;
							int assignmentLineCount = 0;
							int logEntryLine = -1;
							int logEntryLineCount = 0;
							int logAddEntryLine = -1;
							int logAddEntryLineCount = 0;
							if (fileContent.contains(regExConstString)) {
								QRegularExpressionMatch match = regExConstString.match(fileContent);
								constString = match.captured(1);
							} else if (fileContent.contains(regExSVM)) {
								QRegularExpressionMatch match = regExSVM.match(fileContent);
								svmInstance = match.captured(1);
								svmVariable = match.captured(2);
								svmString = match.captured(3);
							} else if (fileContent.contains(regExDescriptionAndText)) {
								QRegularExpressionMatch match = regExDescriptionAndText.match(fileContent);
								descriptionOrTextInstance = match.captured(1);
								descriptionOrTextVariable = match.captured(2);
							} else if (fileContent.contains(regExDescriptionAndTextPrototype)) {
								QRegularExpressionMatch match = regExDescriptionAndTextPrototype.match(fileContent);
								descriptionOrTextPrototype = match.captured(1);
								descriptionOrTextVariable = match.captured(2);
							} else if (fileContent.contains(regExAddChoice)) {
								QRegularExpressionMatch match = regExAddChoice.match(fileContent);
								choiceBase = match.captured(1);
								choiceChild = match.captured(2);
							} else if (fileContent.contains(regExDocPrintLine)) {
								QRegularExpression regExDocPrintLines(QString("Doc_PrintLine[s]*[\\s]*\\([^,]*,[^,]*,[^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

								QStringList list = fileContent.split("\n");
								for (int i = 0; i < list.count(); i++) {
									const QString splitLine = list[i];
									QRegularExpressionMatch match2 = regExDocPrintLines.match(splitLine);
									if (match2.isValid() && match2.lastCapturedIndex() == 1) {
										QString captured = match2.captured(1);
										if (!captured.trimmed().isEmpty()) {
											printLineCount++;
										}
										if (captured.trimmed() == searchText) {
											printLine = printLineCount - 1;
										}
									}
								}
							} else if (fileContent.contains(regExPrintScreen)) {
								QRegularExpression regExPrintScreenLines(QString("PrintScreen[\\s]*\\([^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

								QStringList list = fileContent.split("\n");
								for (int i = 0; i < list.count(); i++) {
									const QString splitLine = list[i];
									QRegularExpressionMatch match2 = regExPrintScreenLines.match(splitLine);
									if (match2.isValid() && match2.lastCapturedIndex() == 1) {
										QString captured = match2.captured(1);
										if (!captured.trimmed().isEmpty()) {
											printScreenLineCount++;
										}
										if (captured.trimmed() == searchText) {
											printScreenLine = printScreenLineCount - 1;
										}
									}
								}
							} else if (fileContent.contains(regExPrint)) {
								QRegularExpression regExPrintLines(QString("Print[\\s]*\\([^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

								QStringList list = fileContent.split("\n");
								for (int i = 0; i < list.count(); i++) {
									const QString splitLine = list[i];
									QRegularExpressionMatch match2 = regExPrintLines.match(splitLine);
									if (match2.isValid() && match2.lastCapturedIndex() == 1) {
										QString captured = match2.captured(1);
										if (!captured.trimmed().isEmpty()) {
											printNormalLineCount++;
										}
										if (captured.trimmed() == searchText) {
											printNormalLine = printNormalLineCount - 1;
										}
									}
								}
							} else if (fileContent.contains(regAIOutput) || fileContent.contains(regNpcExchangeRoutine) || fileContent.contains(regStopFX) || fileContent.contains(regMobHasItems) || searchText.endsWith(".wav", Qt::CaseInsensitive)) {
								isPassthrough = true;
							} else if (fileContent.contains(regStringAssignment)) {
								QRegularExpression regExStringAssignmentLines(QString("[ \t]*[\\S]+[ \t]*=[^\"\\\n]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

								QStringList list = fileContent.split("\n");
								for (int i = 0; i < list.count(); i++) {
									const QString splitLine = list[i];
									QRegularExpressionMatch match2 = regExStringAssignmentLines.match(splitLine);
									if (match2.isValid() && match2.lastCapturedIndex() == 1) {
										QString captured = match2.captured(1);
										if (!captured.trimmed().isEmpty()) {
											assignmentLineCount++;
										}
										if (captured.trimmed() == searchText) {
											assignmentLine = assignmentLineCount - 1;
										}
									}
								}
							} else if (fileContent.contains(regExLogEntry)) {
								QRegularExpression regExLogEntryLines(QString("B_LogEntry[\\s]*\\([^,]+,[^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

								QStringList list = fileContent.split("\n");
								for (int i = 0; i < list.count(); i++) {
									const QString splitLine = list[i];
									QRegularExpressionMatch match2 = regExLogEntryLines.match(splitLine);
									if (match2.isValid() && match2.lastCapturedIndex() == 1) {
										QString captured = match2.captured(1);
										if (!captured.trimmed().isEmpty()) {
											logEntryLineCount++;
										}
										if (captured.trimmed() == searchText) {
											logEntryLine = logEntryLineCount - 1;
										}
									}
								}
							} else if (fileContent.contains(regExLogAddEntry)) {
								QRegularExpression regExLogEntryLines(QString("Log_AddEntry[\\s]*\\([^,]+,[^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

								QStringList list = fileContent.split("\n");
								for (int i = 0; i < list.count(); i++) {
									const QString splitLine = list[i];
									QRegularExpressionMatch match2 = regExLogEntryLines.match(splitLine);
									if (match2.isValid() && match2.lastCapturedIndex() == 1) {
										QString captured = match2.captured(1);
										if (!captured.trimmed().isEmpty()) {
											logAddEntryLineCount++;
										}
										if (captured.trimmed() == searchText) {
											logAddEntryLine = logAddEntryLineCount - 1;
										}
									}
								}
							}
							if (constString.isEmpty() && svmInstance.isEmpty() && descriptionOrTextInstance.isEmpty() && descriptionOrTextPrototype.isEmpty() && choiceBase.isEmpty() && printLine == -1 && printScreenLine == -1 && printNormalLine == -1 && !isPassthrough && assignmentLine == -1 && logEntryLine == -1 && logAddEntryLine == -1) { // shouldn't happen
								std::cout << "No regex matches for Text \"" << q2s(text) << "\" in file " << q2s(filePath) << std::endl;
								failed++;
								return;
							}
							QDirIterator translatedIt(translatedDir, QStringList() << fileName, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
							if (!translatedIt.hasNext()) {
								std::cout << "File not found for translated search: " << q2s(fileName) << std::endl;
							}
							while (translatedIt.hasNext()) {
								translatedIt.next();
								const QString translatedFilePath = translatedIt.filePath();
								QFile translatedFile(translatedFilePath);
								translatedFile.open(QIODevice::ReadOnly);
								QTextStream translatedFileStream(&translatedFile);
								QString translatedFileContent = translatedFileStream.readAll();
								QRegularExpression regExConstStringTranslated(QString(R"(CONST[\s]+STRING[\s]+%1[\s]+=[\s]+"([^"]+))").arg(convertStringForRegex(constString)), QRegularExpression::CaseInsensitiveOption);
								QRegularExpression regExSVMTranslated(QString("INSTANCE[\\s]*%1[\\s]*\\(C_SVM\\)[^}]*%2[\\s]*=[\\s]*\"%3\"[\\s]*;[\\s]*//([^\\\n]+)\\\n[^}]*};").arg(convertStringForRegex(svmInstance), convertStringForRegex(svmVariable), convertStringForRegex(svmString)), QRegularExpression::CaseInsensitiveOption);
								QRegularExpression regExDescriptionOrTextTranslated(QString("INSTANCE[\\s]*%1[\\s]*\\([^}]*%2[\\s]*=[\\s]*\"([^\"]+)\"[^}]*};").arg(convertStringForRegex(descriptionOrTextInstance), convertStringForRegex(descriptionOrTextVariable)), QRegularExpression::CaseInsensitiveOption);
								QRegularExpression regExDescriptionOrTextPrototypeTranslated(QString("PROTOTYPE[\\s]*%1[\\s]*\\([^}]*%2[\\s]*=[\\s]*\"([^\"]+)\"[^}]*};").arg(convertStringForRegex(descriptionOrTextPrototype), convertStringForRegex(descriptionOrTextVariable)), QRegularExpression::CaseInsensitiveOption);
								QRegularExpression regExAddChoiceTranslated(QString("Info_AddChoice[\\s]*\\([\\s]*%1[^,\\\n]*,[^\\\n\"]*\"([^\"]+)\"[^\\\n]*,[\\s]*%2[\\s]*\\);").arg(convertStringForRegex(choiceBase), convertStringForRegex(choiceChild)), QRegularExpression::CaseInsensitiveOption);

								QString translatedText;
								if (printLine != -1) {
									QRegularExpression regExDocPrintLines2(QString("Doc_PrintLine[s]*[\\s]*\\([^,]*,[^,]*,[^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

									QStringList printLineStrings;
									QStringList list = translatedFileContent.split("\n");
									for (int i = 0; i < list.count(); i++) {
										const QString splitLine = list[i];
										QRegularExpressionMatch match2 = regExDocPrintLines2.match(splitLine);
										if (match2.isValid() && match2.lastCapturedIndex() == 1) {
											QString captured = match2.captured(1);
											if (!captured.trimmed().isEmpty()) {
												printLineStrings << captured;
											}
										}
									}
									if (printLineStrings.count() == printLineCount) {
										translatedText = printLineStrings[printLine];
									} else {
										std::cout << "Doc_PrintLine, Orig: " << printLineCount << ", Translated: " << printLineStrings.count() << std::endl;
									}
								} else if (printScreenLine != -1) {
									QRegularExpression regExPrintScreenLines(QString("PrintScreen[\\s]*\\([^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

									QStringList printLineStrings;
									QStringList list = translatedFileContent.split("\n");
									for (int i = 0; i < list.count(); i++) {
										const QString splitLine = list[i];
										QRegularExpressionMatch match2 = regExPrintScreenLines.match(splitLine);
										if (match2.isValid() && match2.lastCapturedIndex() == 1) {
											QString captured = match2.captured(1);
											if (!captured.trimmed().isEmpty()) {
												printLineStrings << captured;
											}
										}
									}
									if (printLineStrings.count() == printScreenLineCount) {
										translatedText = printLineStrings[printScreenLine];
									}
								} else if (printNormalLine != -1) {
									QRegularExpression regExPrintLines(QString("Print[\\s]*\\([^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

									QStringList printLineStrings;
									QStringList list = translatedFileContent.split("\n");
									for (int i = 0; i < list.count(); i++) {
										const QString splitLine = list[i];
										QRegularExpressionMatch match2 = regExPrintLines.match(splitLine);
										if (match2.isValid() && match2.lastCapturedIndex() == 1) {
											QString captured = match2.captured(1);
											if (!captured.trimmed().isEmpty()) {
												printLineStrings << captured;
											}
										}
									}
									if (printLineStrings.count() == printNormalLineCount) {
										translatedText = printLineStrings[printNormalLine];
									}
								} else if (assignmentLine != -1) {
									QRegularExpression regExStringAssignmentLines(QString("[ \t]*[\\S]*[ \t]*=[^\"\\\n]*\"[^\"]+\""), QRegularExpression::CaseInsensitiveOption);

									QStringList printLineStrings;
									QStringList list = translatedFileContent.split("\n");
									for (int i = 0; i < list.count(); i++) {
										const QString splitLine = list[i];
										QRegularExpressionMatch match2 = regExStringAssignmentLines.match(splitLine);
										if (match2.isValid() && match2.lastCapturedIndex() == 1) {
											QString captured = match2.captured(1);
											if (!captured.trimmed().isEmpty()) {
												printLineStrings << captured;
											}
										}
									}
									if (printLineStrings.count() == assignmentLineCount) {
										translatedText = printLineStrings[assignmentLine];
									}
								} else if (logEntryLine != -1) {
									QRegularExpression regExLogEntryLines(QString("B_LogEntry[\\s]*\\([^,]+,[^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

									QStringList printLineStrings;
									QStringList list = translatedFileContent.split("\n");
									for (int i = 0; i < list.count(); i++) {
										const QString splitLine = list[i];
										QRegularExpressionMatch match2 = regExLogEntryLines.match(splitLine);
										if (match2.isValid() && match2.lastCapturedIndex() == 1) {
											QString captured = match2.captured(1);
											if (!captured.trimmed().isEmpty()) {
												printLineStrings << captured;
											}
										}
									}
									if (printLineStrings.count() == logEntryLineCount) {
										translatedText = printLineStrings[logEntryLine];
									}
								} else if (logAddEntryLine != -1) {
									QRegularExpression regExLogEntryLines(QString("Log_AddEntry[\\s]*\\([^,]+,[^\"]*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);

									QStringList printLineStrings;
									QStringList list = translatedFileContent.split("\n");
									for (int i = 0; i < list.count(); i++) {
										const QString splitLine = list[i];
										QRegularExpressionMatch match2 = regExLogEntryLines.match(splitLine);
										if (match2.isValid() && match2.lastCapturedIndex() == 1) {
											QString captured = match2.captured(1);
											if (!captured.trimmed().isEmpty()) {
												printLineStrings << captured;
											}
										}
									}
									if (printLineStrings.count() == logAddEntryLineCount) {
										translatedText = printLineStrings[logAddEntryLine];
									}
								} else if (translatedFileContent.contains(regExConstStringTranslated)) {
									QRegularExpressionMatch match = regExConstStringTranslated.match(translatedFileContent);
									translatedText = match.captured(1);
								} else if (translatedFileContent.contains(regExSVMTranslated)) {
									QRegularExpressionMatch match = regExSVMTranslated.match(translatedFileContent);
									translatedText = match.captured(1);
								} else if (translatedFileContent.contains(regExDescriptionOrTextTranslated)) {
									QRegularExpressionMatch match = regExDescriptionOrTextTranslated.match(translatedFileContent);
									translatedText = match.captured(1);
								} else if (translatedFileContent.contains(regExDescriptionOrTextPrototypeTranslated)) {
									QRegularExpressionMatch match = regExDescriptionOrTextPrototypeTranslated.match(translatedFileContent);
									translatedText = match.captured(1);
								} else if (translatedFileContent.contains(regExAddChoiceTranslated)) {
									QRegularExpressionMatch match = regExAddChoiceTranslated.match(translatedFileContent);
									translatedText = match.captured(1);
								} else if (isPassthrough) {
									translatedText = text;
								}
								translatedText = translatedText.trimmed();
								if (translatedText.isEmpty()) { // shouldn't happen
									std::cout << "No regex matches for translated Text \"" << q2s(text) << "\" in file " << q2s(translatedIt.filePath()) << std::endl;
									failed++;
									return;
								}
								std::cout << "Translated Text \"" << q2s(text) << "\" in file " << q2s(filePath) << " to \"" << q2s(translatedText) << "\"" << std::endl;
								translator::api::TranslatorAPI::sendTranslationDraft(TRANSLATOR_NAME, destinationLanguage, "", q2s(translatedText), {}, id);
								succeeded++;
								fits.insert(filePath, new QString(filePath));
							}
							return;
						}
					}
					// if we parsed the whole line and didn't close /* start next line
					if (atEnd) {
						break;
					}
				}
			}
		}
	}
	failed++;
}

void findAndTranslateDialog(const QString & sourceDir, const QString & translatedDir, const std::string & destinationLanguage, const uint32_t id, const QStringList & dialog) {
	static QCache<QString, QString> fits;
	QList<QString> cachePaths = fits.keys();
	auto cacheIt = cachePaths.constBegin();
	QDirIterator it(sourceDir, QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (cacheIt != cachePaths.constEnd() || it.hasNext()) {
		QString filePath;
		QString fileName;
		if (cacheIt != cachePaths.constEnd()) {
			filePath = *cacheIt;
			fileName = QFileInfo(filePath).fileName();
			++cacheIt;
		} else {
			it.next();
			filePath = it.filePath();
			fileName = it.fileName();
		}
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
						// not important here
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
								QStringList foundDialog;
								for (const QString & s : dialogsInFunction) {
									foundDialog.append(s);
								}
								if (foundDialog == dialog) {
									f.close();
									f.open(QIODevice::ReadOnly);
									QTextStream fileStream(&f);
									QString fileContent = fileStream.readAll();
									QStringList identifierList;
									for (const QString & s : foundDialog) {
										QRegularExpression regExOutput(QString(R"lit(AI_Output[\s]*\([^,]+,[^,]+,[^"]*"([^"]+)"[\s]*\);[\s]+//%1)lit").arg(convertStringForRegex(s)), QRegularExpression::CaseInsensitiveOption);
										if (fileContent.contains(regExOutput)) {
											QRegularExpressionMatch match = regExOutput.match(fileContent);
											identifierList.append(match.captured(1));
										}
									}
									if (foundDialog.size() != identifierList.size()) { // shouldn't happen
										std::cout << "No regex matches for Dialog in file " << q2s(filePath) << std::endl;
										failed++;
										return;
									}
									QDirIterator translatedIt(translatedDir, QStringList() << fileName, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
									if (!translatedIt.hasNext()) {
										std::cout << "File not found for translated search: " << q2s(fileName) << std::endl;
									}
									while (translatedIt.hasNext()) {
										translatedIt.next();
										const QString translatedFilePath = translatedIt.filePath();
										QFile translatedFile(translatedFilePath);
										translatedFile.open(QIODevice::ReadOnly);
										QTextStream translatedFileStream(&translatedFile);
										QString translatedFileContent = translatedFileStream.readAll();

										std::vector<std::string> translatedDialog;
										for (const QString & identifier : identifierList) {
											QRegularExpression regExConstStringTranslated(QString("AI_Output[\\s]*\\([^,]+,[^,]+,[^\"]*\"%1\"[\\s]*\\);[\\s]+//([^\\\n]+)\\\n").arg(convertStringForRegex(identifier)), QRegularExpression::CaseInsensitiveOption);

											if (translatedFileContent.contains(regExConstStringTranslated)) {
												QRegularExpressionMatch match = regExConstStringTranslated.match(translatedFileContent);
												QString translatedText = match.captured(1);
												translatedDialog.push_back(q2s(translatedText.trimmed()));
											}
										}
										if (int(translatedDialog.size()) == foundDialog.size()) {
											std::cout << "Translated Dialog in file " << q2s(filePath) << std::endl;
											for (int j = 0; j < foundDialog.size(); j++) {
												std::cout << "Translated Text \"" << q2s(foundDialog[j]) << "\" to \"" << translatedDialog[j] << "\"" << std::endl;
											}
											translator::api::TranslatorAPI::sendTranslationDraft(TRANSLATOR_NAME, destinationLanguage, "", "", translatedDialog, id);
											succeeded++;
											fits.insert(filePath, new QString(filePath));
										}
									}
									return;
								}
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
					if (!text.isEmpty() && text.contains(QRegularExpression("[a-zA-Z]+")) && !text.startsWith("$") && !text.endsWith(".TGA", Qt::CaseInsensitive) && !text.endsWith(".BIK", Qt::CaseInsensitive) && !text.endsWith(".ZEN", Qt::CaseInsensitive) && !text.endsWith(".3DS", Qt::CaseInsensitive) && !text.endsWith(".ASC", Qt::CaseInsensitive)) {
					}
					// if we parsed the whole line and didn't close /* start next line
					if (atEnd) {
						break;
					}
				}
			}
		}
	}
	failed++;
}
