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

#include "widgets/StartPageWidget.h"

#include "IconCache.h"
#include "SpineConfig.h"

#include "gui/DownloadQueueWidget.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"
#include "utils/FileDownloader.h"
#include "utils/LanguageConverter.h"
#include "utils/MultiFileDownloader.h"
#include "utils/WindowsExtensions.h"

#include "widgets/NewsWidget.h"
#include "widgets/NewsWriterDialog.h"
#include "widgets/UpdateLanguage.h"

#include <QApplication>
#include <QDate>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStandardItemModel>
#include <QtConcurrentRun>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::common;
using namespace spine::gui;
using namespace spine::https;
using namespace spine::utils;
using namespace spine::widgets;

StartPageWidget::StartPageWidget(QWidget * par) : QWidget(par) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);
	
	setProperty("default", true);

	auto * welcomeLabel = new QLabel(QApplication::tr("WelcomeText"), this);
	welcomeLabel->setWordWrap(true);
	welcomeLabel->setAlignment(Qt::AlignCenter);
	welcomeLabel->setProperty("StartPageWelcome", true);
	UPDATELANGUAGESETTEXT(welcomeLabel, "WelcomeText");

	l->addWidget(welcomeLabel);

	{
		auto * hl = new QHBoxLayout();
		{
			auto * scrollArea = new QScrollArea(this);
			auto * newsTickerWidget = new QWidget(scrollArea);
			_newsTickerLayout = new QVBoxLayout();
			_newsTickerLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
			newsTickerWidget->setLayout(_newsTickerLayout);
			scrollArea->setWidgetResizable(true);
			scrollArea->setWidget(newsTickerWidget);
			hl->addWidget(scrollArea, 25);
		}
		
		_scrollArea = new QScrollArea(this);
		_newsContainer = new QWidget(_scrollArea);
		_newsLayout = new QVBoxLayout();
		_newsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		_newsContainer->setLayout(_newsLayout);
		_scrollArea->setWidgetResizable(true);
		_scrollArea->setWidget(_newsContainer);

		_startModButton = new QPushButton(this);
		_startModButton->setProperty("library", true);
		_startModButton->hide();
		connect(_startModButton, &QPushButton::released, this, &StartPageWidget::startMod);

		hl->addWidget(_scrollArea);
		hl->addWidget(_startModButton, 0, Qt::AlignTop);
		hl->setStretchFactor(_scrollArea, 75);
		hl->setStretchFactor(_startModButton, 25);
		l->addLayout(hl);
	}

	_writeNewsButton = new QPushButton(IconCache::getInstance()->getOrLoadIcon(":/svg/edit.svg"), "", this);
	_writeNewsButton->setToolTip(QApplication::tr("WriteNewsTooltip"));
	UPDATELANGUAGESETTOOLTIP(_writeNewsButton, "WriteNewsTooltip");
	l->addWidget(_writeNewsButton, 0, Qt::AlignBottom | Qt::AlignRight);
	_writeNewsButton->hide();

	setLayout(l);

	connect(this, &StartPageWidget::receivedNews, this, &StartPageWidget::updateNews);
	connect(_writeNewsButton, &QPushButton::released, this, &StartPageWidget::executeNewsWriter);

	requestNewsUpdate();
}

