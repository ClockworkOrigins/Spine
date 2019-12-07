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

#include "widgets/management/ModFilesWidget.h"

#include <fstream>
#include <thread>

#include "SpineConfig.h"

#include "utils/Conversion.h"

#include "widgets/MainWindow.h"
#include "widgets/WaitSpinner.h"

#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filter/zlib.hpp"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTime>
#include <QTreeView>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include <QWinTaskbarButton>
	#include <QWinTaskbarProgress>
#endif

namespace spine {
namespace widgets {
namespace {
	enum FileRoles {
		PathRole = Qt::UserRole
	};
}

	ModFilesWidget::ModFilesWidget(QString username, QString language, QWidget * par) : QWidget(par), _fileList(nullptr), _username(username), _language(language), _mods(), _fileTreeView(nullptr), _modIndex(-1), _directory(), _fileMap(), _majorVersionBox(nullptr), _minorVersionBox(nullptr), _patchVersionBox(nullptr), _waitSpinner(nullptr) {
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
			_majorVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
			_minorVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
			_patchVersionBox->setToolTip(QApplication::tr("VersionNumberTooltip"));
			hl2->addWidget(_majorVersionBox);
			hl2->addWidget(_minorVersionBox);
			hl2->addWidget(_patchVersionBox);
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
			connect(pbDelete, &QPushButton::released, this, &ModFilesWidget::deleteFile);

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

#ifdef Q_OS_WIN
		QWinTaskbarButton * button = new QWinTaskbarButton(this);
		button->setWindow(MainWindow::getInstance()->windowHandle());

		_taskbarProgress = button->progress();
		_taskbarProgress->setMinimum(0);
		_taskbarProgress->setMaximum(1);
		_taskbarProgress->setValue(0);
		_taskbarProgress->hide();
		
		connect(this, &ModFilesWidget::updateProgress, _taskbarProgress, &QWinTaskbarProgress::setValue);
		connect(this, &ModFilesWidget::updateProgressMax, _taskbarProgress, &QWinTaskbarProgress::setMaximum);
#endif
	}

	void ModFilesWidget::addFile() {
		// TODO
		// adds a file... if already existing => just update internally
		/*QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectFile"));
		if (path.isEmpty()) return;

		QString mapping = QInputDialog::getText(this, QApplication::tr("PathInDirectoryStructure"), QApplication::tr("PathInDirectoryStructureDescription"));
		QStringList realMappingSplit = mapping.split("/", QString::SplitBehavior::SkipEmptyParts);
		QString realMapping;
		for (const QString & rm : realMappingSplit) {
			if (!realMapping.isEmpty()) {
				realMapping.append("/");
			}
			realMapping += rm;
		}
		QString file = realMapping + "/" + QFileInfo(path).fileName();
		while (file.startsWith("/")) {
			file.remove(0, 1);
		}

		addFile(_directory.value("/"), file, "All");
		_fileTreeView->expandAll();
		_fileTreeView->resizeColumnToContents(0);
		_fileTreeView->resizeColumnToContents(1);
		bool found = false;
		for (auto it = _mods[_modIndex].files.begin(); it != _mods[_modIndex].files.end(); ++it) {
			QString currentFileName = s2q(it->filename);
			while (currentFileName[0] == '/') {
				currentFileName = currentFileName.mid(1);
			}
			if (currentFileName.endsWith(".z")) {
				currentFileName.chop(2);
			}
			if (file == currentFileName) {
				if (it->deleted) {
					it->deleted = false;
				}
				// check hash of new file
				QFile f(path);
				if (f.open(QIODevice::ReadOnly)) {
					QCryptographicHash hash(QCryptographicHash::Sha512);
					hash.addData(&f);
					const QString hashSum = QString::fromLatin1(hash.result().toHex());
					if (hashSum != s2q(it->hash)) { // hash changed
						it->changed = true;
						_fileMap.insert(file, path);
					}
				}
				found = true;
				break;
			}
		}
		if (!found) {
			common::ModFile mf;
			mf.filename = q2s(file);
			mf.language = "All";
			mf.changed = true;
			_mods[_modIndex].files.push_back(mf);
			_fileMap.insert(file, path);
		}*/
	}

