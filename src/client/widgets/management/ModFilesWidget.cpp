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

#include "widgets/management/ModFilesWidget.h"

#include <fstream>
#include <regex>

#include "SpineConfig.h"

#include "client/widgets/management/EnterChangelogDialog.h"

#include "common/MessageStructs.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Compression.h"
#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/GothicVdf.h"
#include "utils/Hashing.h"

#include "widgets/MainWindow.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDirIterator>
#include <QFileDialog>
#include <QFutureSynchronizer>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QTextStream>
#include <QTime>
#include <QTreeView>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;
namespace {
	enum FileRoles {
		PathRole = Qt::UserRole
	};
}

ModFilesWidget::ModFilesWidget(QWidget * par) : QWidget(par), _fileList(nullptr), _fileTreeView(new QTreeView(this)), _modIndex(-1), _majorVersionBox(nullptr), _minorVersionBox(nullptr), _patchVersionBox(nullptr), _spineVersionBox(nullptr), _waitSpinner(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * hl2 = new QHBoxLayout();
		_majorVersionBox = new QSpinBox(this);
		_majorVersionBox->setMinimum(0);
		_majorVersionBox->setMaximum(127);
		_minorVersionBox = new QSpinBox(this);
		_minorVersionBox->setMinimum(0);
		_minorVersionBox->setMaximum(127);
		_patchVersionBox = new QSpinBox(this);
		_patchVersionBox->setMinimum(0);
		_patchVersionBox->setMaximum(127);
		_spineVersionBox = new QSpinBox(this);
		_spineVersionBox->setMinimum(0);
		_spineVersionBox->setMaximum(127);
		_majorVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
		_minorVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
		_patchVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
		_spineVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
		hl2->addWidget(_majorVersionBox);
		hl2->addWidget(_minorVersionBox);
		hl2->addWidget(_patchVersionBox);
		hl2->addWidget(_spineVersionBox);
		_versionButton = new QPushButton(QApplication::tr("Submit"), this);
		_versionButton->setToolTip(QApplication::tr("UpdateVersionNumberTooltip"));
		hl2->addWidget(_versionButton);
		connect(_versionButton, &QPushButton::released, this, &ModFilesWidget::updateVersion);

		l->addLayout(hl2);
	}

	_fileList = new QStandardItemModel(_fileTreeView);
	_fileTreeView->header()->hide();
	_fileTreeView->setModel(_fileList);
	_fileTreeView->setToolTip(QApplication::tr("FileStructureTooltip"));
	connect(_fileList, &QStandardItemModel::itemChanged, this, &ModFilesWidget::changedLanguage);
	l->addWidget(_fileTreeView);

	{
		auto * hl2 = new QHBoxLayout();
		auto * pbAdd = new QPushButton("+", this);
		pbAdd->setToolTip(QApplication::tr("AddFileTooltip"));
		hl2->addWidget(pbAdd);
		connect(pbAdd, &QPushButton::released, this, static_cast<void(ModFilesWidget::*)()>(&ModFilesWidget::addFile));

		auto * pbDelete = new QPushButton("-", this);
		pbDelete->setToolTip(QApplication::tr("RemoveFileTooltip"));
		hl2->addWidget(pbDelete);
		connect(pbDelete, &QPushButton::released, this, static_cast<void(ModFilesWidget::*)()>(&ModFilesWidget::deleteFile));

		auto * pbAddFolder = new QPushButton(QApplication::tr("AddFolder"), this);
		hl2->addWidget(pbAddFolder);
		connect(pbAddFolder, &QPushButton::released, this, &ModFilesWidget::addFolder);

		l->addLayout(hl2);
	}

	auto * uploadButton = new QPushButton(QApplication::tr("Upload"), this);
	uploadButton->setToolTip(QApplication::tr("UploadFileTooltip"));
	l->addWidget(uploadButton);
	connect(uploadButton, &QPushButton::released, this, &ModFilesWidget::uploadCurrentMod);

	auto * testUpdateButton = new QPushButton(QApplication::tr("TestUpdate"), this);
	testUpdateButton->setToolTip(QApplication::tr("TestUpdateTooltip"));
	l->addWidget(testUpdateButton);
	connect(testUpdateButton, &QPushButton::released, this, &ModFilesWidget::testUpdate);

	connect(this, &ModFilesWidget::finishedUpload, this, &ModFilesWidget::finishUpload);

	setLayout(l);

	qRegisterMetaType<ManagementModFilesData>("ManagementModFilesData");

	connect(this, &ModFilesWidget::removeSpinner, [this]() {
		if (!_waitSpinner)
			return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});

	connect(this, &ModFilesWidget::loadedData, this, &ModFilesWidget::updateData);

	connect(this, &ModFilesWidget::versionUpdated, this, &ModFilesWidget::showVersionUpdate);

	QFile f(":/invalidFileChars.txt");
	
	if (!f.open(QIODevice::ReadOnly))
		return;

	QTextStream ts(&f);

	_filterString = ts.readAll();

