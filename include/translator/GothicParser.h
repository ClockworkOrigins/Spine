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

#include <atomic>

#include <QObject>

namespace translator {
namespace common {
	class TranslationModel;
} /* namespace common */
} /* namespace translator */

namespace spine {
namespace translation {

	class GothicParser : public QObject {
		Q_OBJECT

	public:
		GothicParser(QObject * par);

		void parseTexts(QString path, translator::common::TranslationModel * model);
		void parseFile(QString filePath, translator::common::TranslationModel * model);

	signals:
		void updatedProgress(int current, int max);
		void finished();

	private:
		std::atomic<int> _currentProgress;
		int _maxProgress;

		void parseName(QString line, translator::common::TranslationModel * model);
	};

} /* namespace translation */
} /* namespace spine */
