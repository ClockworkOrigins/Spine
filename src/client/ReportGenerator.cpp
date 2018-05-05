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

#include "ReportGenerator.h"

#include "Config.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QtConcurrentRun>
#include <QDirIterator>
#include <QFutureWatcher>
#include <QProgressDialog>

namespace spine {

	void ReportGenerator::generateReport(QString folder, QString filename) {
		QStringList fileList;
		QDirIterator it(folder, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
		while (it.hasNext()) {
			it.next();
			fileList << it.filePath();
		}
		if (fileList.empty()) {
			return;
		}
		QProgressDialog progressDialog(QApplication::tr("GenerateReport"), "", 0, fileList.size());
		progressDialog.setCancelButton(nullptr);
		progressDialog.setWindowFlags(progressDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		connect(this, &ReportGenerator::updateProgress, &progressDialog, &QProgressDialog::setValue);
		QEventLoop loop;
		QFutureWatcher<void> watcher;
		connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
		const QFuture<void> future = QtConcurrent::run([this, fileList, filename]() {
			QString directory = Config::BASEDIR + "/reports";
			if (!QDir(directory).exists()) {
				bool b = QDir().mkpath(directory);
				Q_UNUSED(b);
			}
			QFile f(directory + "/" + filename + ".txt");
			if (f.open(QIODevice::WriteOnly)) {
				QTextStream ts(&f);
				int currentProgress = 0;
				for (int i = 0; i < fileList.size(); i++) {
					QFile checkFile(fileList[i]);
					if (checkFile.open(QIODevice::ReadOnly)) {
						QCryptographicHash hash(QCryptographicHash::Sha512);
						hash.addData(&checkFile);
						QString hashSum = QString::fromLatin1(hash.result().toHex());
						ts << fileList[i] << ": " << hashSum << "\n";
					}
					emit updateProgress(++currentProgress);
				}
			}
		});
		watcher.setFuture(future);
		loop.exec();
	}

} /* namespace spine */
