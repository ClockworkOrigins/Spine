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

#include "SpineConfig.h"

#include "client/widgets/management/EnterChangelogDialog.h"

#include "common/MessageStructs.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Compression.h"
#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Hashing.h"

#include "widgets/MainWindow.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFileDialog>
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
#include <QTime>
#include <QTreeView>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

using namespace spine;
using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;
namespace {
	enum FileRoles {
		PathRole = Qt::UserRole
	};
}

ModFilesWidget::ModFilesWidget(QWidget * par) : QWidget(par), _fileList(nullptr), _fileTreeView(nullptr), _modIndex(-1), _majorVersionBox(nullptr), _minorVersionBox(nullptr), _patchVersionBox(nullptr), _spineVersionBox(nullptr), _waitSpinner(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QHBoxLayout * hl2 = new QHBoxLayout();
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
		QPushButton * submitButton = new QPushButton(QApplication::tr("Submit"), this);
		submitButton->setToolTip(QApplication::tr("UpdateVersionNumberTooltip"));
		hl2->addWidget(submitButton);
		connect(submitButton, &QPushButton::released, this, &ModFilesWidget::updateVersion);

		l->addLayout(hl2);
	}

	_fileTreeView = new QTreeView(this);
	_fileList = new QStandardItemModel(_fileTreeView);
	_fileTreeView->header()->hide();
	_fileTreeView->setModel(_fileList);
	_fileTreeView->setToolTip(QApplication::tr("FileStructureTooltip"));
	connect(_fileList, &QStandardItemModel::itemChanged, this, &ModFilesWidget::changedLanguage);
	l->addWidget(_fileTreeView);

	{
		QHBoxLayout * hl2 = new QHBoxLayout();
		QPushButton * pbAdd = new QPushButton("+", this);
		pbAdd->setToolTip(QApplication::tr("AddFileTooltip"));
		hl2->addWidget(pbAdd);
		connect(pbAdd, &QPushButton::released, this, static_cast<void(ModFilesWidget::*)()>(&ModFilesWidget::addFile));

		QPushButton * pbDelete = new QPushButton("-", this);
		pbDelete->setToolTip(QApplication::tr("RemoveFileTooltip"));
		hl2->addWidget(pbDelete);
		connect(pbDelete, &QPushButton::released, this, static_cast<void(ModFilesWidget::*)()>(&ModFilesWidget::deleteFile));

		QPushButton * pbAddFolder = new QPushButton(QApplication::tr("AddFolder"), this);
		hl2->addWidget(pbAddFolder);
		connect(pbAddFolder, &QPushButton::released, this, &ModFilesWidget::addFolder);

		l->addLayout(hl2);
	}

	QPushButton * uploadButton = new QPushButton(QApplication::tr("Upload"), this);
	uploadButton->setToolTip(QApplication::tr("UploadFileTooltip"));
	l->addWidget(uploadButton);
	connect(uploadButton, &QPushButton::released, this, &ModFilesWidget::uploadCurrentMod);

	QPushButton * testUpdateButton = new QPushButton(QApplication::tr("TestUpdate"), this);
	testUpdateButton->setToolTip(QApplication::tr("TestUpdateTooltip"));
	l->addWidget(testUpdateButton);
	connect(testUpdateButton, &QPushButton::released, this, &ModFilesWidget::testUpdate);

	connect(this, &ModFilesWidget::finishedUpload, this, &ModFilesWidget::finishUpload);

	setLayout(l);

	qRegisterMetaType<ManagementModFilesData>("ManagementModFilesData");

	connect(this, &ModFilesWidget::removeSpinner, [this]() {
		if (!_waitSpinner) return;
		
		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});

	connect(this, &ModFilesWidget::loadedData, this, &ModFilesWidget::updateData);

	connect(this, &ModFilesWidget::versionUpdated, this, &ModFilesWidget::showVersionUpdate);

#ifdef Q_OS_WIN
	QWinTaskbarButton * button = new QWinTaskbarButton(this);
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
	if (path.isEmpty()) return;

	QInputDialog dlg(this);
	dlg.setInputMode(QInputDialog::TextInput);
	dlg.setWindowTitle(QApplication::tr("PathInDirectoryStructure"));
	dlg.setLabelText(QApplication::tr("PathInDirectoryStructureDescription"));

	if (dlg.exec() == QDialog::Rejected) return;
	
	const QString mapping = dlg.textValue();
	
	QStringList realMappingSplit = mapping.split("/", QString::SkipEmptyParts);
	QString realMapping;
	for (const QString & rm : realMappingSplit) {
		if (!realMapping.isEmpty()) {
			realMapping.append("/");
		}
		realMapping += rm;
	}

	addFile(path, realMapping, QFileInfo(path).fileName());
}

