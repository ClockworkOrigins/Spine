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

#include "widgets/NewsWidget.h"

#include "IconCache.h"
#include "InstallMode.h"
#include "SpineConfig.h"

#include "https/Https.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"

#include "widgets/AchievementView.h"

#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QStyleOption>
#include <QtConcurrentRun>
#include <QTextBrowser>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
	#include "clockUtils/log/Log.h"
#endif

using namespace spine::client;
using namespace spine::common;
using namespace spine::utils;
using namespace spine::widgets;

NewsWidget::NewsWidget(News news, bool onlineMode, QWidget * par) : QWidget(par), _titleLabel(nullptr), _textBrowser(nullptr), _timestampLabel(nullptr), _newsID(news.id) {
	setObjectName("NewsWidget");

	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	_titleLabel = new QLabel(s2q(news.title), this);
	_titleLabel->setProperty("newsTitle", true);
	_timestampLabel = new QLabel(QDate(2000, 1, 1).addDays(news.timestamp).toString("dd.MM.yyyy"), this);
	_timestampLabel->setAlignment(Qt::AlignRight);
	_timestampLabel->setProperty("newsTimestamp", true);
	auto * hbl = new QHBoxLayout();
	hbl->addWidget(_titleLabel, 0, Qt::AlignLeft);
	hbl->addWidget(_timestampLabel, 0, Qt::AlignRight);
	l->addLayout(hbl);

	_textBrowser = new QTextBrowser(this);
	_textBrowser->setProperty("newsText", true);
	QString newsText = s2q(news.body);
	if (newsText.contains("%1")) {
		newsText = newsText.arg(Config::NEWSIMAGEDIR);
	}
	_textBrowser->setHtml(newsText);
	_textBrowser->setOpenLinks(false);
	_textBrowser->setFontFamily("Lato Semibold");
	_textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	l->addWidget(_textBrowser);
	l->setStretchFactor(_textBrowser, 1);

	int installButtonSize = 0;
	for (const auto & mod : news.referencedMods) {
		Database::DBError err;
		const bool installed = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(mod.first) + " LIMIT 1;", err) > 0;
		
		if (installed || !onlineMode) continue;
		
		auto * installButton = new QPushButton(IconCache::getInstance()->getOrLoadIcon(":/svg/download.svg"), s2q(mod.second), this);
		l->addWidget(installButton, 0, Qt::AlignLeft);
		installButtonSize += installButton->height();
		installButton->setProperty("modid", static_cast<int>(mod.first));
		connect(installButton, &QPushButton::released, this, &NewsWidget::installMod);
		_installButtons.append(installButton);
	}

	setLayout(l);

	connect(_textBrowser, &QTextBrowser::anchorClicked, this, &NewsWidget::urlClicked);

	_textBrowser->document()->adjustSize();

	_textBrowser->verticalScrollBar()->setValue(_textBrowser->verticalScrollBar()->minimum());

	static const QMap<int, double> additionalPercentage = { { 200, 0.3 }, { 350, 0.4 }, { 2000, 0.45 } };

	const int height = static_cast<int>(_textBrowser->document()->size().height());
	const double percent = additionalPercentage.upperBound(height).value();
	_textBrowser->setMinimumHeight(height + static_cast<int>(percent * height));
	_textBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	setMinimumSize(800, std::max(_titleLabel->size().height(), _timestampLabel->size().height()) + height + installButtonSize + static_cast<int>(percent * height) + 50);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}

void NewsWidget::finishedInstallation(int modID, int, bool success) {
	if (!success) return;

	for (QPushButton * pb : _installButtons) {
		const int buttonModID = pb->property("modid").toInt();
		if (modID == buttonModID) {
			Database::DBError err;
			const bool installed = Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT * FROM mods WHERE ModID = " + std::to_string(buttonModID) + " LIMIT 1;", err) > 0;
			pb->setVisible(!installed);
			break;
		}
	}
}

void NewsWidget::urlClicked(const QUrl & url) {
	if (!url.toString().startsWith("http://") && !url.toString().startsWith("https://") && !url.toString().startsWith("www.")) return;
	
	QDesktopServices::openUrl(url);

	QJsonObject json;
	json["NewsID"] = _newsID;
	json["Url"] = url.toString();
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "linkClicked", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject &, int) {});
}

void NewsWidget::installMod() {
	const int32_t modID = sender()->property("modid").toInt();
	emit tryInstallMod(modID, -1, InstallMode::UI);
}

void NewsWidget::paintEvent(QPaintEvent *) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void NewsWidget::update(News news) {
	_titleLabel->setText(s2q(news.title));
	_timestampLabel->setText(QDate(2000, 1, 1).addDays(news.timestamp).toString("dd.MM.yyyy"));
	QString newsText = s2q(news.body);
	if (newsText.contains("%1")) {
		newsText = newsText.arg(Config::NEWSIMAGEDIR);
	}
	_textBrowser->setHtml(newsText);
}
