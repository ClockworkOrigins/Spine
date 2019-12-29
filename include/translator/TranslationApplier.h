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

#include <atomic>
#include <cstdint>

#include <QObject>

namespace spine {
namespace translator {

	class TranslationApplier : public QObject {
		Q_OBJECT

	public:
		TranslationApplier(uint32_t requestID, QObject * par);

		void parseTexts(QString path);
		void parseFile(QString filePath, QString basePath, QString translatedPath, QMap<QString, QString> names, QMap<QString, QString> texts, QList<QPair<QStringList, QStringList>> dialogs);

	signals:
		void updatedCurrentProgress(int current);
		void updateMaxProgress(int max);
		void finished();

	private:
		uint32_t _requestID;
		std::atomic<int> _currentProgress;
		int _maxProgress;

		void parseName(QString line, QMap<QString, QString> names, QString & newFileContent);
	};

} /* namespace translator */
} /* namespace spine */