	void ModFilesWidget::deleteFile() {
		// TODO
		// removes selected modfile
		/*if (_fileTreeView->selectionModel()->selectedIndexes().isEmpty()) return;

		QModelIndex idx = _fileTreeView->selectionModel()->selectedIndexes().front();
		QVariant v = idx.data(PathRole);
		if (v.isValid()) {
			const QString path = v.toString();
			for (auto it = _mods[_modIndex].files.begin(); it != _mods[_modIndex].files.end(); ++it) {
				QString currentFileName = s2q(it->filename);
				if (currentFileName.endsWith(".z")) {
					currentFileName.chop(2);
				}
				if (path == currentFileName) {
					it->deleted = true;
					break;
				}
			}
			_directory.remove(path);
			_fileList->removeRow(idx.row(), idx.parent());
		}*/
	}

	void ModFilesWidget::uploadCurrentMod() {
		// TODO
		// create diff and upload mods
		// own port on server
		/*_waitSpinner = new WaitSpinner(QApplication::tr("PreparingUpload"), this);
		connect(this, &ModFilesWidget::updateUploadText, _waitSpinner, &WaitSpinner::setText);
#ifdef Q_OS_WIN
		_taskbarProgress->setValue(0);
		_taskbarProgress->show();
#endif
		std::thread([this]() {
			common::UploadModfilesMessage umm;
			umm.modID = _mods[_modIndex].modID;
			for (auto mf : _mods[_modIndex].files) {
				if (mf.deleted) {
					if (!s2q(mf.filename).endsWith(".z")) {
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
			for (auto mf : _mods[_modIndex].files) {
				if (mf.changed) {
					QString currentFileName = s2q(mf.filename);
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
						QFile f(it.value());
						if (f.open(QIODevice::ReadOnly)) {
							QCryptographicHash hash(QCryptographicHash::Sha512);
							hash.addData(&f);
							QString hashSum = QString::fromLatin1(hash.result().toHex());
							if (hashSum == s2q(mf.hash)) { // hash the same, so just update the language
								mf.size = 0;
							} else {
								f.close();

								{
									std::ifstream uncompressedFile(q2s(it.value()), std::ios_base::in | std::ios_base::binary);
									boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
									in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
									in.push(uncompressedFile);
									std::ofstream compressedFile(q2s(it.value()) + ".z", std::ios_base::out | std::ios_base::binary);
									boost::iostreams::copy(in, compressedFile);
								}
								std::ifstream in(q2s(it.value()) + ".z", std::ifstream::ate | std::ifstream::binary);
								const auto size = in.tellg();
								maxBytes += size;

								mf.hash = q2s(hashSum);
								mf.size = size;
								uploadFiles.push_back(it.value() + ".z");
							}
						}
					}
					if (!s2q(mf.filename).endsWith(".z")) {
						mf.filename += ".z";
					}
					while (mf.filename[0] == '/') {
						mf.filename = mf.filename.substr(1);
					}
					umm.files.push_back(mf);
				}
			}
			emit updateProgressMax(maxBytes / 1024);
			std::string serialized = umm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", UPLOADSERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				QTime startTime;
				startTime.start();
				qint64 writtenBytes = 0;
				int size = serialized.size();
				sock.write(&size, 4);
				sock.write(serialized);
				for (QString file : uploadFiles) {
					std::ifstream in(q2s(file), std::ios_base::in | std::ios_base::binary);
					while (in.good()) {
						char buffer[1024];
						in.read(buffer, 1024);
						const auto fileSize = in.gcount();
						sock.write(buffer, fileSize);
						writtenBytes += fileSize;
						emit updateUploadText(QApplication::tr("UploadingFiles").arg(byteToString(writtenBytes), byteToString(maxBytes), bytePerTimeToString(writtenBytes, startTime.elapsed())));
						emit updateProgress(writtenBytes / 1024);
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
		}).detach();*/
	}

