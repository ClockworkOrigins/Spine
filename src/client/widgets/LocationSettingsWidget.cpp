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

#include "widgets/LocationSettingsWidget.h"

#include "DirValidator.h"

#include "utils/Config.h"

#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QCheckBox>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFutureSynchronizer>
#include <QFutureWatcher>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QtConcurrentRun>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

LocationSettingsWidget * LocationSettingsWidget::instance = nullptr;

LocationSettingsWidget::LocationSettingsWidget(bool temporary, QWidget * par) : QWidget(par), _gothicPathLineEdit(nullptr), _gothic2PathLineEdit(nullptr), _downloadPathLineEdit(nullptr), _screenshotPathLineEdit(nullptr), _futureCounter(0), _cancelSearch(false), _gothicSteam(nullptr), _gothic2Steam(nullptr) {
	instance = this;
	
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QLabel * infoLabel = new QLabel(QApplication::tr("GothicPathDescription"), this);
		infoLabel->setWordWrap(true);
		if (!temporary) {
			UPDATELANGUAGESETTEXT(infoLabel, "GothicPathDescription");
		}
		l->addWidget(infoLabel);
		infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		infoLabel->setMinimumHeight(100);
		l->addSpacing(10);
	}
	{
		QGridLayout * hl = new QGridLayout();

		QString path = Config::IniParser->value("PATH/Gothic", "").toString();

		QLabel * gothicPathLabel = new QLabel(QApplication::tr("GothicPath"), this);
		if (!temporary) {
			UPDATELANGUAGESETTEXT(gothicPathLabel, "GothicPath");
		}
		_gothicPathLineEdit = new QLineEdit(path, this);
		_gothicPathLineEdit->setValidator(new DirValidator());
		_gothicPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		QPushButton * gothicPathPushButton = new QPushButton("...", this);
		_gothicSteam = new QCheckBox(QApplication::tr("StartWithSteam"), this);
		UPDATELANGUAGESETTEXT(_gothicSteam, "StartWithSteam");
		hl->addWidget(gothicPathLabel, 0, 0);
		hl->addWidget(_gothicPathLineEdit, 0, 1);
		hl->addWidget(gothicPathPushButton, 0, 2);
		hl->addWidget(_gothicSteam, 0, 3);
		connect(gothicPathPushButton, &QPushButton::released, this, &LocationSettingsWidget::openGothicFileDialog);

		_gothicSteam->setChecked(Config::IniParser->value("PATH/GothicWithSteam", false).toBool());

		hl->setAlignment(Qt::AlignLeft);

		path = Config::IniParser->value("PATH/Gothic2", "").toString();

		gothicPathLabel = new QLabel(QApplication::tr("Gothic2Path"), this);
		if (!temporary) {
			UPDATELANGUAGESETTEXT(gothicPathLabel, "Gothic2Path");
		}
		_gothic2PathLineEdit = new QLineEdit(path, this);
		_gothic2PathLineEdit->setValidator(new DirValidator());
		_gothic2PathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		gothicPathPushButton = new QPushButton("...", this);
		_gothic2Steam = new QCheckBox(QApplication::tr("StartWithSteam"), this);
		UPDATELANGUAGESETTEXT(_gothic2Steam, "StartWithSteam");
		hl->addWidget(gothicPathLabel, 1, 0);
		hl->addWidget(_gothic2PathLineEdit, 1, 1);
		hl->addWidget(gothicPathPushButton, 1, 2);
		hl->addWidget(_gothic2Steam, 1, 3);
		connect(gothicPathPushButton, &QPushButton::released, this, &LocationSettingsWidget::openGothic2FileDialog);

		_gothic2Steam->setChecked(Config::IniParser->value("PATH/Gothic2WithSteam", false).toBool());

		hl->setAlignment(Qt::AlignLeft);

		l->addLayout(hl);
		l->addSpacing(10);
	}
	{
		QPushButton * w = new QPushButton(QApplication::tr("SearchGothic"), this);
		if (!temporary) {
			UPDATELANGUAGESETTEXT(w, "SearchGothic");
		}
		w->setToolTip(QApplication::tr("SearchGothicText"));
		if (!temporary) {
			UPDATELANGUAGESETTOOLTIP(w, "SearchGothicText");
		}
		connect(w, &QPushButton::released, this, &LocationSettingsWidget::searchGothic);

		l->addWidget(w);
		l->addSpacing(10);
	}
	{
		QHBoxLayout * hl = new QHBoxLayout();

		const QString path = Config::IniParser->value("PATH/Downloads", Config::BASEDIR).toString();

		QLabel * downloadPathLabel = new QLabel(QApplication::tr("DownloadPath"), this);
		if (!temporary) {
			UPDATELANGUAGESETTEXT(downloadPathLabel, "DownloadPath");
		}
		_downloadPathLineEdit = new QLineEdit(path, this);
		_downloadPathLineEdit->setReadOnly(true);
		_downloadPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		QPushButton * downloadPathPushButton = new QPushButton("...", this);
		hl->addWidget(downloadPathLabel);
		hl->addWidget(_downloadPathLineEdit);
		hl->addWidget(downloadPathPushButton);
		connect(downloadPathPushButton, &QPushButton::released, this, &LocationSettingsWidget::openDownloadFileDialog);

		hl->setAlignment(Qt::AlignLeft);

		l->addLayout(hl);
	}
	{
		QHBoxLayout * hl = new QHBoxLayout();

		const QString path = Config::IniParser->value("PATH/Screenshots", Config::BASEDIR + "/screens/").toString();

		QLabel * screenshotPathLabel = new QLabel(QApplication::tr("ScreenshotPath"), this);
		if (!temporary) {
			UPDATELANGUAGESETTEXT(screenshotPathLabel, "ScreenshotPath");
		}
		_screenshotPathLineEdit = new QLineEdit(path, this);
		_screenshotPathLineEdit->setReadOnly(true);
		_screenshotPathLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		screenshotPathLabel->setToolTip(QApplication::tr("ScreenshotTooltip"));
		if (!temporary) {
			UPDATELANGUAGESETTOOLTIP(screenshotPathLabel, "ScreenshotTooltip");
		}
		_screenshotPathLineEdit->setToolTip(QApplication::tr("ScreenshotTooltip"));
		if (!temporary) {
			UPDATELANGUAGESETTOOLTIP(_screenshotPathLineEdit, "ScreenshotTooltip");
		}
		QPushButton * screenshotPathPushButton = new QPushButton("...", this);
		hl->addWidget(screenshotPathLabel);
		hl->addWidget(_screenshotPathLineEdit);
		hl->addWidget(screenshotPathPushButton);
		connect(screenshotPathPushButton, &QPushButton::released, this, &LocationSettingsWidget::openScreenshotDirectoryFileDialog);

		hl->setAlignment(Qt::AlignLeft);

		l->addLayout(hl);
	}

	l->addStretch(1);

	setLayout(l);

	connect(this, &LocationSettingsWidget::foundGothic, this, &LocationSettingsWidget::setGothicDirectory);
	connect(this, &LocationSettingsWidget::foundGothic2, this, &LocationSettingsWidget::setGothic2Directory);
}

