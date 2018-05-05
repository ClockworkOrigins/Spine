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

#include "widgets/gothicIniPages/GamePage.h"

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace widgets {
namespace g1 {

	GamePage::GamePage(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser) {
		QVBoxLayout * l = new QVBoxLayout();

		{
			QHBoxLayout * hl = new QHBoxLayout();

			{
				QGroupBox * subtitleBox = new QGroupBox(QApplication::tr("Subtitles"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("subTitles", subtitleBox);
				lbl->setToolTip(QApplication::tr("subTitlesTooltip"));
				_subtitleComboBox = new QComboBox(subtitleBox);
				_subtitleComboBox->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_subtitleComboBox, 0, 1);

				subtitleBox->setLayout(gl);

				hl->addWidget(subtitleBox);
			}

			{
				QGroupBox * cameraAndFocusBox = new QGroupBox(QApplication::tr("CameraAndFocus"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("camLookaroundInverse", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("camLookaroundInverseTooltip"));
				_camLookaroundInverse = new QComboBox(cameraAndFocusBox);
				_camLookaroundInverse->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_camLookaroundInverse, 0, 1);

				lbl = new QLabel("cameraLightRange", cameraAndFocusBox);
				lbl->setToolTip(QApplication::tr("cameraLightRangeTooltip"));
				_cameraLightRange = new QSpinBox(cameraAndFocusBox);
				_cameraLightRange->setMinimum(0);
				_cameraLightRange->setMaximum(100000);
				_cameraLightRange->setSuffix(" " + QApplication::tr("cm"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_cameraLightRange, 1, 1);

				cameraAndFocusBox->setLayout(gl);

				hl->addWidget(cameraAndFocusBox);
			}

			{
				QGroupBox * inventoryBox = new QGroupBox(QApplication::tr("Inventory"), this);

				QGridLayout * gl = new QGridLayout();
				gl->setAlignment(Qt::AlignTop);

				QLabel * lbl = new QLabel("invShowArrows", inventoryBox);
				lbl->setToolTip(QApplication::tr("invShowArrowsTooltip"));
				_invShowArrows = new QComboBox(inventoryBox);
				_invShowArrows->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 0, 0);
				gl->addWidget(_invShowArrows, 0, 1);

				lbl = new QLabel("invSwitchToFirstCategory", inventoryBox);
				lbl->setToolTip(QApplication::tr("invSwitchToFirstCategoryTooltip"));
				_invSwitchToFirstCategory = new QComboBox(inventoryBox);
				_invSwitchToFirstCategory->addItems(QStringList() << QApplication::tr("Off") << QApplication::tr("On"));
				gl->addWidget(lbl, 1, 0);
				gl->addWidget(_invSwitchToFirstCategory, 1, 1);

				inventoryBox->setLayout(gl);

				hl->addWidget(inventoryBox);
			}

			l->addLayout(hl);
		}

		setLayout(l);

		reject();
	}

	GamePage::~GamePage() {
	}

	void GamePage::reject() {
		// Subtitles
		int idx;
		int value;
		_iniParser->beginGroup("GAME");
		idx = _iniParser->value("subTitles", 0).toInt();
		_subtitleComboBox->setCurrentIndex(idx);

		// Camera and Focus
		idx = _iniParser->value("camLookaroundInverse", 0).toInt();
		_camLookaroundInverse->setCurrentIndex(idx);
		value = _iniParser->value("cameraLightRange", 0).toInt();
		_cameraLightRange->setValue(value);

		// Inventory
		idx = _iniParser->value("invShowArrows", 0).toInt();
		_invShowArrows->setCurrentIndex(idx);
		idx = _iniParser->value("invSwitchToFirstCategory", 0).toInt();
		_invSwitchToFirstCategory->setCurrentIndex(idx);
		_iniParser->endGroup();
	}

	void GamePage::accept() {
		_iniParser->beginGroup("GAME");
		// Subtitles
		_iniParser->setValue("subTitles", _subtitleComboBox->currentIndex());

		// Camera and Focus
		_iniParser->setValue("camLookaroundInverse", _camLookaroundInverse->currentIndex());
		_iniParser->setValue("cameraLightRange", _cameraLightRange->value());

		// Inventory
		_iniParser->setValue("invShowArrows", _invShowArrows->currentIndex());
		_iniParser->setValue("invSwitchToFirstCategory", _invSwitchToFirstCategory->currentIndex());
		_iniParser->endGroup();
	}

} /* namespace g1 */
} /* namespace widgets */
} /* namespace spine */
