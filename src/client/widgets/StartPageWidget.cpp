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

#include "widgets/StartPageWidget.h"

#include <thread>

#include "Config.h"
#include "Conversion.h"
#include "Database.h"
#include "FileDownloader.h"
#include "MultiFileDownloader.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "widgets/DownloadProgressDialog.h"
#include "widgets/GeneralSettingsWidget.h"
#include "widgets/NewsWidget.h"
#include "widgets/NewsWriterDialog.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QDate>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include "WindowsExtensions.h"
#endif

namespace spine {
namespace widgets {

	StartPageWidget::StartPageWidget(bool onlineMode, QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par) : QWidget(par), _language(), _news(), _mainWindow(mainWindow), _newsTicker(nullptr), _newsTickerModel(nullptr), _generalSettingsWidget(generalSettingsWidget), _onlineMode(onlineMode) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QLabel * welcomeLabel = new QLabel(QApplication::tr("WelcomeText"), this);
		welcomeLabel->setWordWrap(true);
		welcomeLabel->setAlignment(Qt::AlignCenter);
		welcomeLabel->setProperty("StartPageWelcome", true);
		UPDATELANGUAGESETTEXT(generalSettingsWidget, welcomeLabel, "WelcomeText");

		l->addWidget(welcomeLabel);

		{
			QHBoxLayout * hl = new QHBoxLayout();
			_newsTicker = new QTableView(this);
			_newsTicker->horizontalHeader()->hide();
			_newsTicker->verticalHeader()->hide();
			_newsTicker->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
			_newsTicker->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
			_newsTicker->setShowGrid(false);
			_newsTickerModel = new QStandardItemModel(_newsTicker);
			_newsTicker->setModel(_newsTickerModel);
			_newsTicker->setProperty("newsTicker", true);
			_scrollArea = new QScrollArea(this);
			_newsContainer = new QWidget(_scrollArea);
			_newsLayout = new QVBoxLayout();
			_newsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
			_newsContainer->setLayout(_newsLayout);
			_scrollArea->setWidgetResizable(true);
			_scrollArea->setWidget(_newsContainer);

			_startModButton = new QPushButton(this);
			_startModButton->hide();
			connect(_startModButton, &QPushButton::released, this, &StartPageWidget::startMod);

			hl->addWidget(_newsTicker);
			hl->addWidget(_scrollArea);
			hl->addWidget(_startModButton, 0, Qt::AlignTop);
			hl->setStretchFactor(_newsTicker, 25);
			hl->setStretchFactor(_scrollArea, 75);
			hl->setStretchFactor(_startModButton, 25);
			l->addLayout(hl);

			connect(_newsTicker, SIGNAL(clicked(const QModelIndex &)), this, SLOT(selectedNews(const QModelIndex &)));
		}

		_writeNewsButton = new QPushButton(QIcon(":/svg/edit.svg"), "", this);
		_writeNewsButton->setToolTip(QApplication::tr("WriteNewsTooltip"));
		UPDATELANGUAGESETTOOLTIP(generalSettingsWidget, _writeNewsButton, "WriteNewsTooltip");
		l->addWidget(_writeNewsButton, 0, Qt::AlignBottom | Qt::AlignRight);
		_writeNewsButton->hide();

		setLayout(l);

		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "CREATE TABLE IF NOT EXISTS news (NewsID INT NOT NULL PRIMARY KEY, Title TEXT NOT NULL, Body TEXT NOT NULL, Timestamp INT NOT NULL, Language TEXT NOT NULL);", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "CREATE TABLE IF NOT EXISTS newsModReferences (NewsID INT NOT NULL, ModID INT NOT NULL, Name TEXT NOT NULL, Language TEXT NOT NULL);", err);
		Database::execute(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "CREATE TABLE IF NOT EXISTS newsImageReferences (NewsID INT NOT NULL, File TEXT NOT NULL, Hash TEXT NOT NULL);", err);