#ifdef Q_OS_WIN
	auto * button = new QWinTaskbarButton(this);
	button->setWindow(spine::widgets::MainWindow::getInstance()->windowHandle());

	_taskbarProgress = button->progress();
	_taskbarProgress->setMinimum(0);
	_taskbarProgress->setMaximum(1);
	_taskbarProgress->setValue(0);
	_taskbarProgress->hide();
	
	connect(this, &ModFilesWidget::updateProgress, _taskbarProgress, &QWinTaskbarProgress::setValue);
	connect(this, &ModFilesWidget::updateProgressMax, _taskbarProgress, &QWinTaskbarProgress::setMaximum);
#endif
}

ModFilesWidget::~ModFilesWidget() {
	_futureWatcher.waitForFinished();
}

void ModFilesWidget::addFile() {
	// adds a file... if already existing => just update internally
	const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectFile"));
	
	if (path.isEmpty())
		return;

	const auto pathSuggestion = getPathSuggestion(path);

	QInputDialog dlg(this);
	dlg.setInputMode(QInputDialog::TextInput);
	dlg.setWindowTitle(QApplication::tr("PathInDirectoryStructure"));
	dlg.setLabelText(QApplication::tr("PathInDirectoryStructureDescription"));
	dlg.setTextValue(pathSuggestion);

	connect(&dlg, &QInputDialog::textValueChanged, this, [&dlg](const QString & text) {
		QSignalBlocker blocker(dlg);
		auto textCopy = text;
		textCopy = textCopy.remove(".");
		textCopy = textCopy.replace('\\', '/');
		if (textCopy.startsWith('/')) {
			textCopy.remove(0, 1);
		}
		dlg.setTextValue(textCopy);
	});

	if (dlg.exec() == QDialog::Rejected)
		return;
	
	const QString mapping = dlg.textValue();
	
	QStringList realMappingSplit = mapping.split("/", Qt::SkipEmptyParts);
	QString realMapping;
	for (const QString & rm : realMappingSplit) {
		if (!realMapping.isEmpty()) {
			realMapping.append("/");
		}
		realMapping += rm;
	}

	QFutureWatcher<QString> watcher;

	watcher.setFuture(QtConcurrent::run([this, path] {
		return hashFile(path);
	}));

	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QEventLoop loop;
	connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
	loop.exec();

	delete _waitSpinner;
	_waitSpinner = nullptr;

	const auto hash = watcher.result();

	if (hash.isEmpty())
		return;

	addFile(path, realMapping, QFileInfo(path).fileName(), hash);
}

void ModFilesWidget::deleteFile() {
	// removes selected modfile
	if (_fileTreeView->selectionModel()->selectedIndexes().isEmpty())
		return;

	const QModelIndex idx = _fileTreeView->selectionModel()->selectedIndexes().front();

	deleteFile(idx);
}