void ModFilesWidget::deleteFile() {
	// removes selected modfile
	if (_fileTreeView->selectionModel()->selectedIndexes().isEmpty()) return;

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
		for (const auto & mmf : _data.files) {
			common::ModFile mf;
			mf.language = q2s(mmf.language);
			mf.changed = mmf.changed;
			mf.deleted = mmf.deleted;
			mf.filename = q2s(mmf.filename);
			mf.hash = q2s(mmf.hash);
			mf.size = mmf.size;
			if (mf.deleted) {
				if (!mmf.filename.endsWith(".z")) {
					mf.filename += ".z";
				}
				while (mf.filename[0] == '/') {
					mf.filename = mf.filename.substr(1);
				}
				umm.files.push_back(mf);
			}
		}
		qint64 maxBytes = 0;
		QStringList uploadFiles;
		for (const auto & mmf : _data.files) {
			common::ModFile mf;
			mf.language = q2s(mmf.language);
			mf.changed = mmf.changed;
			mf.deleted = mmf.deleted;
			mf.filename = q2s(mmf.filename);
			mf.hash = q2s(mmf.hash);
			mf.size = mmf.size;
			
			if (mf.changed) {
				QString currentFileName = mmf.filename;
				while (currentFileName.startsWith("/")) {
					currentFileName.remove(0, 1);
				}
				if (currentFileName.endsWith(".z")) {
					currentFileName.chop(2);
				}
				auto it = _fileMap.find(currentFileName);
				if (it == _fileMap.end()) {
					mf.size = 0;
				} else {
					// hash check
					QString hashSum;
					const bool b = utils::Hashing::hash(it.value(), hashSum);
					if (b) {
						if (hashSum == s2q(mf.hash)) { // hash the same, so just update the language
							mf.size = 0;
						} else {
							utils::Compression::compress(it.value(), false);

#ifdef Q_OS_WIN
							const auto path = q2ws(it.value() + ".z");
#else
							const auto path = q2s(it.value() + ".z");
#endif
							std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
							const auto size = in.tellg();
							maxBytes += size;

							mf.hash = q2s(hashSum);
							mf.size = size;
							uploadFiles.push_back(it.value() + ".z");
						}
					}
				}
				if (!mmf.filename.endsWith(".z")) {
					mf.filename += ".z";
				}
				while (mf.filename[0] == '/') {
					mf.filename = mf.filename.substr(1);
				}
				umm.files.push_back(mf);
			}
		}
		emit updateProgressMax(static_cast<int>(maxBytes / 1024));
		std::string serialized = umm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", UPLOADSERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			QElapsedTimer startTime;
			startTime.start();
			qint64 writtenBytes = 0;
			int32_t size = static_cast<int32_t>(serialized.size()); // TODO: support 64bit files in the future!
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
					emit updateUploadText(QApplication::tr("UploadingFiles").arg(utils::byteToString(writtenBytes), utils::byteToString(maxBytes), utils::bytePerTimeToString(writtenBytes, startTime.elapsed())));
					emit updateProgress(static_cast<int>(writtenBytes / 1024));
				}
				in.close();
				QFile::remove(file);
			}
			sock.receivePacket(serialized);
			common::Message * msg = common::Message::DeserializePublic(serialized);
			if (msg) {
				common::AckMessage * am = dynamic_cast<common::AckMessage *>(msg);
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

void ModFilesWidget::updateModList(QList<client::ManagementMod> modList) {
	_mods = modList;
}

void ModFilesWidget::selectedMod(int index) {
	_modIndex = index;
	_fileList->clear();
	_fileList->setColumnCount(2);
	_directory.clear();
}

void ModFilesWidget::updateView() {
	if (_modIndex == -1 || _modIndex >= _mods.size()) return;

	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ModID"] = _mods[_modIndex].id;
	
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

	QStandardItem * baseItem = new QStandardItem("/");
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
}

void ModFilesWidget::addFolder() {
	const auto dir = QFileDialog::getExistingDirectory(this, QApplication::tr("AddFolder"), ".", QFileDialog::ShowDirsOnly);
	
	if (dir.isEmpty()) return;

	QList<QString> fileList;

	for (const auto & mmf : _data.files) {
		auto fileName = mmf.filename;
		if (fileName.endsWith(".z")) {
			fileName.chop(2);
		}
		fileList.append(fileName);
	}

	qDebug() << fileList.count();

	QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		const QString path = it.filePath().replace(dir, "");
		const QString fileName = it.fileName();

		addFile(it.filePath(), path.split(fileName)[0], fileName);

		fileList.removeAll(path.right(path.length() - 1));
	}

	for (const QString & file : fileList) {
		qDebug() << file;
		
		const auto idxList = _fileList->match(_fileList->index(0, 0), PathRole, QVariant::fromValue(file), 2, Qt::MatchRecursive);

		if (idxList.isEmpty()) continue;
		
		deleteFile(idxList[0]);
	}
}

void ModFilesWidget::showVersionUpdate(bool success) {
	QMessageBox resultMsg(success ? QMessageBox::Icon::NoIcon : QMessageBox::Icon::Critical, QApplication::tr("VersionUpdate"), QApplication::tr(success ? "VersionUpdateSuccess" : "VersionUpdateFailed"), QMessageBox::StandardButton::Ok);
	resultMsg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
	resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	resultMsg.exec();
}

void ModFilesWidget::changedLanguage(QStandardItem * itm) {
	const QVariant v = itm->data(PathRole);
	if (!v.isValid()) return;

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
	if (_modIndex == -1) return;

	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["ModID"] = _mods[_modIndex].id;
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
	if (_modIndex == -1) return;

	emit checkForUpdate(_mods[_modIndex].id, false);
}

void ModFilesWidget::addFile(QString fullPath, QString relativePath, QString file) {
	while (relativePath.endsWith("/")) {
		relativePath.resize(relativePath.length() - 1);
	}
	QString fullRelativePath = relativePath + "/" + file;
	while (fullRelativePath.startsWith("/")) {
		fullRelativePath.remove(0, 1);
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
			const bool b = utils::Hashing::checkHash(fullPath, it.hash);
			if (!b) { // hash changed
				it.changed = true;
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
			QStandardItem * newItm = new QStandardItem(d);
			newItm->setEditable(false);
			QStandardItem * languageItm = nullptr;
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
	
	if (!v.isValid()) return;
	
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
