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

#include "https/Https.h"

#include "widgets/WaitSpinner.h"

#include "clockUtils/sockets/TcpSocket.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace spine {
namespace client {
namespace widgets {

	GeneralConfigurationWidget::GeneralConfigurationWidget(const QString & username, const QString & password, QWidget * par) : QWidget(par), _mods(), _modIndex(-1), _waitSpinner(nullptr), _username(username), _password(password) {
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

		connect(this, &GeneralConfigurationWidget::removeSpinner, [this]() {
			if (!_waitSpinner) return;

			delete _waitSpinner;
			_waitSpinner = nullptr;
		});

		connect(this, &GeneralConfigurationWidget::loadedData, this, &GeneralConfigurationWidget::updateData);
	}

	GeneralConfigurationWidget::~GeneralConfigurationWidget() {
	}

	void GeneralConfigurationWidget::updateModList(QList<client::ManagementMod> modList) {
		_mods = modList;
	}

	void GeneralConfigurationWidget::selectedMod(int index) {
		_modIndex = index;
	}

	void GeneralConfigurationWidget::updateView() {
		delete _waitSpinner;
		_waitSpinner = new spine::widgets::WaitSpinner(QApplication::tr("Updating"), this);

		QJsonObject json;
		json["Username"] = _username;
		json["Password"] = _password;
		json["ModID"] = _mods[_modIndex].id;
		
		https::Https::postAsync(MANAGEMENTSERVER_PORT, "getGeneralConfiguration", QJsonDocument(json).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
			if (statusCode != 200) {
				emit removeSpinner();
				return;
			}
			ManagementGeneralData content;
			content.read(json);
			
			emit loadedData(content);
			emit removeSpinner();
		});
	}

	void GeneralConfigurationWidget::updateMod() {
		if (_modIndex == -1) return;

		ManagementGeneralData mgd;
		mgd.enabled = _enabledBox->isChecked();
		mgd.gothicVersion = static_cast<common::GothicVersion>(_gothicVersionBox->currentIndex());
		mgd.modType = static_cast<common::ModType>(_typeBox->currentIndex());
		mgd.duration = _devDurationBox->value();
		mgd.releaseDate = _releaseDateEdit->date();

		QJsonObject json;
		json["Username"] = _username;
		json["Password"] = _password;
		json["ModID"] = _mods[_modIndex].id;
		mgd.write(json);
		
		https::Https::postAsync(MANAGEMENTSERVER_PORT, "updateGeneralConfiguration", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject & json, int statusCode) {
			// maybe add error handling here
		});
	}

	void GeneralConfigurationWidget::openInfoPage() {
		if (_modIndex == -1) return;
		
		emit triggerInfoPage(_mods[_modIndex].id);
	}

	void GeneralConfigurationWidget::updateData(ManagementGeneralData content) {
		_enabledBox->setChecked(content.enabled);
		_gothicVersionBox->setCurrentIndex(int(content.gothicVersion));
		_typeBox->setCurrentIndex(int(content.modType));
		_devDurationBox->setValue(content.duration);
		_releaseDateEdit->setDate(content.releaseDate);
	}

} /* namespace widgets */
} /* namespace client */
} /* namespace spine */