void ModFilesWidget::uploadCurrentMod() {
	// create diff and upload mods
	// own port on server
	_waitSpinner = new WaitSpinner(QApplication::tr("PreparingUpload"), this);
	connect(this, &ModFilesWidget::updateUploadText, _waitSpinner, &WaitSpinner::setText);
#ifdef Q_OS_WIN
	_taskbarProgress->setValue(0);
	_taskbarProgress->show();
#endif
	QtConcurrent::run([this]() {
		common::UploadModfilesMessage umm;
		umm.modID = _mods[_modIndex].id;
		umm.packageID = _mods[_modIndex].packageID;

		for (const auto & mmf : _data.files) {
			if (!mmf.deleted)
				continue;

			common::ModFile mf;
			mf.language = q2s(mmf.language);
			mf.changed = mmf.changed;
			mf.deleted = mmf.deleted;
			mf.filename = q2s(mmf.filename);
			mf.hash = q2s(mmf.hash);
			mf.size = mmf.size;

			if (!mmf.filename.endsWith(".z")) {
				mf.filename += ".z";
			}
			while (mf.filename[0] == '/') {
				mf.filename = mf.filename.substr(1);
			}
			umm.files.push_back(mf);
		}
		qint64 maxBytes = 0;
		QStringList uploadFiles;

		QMutex lock;

		QFutureSynchronizer<void> syncher;
		for (const auto & mmf : _data.files) {
			const auto f = QtConcurrent::run([this, mmf, &lock, &maxBytes, &uploadFiles, &umm] {
				common::ModFile mf;
				mf.language = q2s(mmf.language);
				mf.changed = mmf.changed;
				mf.deleted = mmf.deleted;
				mf.filename = q2s(mmf.filename);
				mf.hash = q2s(mmf.hash);

				if (!mf.changed)
					return;

				QString currentFileName = mmf.filename;
				while (currentFileName.startsWith("/")) {
					currentFileName.remove(0, 1);
				}
				if (currentFileName.endsWith(".z")) {
					currentFileName.chop(2);
				}
				const auto it = _fileMap.find(currentFileName);
				if (it == _fileMap.end()) {
					mf.size = 0;
				} else {
					// hash check
					if (mmf.oldHash == mmf.hash) { // hash the same, so just update the language
						mf.size = 0;
					} else {
						mf.uncompressedSize = getSize(it.value());

						Compression::compress(it.value(), false);

						mf.size = getSize(it.value() + ".z");
					}
				}
				if (!mmf.filename.endsWith(".z")) {
					mf.filename += ".z";
				}
				while (mf.filename[0] == '/') {
					mf.filename = mf.filename.substr(1);
				}

				QMutexLocker lg(&lock);
				if (mf.size > 0) {
					maxBytes += mf.size;
					uploadFiles.push_back(it.value() + ".z");
				}

				umm.files.push_back(mf);
			});
			syncher.addFuture(f);
		}
		syncher.waitForFinished();

		emit updateProgressMax(static_cast<int>(maxBytes / 1024));
		std::string serialized = umm.SerializeBlank();
		serialized = std::regex_replace(serialized, std::regex("serialization::archive 19"), "serialization::archive 14"); // hack to make archive of VS2019 compatible with current server as for some reason that number changed
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", UPLOADSERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			QElapsedTimer startTime;
			startTime.start();
			qint64 writtenBytes = 0;
			auto size = static_cast<int32_t>(serialized.size());
			sock.write(&size, 4);
			sock.write(serialized);
			for (const QString & file : uploadFiles) {
#ifdef Q_OS_WIN
				const auto path = q2ws(file);
#else
				const auto path = q2s(file);
#endif
				
				std::ifstream in(path, std::ios_base::in | std::ios_base::binary);
				while (in.good()) {
					char buffer[1024];
					in.read(buffer, 1024);
					const auto fileSize = in.gcount();
					sock.write(buffer, fileSize);
					writtenBytes += fileSize;
					emit updateUploadText(QApplication::tr("UploadingFiles").arg(byteToString(writtenBytes), byteToString(maxBytes), bytePerTimeToString(writtenBytes, static_cast<double>(startTime.elapsed()))));
					emit updateProgress(static_cast<int>(writtenBytes / 1024));
				}
				in.close();
				QFile::remove(file);
			}
			auto cErrReceive = sock.receivePacket(serialized);

			if (cErrReceive != clockUtils::ClockError::SUCCESS) {
				emit finishedUpload(false, uploadFiles.size());
				return;
			}

			common::Message * msg = common::Message::DeserializeBlank(serialized);
			if (msg) {
				auto * am = dynamic_cast<common::AckMessage *>(msg);
				if (msg) {
					emit finishedUpload(am->success, uploadFiles.size());
				} else {
					emit finishedUpload(false, uploadFiles.size());
				}
			} else {
				emit finishedUpload(false, uploadFiles.size());
			}
			delete msg;
		} else {
			emit finishedUpload(false, uploadFiles.size());
		}
	});
}

void ModFilesWidget::updateModList(QList<ManagementMod> modList) {
	_mods = modList;
}

void ModFilesWidget::selectedMod(int index) {
	if (_directory.contains("/")) {
		delete _directory["/"];
	}

	_modIndex = index;
	_fileList->clear();
	_fileList->setColumnCount(2);
	_directory.clear();
}