		connect(this, SIGNAL(receivedNews()), this, SLOT(updateNews()));
		connect(_writeNewsButton, SIGNAL(released()), this, SLOT(executeNewsWriter()));		
	}

	void StartPageWidget::requestNewsUpdate() {
		if (!_onlineMode) {
			updateNews();
			return;
		}
		std::thread([this]() {
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage requestNewsUpdate #1: " << getPRAMValue());
#endif
			Database::DBError err;
			std::vector<int> res = Database::queryAll<int, int>(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "SELECT IFNULL(MAX(NewsID), 0) FROM news WHERE Language = '" + _language.toStdString() + "';", err);
			if (res.empty()) {
				res.push_back(0);
			}
			common::RequestAllNewsMessage ranm;
			ranm.lastNewsID = res[0];
			ranm.language = _language.toStdString();
			std::string serialized = ranm.SerializePublic();
			clockUtils::sockets::TcpSocket sock;
			clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
			if (clockUtils::ClockError::SUCCESS == cErr) {
				sock.writePacket(serialized);
				if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
					try {
						common::Message * m = common::Message::DeserializePublic(serialized);
						if (m) {
							common::SendAllNewsMessage * sanm = dynamic_cast<common::SendAllNewsMessage *>(m);
							for (const common::SendAllNewsMessage::News & news : sanm->news) {
								Database::execute(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "INSERT INTO news (NewsID, Title, Body, Timestamp, Language) VALUES (" + std::to_string(news.id) + ", '" + news.title + "', '" + news.body + "', " + std::to_string(news.timestamp) + ", '" + _language.toStdString() + "');", err);
								for (const std::pair<int32_t, std::string> & mod : news.referencedMods) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "INSERT INTO newsModReferences (NewsID, ModID, Name, Language) VALUES (" + std::to_string(news.id) + ", " + std::to_string(mod.first) + ", '" + mod.second + "', '" + _language.toStdString() + "');", err);
								}
								for (const std::pair<std::string, std::string> & p : news.imageFiles) {
									Database::execute(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "INSERT INTO newsImageReferences (NewsID, File, Hash) VALUES (" + std::to_string(news.id) + ", '" + p.first + "', '" + p.second + "');", err);
								}
							}
						}
						delete m;
					} catch (...) {
					}
				}
			}
			emit receivedNews();
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage requestNewsUpdate #2: " << getPRAMValue());
#endif
		}).detach();
	}

	void StartPageWidget::setLanguage(QString language) {
		_language = language;
		requestNewsUpdate();
	}

	void StartPageWidget::setUsername(QString username, QString password) {
		_writeNewsButton->setVisible(!username.isEmpty() && _onlineMode);
		_username = username;
		_password = password;
	}

	void StartPageWidget::updateNews() {
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #1: " << getPRAMValue());
#endif
		for (NewsWidget * nw : _news) {
			nw->deleteLater();
		}
		_news.clear();
		_newsTickerModel->clear();
		Database::DBError err;
		std::vector<common::SendAllNewsMessage::News> news = Database::queryAll<common::SendAllNewsMessage::News, std::string, std::string, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "SELECT NewsID, Title, Body, Timestamp FROM news WHERE Language = '" + _language.toStdString() + "' ORDER BY Timestamp DESC, NewsID DESC;", err);
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #2: " << getPRAMValue());
#endif
		if (_onlineMode) {
			std::vector<std::pair<std::string, std::string>> images = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "SELECT DISTINCT File, Hash FROM newsImageReferences;", err);
			MultiFileDownloader * mfd = new MultiFileDownloader(this);
			connect(mfd, SIGNAL(downloadFailed(DownloadError)), mfd, SLOT(deleteLater()));
			connect(mfd, SIGNAL(downloadSucceeded()), mfd, SLOT(deleteLater()));
			bool download = false;
			for (const auto & p : images) {
				QString filename = QString::fromStdString(p.first);
				filename.chop(2); // every image is compressed, so it has a .z at the end
				if (!QFile(Config::NEWSIMAGEDIR + "/" + filename).exists()) {
					QFileInfo fi(QString::fromStdString(p.first));
					FileDownloader * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/news/images/" + QString::fromStdString(p.first)), Config::NEWSIMAGEDIR + "/" + fi.path(), fi.fileName(), QString::fromStdString(p.second), mfd);
					mfd->addFileDownloader(fd);
					download = true;
				}
			}
			if (download) {
				DownloadProgressDialog progressDlg(mfd, "DownloadingFile", 0, 100, 0, _mainWindow);
				progressDlg.setCancelButton(nullptr);
				progressDlg.setWindowFlags(progressDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
				progressDlg.exec();
			}
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #3: " << getPRAMValue());
#endif
		}
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #4: " << getPRAMValue());
#endif
		for (common::SendAllNewsMessage::News n : news) {			
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #4.1: " << getPRAMValue());
#endif
			std::vector<std::pair<std::string, std::string>> mods = Database::queryAll<std::pair<std::string, std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + NEWS_DATABASE, "SELECT DISTINCT ModID, Name FROM newsModReferences WHERE NewsID = " + std::to_string(n.id) + " AND Language = '" + _language.toStdString() + "';", err);
			for (auto p : mods) {
				n.referencedMods.emplace_back(std::stoi(p.first), p.second);
			}
			NewsWidget * newsWidget = new NewsWidget(n, _onlineMode, _newsContainer);
			_newsLayout->addWidget(newsWidget);
			_news.push_back(newsWidget);
			connect(newsWidget, SIGNAL(tryInstallMod(int)), this, SIGNAL(tryInstallMod(int)));
			QStandardItem * itmTitle = new QStandardItem(s2q(n.title));
			QStandardItem * itmTimestamp = new QStandardItem(QDate(2000, 1, 1).addDays(n.timestamp).toString("dd.MM.yyyy"));
			itmTimestamp->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			itmTitle->setEditable(false);
			itmTimestamp->setEditable(false);
			itmTitle->setSelectable(false);
			itmTimestamp->setSelectable(false);
			QFont f = itmTitle->font();
			f.setFamily("Lato Semibold");
			f.setPointSize(10);
			f.setUnderline(true);
			itmTitle->setFont(f);
			itmTimestamp->setFont(f);
			_newsTickerModel->appendRow(QList<QStandardItem *>() << itmTitle << itmTimestamp);
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #4.2: " << getPRAMValue());
#endif
		}
		if (!_news.empty()) {
			_newsTicker->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
			_newsTicker->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
		}