void StartPageWidget::requestNewsUpdate() {
	if (!Config::OnlineMode) {
		updateNews();
		return;
	}
	QtConcurrent::run([this]() {
		QJsonObject json;
		json["Language"] = LanguageConverter::convert(Config::Language);

		Https::postAsync(DATABASESERVER_PORT, "requestAllNews", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & data, int statusCode) {
			if (statusCode != 200) return;

			if (data.contains("News")) {
				for (const auto jsonRef : data["News"].toArray()) {
					const auto jsonNews = jsonRef.toObject();

					const QString title = jsonNews["Title"].toString();
					const QString body = jsonNews["Body"].toString();
					const int timestamp = jsonNews["Timestamp"].toString().toInt();

					News news;
					news.title = q2s(title);
					news.body = q2s(body);
					news.timestamp = timestamp;

					if (jsonNews.contains("ProjectReferences")) {
						for (const auto jsonRef2 : jsonNews["ProjectReferences"].toArray()) {
							const auto jsonProjectReference = jsonRef2.toObject();

							const auto projectID = jsonProjectReference["ProjectID"].toString().toInt();
							const auto name = jsonProjectReference["Name"].toString();

							news.referencedMods.emplace_back(projectID, q2s(name));
						}
					}
					if (jsonNews.contains("Images")) {
						for (const auto jsonRef2 : jsonNews["Images"].toArray()) {
							const auto jsonImage = jsonRef2.toObject();

							const auto file = jsonImage["File"].toString();
							const auto hash = jsonImage["Hash"].toString();

							news.imageFiles.emplace_back(q2s(file), q2s(hash));
						}
					}

					_newsEntries << news;
				}
			}

			_newsTickers.clear();
			if (data.contains("NewsTicker")) {
				for (const auto jsonRef : data["NewsTicker"].toArray()) {
					const auto jsonNewsTicker = jsonRef.toObject();

					NewsTicker nt;
					nt.type = static_cast<NewsTickerType>(jsonNewsTicker["Type"].toString().toInt());
					nt.name = q2s(jsonNewsTicker["Name"].toString());
					nt.projectID = jsonNewsTicker["ProjectID"].toString().toInt();
					nt.timestamp = jsonNewsTicker["Timestamp"].toString().toInt();
					nt.majorVersion = static_cast<int8_t>(jsonNewsTicker["MajorVersion"].toString().toInt());
					nt.minorVersion = static_cast<int8_t>(jsonNewsTicker["MinorVersion"].toString().toInt());
					nt.patchVersion = static_cast<int8_t>(jsonNewsTicker["PatchVersion"].toString().toInt());

					_newsTickers << nt;
				}
			}

			emit receivedNews();
		});
	});
}

void StartPageWidget::setLanguage() {
	requestNewsUpdate();
}

void StartPageWidget::loginChanged() {
	_writeNewsButton->setVisible(!Config::Username.isEmpty() && Config::OnlineMode);
}

