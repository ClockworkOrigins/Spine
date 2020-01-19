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

#include <cstdint>

#include <QProgressDialog>

class QMainWindow;
class QWinTaskbarProgress;

namespace spine {
namespace widgets {

	class GeneralSettingsWidget;

	class IntegrityCheckDialog : public QProgressDialog {
		Q_OBJECT

	public:
		struct ModFile {
			int32_t modID;
			QString file;
			QString hash;

			ModFile() : modID(), file(), hash() {
			}
			ModFile(std::string s1, std::string s2, std::string s3) : modID(std::stoi(s1)), file(QString::fromStdString(s2)), hash(QString::fromStdString(s3)) {
			}
			ModFile(QString s1, QString s2) : modID(-1), file(s1), hash(s2) {
			}
		};

		IntegrityCheckDialog(QMainWindow * mainWindow, QWidget * par);

		QList<ModFile> getCorruptFiles() const {
			return _corruptFiles;
		}

		QList<ModFile> getCorruptGothicFiles() const {
			return _corruptGothicFiles;
		}

		QList<ModFile> getCorruptGothic2Files() const {
			return _corruptGothic2Files;
		}

	signals:
		void updateText(QString);
		void updateValue(int);
		void updateCount(int);

	public slots:
		int exec() override;
		void setGothicDirectory(QString path);
		void setGothic2Directory(QString path);

	private slots:
		void setText(QString text);
		void setValue(int value);
		void setMaximum(int max);
		void cancel();

	private:
		QWinTaskbarProgress * _taskbarProgress;
		bool _running;
		QList<ModFile> _corruptFiles;
		QList<ModFile> _corruptGothicFiles;
		QList<ModFile> _corruptGothic2Files;
		QString _gothicDirectory;
		QString _gothic2Directory;

		void closeEvent(QCloseEvent * evt) override;
		void process();
	};

} /* namespace widgets */
} /* namespace spine */