#ifdef Q_OS_WIN
		LOGINFO("Memory Usage updateNews #5: " << getPRAMValue());
#endif
	}

	void StartPageWidget::selectedNews(const QModelIndex & index) {
		_scrollArea->ensureWidgetVisible(_news[index.row()]);
	}

	void StartPageWidget::executeNewsWriter() {
		NewsWriterDialog dlg(_generalSettingsWidget, this);
		dlg.setUsername(_username, _password);
		connect(&dlg, SIGNAL(refresh()), this, SLOT(refresh()));
		dlg.exec();
	}

	void StartPageWidget::refresh() {
		for (NewsWidget * nw : _news) {
			nw->deleteLater();
		}
		_news.clear();
		_newsTickerModel->clear();
		requestNewsUpdate();
	}

	void StartPageWidget::startMod() {
		QObject * obj = sender();
		const int modID = obj->property("modID").toInt();
		const QString ini = obj->property("ini").toString();
		emit triggerModStart(modID, ini);
	}

	void StartPageWidget::showEvent(QShowEvent *) {
		Database::DBError err;
		std::vector<std::vector<std::string>> vec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "SELECT ModID, Ini FROM lastPlayed LIMIT 1;", err);

		if (!vec.empty()) {
			const QString ini = s2q(vec[0][1]);
			if (QFile::exists(ini)) {
				_startModButton->setProperty("modID", std::stoi(vec[0][0]));
				_startModButton->setProperty("ini", ini);

				QSettings iniParser(ini, QSettings::IniFormat);
				const QString title = iniParser.value("INFO/Title", "").toString();
				_startModButton->setText(QApplication::tr("StartModName").arg(title));
			} else {
				vec.clear();
			}
		}

		_startModButton->setHidden(vec.empty());
	}

} /* namespace widgets */
} /* namespace spine */