void ModFilesWidget::updateView() {
	if (_modIndex == -1 || _modIndex >= _mods.size())
		return;

	selectedMod(_modIndex);

	_majorVersionBox->setDisabled(_mods[_modIndex].packageID >= 0);
	_minorVersionBox->setDisabled(_mods[_modIndex].packageID >= 0);
	_patchVersionBox->setDisabled(_mods[_modIndex].packageID >= 0);
	_spineVersionBox->setDisabled(_mods[_modIndex].packageID >= 0);
	_versionButton->setDisabled(_mods[_modIndex].packageID >= 0);

	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ModID"] = _mods[_modIndex].id;
	requestData["PackageID"] = _mods[_modIndex].packageID;
	
	const auto f = https::Https::postAsync(MANAGEMENTSERVER_PORT, "getModFiles", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		ManagementModFilesData mmdfd;
		mmdfd.read(json);
		
		emit loadedData(mmdfd);
		emit removeSpinner();
	});
	_futureWatcher.setFuture(f);
}

void ModFilesWidget::updateData(ManagementModFilesData content) {
	_data = content;

	auto * baseItem = new QStandardItem("/");
	baseItem->setEditable(false);
	_directory.insert("/", baseItem);
	for (const auto & f : _data.files) {
		addFile(baseItem, f.filename, f.language);
	}
	_fileList->appendRow(baseItem);
	_fileTreeView->expandAll();
	_fileTreeView->resizeColumnToContents(0);
	_fileTreeView->resizeColumnToContents(1);

	_majorVersionBox->setValue(_data.versionMajor);
	_minorVersionBox->setValue(_data.versionMinor);
	_patchVersionBox->setValue(_data.versionPatch);
	_spineVersionBox->setValue(_data.versionSpine);
}

void ModFilesWidget::addFolder() {
	const auto dir = QFileDialog::getExistingDirectory(this, QApplication::tr("AddFolder"), ".", QFileDialog::ShowDirsOnly);
	
	if (dir.isEmpty())
		return;

	QStringList fileList;

	for (const auto & mmf : _data.files) {
		auto fileName = mmf.filename;
		if (fileName.endsWith(".z")) {
			fileName.chop(2);
		}
		fileList << fileName;
	}

	QFutureSynchronizer<QString> syncer;
	QList<FileToAdd> filesToAdd;
	QFutureWatcher<void> watcher;

	QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		const QString path = it.filePath().replace(dir, "");
		const QString fileName = it.fileName();

		if (fileName.endsWith(".z"))
			continue; // skip already compressed/uploaded files, they can corrupt the file list!

		const auto fullPath = it.filePath();

		FileToAdd fta;
		fta.fullPath = fullPath;
		fta.relativePath = path.split(fileName)[0];
		fta.file = fileName;
		fta.path = path;
		fta.hashFuture = QtConcurrent::run([this, fta] {
			return hashFile(fta);
		});

		filesToAdd << fta;

		syncer.addFuture(fta.hashFuture);
	}

	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	const auto f = QtConcurrent::run([&syncer] {
		syncer.waitForFinished();
	});
	watcher.setFuture(f);

	QEventLoop loop;
	connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
	loop.exec();

	for (const auto & fta : filesToAdd) {
		addFile(fta.fullPath, fta.relativePath, fta.file, fta.hashFuture.result());

		fileList.removeAll(fta.path.right(fta.path.length() - 1));

		loop.processEvents();
	}

	for (const QString & file : fileList) {
		const auto idxList = _fileList->match(_fileList->index(0, 0), PathRole, QVariant::fromValue(file), 2, Qt::MatchRecursive);

		if (idxList.isEmpty())
			continue;
		
		deleteFile(idxList[0]);

		loop.processEvents();
	}

	delete _waitSpinner;
	_waitSpinner = nullptr;
}

void ModFilesWidget::showVersionUpdate(bool success) {
	QMessageBox resultMsg(success ? QMessageBox::Icon::NoIcon : QMessageBox::Icon::Critical, QApplication::tr("VersionUpdate"), QApplication::tr(success ? "VersionUpdateSuccess" : "VersionUpdateFailed"), QMessageBox::StandardButton::Ok);
	resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
	resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	resultMsg.exec();
}

void ModFilesWidget::changedLanguage(QStandardItem * itm) {
	const QVariant v = itm->data(PathRole);

	if (!v.isValid())
		return;

	const QString path = v.toString();
	for (auto & file : _data.files) {
		QString currentFileName = file.filename;
		if (currentFileName.endsWith(".z")) {
			currentFileName.chop(2);
		}
		if (path == currentFileName) {
			file.changed = true;
			file.language = itm->data(Qt::DisplayRole).toString();
			break;
		}
	}
}

