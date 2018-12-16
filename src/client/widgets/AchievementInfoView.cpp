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

#include "widgets/AchievementInfoView.h"

#include "Conversion.h"

#include <QDebug>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

namespace spine {
namespace widgets {

	AchievementInfoView::AchievementInfoView(common::SendAllAchievementStatsMessage::AchievementStats as, QWidget * par) : QWidget(par), _name(s2q(as.name)), _description(s2q(as.description)), _progress(as.unlockedPercent), _currentProgress(as.currentProgress), _maxProgress(as.maxProgress), _unlocked(as.unlocked) {
		setFixedSize(AchievementInfoView::sizeHint());
	}

	AchievementInfoView::~AchievementInfoView() {
	}
	
	QSize AchievementInfoView::sizeHint() const {
		return QSize(800 - 2 * 70, 70);
	}

	void AchievementInfoView::paintEvent(QPaintEvent *) {
		QPainter p(this);
		p.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
		QColor backgroundColor;
		backgroundColor.setNamedColor("#808080");
		{
			QPainterPath path;
			path.addRect(QRectF(0, 0, 800 - 2 * 70, 70));
			p.fillPath(path, QBrush(backgroundColor));
			p.drawPath(path);
		}

		QColor progressColor;
		progressColor.setNamedColor("#800000");
		{
			QPainterPath path;
			path.addRect(QRectF(5, 5, (800 - 2 * 70 - 10) * _progress / 100.0, 60));
			p.fillPath(path, QBrush(progressColor));
			p.drawPath(path);
		}

		if (!_unlocked && _maxProgress > 0 && _currentProgress < _maxProgress) {
			QColor progressbackgroundColor;
			progressbackgroundColor.setNamedColor("#808080");
			{
				QPainterPath path;
				path.addRect(QRectF(20, 49, 800 - 2 * 70 - 40, 18));
				p.fillPath(path, QBrush(progressbackgroundColor));
				p.drawPath(path);
			}
			{
				QColor ownProgressColor;
				ownProgressColor.setNamedColor("#B0B0B0");
				QPainterPath path;
				path.addRect(QRectF(21, 50, (800 - 2 * 70 - 42) * _currentProgress / double(_maxProgress), 16));
				p.fillPath(path, QBrush(ownProgressColor));
				p.drawPath(path);
			}
			{
				QFont f = p.font();
				f.setFamily("Lato Semibold");
				f.setPointSize(10);
				p.setFont(f);
				const QString progressString = QString("%1 / %2").arg(_currentProgress).arg(_maxProgress);
				QFontMetrics fm(f);
				p.setPen(QColor(255, 255, 255));
				p.drawText((800 - 2 * 70) / 2 - fm.width(progressString) / 2, 50 + 14 /*- fm.height() / 2*/, progressString);
			}
		}

		p.setPen(QColor(255, 255, 255));
		QFont f = p.font();
		f.setFamily("Lato Semibold");
		f.setPointSize(18);
		p.setFont(f);
		p.drawText(20, 30, _name);
		f.setPointSize(12);
		p.setFont(f);

		QFontMetrics fm(f);

		QStringList lines = { _description };

		int idx = 0;
		while (idx < lines.count()) {
			QString line = lines[idx];
			QString nextLine;
			while (fm.width(line) > 500) {
				QStringList split = line.split(' ');
				if (split.count() <= 1) break;

				line.remove(line.length() - split.last().length() - 1, line.length());
				nextLine.prepend(' ');
				nextLine.prepend(split.last());
			}
			lines[idx] = line;
			nextLine = nextLine.trimmed();
			if (!nextLine.isEmpty()) {
				lines.append(nextLine);
			}
			idx++;
		}

		for (int i = 0; i < lines.count(); i++) {
			p.drawText(20, 45 + ((_unlocked || _maxProgress == 0) && lines.count() == 1 ? 5 : 0) + i * 15, lines[i]);
		}

		p.drawText(800 - 2 * 70 - 65, 40, QString::number(_progress, 'f', 1) + "%");
	}

} /* namespace widgets */
} /* namespace spine */
