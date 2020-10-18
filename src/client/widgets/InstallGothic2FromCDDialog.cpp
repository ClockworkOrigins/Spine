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

#include "widgets/InstallGothic2FromCDDialog.h"

#ifdef Q_OS_WIN
#include "utils/Conversion.h"
#include "utils/WindowsExtensions.h"

#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <Windows.h>
#include <shellapi.h>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

InstallGothic2FromCDDialog::InstallGothic2FromCDDialog() : QDialog(), _gothicPathLineEdit(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();

	QLabel * lbl = new QLabel(QApplication::tr("SelectGothic2InstallationFolder"), this);
	UPDATELANGUAGESETTEXT(lbl, "SelectGothic2InstallationFolder");

	l->addWidget(lbl);

	QHBoxLayout * hl = new QHBoxLayout();

	_gothicPathLineEdit = new QLineEdit(QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", QProcessEnvironment::systemEnvironment().value("ProgramFiles")) + "/JoWooD/Gothic II", this);
	_gothicPathLineEdit->setReadOnly(true);
	_gothicPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	QPushButton * gothicPathPushButton = new QPushButton("...", this);
	hl->addWidget(_gothicPathLineEdit);
	hl->addWidget(gothicPathPushButton);
	connect(gothicPathPushButton, &QPushButton::released, this, &InstallGothic2FromCDDialog::openGothicFileDialog);

	hl->setAlignment(Qt::AlignLeft);

	l->addLayout(hl);

	QPushButton * installButton = new QPushButton(QApplication::tr("Install"), this);
	UPDATELANGUAGESETTEXT(installButton, "Install");
	connect(installButton, &QPushButton::released, this, &InstallGothic2FromCDDialog::startInstallation);

	l->addWidget(installButton);

	setLayout(l);

	setWindowTitle(QApplication::tr("InstallGothic2FromCD"));
	UPDATELANGUAGESETWINDOWTITLE(this, "InstallGothic2FromCD");

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	setMinimumWidth(500);
}

int InstallGothic2FromCDDialog::exec() {
	return QDialog::exec();
}

void InstallGothic2FromCDDialog::openGothicFileDialog() {
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothic2Dir"), _gothicPathLineEdit->text());
	if (!path.isEmpty()) {
		if (_gothicPathLineEdit->text() != path) {
			_gothicPathLineEdit->setText(path);
		}
	}
}

void InstallGothic2FromCDDialog::startInstallation() {
	if (_gothicPathLineEdit->text().contains(QProcessEnvironment::systemEnvironment().value("ProgramFiles(x86)", QProcessEnvironment::systemEnvironment().value("ProgramFiles"))) && !IsRunAsAdmin()) {
		QMessageBox resultMsg(QMessageBox::Icon::Critical, QApplication::tr("AdminRightsRequired"), QApplication::tr("InstallationRequiresAdminRightsText").arg(_gothicPathLineEdit->text()), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		resultMsg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		if (QMessageBox::StandardButton::Ok == resultMsg.exec()) {
			const QString exeFileName = qApp->applicationDirPath() + "/" + qApp->applicationName();
			const int result = reinterpret_cast<int>(::ShellExecuteA(nullptr, "runas", exeFileName.toUtf8().constData(), nullptr, nullptr, SW_SHOWNORMAL));
			if (result > 32) { // no error
				qApp->quit();
			}
		}
		return;
	}
	QString g2Cdr = ".";
	while (!QFileInfo::exists(g2Cdr + "/Gothic2-Setup.exe")) {
		QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("EnterGothic2CD"), QApplication::tr("EnterGothic2CDText"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		resultMsg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
		
		if (QMessageBox::StandardButton::Cancel == resultMsg.exec()) return;

		for (const QFileInfo & fi : QDir::drives()) {
			if (QFileInfo::exists(fi.absolutePath() + "/Gothic2-Setup.exe")) {
				g2Cdr = fi.absolutePath();
				break;
			}
		}
	}

	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("G2DIR", _gothicPathLineEdit->text());
	env.insert("G2CDR", g2Cdr);
	QProcess * installProcess = new QProcess();
	connect(installProcess, &QProcess::readyRead, this, &InstallGothic2FromCDDialog::updateLog);
	connect(installProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &InstallGothic2FromCDDialog::startAddonInstallation);
	connect(installProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), installProcess, &QProcess::deleteLater);
	installProcess->setProcessChannelMode(QProcess::MergedChannels);
	installProcess->setWorkingDirectory(qApp->applicationDirPath() + "/../");
	installProcess->setEnvironment(env.toStringList());
	QProgressDialog * dlg = new QProgressDialog();
	QTextBrowser * tb = new QTextBrowser(dlg);
	QVBoxLayout * l = new QVBoxLayout();
	l->addWidget(tb);
	dlg->setLayout(l);
	dlg->setCancelButton(nullptr);
	connect(this, &InstallGothic2FromCDDialog::updateProgressLog, tb, &QTextBrowser::setText);
	connect(installProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), dlg, &QProgressDialog::accept);
	installProcess->start("cmd.exe", QStringList() << "/c" << "Gothic2-Setup.bat");
	dlg->exec();
	delete dlg;
}

void InstallGothic2FromCDDialog::startAddonInstallation(int exitCode, QProcess::ExitStatus exitStatus) {
	const bool b = exitCode == 0 && exitStatus == QProcess::ExitStatus::NormalExit;
	if (b) {
		QString g2Cdr = ".";
		while (!QFileInfo::exists(g2Cdr + "/Gothic2-Addon-Setup.exe")) {
			QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("EnterGothic2CD"), QApplication::tr("EnterGothic2AddonCDText"), QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
			resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
			resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			resultMsg.button(QMessageBox::StandardButton::Cancel)->setText(QApplication::tr("Cancel"));
			
			if (QMessageBox::StandardButton::Cancel == resultMsg.exec()) return;

			for (const QFileInfo & fi : QDir::drives()) {
				if (QFileInfo::exists(fi.absolutePath() + "/Gothic2-Addon-Setup.exe")) {
					g2Cdr = fi.absolutePath();
					break;
				}
			}
		}

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("G2DIR", _gothicPathLineEdit->text());
		env.insert("G2CDR", g2Cdr);
		QProcess * installProcess = new QProcess();
		connect(installProcess, &QProcess::readyRead, this, &InstallGothic2FromCDDialog::updateLog);
		connect(installProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &InstallGothic2FromCDDialog::finishedInstallation);
		connect(installProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), installProcess, &QProcess::deleteLater);
		installProcess->setProcessChannelMode(QProcess::MergedChannels);
		installProcess->setWorkingDirectory(qApp->applicationDirPath() + "/../");
		installProcess->setEnvironment(env.toStringList());
		QProgressDialog * dlg = new QProgressDialog();
		QTextBrowser * tb = new QTextBrowser(dlg);
		QVBoxLayout * l = new QVBoxLayout();
		l->addWidget(tb);
		dlg->setLayout(l);
		dlg->setCancelButton(nullptr);
		connect(this, &InstallGothic2FromCDDialog::updateProgressLog, tb, &QTextBrowser::setText);
		connect(installProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), dlg, &QProgressDialog::accept);
		installProcess->start("cmd.exe", QStringList() << "/c" << "Gothic2-Addon-Setup.bat");
		dlg->exec();
		delete dlg;
	} else {
		QMessageBox resultMsg(QMessageBox::Icon::Critical, QApplication::tr("ErrorOccurred"), QApplication::tr("ErrorDuringGothic2Installation") + "\n" + _installationLog, QMessageBox::StandardButton::Ok);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		resultMsg.exec();
		QFile logFile(qApp->applicationDirPath() + "/../installation.log");
		if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			logFile.write(_installationLog.toStdString().c_str(), _installationLog.length());
		}
	}
}

void InstallGothic2FromCDDialog::finishedInstallation(int exitCode, QProcess::ExitStatus exitStatus) {
	const bool b = exitCode == 0 && exitStatus == QProcess::ExitStatus::NormalExit;
	if (b) {
		QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("InstallationSuccessful"), QApplication::tr("Gothic2InstallationSuccessfulText"), QMessageBox::StandardButton::Ok);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		resultMsg.exec();
		emit updateGothic2Directory(_gothicPathLineEdit->text());
	} else {
		QMessageBox resultMsg(QMessageBox::Icon::Critical, QApplication::tr("ErrorOccurred"), QApplication::tr("ErrorDuringGothic2Installation") + "\n" + _installationLog, QMessageBox::StandardButton::Ok);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		resultMsg.exec();
		QFile logFile(qApp->applicationDirPath() + "/../installation.log");
		if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			logFile.write(_installationLog.toStdString().c_str(), _installationLog.length());
		}
	}
}

void InstallGothic2FromCDDialog::updateLog() {
	QProcess * p = qobject_cast<QProcess *>(sender());
	if (p) {
		const QString newLog = s2q(p->readAll());
		_installationLog.append(newLog);
		emit updateProgressLog(_installationLog);
	}
}

#endif