LocationSettingsWidget * LocationSettingsWidget::getInstance() {
	return instance;
}

void LocationSettingsWidget::saveSettings() {
	bool changedG1Path = false;
	bool changedG2Path = false;
	{
		const QString path = Config::IniParser->value("PATH/Gothic", "").toString();
		if (path != _gothicPathLineEdit->text()) {
			if (dynamic_cast<const DirValidator *>(_gothicPathLineEdit->validator())->isValid(_gothicPathLineEdit->text()) && isGothicValid(false)) {
				changedG1Path = true;
				Config::IniParser->setValue("PATH/Gothic", _gothicPathLineEdit->text());
			} else {
				if (!_gothicPathLineEdit->text().isEmpty()) {
					QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("InvalidPath"), QApplication::tr("InvalidGothicPath"), QMessageBox::StandardButton::Ok);
					resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
					resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
					resultMsg.exec();
				}
				_gothicPathLineEdit->setText("");
			}
		}
		Config::IniParser->setValue("PATH/GothicWithSteam", _gothicSteam->isChecked());
	}
	{
		const QString path = Config::IniParser->value("PATH/Gothic2", "").toString();
		if (path != _gothic2PathLineEdit->text()) {
			if (dynamic_cast<const DirValidator *>(_gothic2PathLineEdit->validator())->isValid(_gothic2PathLineEdit->text()) && isGothic2Valid(false)) {
				changedG2Path = true;
				Config::IniParser->setValue("PATH/Gothic2", _gothic2PathLineEdit->text());
			} else {
				if (!_gothic2PathLineEdit->text().isEmpty()) {
					QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("InvalidPath"), QApplication::tr("InvalidGothic2Path"), QMessageBox::StandardButton::Ok);
					resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
					resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
					resultMsg.exec();
				}
				_gothic2PathLineEdit->setText("");
			}
		}
		Config::IniParser->setValue("PATH/Gothic2WithSteam", _gothic2Steam->isChecked());
	}
	if (changedG1Path || changedG2Path) {
		emit pathChanged();
		if (changedG1Path) {
			emit validGothic(isGothicValid(false));
		}
		if (changedG2Path) {
			emit validGothic2(isGothic2Valid(false));
		}
	}
	Config::IniParser->setValue("PATH/Downloads", _downloadPathLineEdit->text());
	if (Config::DOWNLOADDIR != _downloadPathLineEdit->text()) {
		{
			QDir oldDir(Config::DOWNLOADDIR + "/mods");
			const QDir dir = _downloadPathLineEdit->text() + "/mods";
			if (oldDir.exists() && !dir.exists()) {
				QDirIterator it(oldDir.absolutePath(), QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
				QStringList files;
				while (it.hasNext()) {
					QString fileName = it.filePath();
					if (!fileName.isEmpty()) {
						files.append(fileName);
					}
					it.next();
				}
				QString fileName = it.filePath();
				for (QString file : files) {
					QFileInfo fi(file);
					QDir newDir(dir.absolutePath() + fi.absolutePath().replace(oldDir.absolutePath(), ""));
					if (!newDir.exists()) {
						bool b = newDir.mkpath(newDir.absolutePath());
						Q_UNUSED(b);
					}
					QFile copyFile(file);
					copyFile.rename(dir.absolutePath() + file.replace(oldDir.absolutePath(), ""));
				}
				oldDir.removeRecursively();
			}
			if (!dir.exists()) {
				bool b = dir.mkpath(dir.absolutePath());
				Q_UNUSED(b);
			}
		}
		Config::DOWNLOADDIR = _downloadPathLineEdit->text();
		emit downloadPathChanged();
	}
	{
		const QString path = Config::IniParser->value("PATH/Screenshots", "").toString();
		if (path != _screenshotPathLineEdit->text()) {
			Config::IniParser->setValue("PATH/Screenshots", _screenshotPathLineEdit->text());
			emit screenshotDirectoryChanged(_screenshotPathLineEdit->text());
		}
	}
}

