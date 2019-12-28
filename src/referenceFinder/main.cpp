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
#include <QRegularExpression>
#include <QTextStream>

int main(int, char ** argv) {
	
	QList<QRegularExpression> expressionList;
	expressionList << QRegularExpression("B_SetNpcVisual[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("EquipItem[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("MEM_AssignInst[ \t]*\\([ \t]*[0-9]+[ \t]*,[ \t]*[^,]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Wld_SpawnNpcRange[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*,", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Wld_InsertNpc[ \t]*\\([ \t]*[0-9]+[ \t]*,[ \t]*\"[^,]+\"[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Wld_InsertItem[ \t]*\\([ \t]*[0-9]+[ \t]*,[ \t]*\"[^,]+\"[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("ai_output");
	expressionList << QRegularExpression("B_RemoveNpc[ \t]*\\([ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("B_KillNpc[ \t]*\\([ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("AI_UseItem[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("AI_UseItemToState[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*,", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("B_GiveInvItems[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*,", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Info_ClearChoices[ \t]*\\([ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("B_UseItem[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Info_AddChoice[ \t]*\\([ \t]*[0-9]+[ \t]*,", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Mdl_SetVisualBody[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Npc_RemoveInvItems[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*,", QRegularExpression::PatternOption::CaseInsensitiveOption);
	expressionList << QRegularExpression("Npc_RemoveInvItem[ \t]*\\([ \t]*[^,]+[ \t]*,[ \t]*[0-9]+[ \t]*\\);", QRegularExpression::PatternOption::CaseInsensitiveOption);

	QDirIterator it(argv[1], QStringList() << "*.d", QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
	while (it.hasNext()) {
		it.next();
		QString filePath = it.filePath();
		int lineNumber = 1;
		QFile f(filePath);
		if (f.open(QIODevice::ReadOnly)) {
			QTextStream ts(&f);
			while (!ts.atEnd()) {
				QString line = ts.readLine();
				for (const QRegularExpression & regEx : expressionList) {
					if (line.contains(regEx)) {
						std::cout << filePath.toStdString() << ":" << lineNumber << " " << line.toStdString() << std::endl;
						break;
					}
				}
				lineNumber++;
			}
		}
	}
	return 0;
}
