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

#include <iostream>

#include <QDirIterator>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

#include <tinyxml2.h>

int main(const int argc, char ** argv) {
	if (argc < 4) {
		return -1;
	}
	struct Voice {
		QString name;
		QString speed;
		QString pitch;
	};
	QMap<QString, Voice> voices;

	tinyxml2::XMLDocument doc;

	const tinyxml2::XMLError e = doc.LoadFile(argv[1]);

	if (e) {
		return -1;
	}

	tinyxml2::XMLElement * rootNode = doc.FirstChildElement("TTS");
	for (tinyxml2::XMLElement * node = rootNode->FirstChildElement("Voice"); node != nullptr; node = node->NextSiblingElement("Voice")) {
		Voice voice;
		Q_ASSERT(node->Attribute("name"));
		Q_ASSERT(node->Attribute("number"));
		voice.name = node->Attribute("name");
		voice.speed = node->Attribute("speed") ? node->Attribute("speed") : "0";
		voice.pitch = node->Attribute("pitch") ? node->Attribute("pitch") : "0";
		voices[node->Attribute("number")] = voice;
	}

	const QString inputFolder = argv[2];
	const QString outputFolder = argv[3];

	if (!QDir().exists(inputFolder)) {
		return -1;
	}
	if (!QDir().exists(outputFolder)) {
		bool b = QDir().mkpath(outputFolder);
		Q_UNUSED(b);
	}

	QSet<QString> files;

	const QRegularExpression regex("AI_Output[\\s]*\\([^,]+,[^,]+,[^\"]*\"([^\"]+_(\\d+)_\\d+)\"[\\s]*\\); //([^\\\n]+)");
	const QRegularExpression outputRegex("AI_Output[\\s]*\\([^,]+,[^,]+,[^\"]*\"[^\"]+\"[\\s]*\\); //([^\\\n]+)");
	const QRegularExpression svmStartRegex(R"(instance[\s]+SVM_(\d+)[\s]*\(C_SVM\))", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression svmRegex("[\\s]*[^\\s]+[\\s]*=[\\s]*\"([^\"]+)\"[\\s]*;[\\s]*//([^\\\n]+)", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression svmSmalltakRegex("_Smalltalk", QRegularExpression::CaseInsensitiveOption);
	QDirIterator it(inputFolder, QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QString filePath = it.filePath();
		QString svmVoiceNumber;
		QFile f(filePath);
		if (f.open(QIODevice::ReadOnly)) {
			QTextStream ts(&f);
			while (!ts.atEnd()) {
				QString line = ts.readLine();
				if (line.contains(regex)) {
					QRegularExpressionMatch match = regex.match(line);
					QString wavName = match.captured(1);
					QString voiceNumber = QString::number(match.captured(2).toInt());
					QString text = match.captured(3);
					static QSet<QString> missingVoice;
					if (!voices.contains(voiceNumber) && !missingVoice.contains(voiceNumber)) {
						std::cout << "Missing Voice number: " << voiceNumber.toStdString() << " in " << filePath.toStdString() << std::endl;
						missingVoice.insert(voiceNumber);
					}
					while (text.contains("(") && text.contains(")")) {
						const int indexOfStartBracket = text.indexOf("(");
						const int indexOfEndBracket = text.indexOf(")") + 1;
						if (indexOfEndBracket < indexOfStartBracket) {
							break;
						}
						text.remove(indexOfStartBracket, indexOfEndBracket - indexOfStartBracket);
					}
					text = text.remove('\"');
					text = text.trimmed();
					if (text.isEmpty()) {
						continue;
					}
					QStringList args;
					args << "-enc" << "utf8";
					args << "-n" << voices[voiceNumber].name;
					args << "-s" << voices[voiceNumber].speed;
					args << "-p" << voices[voiceNumber].pitch;
					args << "-t" << text.toUtf8();
					args << "-w" << outputFolder + "/" + wavName + ".wav";
					files.insert(wavName.toUpper());
					QProcess process;
					process.start("D:/Balcon/Balcon.exe", args);
					process.waitForFinished();
				} else if (line.contains(outputRegex)) {
					QRegularExpressionMatch match = outputRegex.match(line);
					std::cout << match.captured(0).toStdString() << std::endl;
				} else if (line.contains(svmStartRegex)) {
					QRegularExpressionMatch match = svmStartRegex.match(line);
					svmVoiceNumber = QString::number(match.captured(1).toInt());
				} else if (line.contains(svmRegex)) {
					QRegularExpressionMatch match = svmRegex.match(line);
					QString wavName = match.captured(1);
					QString text = match.captured(2);
					static QSet<QString> missingVoice;
					if (!voices.contains(svmVoiceNumber) && !missingVoice.contains(svmVoiceNumber)) {
						std::cout << "Missing Voice number: " << svmVoiceNumber.toStdString() << " in " << filePath.toStdString() << std::endl;
						missingVoice.insert(svmVoiceNumber);
					}
					while (text.contains("(") && text.contains(")")) {
						const int indexOfStartBracket = text.indexOf("(");
						const int indexOfEndBracket = text.indexOf(")") + 1;
						if (indexOfEndBracket < indexOfStartBracket) {
							break;
						}
						text.remove(indexOfStartBracket, indexOfEndBracket - indexOfStartBracket);
					}
					text = text.remove('\"');
					text = text.trimmed();
					if (text.isEmpty()) {
						continue;
					}
					QStringList args;
					args << "-n" << voices[svmVoiceNumber].name;
					args << "-s" << voices[svmVoiceNumber].speed;
					args << "-p" << voices[svmVoiceNumber].pitch;
					args << "-t" << text.trimmed().remove('\"');
					args << "-w" << outputFolder + "/" + wavName + ".wav";
					if (wavName.contains(svmSmalltakRegex)) {
						args << "-v" << "33";
					}
					files.insert(wavName.toUpper());
					QProcess process;
					process.start("D:/Balcon/Balcon.exe", args);
					process.waitForFinished();
				}
			}
		}
	}
	std::cout << files.size() << std::endl;
	return 0;
}