void LocationSettingsWidget::rejectSettings() {
	{
		const QString path = Config::IniParser->value("PATH/Gothic", "").toString();
		_gothicPathLineEdit->setText(path);
	}
	{
		_gothicSteam->setChecked(Config::IniParser->value("PATH/GothicWithSteam", false).toBool());
	}
	{
		const QString path = Config::IniParser->value("PATH/Gothic2", "").toString();
		_gothic2PathLineEdit->setText(path);
	}
	{
		_gothic2Steam->setChecked(Config::IniParser->value("PATH/Gothic2WithSteam", false).toBool());
	}
	{
		const QString path = Config::IniParser->value("PATH/Downloads", "").toString();
		_downloadPathLineEdit->setText(path);
	}
	{
		const QString path = Config::IniParser->value("PATH/Screenshots", Config::BASEDIR + "/screens/").toString();
		_screenshotPathLineEdit->setText(path);
	}
}

QString LocationSettingsWidget::getGothicDirectory() const {
	return _gothicPathLineEdit->text();
}

QString LocationSettingsWidget::getGothic2Directory() const {
	return _gothic2PathLineEdit->text();
}

QString LocationSettingsWidget::getScreenshotDirectory() const {
	return _screenshotPathLineEdit->text();
}

void LocationSettingsWidget::setGothicDirectory(QString path) {
	_gothicPathLineEdit->setText(path);
	Config::IniParser->setValue("PATH/Gothic", _gothicPathLineEdit->text());
	Config::IniParser->setValue("PATH/Gothic2", _gothic2PathLineEdit->text());
	emit pathChanged();
	emit validGothic(isGothicValid(false));
}

