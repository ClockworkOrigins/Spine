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

#include "widgets/NewsWriterDialog.h"

#include <thread>

#include "Config.h"
#include "utils/Conversion.h"
#include "Database.h"
#include "SpineConfig.h"

#include "utils/Compression.h"
#include "utils/Hashing.h"

#include "widgets/GeneralSettingsWidget.h"
#include "widgets/NewsWidget.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	NewsWriterDialog::NewsWriterDialog(GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QDialog(par), _newsPreviewWidget(nullptr), _titleEdit(nullptr), _dateEdit(nullptr), _bodyEdit(nullptr), _imageReferencesEdit(nullptr), _username(), _mods() {
		QHBoxLayout * l = new QHBoxLayout();
		l->setAlignment(Qt::AlignTop);

		_newsPreviewWidget = new NewsWidget(common::SendAllNewsMessage::News(), true, this);
		l->addWidget(_newsPreviewWidget);

		{
			QVBoxLayout * vl = new QVBoxLayout();
			vl->setAlignment(Qt::AlignTop);

			_titleEdit = new QLineEdit(this);
			_titleEdit->setPlaceholderText(QApplication::tr("TitlePlaceholder"));
			_dateEdit = new QDateEdit(this);
			_dateEdit->setAlignment(Qt::AlignRight);
			_dateEdit->setCalendarPopup(true);
			_dateEdit->setDate(QDate::currentDate());
			_dateEdit->setMinimumDate(QDate::currentDate());
			QHBoxLayout * hbl = new QHBoxLayout();
			hbl->addWidget(_titleEdit);
			hbl->addWidget(_dateEdit);
			vl->addLayout(hbl);

			_bodyEdit = new QTextEdit(this);
			_bodyEdit->setPlaceholderText(QApplication::tr("BodyPlaceholder") + Config::NEWSIMAGEDIR);
			_bodyEdit->setProperty("newsText", true);
			_bodyEdit->setFontFamily("Lato Semibold");
			vl->addWidget(_bodyEdit);

			QScrollArea * scrollArea = new QScrollArea(this);
			scrollArea->setWidgetResizable(true);
			QWidget * modContainer = new QWidget(scrollArea);
			modContainer->setProperty("default", true);
			scrollArea->setWidget(modContainer);
			_modListLayout = new QGridLayout();
			modContainer->setLayout(_modListLayout);
			vl->addWidget(scrollArea);

			QHBoxLayout * imageLayout = new QHBoxLayout();
			_imageReferencesEdit = new QLineEdit(this);
			_imageReferencesEdit->setPlaceholderText(QApplication::tr("ImagesPlaceholder"));
			QPushButton * pb = new QPushButton(QIcon(":/svg/add.svg"), "", this);
			connect(pb, SIGNAL(released()), this, SLOT(addImage()));
			imageLayout->addWidget(_imageReferencesEdit, 1);
			imageLayout->addWidget(pb);
			vl->addLayout(imageLayout);

			QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
			vl->addWidget(dbb);

			setLayout(l);

			QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
			b->setText(QApplication::tr("Submit"));

			connect(b, SIGNAL(clicked()), this, SLOT(accept()));

			b = dbb->button(QDialogButtonBox::StandardButton::Discard);
			b->setText(QApplication::tr("Discard"));

			connect(b, SIGNAL(clicked()), this, SIGNAL(rejected()));
			connect(b, SIGNAL(clicked()), this, SLOT(reject()));
			connect(b, SIGNAL(clicked()), this, SLOT(hide()));

			l->addLayout(vl);

			connect(_titleEdit, SIGNAL(textChanged(const QString&)), this, SLOT(changedNews()));
			connect(_dateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(changedNews()));
			connect(_bodyEdit, SIGNAL(textChanged()), this, SLOT(changedNews()));
		}

		_language = generalSettingsWidget->getLanguage();
		connect(generalSettingsWidget, SIGNAL(languageChanged(QString)), this, SLOT(setLanguage(QString)));

		qRegisterMetaType<std::vector<common::Mod>>("std::vector<common::Mod>");
		connect(this, SIGNAL(receivedModList(std::vector<common::Mod>)), this, SLOT(updateModList(std::vector<common::Mod>)));

		setLayout(l);

		l->setStretch(0, 50);
		l->setStretch(1, 50);

		setMinimumWidth(1700);
		setMinimumHeight(600);

		setWindowTitle(QApplication::tr("NewsWriter"));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		changedNews();
		requestMods();
	}

	NewsWriterDialog::~NewsWriterDialog() {
	}

	void NewsWriterDialog::setUsername(QString username, QString password) {
		_username = username;
		_password = password;
	}

	void NewsWriterDialog::changedNews() {
		common::SendAllNewsMessage::News news;

		news.title = q2s(_titleEdit->text());
		news.body = q2s(_bodyEdit->toPlainText());
		news.timestamp = QDate(2000, 1, 1).daysTo(_dateEdit->date());

		_newsPreviewWidget->update(news);
	}

	void NewsWriterDialog::accept() {
		if (_titleEdit->text().isEmpty() || _bodyEdit->toPlainText().isEmpty() || _username.isEmpty()) {
			return;
		}
		common::SubmitNewsMessage snm;
		snm.username = _username.toStdString();
		snm.password = _password.toStdString();
		common::SendAllNewsMessage::News news;

		news.title = q2s(_titleEdit->text());
		news.body = q2s(_bodyEdit->toPlainText());
		news.timestamp = QDate(2000, 1, 1).daysTo(_dateEdit->date());

		for (QString imageFile : _imageReferencesEdit->text().split(";", QString::SplitBehavior::SkipEmptyParts)) {
			Database::DBError err;
			std::vector<std::string> images = Database::queryAll<std::string, std::string>(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "SELECT Hash FROM newsImageReferences WHERE File = '" + imageFile.toStdString() + ".z' LIMIT 1;", err);
			if (images.empty()) {
				QString hashString;
				// 1. calculate hash
				utils::Hashing::hash(Config::NEWSIMAGEDIR + "/" + imageFile, hashString);
				// 2. compress
				utils::Compression::compress(Config::NEWSIMAGEDIR + "/" + imageFile, false);
				imageFile += ".z";
				// 3. add image to message
				{
					QFile f(imageFile);
					if (f.open(QIODevice::ReadOnly)) {
						QByteArray d = f.readAll();
						std::vector<uint8_t> buffer(d.length());
						memcpy(&buffer[0], d.data(), d.length());
						snm.images.push_back(buffer);
					}
					f.close();
					f.remove();
				}
				news.imageFiles.emplace_back(imageFile.toStdString(), hashString.toStdString());
			} else {
				news.imageFiles.emplace_back(imageFile.toStdString() + ".z", images[0]);
				snm.images.emplace_back();
			}
		}
		for (QCheckBox * cb : _mods) {
			if (cb->isChecked()) {
				snm.mods.push_back(cb->property("modid").toInt());
			}
		}

		snm.news = news;
		snm.language = _language.toStdString();

		const std::string serialized = snm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == err) {
			if (clockUtils::ClockError::SUCCESS == sock.writePacket(serialized)) {
				QMessageBox resultMsg(QMessageBox::Icon::Information, QApplication::tr("NewsSuccessful"), QApplication::tr("NewsSuccessfulText"), QMessageBox::StandardButton::Ok);
				resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				resultMsg.exec();
				emit refresh();
				QDialog::accept();
				return;
			}
		}
		QMessageBox resultMsg(QMessageBox::Icon::Warning, QApplication::tr("NewsUnsuccessful"), QApplication::tr("NewsUnsuccessfulText"), QMessageBox::StandardButton::Ok);
		resultMsg.setWindowFlags(resultMsg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		resultMsg.exec();
		QDialog::accept();
	}

	void NewsWriterDialog::updateModList(std::vector<common::Mod> mods) {
		std::sort(mods.begin(), mods.end(), [](const common::Mod & a, const common::Mod & b) {
			return a.name < b.name;
		});
		int counter = 0;
		for (const common::Mod & mod : mods) {
			QCheckBox * cb = new QCheckBox(s2q(mod.name), this);
			cb->setProperty("modid", int(mod.id));
			_modListLayout->addWidget(cb, counter / 3, counter % 3);
			_mods.append(cb);
			counter++;
		}
	}

	void NewsWriterDialog::setLanguage(QString language) {
		_language = language;
	}

	void NewsWriterDialog::addImage() {
		const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectImage"), Config::NEWSIMAGEDIR + "/", "Images (*.png *.jpg)");
		if (path.isEmpty()) {
			return;
		}
		const QString folder = QFileInfo(path).path() + "/";
		if (folder != Config::NEWSIMAGEDIR) {
			QFile::copy(path, Config::NEWSIMAGEDIR + "/" + QFileInfo(path).fileName());
		}
		_imageReferencesEdit->setText(_imageReferencesEdit->text() + QFileInfo(path).fileName() + ";");
	}

	void NewsWriterDialog::showEvent(QShowEvent * evt) {
		QDialog::showEvent(evt);

		const QRect scr = QApplication::desktop()->screenGeometry();
		move(scr.center() - rect().center());
	}

	void NewsWriterDialog::requestMods() {
		std::thread([this]() {
			common::RequestAllModsMessage ramm;
			ramm.language = _language.toStdString();
			ramm.username = _username.toStdString();
			ramm.password = _password.toStdString();
			std::string serialized = ramm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError err = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == err) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::UpdateAllModsMessage * uamm = dynamic_cast<common::UpdateAllModsMessage *>(m);
							if (uamm) {
								emit receivedModList(uamm->mods);
							}
						}
						delete m;
					} catch (...) {
						return;
					}
				} else {
					qDebug() << "Error occurred: " << int(err);
				}
			}
		}).detach();
	}

} /* namespace widgets */
} /* namespace spine */