	void ModFilesWidget::updateModList(QList<client::ManagementMod> modList) {
		_mods = modList;
	}

	void ModFilesWidget::selectedMod(int index) {
		// TODO
		/*_modIndex = index;
		_fileList->clear();
		_fileList->setColumnCount(2);
		_directory.clear();
		QStandardItem * baseItem = new QStandardItem("/");
		baseItem->setEditable(false);
		_directory.insert("/", baseItem);
		for (const auto & f : _mods[index].files) {
			addFile(baseItem, s2q(f.filename), s2q(f.language));
		}
		_fileList->appendRow(baseItem);
		_fileTreeView->expandAll();
		_fileTreeView->resizeColumnToContents(0);
		_fileTreeView->resizeColumnToContents(1);

		_majorVersionBox->setValue(int(_mods[index].majorVersion));
		_minorVersionBox->setValue(int(_mods[index].minorVersion));
		_patchVersionBox->setValue(int(_mods[index].patchVersion));*/
	}

	void ModFilesWidget::changedLanguage(QStandardItem * itm) {
		// TODO
		/*QVariant v = itm->data(PathRole);
		if (!v.isValid()) return;

		const QString path = v.toString();
		for (auto it = _mods[_modIndex].files.begin(); it != _mods[_modIndex].files.end(); ++it) {
			QString currentFileName = s2q(it->filename);
			if (currentFileName.endsWith(".z")) {
				currentFileName.chop(2);
			}
			if (path == currentFileName) {
				it->changed = true;
				it->language = q2s(itm->data(Qt::DisplayRole).toString());
				break;
			}
		}*/
	}

	void ModFilesWidget::updateVersion() {
		// TODO
		if (_modIndex == -1) return;

		/*common::UpdateModVersionMessage umvm;
		umvm.modID = _mods[_modIndex].modID;
		umvm.majorVersion = _majorVersionBox->value();
		umvm.minorVersion = _minorVersionBox->value();
		umvm.patchVersion = _patchVersionBox->value();
		_mods[_modIndex].majorVersion = _majorVersionBox->value();
		_mods[_modIndex].minorVersion = _minorVersionBox->value();
		_mods[_modIndex].patchVersion = _patchVersionBox->value();
		const std::string serialized = umvm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			sock.writePacket(serialized);
		}*/
	}

	void ModFilesWidget::finishUpload(bool success, int updatedCount) {
		// TODO
		/*delete _waitSpinner;
		_waitSpinner = nullptr;

#ifdef Q_OS_WIN
		_taskbarProgress->hide();
#endif

		if (success) {
			QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("UploadSuccessful"), QApplication::tr("UploadSuccessfulText") + "\n" + QApplication::tr("xOfyFilesHaveBeenUpdated").arg(updatedCount).arg(_mods[_modIndex].files.size()), QMessageBox::StandardButton::Ok);
			msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			msg.exec();
		} else {
			QMessageBox msg(QMessageBox::Icon::Information, QApplication::tr("UploadUnsuccessful"), QApplication::tr("UploadUnsuccessfulText"), QMessageBox::StandardButton::Ok);
			msg.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
			msg.button(QMessageBox::StandardButton::Ok)->setText(QApplication::tr("Ok"));
			msg.exec();
		}*/
	}

	void ModFilesWidget::testUpdate() {
		// TODO
		/*if (_modIndex == -1) return;

		emit checkForUpdate(_mods[_modIndex].modID);*/
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
				parentItem->appendRow(QList<QStandardItem *>() << newItm << languageItm);
				it = _directory.insert(currentPath, newItm);
			}
			parentItem = it.value();
		}
	}

} /* namespace widgets */
} /* namespace spine */