void LocationSettingsWidget::setGothic2Directory(QString path) {
	_gothic2PathLineEdit->setText(path);
	Config::IniParser->setValue("PATH/Gothic", _gothicPathLineEdit->text());
	Config::IniParser->setValue("PATH/Gothic2", _gothic2PathLineEdit->text());
	emit pathChanged();
	emit validGothic2(isGothic2Valid(false));
}

bool LocationSettingsWidget::isGothicValid(bool restored) const {
	return isGothicValid(_gothicPathLineEdit->text(), "Gothic.exe", restored);
}

bool LocationSettingsWidget::isGothic2Valid(bool restored) const {
	return isGothicValid(_gothic2PathLineEdit->text(), "Gothic2.exe", restored);
}

void LocationSettingsWidget::openGothicFileDialog() {
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothicDir"), _gothicPathLineEdit->text());
	if (!path.isEmpty()) {
		if (_gothicPathLineEdit->text() != path) {
			_gothicPathLineEdit->setText(path);
			if (!isGothicValid(false)) {
				if (isGothicValid(_gothicPathLineEdit->text() + "/../", "Gothic.exe", false)) {
					QDir g1Dir(_gothicPathLineEdit->text());
					g1Dir.cdUp();
					_gothicPathLineEdit->setText(g1Dir.absolutePath());
					return;
				}
				QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("InvalidPath"), QApplication::tr("InvalidGothicPath"), QMessageBox::StandardButton::Ok);
				resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				resultMsg.exec();
			}
		}
	}
}

void LocationSettingsWidget::openGothic2FileDialog() {
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothic2Dir"), _gothic2PathLineEdit->text());
	if (!path.isEmpty()) {
		if (_gothic2PathLineEdit->text() != path) {
			_gothic2PathLineEdit->setText(path);
			if (!isGothic2Valid(false)) {
				if (isGothicValid(_gothic2PathLineEdit->text() + "/../", "Gothic2.exe", false)) {
					QDir g2Dir(_gothic2PathLineEdit->text());
					g2Dir.cdUp();
					_gothic2PathLineEdit->setText(g2Dir.absolutePath());
					return;
				}
				QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("InvalidPath"), QApplication::tr("InvalidGothic2Path"), QMessageBox::StandardButton::Ok);
				resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				resultMsg.exec();
			}
		}
	}
}

void LocationSettingsWidget::openDownloadFileDialog() {
	QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectDownloadDir"), _downloadPathLineEdit->text());
	if (!path.isEmpty()) {
		if (_downloadPathLineEdit->text() != path) {
			// perform checks
			path = path.replace("\\", "/");
			QString programFiles = QProcessEnvironment::systemEnvironment().value("ProgramFiles", "Foobar123");
			programFiles = programFiles.replace("\\", "/");
			if (path.contains(programFiles, Qt::CaseInsensitive) || (!_gothicPathLineEdit->text().isEmpty() && path.contains(_gothicPathLineEdit->text(), Qt::CaseInsensitive)) || (!_gothic2PathLineEdit->text().isEmpty() && path.contains(_gothic2PathLineEdit->text(), Qt::CaseInsensitive))) {
				QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("InvalidPath"), QApplication::tr("InvalidDownloadPath").arg(path), QMessageBox::StandardButton::Ok);
				resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
				resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				resultMsg.exec();
				return;
			}
			_downloadPathLineEdit->setText(path);
		}
	}
}

