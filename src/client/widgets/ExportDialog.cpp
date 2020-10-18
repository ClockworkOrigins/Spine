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

#include "widgets/ExportDialog.h"

#include "SpineConfig.h"

#include "utils/Compression.h"
#include "utils/Config.h"
#include "utils/Database.h"
#include "utils/Conversion.h"

#include <QApplication>
#include <QtConcurrentRun>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

ExportDialog::ExportDialog(QWidget * par) : QDialog(par), _exportPathLineEdit(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	QLabel * lbl = new QLabel(QApplication::tr("ExportDescription"), this);
	lbl->setWordWrap(true);
	l->addWidget(lbl);
	{
		QHBoxLayout * hl = new QHBoxLayout();

		QLabel * exportPathLabel = new QLabel(QApplication::tr("ExportPath"), this);
		_exportPathLineEdit = new QLineEdit(this);
		_exportPathLineEdit->setReadOnly(true);
		_exportPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		QPushButton * exportPathPushButton = new QPushButton("...", this);
		hl->addWidget(exportPathLabel);
		hl->addWidget(_exportPathLineEdit);
		hl->addWidget(exportPathPushButton);
		connect(exportPathPushButton, &QPushButton::released, this, &ExportDialog::openExportPathDialog);

		hl->setAlignment(Qt::AlignLeft);

		l->addLayout(hl);
	}
	_exportPushButton = new QPushButton(QApplication::tr("Export"), this);
	connect(_exportPushButton, &QPushButton::released, this, &ExportDialog::exportMods);
	l->addWidget(_exportPushButton);
	_exportPushButton->setDisabled(true);

	setLayout(l);

	setWindowTitle(QApplication::tr("Export"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

ExportDialog::~ExportDialog() {
}

void ExportDialog::openExportPathDialog() {
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectExportDir"), _exportPathLineEdit->text());
	if (!path.isEmpty()) {
		if (_exportPathLineEdit->text() != path) {
			_exportPathLineEdit->setText(path);
			const QDir exportDir(_exportPathLineEdit->text() + "/");
			const bool exists = exportDir.exists();
			_exportPushButton->setEnabled(!_exportPathLineEdit->text().isEmpty() && exists);
		}
	}
}

void ExportDialog::exportMods() {
	if (_exportPathLineEdit->text().isEmpty() || !QDir(_exportPathLineEdit->text()).exists()) {
		return;
	}
	Database::DBError dbErr;
	std::vector<std::vector<std::string>> modfiles = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, File, Hash FROM modfiles;", dbErr);

	if (modfiles.empty()) {
		return;
	}

	QProgressDialog dlg(QApplication::tr("Exporting").arg(QString::fromStdString(modfiles[0][1])), QApplication::tr("Cancel"), 0, modfiles.size(), this);
	dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	connect(this, &ExportDialog::updateProgress, &dlg, &QProgressDialog::setValue);
	connect(this, &ExportDialog::updateFile, [&dlg](QString file) {
		dlg.setLabelText(QApplication::tr("Exporting").arg(file));
	});
	bool running = true;
	QFutureWatcher<void> watcher(this);
	QString exportPath = _exportPathLineEdit->text();
	const QFuture<void> future = QtConcurrent::run([this, modfiles, exportPath, &running]() {
		const std::string database = exportPath.toStdString() + "/backup.spex";
		Database::DBError err;
		Database::execute(database, "CREATE TABLE IF NOT EXISTS mods(ModID INT NOT NULL, GothicVersion INT NOT NULL, MajorVersion INT NOT NULL, MinorVersion INT NOT NULL, PatchVersion INT NOT NULL, SpineVersion INT NOT NULL);", err);
		Database::execute(database, "CREATE TABLE IF NOT EXISTS modfiles(ModID INT NOT NULL, File TEXT NOT NULL, Hash TEXT NOT NULL);", err);
		Database::execute(database, "CREATE TABLE IF NOT EXISTS patches(ModID INT NOT NULL, Name TEXT NOT NULL);", err);
		Database::execute(database, "CREATE TABLE IF NOT EXISTS packages(ModID INT NOT NULL, PackageID INT NOT NULL, File TEXT NOT NULL);", err);

		const auto mods = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, GothicVersion, MajorVersion, MinorVersion, PatchVersion, SpineVersion FROM mods;", err);
		Database::open(database, err);
		Database::execute(database, "BEGIN TRANSACTION;", err);
		for (auto vec : mods) {
			Database::execute(database, "INSERT INTO mods (ModID, GothicVersion, MajorVersion, MinorVersion, PatchVersion, SpineVersion) VALUES (" + vec[0] + ", " + vec[1] + ", " + vec[2] + ", " + vec[3] + ", " + vec[4] + ", " + vec[5] + ");", err);
		}
		Database::execute(database, "END TRANSACTION;", err);
		Database::close(database, err);

		const auto patches = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, Name FROM patches;", err);
		Database::open(database, err);
		Database::execute(database, "BEGIN TRANSACTION;", err);
		for (auto vec : patches) {
			Database::execute(database, "INSERT INTO patches (ModID, Name) VALUES (" + vec[0] + ", '" + vec[1] + "');", err);
		}
		Database::execute(database, "END TRANSACTION;", err);
		Database::close(database, err);

		const auto packages = Database::queryAll<std::vector<std::string>, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT ModID, PackageID, File FROM packages;", err);
		Database::open(database, err);
		Database::execute(database, "BEGIN TRANSACTION;", err);
		for (auto vec : packages) {
			Database::execute(database, "INSERT INTO packages (ModID, PackageID, File) VALUES (" + vec[0] + ", " + vec[1] + ", '" + vec[2] + "');", err);
		}
		Database::execute(database, "END TRANSACTION;", err);
		Database::close(database, err);

		Database::open(database, err);
		Database::execute(database, "BEGIN TRANSACTION;", err);
		for (auto vec : modfiles) {
			Database::execute(database, "INSERT INTO modfiles (ModID, File, Hash) VALUES (" + vec[0] + ", '" + vec[1] + "', '" + vec[2] + "');", err);
		}
		Database::execute(database, "END TRANSACTION;", err);
		Database::close(database, err);

		for (size_t i = 0; i < modfiles.size() && running; i++) {
			emit updateProgress(static_cast<int>(i));
			emit updateFile(QString::fromStdString(modfiles[i][1]));
			QString targetFolder = exportPath + "/" + QString::fromStdString(modfiles[i][0]) + "/";
			if (!QDir(targetFolder + QFileInfo(QString::fromStdString(modfiles[i][1])).path()).exists()) {
				bool b = QDir(targetFolder + QFileInfo(QString::fromStdString(modfiles[i][1])).path()).mkpath(targetFolder + QFileInfo(QString::fromStdString(modfiles[i][1])).path());
				Q_UNUSED(b);
			}
			utils::Compression::compress(Config::DOWNLOADDIR + "/mods/" + s2q(modfiles[i][0]) + "/" + s2q(modfiles[i][1]), targetFolder + s2q(modfiles[i][1]), false);
		}
		emit updateProgress(static_cast<int>(modfiles.size()));
	});
	watcher.setFuture(future);
	connect(&dlg, &QProgressDialog::canceled, &watcher, &QFutureWatcher<void>::cancel);
	connect(&dlg, &QProgressDialog::canceled, [&running]() {
		running = false;
	});
	const int result = dlg.exec();
	watcher.waitForFinished();
	if (result == QDialog::DialogCode::Accepted) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("ExportSuccessful"), QApplication::tr("ExportSuccessfulText"), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
	} else {
		QMessageBox msg(QMessageBox::Icon::Critical, QApplication::tr("ExportUnsuccessful"), QApplication::tr("ExportUnsuccessfulText"), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
	}
}