void StartPageWidget::updateNews() {
	for (NewsWidget * nw : _news) {
		nw->deleteLater();
	}
	_news.clear();

	for (QWidget * w : _newsTickerWidgets) {
		w->deleteLater();
	}
	_newsTickerWidgets.clear();
	
	if (Config::OnlineMode) {
		QSet<QPair<QString, QString>> files;

		for (const auto & news : _newsEntries) {
			for (const auto & file : news.imageFiles) {
				files << qMakePair(s2q(file.first), s2q(file.second));
			}
		}

		auto * mfd = new MultiFileDownloader(this);
		bool download = false;
		for (const auto & p : files) {
			QString filename = p.first;
			filename.chop(2); // every image is compressed, so it has a .z at the end
			if (!QFileInfo::exists(Config::NEWSIMAGEDIR + "/" + filename)) {
				QFileInfo fi(p.first);
				auto * fd = new FileDownloader(QUrl("https://clockwork-origins.de/Gothic/downloads/news/images/" + p.first), Config::NEWSIMAGEDIR + "/" + fi.path(), fi.fileName(), p.second, mfd);
				mfd->addFileDownloader(fd);
				download = true;
			}
		}

		if (download) {
			connect(mfd, &MultiFileDownloader::downloadSucceeded, this, &StartPageWidget::updateNews);
			DownloadQueueWidget::getInstance()->addDownload(QApplication::tr("NewsImages"), mfd);
		} else {
			delete mfd;
		}		
	}
	for (const News & n : _newsEntries) {
		auto * newsWidget = new NewsWidget(n, Config::OnlineMode, _newsContainer);
		_newsLayout->addWidget(newsWidget);
		_news.push_back(newsWidget);
		connect(newsWidget, &NewsWidget::tryInstallMod, this, &StartPageWidget::tryInstallMod);
		auto * itmTitle = new QStandardItem(s2q(n.title));
		auto * itmTimestamp = new QStandardItem(QDate(2000, 1, 1).addDays(n.timestamp).toString("dd.MM.yyyy"));
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
	}

	for (const auto & nt : _newsTickers) {
		if (nt.type != NewsTickerType::Update && nt.type != NewsTickerType::Release) continue;
		
		auto * pb = new QPushButton(this);
		auto * hl = new QHBoxLayout();

		QLabel * lblTitle = nullptr;
		
		if (nt.type == NewsTickerType::Update) {		
			lblTitle = new QLabel(QString("[%1] %2 %3.%4.%5").arg(QApplication::tr("Update").toUpper()).arg(s2q(nt.name)).arg(static_cast<int>(nt.majorVersion)).arg(static_cast<int>(nt.minorVersion)).arg(static_cast<int>(nt.patchVersion)), this);
		} else if (nt.type == NewsTickerType::Release) {
			lblTitle = new QLabel(QString("[%1] %2").arg(QApplication::tr("ReleaseTag").toUpper()).arg(s2q(nt.name)), this);
		}
		
		auto * lblDate = new QLabel(QDate(2000, 1, 1).addDays(nt.timestamp).toString("dd.MM.yyyy"), this);

		pb->setProperty("newsTicker", true);
		pb->setProperty("ProjectID", nt.projectID);
		
		lblTitle->setProperty("newsTicker", true);
		lblDate->setProperty("newsTicker", true);

		hl->addWidget(lblTitle);
		hl->addStretch(1);
		hl->addWidget(lblDate);

		pb->setLayout(hl);

		_newsTickerWidgets.append(pb);
		
		_newsTickerLayout->addWidget(pb);

		connect(pb, &QPushButton::released, [this, pb]() {
			emit showInfoPage(pb->property("ProjectID").toInt());
		});
	}
}

void StartPageWidget::selectedNews(const QModelIndex & index) {
	_scrollArea->ensureWidgetVisible(_news[index.row()]);
}

void StartPageWidget::executeNewsWriter() {
	NewsWriterDialog dlg(this);
	connect(&dlg, &NewsWriterDialog::refresh, this, &StartPageWidget::refresh);
	dlg.exec();
}

void StartPageWidget::refresh() {
	for (NewsWidget * nw : _news) {
		nw->deleteLater();
	}
	_news.clear();

	for (QWidget * w : _newsTickerWidgets) {
		w->deleteLater();
	}
	_newsTickerWidgets.clear();
	
	requestNewsUpdate();
}

void StartPageWidget::startMod() {
	const QObject * obj = sender();
	const int modID = obj->property("projectID").toInt();
	const QString ini = obj->property("ini").toString();
	emit triggerModStart(modID, ini);
}

void StartPageWidget::showEvent(QShowEvent *) {
	Database::DBError err;
	auto vec = Database::queryAll<std::vector<std::string>, std::string, std::string>(Config::BASEDIR.toStdString() + "/" + LASTPLAYED_DATABASE, "SELECT ModID, Ini FROM lastPlayed LIMIT 1;", err);

	if (!vec.empty()) {
		const QString ini = s2q(vec[0][1]);
		if (QFileInfo::exists(ini)) {
			_startModButton->setProperty("projectID", std::stoi(vec[0][0]));
			_startModButton->setProperty("ini", ini);

			const QSettings iniParser(ini, QSettings::IniFormat);
			const QString title = iniParser.value("INFO/Title", "").toString();
			_startModButton->setText(QApplication::tr("StartModName").arg(title));

#ifdef Q_OS_WIN
			const bool requiresAdmin = iniParser.value("INFO/RequiresAdmin", false).toBool();
			_startModButton->setEnabled(!requiresAdmin || IsRunAsAdmin());
#endif
		} else {
			vec.clear();
		}
	}

	_startModButton->setHidden(vec.empty());
}