void ModFilesWidget::updateVersion() {
	if (_modIndex == -1)
		return;

	const auto project = _mods[_modIndex];

	if (_majorVersionBox->value() < _data.versionMajor || _majorVersionBox->value() == _data.versionMajor && _minorVersionBox->value() < _data.versionMinor || _majorVersionBox->value() == _data.versionMajor && _minorVersionBox->value() == _data.versionMinor && _patchVersionBox->value() < _data.versionPatch || _majorVersionBox->value() == _data.versionMajor && _minorVersionBox->value() == _data.versionMinor && _patchVersionBox->value() == _data.versionPatch && _spineVersionBox->value() < _data.versionSpine || _majorVersionBox->value() == _data.versionMajor && _minorVersionBox->value() == _data.versionMinor && _patchVersionBox->value() == _data.versionPatch && _spineVersionBox->value() == _data.versionSpine) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("VersionInvalid"), QApplication::tr("VersionInvalidText").arg(_data.versionMajor).arg(_data.versionMinor).arg(_data.versionPatch).arg(_data.versionSpine), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
		
		return;
	}

	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["ModID"] = project.id;
	json["VersionMajor"] = _majorVersionBox->value();
	json["VersionMinor"] = _minorVersionBox->value();
	json["VersionPatch"] = _patchVersionBox->value();
	json["VersionSpine"] = _spineVersionBox->value();

	const QDate date(2000, 1, 1);
	
	json["Date"] = date.daysTo(QDate::currentDate());

	EnterChangelogDialog dlg(this);
	const auto result = dlg.exec();

	if (result != QDialog::Accepted) return;

	json["SavegameCompatible"] = static_cast<int>(dlg.isSavegameCompatible());

	const auto changelogs = dlg.getChangelogs();

	QJsonArray changelogsArray;
	for (auto it = changelogs.begin(); it != changelogs.end(); ++it) {
		QJsonObject d;

		d["Language"] = it.key();
		d["Changelog"] = it.value();
		
		changelogsArray.append(d);
	}
	json["Changelogs"] = changelogsArray;
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "updateModVersion", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int statusCode) {
		emit versionUpdated(statusCode == 200);
	});
}

void ModFilesWidget::finishUpload(bool success, int updatedCount) {
	delete _waitSpinner;
	_waitSpinner = nullptr;

#ifdef Q_OS_WIN
	_taskbarProgress->hide();
#endif

	if (success) {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("UploadSuccessful"), QApplication::tr("UploadSuccessfulText") + "\n" + QApplication::tr("xOfyFilesHaveBeenUpdated").arg(updatedCount).arg(_data.files.size()), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
	} else {
		QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("UploadUnsuccessful"), QApplication::tr("UploadUnsuccessfulText"), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
	}
}

void ModFilesWidget::testUpdate() {
	if (_modIndex == -1)
		return;

	emit checkForUpdate(_mods[_modIndex].id, false);
}

void ModFilesWidget::addFile(QString fullPath, QString relativePath, QString file, const QString & hash) {
	while (relativePath.endsWith("/")) {
		relativePath.resize(relativePath.length() - 1);
	}
	QString fullRelativePath = relativePath + "/" + file;
	while (fullRelativePath.startsWith("/")) {
		fullRelativePath.remove(0, 1);
	}

	for (const auto & c : _filterString) {
		if (!fullRelativePath.contains(c))
			continue;

		// show error message
		QMessageBox msg(QMessageBox::Icon::Critical, QApplication::tr("FileUnsupported"), QApplication::tr("FileUnsupportedDescription").arg(fullRelativePath).arg(c), QMessageBox::StandardButton::Ok);
		msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
		msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
		msg.exec();
		
		return;
	}

	addFile(_directory.value("/"), fullRelativePath, "All");
	_fileTreeView->expandAll();
	_fileTreeView->resizeColumnToContents(0);
	_fileTreeView->resizeColumnToContents(1);

	bool found = false;
	for (auto & it : _data.files) {
		QString currentFileName = it.filename;
		while (currentFileName[0] == '/') {
			currentFileName = currentFileName.mid(1);
		}
		if (currentFileName.endsWith(".z")) {
			currentFileName.chop(2);
		}
		if (fullRelativePath == currentFileName) {
			if (it.deleted) {
				it.deleted = false;
			}
			// check hash of new file			
			if (hash != it.hash) { // hash changed
				it.changed = true;
				it.hash = hash;
				markAsChanged(fullRelativePath);
				_fileMap.insert(fullRelativePath, fullPath);
			}
			found = true;
			break;
		}
	}
	if (!found) {
		ManagementModFile mf;
		mf.filename = fullRelativePath;
		mf.language = "All";
		mf.changed = true;
		mf.deleted = false;
		mf.newFile = true;
		mf.hash = hash;
		_data.files.append(mf);
		_fileMap.insert(fullRelativePath, fullPath);
	}
}