void LocationSettingsWidget::searchGothic() {
	const bool searchG1 = !isGothicValid(false);
	const bool searchG2 = !isGothic2Valid(false);
	if (searchG1 || searchG2) {
		QProgressDialog dlg(QApplication::tr("SearchGothicWaiting"), "", 0, 1);
		dlg.setWindowTitle(QApplication::tr("SearchGothic"));
		dlg.setCancelButton(nullptr);
		dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		QLabel * lbl = new QLabel(QApplication::tr("SearchGothicWaiting"), &dlg);
		lbl->setWordWrap(true);
		dlg.setLabel(lbl);
		connect(this, &LocationSettingsWidget::finishedSearch, &dlg, &QDialog::accept);
		connect(this, &LocationSettingsWidget::finishedFolder, lbl, &QLabel::setText);

		_cancelSearch = false;
		_futureCounter = 0;
		QFutureWatcher<void> watcher;
		const QFuture<void> future = QtConcurrent::run(this, &LocationSettingsWidget::searchGothicAsync, searchG1, searchG2);
		watcher.setFuture(future);
		const int code = dlg.exec();
		if (code == QDialog::Rejected) {
			_cancelSearch = true;
			watcher.cancel();
		}
		watcher.waitForFinished();
	}
}

void LocationSettingsWidget::openScreenshotDirectoryFileDialog() {
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectScreenshotDir"), _screenshotPathLineEdit->text());
	if (!path.isEmpty()) {
		if (_screenshotPathLineEdit->text() != path) {
			_screenshotPathLineEdit->setText(path);
		}
	}
}

bool LocationSettingsWidget::isGothicValid(QString path, QString executable, bool restored) const {
	bool b = !path.isEmpty();
	b = b && QDir().exists(path);
	b = b && (restored || QFileInfo::exists(path + "/System/" + executable));
	return b;
}

void LocationSettingsWidget::searchGothicAsync(bool searchG1, bool searchG2) {
	QString filter = "Gothic";
	if (searchG1 && searchG2) {
		filter += "*";
	} else if (searchG2) {
		filter += "2";
	}
	filter += "*exe";
	QFutureSynchronizer<void> sync;
	bool gothicFound = !searchG1;
	bool gothic2Found = !searchG2;
	for (const QFileInfo & fi : QDir::drives()) {
		sync.addFuture(QtConcurrent::run<void>(this, &LocationSettingsWidget::checkPartition, fi.absolutePath(), filter, &gothicFound, &gothic2Found, false));
	}
	sync.waitForFinished();
}

void LocationSettingsWidget::checkPartition(QString partition, QString filter, bool * gothicFound, bool * gothic2Found, bool recursive) {
	QDirIterator it(partition, QStringList() << filter, QDir::Files);
	while (it.hasNext() && !_cancelSearch) {
		it.next();
		if (it.fileName().compare("Gothic.exe", Qt::CaseSensitivity::CaseInsensitive) == 0 && !*gothicFound) {
			emit foundGothic(QDir(QFileInfo(it.filePath()).absolutePath() + "/..").absolutePath());
			*gothicFound = true;
		} else if (it.fileName().compare("Gothic2.exe", Qt::CaseSensitivity::CaseInsensitive) == 0 && !*gothic2Found) {
			emit foundGothic2(QDir(QFileInfo(it.filePath()).absolutePath() + "/..").absolutePath());
			*gothic2Found = true;
		}
		if (*gothicFound && *gothic2Found) {
			emit finishedSearch();
			return;
		}
	}
	if (*gothicFound && *gothic2Found) {
		emit finishedSearch();
		return;
	}
	if (_cancelSearch) {
		return;
	}
	QFutureSynchronizer<void> sync;
	QDirIterator dirIt(partition, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	while (dirIt.hasNext() && !_cancelSearch) {
		dirIt.next();
		if (_futureCounter < 1000) {
			++_futureCounter;
			sync.addFuture(QtConcurrent::run<void>(this, &LocationSettingsWidget::checkPartition, dirIt.filePath(), filter, gothicFound, gothic2Found, false));
		} else {
			checkPartition(dirIt.filePath(), filter, gothicFound, gothic2Found, true);
		}
	}
	sync.waitForFinished();
	emit finishedFolder(QApplication::tr("SearchGothicWaiting") + "\n" + partition);
	if (!recursive) {
		--_futureCounter;
	}
}

bool LocationSettingsWidget::startGothicWithSteam() const {
	return _gothicSteam->isChecked();
}

bool LocationSettingsWidget::startGothic2WithSteam() const {
	return _gothic2Steam->isChecked();
}
