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

#include "SavegameManager.h"

#include <QFile>

using namespace spine;

namespace {
	
	bool dialogMode;
	bool dialogBegin;
	bool dialogEnd;
	bool tagebuchmode;

	int jumpNr(QByteArray & b, int & z, int & k) {
		int d2 = 0;
		int r = 0;
		while (b.at(z) == 0x12) { // die 12er sind ein guter Anker
			z += 5;   // die Nummerierung interessiert uns nicht, also überspringen
			if (b.at(z) != 0x01) { // die 1er interressieren uns auch nicht, da die den String einläuten
				if (dialogBegin && b.at(z) == 0x02) { // 2er sind für Integer, da zuerst die dialoge gespeichert sind, wissen wir hier, dass nun nur noch _variables kommen
					tagebuchmode = true;
					dialogMode = false;
					dialogEnd = true;
					dialogBegin = false;
				}
				z++;
				k = z; // Position speichern, um später überschreiben zu können
				if (!dialogBegin && d2 < 2) {
					memcpy(&r, b.constData() + z, 4);
					d2++;
				}
				z += 4;// Integer ist 4 Bytes lang
				if (dialogMode && b.at(z - 5) == 0x06) { // Sollte es eine 6 als Typ sein, haben wir hier Dialoge, also beginnt hier das Dialogsegment
					memcpy(&r, b.constData() + z - 4, 4);
					dialogBegin = true;
					return r;
				}
				// Wieso hier kein Return? Weil nur der letzte Block den Wert speichert (jaja, das Format der PBs ist sehr 'interessant')
			}
		}
		return r;
	}

	int readLength(QByteArray & b, int & z) { // Liest die Länge des _variablesstrings
		int r = 0;
		z += 1;
		memcpy(&r, b.constData() + z, 2);
		z += 2;
		return r;
	}

	std::string readName(QByteArray & b, int & z, int l) { // liest den Namen der Variableiable aus
		std::string s;
		for (int i = z; i < z + l; i++) { // alle Bytes duchgehen
			s += static_cast<char>(b[i]);
		}
		z += s.length() - 1; // z aktualisieren
		if (tagebuchmode && s == ("[]")) {
			tagebuchmode = false;
			return "";
		}
		if (tagebuchmode) {
			return "";
		}
		return s;
	}
}

SavegameManager::SavegameManager(QObject * par) : QObject(par), _variables() {
}

SavegameManager::~SavegameManager() {
}

void SavegameManager::load(QString saveFile) {
	QFile f(saveFile);
	if (f.open(QIODevice::ReadOnly)) {
		QByteArray bytes = f.readAll();
		_variables.clear(); //_variables resetten
		dialogMode = true;
		dialogBegin = false;

		int position = 0;
		std::string varname;
		int value = 0;
		bool readmode = false;
		int i = 0;
		//Eintrittspunkt
		int count0A = 0;
		int maxByte = 0;
		while (i < bytes.length()) { //Überspringt den Einleutungskram und sucht einen guten Einstiegspunkt
			if (bytes.at(i) == 0x0A) {
				count0A++;
				if (count0A > 6) { // Die Anzahl der Absätze 0A sollte fest sein.
					i++;
				}
			}
			if (bytes.at(i) == 0x02	&& bytes.at(i + 1) == 0x00 && bytes.at(i + 2) == 0x00 && bytes.at(i + 3) == 0x00 && bytes.at(i + 4) == 0x01 && bytes.at(i + 5) == 0x00 && bytes.at(i + 6) == 0x00 && bytes.at(i + 7) == 0x00) {
				break;
			}
			i++;
		}

		i += 8; // 02 00 00 00 01 00 00 00 Dann kommt die Maxanzahl;
		memcpy(&maxByte, bytes.constData() + i, 4);
		i += 3;

		while (i < maxByte) { // wir gehen die Bytes durch
			if (bytes.at(i) == 0x12) { // 12er sind ein guter Anker
				if (dialogMode) { // Beim Dialogmode muss erst der Wert und dann der _variablesstring gelesen werden, Dialoge sind leider Extrawürste
					value = jumpNr(bytes, i, position); // alles per Referenz
				} else {
					if (readmode && QString::fromStdString(varname).trimmed().length() > 0) { // readmode gibt an, ob schon ein string eingelesen worden ist, da bei normalen _variables nach dem _variablesstring der Wert kommt.
						value = jumpNr(bytes, i, position); // Wert auslesen.

						if (dialogEnd) { // Wenn Das ende erreicht ist, soll keine Variableiable gespeichert werden, da der Letzte Dia dann einen falschen Wert bekommt
							dialogEnd = false;
						} else {
							_variables.push_back(Variable(varname, value, position)); // Variableiable erzeugen
						}
						i--; // eins zurück wegen unten dem whilebedingten i++
						readmode = false; // es soll ein neuer _variablesstring eingelesen werden, bis ein wert eingelesen wird
					}
				}
			} else if (bytes.at(i) == 0x01) { // Wenn wir stattdessen auf eine 1 stoßen, gehts um einen String
				const int strlength = readLength(bytes, i); // Länge des _variablesstrings bestimmen
				if (strlength > 0) { // Gültig?
					varname = readName(bytes, i, strlength); // Name Lesen
				}

				if (QString::fromStdString(varname).trimmed().length() > 0) { // gültig?
					if (dialogMode && dialogBegin) { // Im dialogmodus kommt der _variablestring zum Schluss, daher wird hier die Variableiable erzeugt
						_variables.push_back(Variable(varname, value, position));
					} else {
						readmode = true; // Name wurde eingelesen, jetzt kann der Wert eingelesen werden
					}
				}
			}
			i++;
		}
		std::sort(_variables.begin(), _variables.end(), [](const Variable & a, const Variable & b) {
			return a.name < b.name;
		});
	}
}

void SavegameManager::save(QString saveFile, QList<Variable> variables) {
	QFile f(saveFile);
	if (!f.open(QIODevice::ReadOnly)) {
		return;
	}
	QByteArray bytes = f.readAll();
	for (Variable v : variables) {
		if (v.changed) {
			memcpy(const_cast<char *>(bytes.constData()) + v.pos, &v.value, 4);
		}
	}
	QFile outFile(saveFile);
	if (outFile.open(QIODevice::WriteOnly)) {
		outFile.write(bytes);
	}
}
