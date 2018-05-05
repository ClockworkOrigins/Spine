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

#include "widgets/gothicIniPages/VisualizationPage.h"

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace g1 {

	VisualizationPage::VisualizationPage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();

		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * skyOutdoorBox = new QGroupBox(QApplication::tr("SkyOutdoor"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("zDayColor0", skyOutdoorBox);
				lbl->setToolTip(QApplication::tr("zDayColor0Tooltip"));
				_zDayColor0 = new QLineEdit(skyOutdoorBox);
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_zDayColor0, 0, 1);

				lbl = new QLabel("zDayColor1", skyOutdoorBox);
				lbl->setToolTip(QApplication::tr("zDayColor1Tooltip"));
				_zDayColor1 = new QLineEdit(skyOutdoorBox);
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_zDayColor1, 1, 1);

				lbl = new QLabel("zDayColor2", skyOutdoorBox);
				lbl->setToolTip(QApplication::tr("zDayColor2Tooltip"));
				_zDayColor2 = new QLineEdit(skyOutdoorBox);
				gl->addWidget(lbl, 2, 0);
				gl->addWidget(_zDayColor2, 2, 1);

				lbl = new QLabel("zDayColor3", skyOutdoorBox);
				lbl->setToolTip(QApplication::tr("zDayColor3Tooltip"));
				_zDayColor3 = new QLineEdit(skyOutdoorBox);
				gl->addWidget(lbl, 3, 0);
				gl->addWidget(_zDayColor3, 3, 1);

				skyOutdoorBox->setLayout(gl);

				hl->addWidget(skyOutdoorBox);
			}

			l->addLayout(hl);
		}

		setLayout(l);

		reject();
	}

	VisualizationPage::~VisualizationPage() {
	}

	void VisualizationPage::reject() {
		// Sky Outdoor
		QString text;
		_iniParser->beginGroup("SKY_OUTDOOR");
		text = _iniParser->value("zDayColor0", "230 115 0").toString();
		_zDayColor0->setText(text);
		text = _iniParser->value("zDayColor1", "255 255 0").toString();
		_zDayColor1->setText(text);
		text = _iniParser->value("zDayColor2", "64 0 0").toString();
		_zDayColor2->setText(text);
		text = _iniParser->value("zDayColor3", "230 115 0").toString();
		_zDayColor3->setText(text);
		_iniParser->endGroup();
	}

	void VisualizationPage::accept() {
		// Sky Outdoor
		_iniParser->beginGroup("SKY_OUTDOOR");
		_iniParser->setValue("zDayColor0", _zDayColor0->text());
		_iniParser->setValue("zDayColor1", _zDayColor1->text());
		_iniParser->setValue("zDayColor2", _zDayColor2->text());
		_iniParser->setValue("zDayColor3", _zDayColor3->text());
		_iniParser->endGroup();
	}

} /* namespace g1 */
} /* namespace widgets */
} /* namespace spine */