void ModFilesWidget::addFile(QStandardItem * itm, QString file, QString language) {
	QString currentFileName = file;
	if (currentFileName.endsWith(".z")) {
		currentFileName.chop(2);
	}
	QStringList dirStructure = currentFileName.split("/");
	QString currentPath;
	QStandardItem * parentItem = itm;
	for (const QString & d : dirStructure) {
		if (!currentPath.isEmpty()) {
			currentPath += "/";
		}
		currentPath += d;
		auto it = _directory.find(currentPath);
		if (it == _directory.end()) {
			auto * newItm = new QStandardItem(d);
			newItm->setEditable(false);
			QStandardItem * languageItm;
			if (currentPath == currentFileName) {
				languageItm = new QStandardItem(language);
				languageItm->setData(currentPath, PathRole);
				newItm->setData(currentPath, PathRole);
			} else {
				languageItm = new QStandardItem();
			}
			languageItm->setEditable(true);
			parentItem->appendRow({ newItm, languageItm });
			it = _directory.insert(currentPath, newItm);
		}
		parentItem = it.value();
	}
}

void ModFilesWidget::deleteFile(const QModelIndex & idx) {
	const QVariant v = idx.data(PathRole);
	
	if (!v.isValid())
		return;
	
	const QString path = v.toString();
	for (auto & file : _data.files) {
		QString currentFileName = file.filename;
		if (currentFileName.endsWith(".z")) {
			currentFileName.chop(2);
		}
		if (path == currentFileName) {
			file.deleted = true;

			if (file.newFile) {
				_data.files.removeAll(file);
			}
			
			break;
		}
	}
	_directory.remove(path);
	_fileList->removeRow(idx.row(), idx.parent());
}

QString ModFilesWidget::getPathSuggestion(const QString & file) const {
	const QFileInfo fi(file);
	const auto fileName = fi.fileName();
	
	for (const auto & f : _data.files) {
		if (!f.filename.contains(fileName))
			continue;

		auto path = f.filename;
		path = path.remove(fileName);
		if (path.endsWith(".z")) {
			path.resize(path.size() - 2);
		}
		if (path.endsWith("/")) {
			path.resize(path.size() - 1);
		}

		return path;
	}
	
	return "";
}

int64_t ModFilesWidget::getSize(const QString& path) const
{
	const QFileInfo fi(path);
	return fi.size();
}

void ModFilesWidget::markAsChanged(const QString& relativePath) {
	const auto idxList = _fileList->match(_fileList->index(0, 0), PathRole, QVariant::fromValue(relativePath), 2, Qt::MatchRecursive);

	if (idxList.isEmpty())
		return;

	auto * itm = _fileList->itemFromIndex(idxList[0]);

	const auto text = itm->data(Qt::DisplayRole).toString();

	if (text.endsWith(" *"))
		return;

	QSignalBlocker sb(_fileList);
	itm->setText(text + " *");
}

QString ModFilesWidget::hashFile(const FileToAdd & fta) const {
	return hashFile(fta.fullPath);
}

QString ModFilesWidget::hashFile(const QString & path) const {
	if (_data.gameType == common::GameType::Gothic || _data.gameType == common::GameType::Gothic2) {
		if (path.endsWith(".mod", Qt::CaseInsensitive) || path.endsWith(".vdf", Qt::CaseInsensitive)) {
			GothicVdf::optimize(path, _data.gameType == common::GameType::Gothic ? ":/g1.txt" : ":/g2.txt");
		} else if (path.endsWith(".ini", Qt::CaseInsensitive)) {
			optimizeIni(path);
		}
	}

	QString hash;
	const bool b = Hashing::hash(path, hash);

	return b ? hash : QString();
}

void ModFilesWidget::optimizeIni(const QString & path) {
	QFile f(path);

	if (!f.open(QIODevice::ReadOnly))
		return;

	QTextStream ts(&f);
	auto iniData = ts.readAll();
	iniData.replace(QRegularExpression("-zRes:[^ ]+ "), "");

	f.close();

	if (!f.open(QIODevice::WriteOnly))
		return;

	QTextStream ts2(&f);
	ts2 << iniData;

	f.close();
}
