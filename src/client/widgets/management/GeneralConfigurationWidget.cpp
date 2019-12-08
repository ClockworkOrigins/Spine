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

#include "widgets/management/GeneralConfigurationWidget.h"

#include "SpineConfig.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace client {
namespace widgets {

	GeneralConfigurationWidget::GeneralConfigurationWidget(QWidget * par) : QWidget(par), _mods(), _modIndex(-1) {
		QVBoxLayout * vl = new QVBoxLayout();

		{
			QGridLayout * l = new QGridLayout();
			l->setAlignment(Qt::AlignTop);

			{
				QLabel * lbl = new QLabel(QApplication::tr("Enabled"), this);
				lbl->setToolTip(QApplication::tr("ModEnabledTooltip"));
				_enabledBox = new QCheckBox(this);
				_enabledBox->setToolTip(QApplication::tr("ModEnabledTooltip"));

				l->addWidget(lbl, 0, 0);
				l->addWidget(_enabledBox, 0, 1);
			}

			{
				QLabel * lbl = new QLabel(QApplication::tr("GothicVersion"), this);
				_gothicVersionBox = new QComboBox(this);
				_gothicVersionBox->setEditable(false);

				QStringList items;
				items << QApplication::tr("Gothic") << QApplication::tr("Gothic2") << QApplication::tr("GothicInGothic2") << QApplication::tr("GothicAndGothic2");

				_gothicVersionBox->addItems(items);

				l->addWidget(lbl, 1, 0);
				l->addWidget(_gothicVersionBox, 1, 1);
			}

			{
				QLabel * lbl = new QLabel(QApplication::tr("ModType"), this);
				_typeBox = new QComboBox(this);
				_typeBox->setEditable(false);

				QStringList items;
				items << QApplication::tr("TotalConversion") << QApplication::tr("Enhancement") << QApplication::tr("Patch") << QApplication::tr("Tool") << QApplication::tr("Original") << QApplication::tr("GothicMultiplayer");

				_typeBox->addItems(items);

				l->addWidget(lbl, 2, 0);
				l->addWidget(_typeBox, 2, 1);
			}

			{
				QLabel * lbl = new QLabel(QApplication::tr("ReleaseDate"), this);
				_releaseDateEdit = new QDateEdit(this);
				_releaseDateEdit->setCalendarPopup(true);

				l->addWidget(lbl, 3, 0);
				l->addWidget(_releaseDateEdit, 3, 1);
			}

			{
				QLabel * lbl = new QLabel(QApplication::tr("DevDurationDescription"), this);
				_devDurationBox = new QSpinBox(this);
				_devDurationBox->setMinimum(0);
				_devDurationBox->setMaximum(500 * 60); // 300h

				l->addWidget(lbl, 4, 0);
				l->addWidget(_devDurationBox, 4, 1);
			}

			vl->addLayout(l);
		}

		QDialogButtonBox * dbb = new QDialogButtonBox(this);
		QPushButton * submitButton = new QPushButton(QApplication::tr("Submit"), this);
		dbb->addButton(submitButton, QDialogButtonBox::ButtonRole::AcceptRole);
		connect(submitButton, &QPushButton::released, this, &GeneralConfigurationWidget::updateMod);

		QPushButton * infoPageButton = new QPushButton(QApplication::tr("InfoPage"), this);
		infoPageButton->setToolTip(QApplication::tr("EditInfoPageTooltip"));
		dbb->addButton(infoPageButton, QDialogButtonBox::ButtonRole::ActionRole);
		connect(infoPageButton, &QPushButton::released, this, &GeneralConfigurationWidget::openInfoPage);

		vl->addStretch(1);

		vl->addWidget(dbb);

		setLayout(vl);
	}

	GeneralConfigurationWidget::~GeneralConfigurationWidget() {
	}

	void GeneralConfigurationWidget::updateModList(QList<client::ManagementMod> modList) {
		_mods = modList;
	}

	void GeneralConfigurationWidget::selectedMod(int index) {
		// TODO
		/*_modIndex = index;
		_enabledBox->setChecked(_mods[_modIndex].enabled);
		_gothicVersionBox->setCurrentIndex(int(_mods[_modIndex].gothicVersion));
		_typeBox->setCurrentIndex(int(_mods[_modIndex].type));
		_devDurationBox->setValue(_mods[_modIndex].duration);

		QDate date(2000, 1, 1);
		date = date.addDays(_mods[_modIndex].releaseDate);
		_releaseDateEdit->setDate(date);*/
	}

	void GeneralConfigurationWidget::updateMod() {
		// TODO
		if (_modIndex == -1) {
			return;
		}
		/*common::UpdateGeneralModConfigurationMessage ugmcm;
		ugmcm.modID = _mods[_modIndex].modID;
		ugmcm.enabled = _enabledBox->isChecked();
		ugmcm.gothicVersion = common::GothicVersion(_gothicVersionBox->currentIndex());
		ugmcm.modType = common::ModType(_typeBox->currentIndex());
		ugmcm.duration = _devDurationBox->value();
		QDate date(2000, 1, 1);
		ugmcm.releaseDate = uint32_t(date.daysTo(_releaseDateEdit->date()));
		const std::string serialized = ugmcm.SerializePublic();
		clockUtils::sockets::TcpSocket sock;
		const clockUtils::ClockError cErr = sock.connectToHostname("clockwork-origins.de", SERVER_PORT, 10000);
		if (clockUtils::ClockError::SUCCESS == cErr) {
			sock.writePacket(serialized);
		}*/
	}

	void GeneralConfigurationWidget::openInfoPage() {
		// TODO
		/*if (_modIndex == -1) {
			return;
		}
		emit triggerInfoPage(_mods[_modIndex].modID);*/
	}

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
