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
// Copyright 2020 Clockwork Origins

#include "widgets/ReviewWidget.h"

#include "utils/Conversion.h"

#include <QApplication>
#include <QDate>
#include <QGroupBox>
#include <QLabel>
#include <QSvgWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

ReviewWidget::ReviewWidget(const QString & reviewer, const QString & review, uint64_t playTime, uint64_t playTimeAtReview, uint64_t date, int rating, QWidget * par) : QWidget(par), _svgs() {
    auto * l = new QVBoxLayout();

    auto * gb = new QGroupBox(reviewer, this);
	
	auto * vl = new QVBoxLayout();

    {
        auto * hl = new QHBoxLayout();
        auto * createdLbl = new QLabel(QApplication::tr("CreatedAt") + ": " + QDate(1970, 1, 1).addDays(date / 60 / 60 / 24).toString("dd.MM.yyyy"));
        createdLbl->setProperty("reviewSmall", true);
		
        hl->addWidget(createdLbl, 0, Qt::AlignLeft);
		
		auto * ratingLayout = new QHBoxLayout();
        for (auto & svg : _svgs) {
            svg = new QSvgWidget(":/svg/star.svg", this);
            svg->setFixedSize(QSize(25, 25));
            ratingLayout->addWidget(svg);
        }
        hl->addLayout(ratingLayout);

        {
            auto * vl2 = new QVBoxLayout();

            auto * lbl1 = new QLabel(QApplication::tr("OverallPlayTime") + ": " + timeToString(static_cast<double>(playTime)), this);
            auto * lbl2 = new QLabel(QApplication::tr("ReviewPlayTime") + ": " + timeToString(static_cast<double>(playTimeAtReview)), this);

            lbl1->setProperty("reviewSmall", true);
            lbl2->setProperty("reviewSmall", true);
                    	
            vl2->addWidget(lbl1, 0, Qt::AlignRight);
            vl2->addWidget(lbl2, 0, Qt::AlignRight);

            hl->addLayout(vl2);
        }

        vl->addLayout(hl);
    }

    auto * tb = new QTextBrowser(this);
    tb->setText(review);

    vl->addWidget(tb);

    gb->setLayout(vl);

    l->addWidget(gb);

    setLayout(l);

    for (size_t i = 0; i < _svgs.size(); i++) {
        if (std::floor(rating) > static_cast<qreal>(i)) {
			_svgs[i]->load(QString(":/svg/star-full.svg"));
        } else if (rating - static_cast<qreal>(i) >= 0.5) {
            _svgs[i]->load(QString(":/svg/star-half.svg"));
        } else {
            _svgs[i]->load(QString(":/svg/star.svg"));
        }
    }
}
