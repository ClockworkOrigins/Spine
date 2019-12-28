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

#include "widgets/FAQEntry.h"

#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	FAQEntry::FAQEntry(QString question, QString answer, QWidget * par) : QWidget(par) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		QLabel * questionLabel = new QLabel(question, this);
		questionLabel->setWordWrap(true);
		questionLabel->setProperty("FAQQuestion", true);
		questionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		QLabel * answerLabel = new QLabel(answer, this);
		answerLabel->setWordWrap(true);

		l->addWidget(questionLabel);
		l->addWidget(answerLabel);

		setLayout(l);

		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	FAQEntry::~FAQEntry() {
	}

} /* namespace widgets */
} /* namespace spine */
