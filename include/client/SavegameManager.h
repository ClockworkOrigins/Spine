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

#pragma once

#include <QObject>

namespace spine {

	struct Variable {
		std::string name;
		int value;
		bool changed;
		int pos;

		Variable(std::string n, int v, int p) : name(n), value(v), changed(false), pos(p) {
		}
	};

	class SavegameManager : public QObject {
	public:
		SavegameManager(QObject * par);

		void load(const QString & saveFile);
		static void save(const QString & saveFile, const QList<Variable> & variables);

		QList<Variable> getVariables() const {
			return _variables;
		}

	private:
		QList<Variable> _variables;
	};

} /* namespace spine */
