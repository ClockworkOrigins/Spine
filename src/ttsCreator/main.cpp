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

int main(const int argc, char ** argv) {
	if (argc < 4) {
		std::cout << "Usage:" << std::endl;
		std::cout << argv[0] << " <config file> <script input folder> <sound output folder> <csv output file>" << std::endl;
		return -1;
	}
	QMap<int, int> voices;

	QFile configFile(argv[1]);

	if (!configFile.open(QIODevice::ReadOnly)) {
		std::cout << "Can't open config file" << std::endl;
		return -1;
	}
	
	QTextStream configReader(&configFile);
	while (!configReader.atEnd()) {
		const auto line = configReader.readLine();

		if (line.isEmpty()) continue;

		const auto split = line.split("=>");

		if (split.count() != 2) continue;

		const auto gothicVoice = split[0].toInt();
		const auto ttsVoice = split[1].toInt();

		voices.insert(gothicVoice, ttsVoice);
	}

	const QString inputFolder = argv[2];
	const QString outputFolder = argv[3];

	if (!QDir().exists(inputFolder)) {
		std::cout << "Input folder does not exist" << std::endl;
		return -1;
	}

	if (!QDir().exists(outputFolder)) {
		const bool b = QDir().mkpath(outputFolder);
		Q_UNUSED(b)
	}

	QSet<QString> files;

	const QRegularExpression regex("AI_Output[\\s]*\\([^,]+,[^,]+,[^\"]*\"([^\"]+_(\\d+)_\\d+)\"[\\s]*\\); //([^\\\n]+)");
	const QRegularExpression outputRegex("AI_Output[\\s]*\\([^,]+,[^,]+,[^\"]*\"[^\"]+\"[\\s]*\\); //([^\\\n]+)");
	const QRegularExpression svmStartRegex(R"(instance[\s]+SVM_(\d+)[\s]*\(C_SVM\))", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression svmRegex("[\\s]*[^\\s]+[\\s]*=[\\s]*\"([^\"]+)\"[\\s]*;[\\s]*//([^\\\n]+)", QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression svmSmalltakRegex("_Smalltalk", QRegularExpression::CaseInsensitiveOption);

	QFile outFile(argv[4]);

	if (!outFile.open(QIODevice::WriteOnly)) {
		std::cout << "Can't open output file" << std::endl;
		return -1;
	}

	QTextStream fileWriter(&outFile);

	QDirIterator it(inputFolder, QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QString filePath = it.filePath();
		QFile f(filePath);
		if (f.open(QIODevice::ReadOnly)) {
			int svmVoiceNumber = 0;
			QTextStream ts(&f);
			while (!ts.atEnd()) {
				QString line = ts.readLine();
				if (line.contains(regex)) {
					QRegularExpressionMatch match = regex.match(line);
					QString wavName = match.captured(1);
					auto voiceNumber = match.captured(2).toInt();
					QString text = match.captured(3);
					static QSet<int> missingVoice;
					if (!voices.contains(voiceNumber) && !missingVoice.contains(voiceNumber)) {
						std::cout << "Missing Voice number: " << voiceNumber << " in " << filePath.toStdString() << std::endl;
						missingVoice.insert(voiceNumber);
					}

					if (!voices.contains(voiceNumber)) continue;

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

					if (text.isEmpty()) continue;

					fileWriter << text << "|" << voices[voiceNumber] << "|" << wavName << "\n";

					files.insert(wavName.toUpper());
				} else if (line.contains(outputRegex)) {
					QRegularExpressionMatch match = outputRegex.match(line);
					std::cout << match.captured(0).toStdString() << std::endl;
				} else if (line.contains(svmStartRegex)) {
					QRegularExpressionMatch match = svmStartRegex.match(line);
					svmVoiceNumber = match.captured(1).toInt();
				} else if (line.contains(svmRegex)) {
					QRegularExpressionMatch match = svmRegex.match(line);
					QString wavName = match.captured(1);
					QString text = match.captured(2);
					static QSet<int> missingVoice;
					if (!voices.contains(svmVoiceNumber) && !missingVoice.contains(svmVoiceNumber)) {
						std::cout << "Missing Voice number: " << svmVoiceNumber << " in " << filePath.toStdString() << std::endl;
						missingVoice.insert(svmVoiceNumber);
					}

					if (!voices.contains(svmVoiceNumber)) continue;

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

					if (text.isEmpty()) continue;

					fileWriter << text << "|" << voices[svmVoiceNumber] << "|" << wavName << "\n";

					files.insert(wavName.toUpper());
				}
			}
		}
	}
	std::cout << files.size() << std::endl;
	return 0;
}
