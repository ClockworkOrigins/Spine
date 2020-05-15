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

#include "widgets/ProfileModView.h"

#include "SpineConfig.h"

#include "client/IconCache.h"

#include "common/GameType.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/Database.h"

#include <QApplication>
#include <QDirIterator>
#include <QLabel>
#include <QGridLayout>
#include <QPainter>
#include <QSettings>
#include <QStyleOption>

#ifdef Q_OS_WIN
	#include <QtWinExtras/qwinfunctions.h>
	#include <shellapi.h>
#endif

using namespace spine;
using namespace spine::client;
using namespace spine::utils;
using namespace spine::widgets;

ProfileModView::ProfileModView(common::ModStats ms, QString gothicDirectory, QString gothic2Directory, QWidget * par) : QWidget(par), _modID(ms.modID), _nameLabel(nullptr), _patchOrTool(ms.type == common::ModType::PATCH || ms.type == common::ModType::TOOL), _duration(ms.duration) {
	QGridLayout * l = new QGridLayout();

	setObjectName("ProfileModView");

	_nameLabel = new QLabel(s2q(ms.name), this);
	QLabel * iconLabel = new QLabel(this);

	QDirIterator it(Config::DOWNLOADDIR + "/mods/" + QString::number(ms.modID), QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);

	if (IconCache::getInstance()->hasIcon(ms.modID)) {
		QPixmap pixmap = IconCache::getInstance()->getIcon(ms.modID);
		pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
		iconLabel->setPixmap(pixmap);
	} else {	
		QStringList files;
		while (it.hasNext()) {
			it.next();
			QString fileName = it.filePath();
			if (!fileName.isEmpty()) {
				files.append(fileName);
			}
		}
		QString fileName = it.filePath();
		if (!fileName.isEmpty()) {
			files.append(fileName);
		}
		for (const QString & s : files) {
			QSettings iniParser(s, QSettings::IniFormat);
			const QString icon = iniParser.value("INFO/Icon", "").toString();
			QFileInfo fi(s);
			const QString iconPath = fi.absolutePath() + "/" + icon;
			
			QString modIDString = fi.absolutePath();
			QDir md(Config::DOWNLOADDIR + "/mods");
			modIDString.replace(md.absolutePath(), "");
			modIDString = modIDString.split("/", QString::SplitBehavior::SkipEmptyParts).front();

			const int32_t modID = modIDString.toInt();

			if (!IconCache::getInstance()->hasIcon(modID)) {
				IconCache::getInstance()->cacheIcon(modID, iconPath);
			}

			QPixmap pixmap = IconCache::getInstance()->getIcon(modID);
			Database::DBError err;
			common::GameType mid = common::GameType::Gothic2;
			bool found = false;
			if (Database::queryCount(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + modIDString.toStdString() + " LIMIT 1;", err) > 0) {
				mid = common::GameType(Database::queryNth<std::vector<int>, int>(Config::BASEDIR.toStdString() + "/" + INSTALLED_DATABASE, "SELECT GothicVersion FROM mods WHERE ModID = " + modIDString.toStdString() + " LIMIT 1;", err, 0).front());
				found = true;
			}
			if (pixmap.isNull()) {
				if (found) {
					QString exeFileName;
					if (mid == common::GameType::Gothic && QFileInfo::exists(gothicDirectory + "/System/Gothic.exe")) {
						exeFileName = gothicDirectory + "/System/Gothic.exe";
					} else if (mid == common::GameType::Gothic2 && QFileInfo::exists(gothic2Directory + "/System/Gothic2.exe")) {
						exeFileName = gothic2Directory + "/System/Gothic2.exe";
					}
#ifdef Q_OS_WIN
					if (!exeFileName.isEmpty()) {
						const HINSTANCE hInstance = GetModuleHandle(nullptr);
						const HICON ic = ExtractIcon(hInstance, exeFileName.toStdString().c_str(), 0);
						pixmap = QtWin::fromHICON(ic);
					}
#endif
				}
			}
			pixmap = pixmap.scaled(QSize(32, 32), Qt::AspectRatioMode::KeepAspectRatio, Qt::SmoothTransformation);
			iconLabel->setPixmap(pixmap);
		}
	}

	QLabel * achievementLabel = new QLabel(R"(<a href="Foobar" style="color: #181C22">)" + QApplication::tr("AchievementText").arg(ms.achievedAchievements).arg(ms.allAchievements).arg(ms.achievedAchievements * 100 / ((ms.allAchievements) ? ms.allAchievements : 1)) + "</a>", this);
	achievementLabel->setProperty("internalLink", true);
	connect(achievementLabel, &QLabel::linkActivated, this, &ProfileModView::prepareAchievementView);
	QLabel * scoreLabel = new QLabel(R"(<a href="Foobar" style="color: #181C22">)" + (ms.bestScoreRank > 0 ? QApplication::tr("BestScoreText").arg(s2q(ms.bestScoreName)).arg(ms.bestScoreRank).arg(ms.bestScore) : QApplication::tr("NoBestScore")) + "</a>", this);
	scoreLabel->setProperty("internalLink", true);
	connect(scoreLabel, &QLabel::linkActivated, this, &ProfileModView::prepareScoreView);

	QString timeString;
	if (ms.duration == -1) {
		timeString = "-";
	} else {
		if (ms.duration > 90) {
			timeString = QString::number((ms.duration + 30) / 60) + " " + ((ms.duration >= 90) ? QApplication::tr("Hours") : QApplication::tr("Hour"));
		} else {
			timeString = QString::number(ms.duration) + " " + (ms.duration > 1 || ms.duration == 0 ? QApplication::tr("Minutes") : QApplication::tr("Minute"));
		}
	}
	QLabel * timeLabel = new QLabel(timeString, this);

	l->addWidget(_nameLabel, 0, 0, Qt::AlignLeft);
	l->addWidget(achievementLabel, 0, 1, Qt::AlignCenter);
	l->addWidget(timeLabel, 0, 2, Qt::AlignRight);
	l->addWidget(iconLabel, 1, 0, Qt::AlignLeft);
	l->addWidget(scoreLabel, 1, 1, Qt::AlignCenter);

	if (ms.allAchievements == 0) {
		achievementLabel->hide();
	}
	if (ms.bestScoreRank == -1) {
		scoreLabel->hide();
	}

	setFixedSize(800, 80);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	setLayout(l);
}

ProfileModView::~ProfileModView() {
}

bool ProfileModView::isPatchOrTool() const {
	return _patchOrTool;
}

QSize ProfileModView::sizeHint() const {
	return QSize(800, 80);
}

uint64_t ProfileModView::getDuration() const {
	return _duration;
}

void ProfileModView::prepareAchievementView() {
	emit openAchievementView(_modID, _nameLabel->text());
}

void ProfileModView::prepareScoreView() {
	emit openScoreView(_modID, _nameLabel->text());
}

void ProfileModView::paintEvent(QPaintEvent *) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
