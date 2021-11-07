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

#include "widgets/management/GeneralConfigurationWidget.h"

#include "SpineConfig.h"

#include "gui/WaitSpinner.h"

#include "https/Https.h"

#include "utils/Config.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace spine::client;
using namespace spine::client::widgets;
using namespace spine::gui;
using namespace spine::utils;

GeneralConfigurationWidget::GeneralConfigurationWidget(QWidget * par) : QWidget(par), _modIndex(-1), _waitSpinner(nullptr) {
	auto * vl = new QVBoxLayout();

	{
		auto * l = new QGridLayout();
		l->setAlignment(Qt::AlignTop);

		int row = 0;

		{
			auto * lbl = new QLabel(QApplication::tr("Enabled"), this);
			lbl->setToolTip(QApplication::tr("ModEnabledTooltip"));
			_enabledBox = new QCheckBox(this);
			_enabledBox->setToolTip(QApplication::tr("ModEnabledTooltip"));
			connect(_enabledBox, &QCheckBox::stateChanged, this, [this]() {
				_releaseDateEdit->setDate(QDate::currentDate());
				_releaseDateEdit->setEnabled(!_enabledBox->isChecked());
			});

			l->addWidget(lbl, row, 0);
			l->addWidget(_enabledBox, row++, 1);
		}

		{
			auto * lbl = new QLabel(QApplication::tr("GothicVersion"), this);
			_gothicVersionBox = new QComboBox(this);
			_gothicVersionBox->setEditable(false);

			QStringList items;
			items << QApplication::tr("Gothic") << QApplication::tr("Gothic2") << QApplication::tr("GothicInGothic2") << QApplication::tr("GothicAndGothic2_2") << QApplication::tr("Game") << QApplication::tr("Gothic3");

			_gothicVersionBox->addItems(items);

			l->addWidget(lbl, row, 0);
			l->addWidget(_gothicVersionBox, row++, 1);
		}

		{
			auto * lbl = new QLabel(QApplication::tr("ModType"), this);
			_typeBox = new QComboBox(this);
			_typeBox->setEditable(false);

			QStringList items;
			items << QApplication::tr("TotalConversion") << QApplication::tr("Enhancement") << QApplication::tr("Patch") << QApplication::tr("Tool") << QApplication::tr("Original") << QApplication::tr("GothicMultiplayer") << QApplication::tr("FullVersion") << QApplication::tr("Demo") << QApplication::tr("PlayTesting");

			_typeBox->addItems(items);

			l->addWidget(lbl, row, 0);
			l->addWidget(_typeBox, row++, 1);
		}

		{
			auto * lbl = new QLabel(QApplication::tr("ReleaseDate"), this);
			_releaseDateEdit = new QDateEdit(this);
			_releaseDateEdit->setCalendarPopup(true);

			l->addWidget(lbl, row, 0);
			l->addWidget(_releaseDateEdit, row++, 1);
		}

		{
			auto * lbl = new QLabel(QApplication::tr("DevDurationDescription"), this);
			_devDurationBox = new QSpinBox(this);
			_devDurationBox->setMinimum(0);
			_devDurationBox->setMaximum(500 * 60); // 300h

			l->addWidget(lbl, row, 0);
			l->addWidget(_devDurationBox, row++, 1);
		}

		{
			auto * lbl = new QLabel(QApplication::tr("Keywords"), this);
			lbl->setToolTip(QApplication::tr("KeywordsTooltip"));
			_keywordsEdit = new QLineEdit(this);
			_keywordsEdit->setToolTip(QApplication::tr("KeywordsTooltip"));

			l->addWidget(lbl, row, 0);
			l->addWidget(_keywordsEdit, row++, 1);
		}

		{
			const QRegularExpression mailRegex("[a-zA-Z0-9+ _.@-]+");
			const QValidator * mailValidator = new QRegularExpressionValidator(mailRegex, this);
			
			auto * lbl = new QLabel(QApplication::tr("FeedbackMail"), this);
			lbl->setToolTip(QApplication::tr("FeedbackMailTooltip"));
			_feedbackMailEdit = new QLineEdit(this);
			_feedbackMailEdit ->setToolTip(QApplication::tr("FeedbackMailTooltip"));
			_feedbackMailEdit->setValidator(mailValidator);

			l->addWidget(lbl, row, 0);
			l->addWidget(_feedbackMailEdit, row++, 1);
		}

		{
			auto * lbl = new QLabel(QApplication::tr("DiscussionUrl"), this);
			lbl->setToolTip(QApplication::tr("DiscussionUrlTooltip"));
			_discussionUrlEdit = new QLineEdit(this);
			_discussionUrlEdit ->setToolTip(QApplication::tr("DiscussionUrlTooltip"));

			l->addWidget(lbl, row, 0);
			l->addWidget(_discussionUrlEdit, row++, 1);
		}

		vl->addLayout(l);
	}

	auto * dbb = new QDialogButtonBox(this);
	auto * submitButton = new QPushButton(QApplication::tr("Submit"), this);
	dbb->addButton(submitButton, QDialogButtonBox::ButtonRole::AcceptRole);
	connect(submitButton, &QPushButton::released, this, &GeneralConfigurationWidget::updateMod);

	auto * infoPageButton = new QPushButton(QApplication::tr("InfoPage"), this);
	infoPageButton->setToolTip(QApplication::tr("EditInfoPageTooltip"));
	dbb->addButton(infoPageButton, QDialogButtonBox::ButtonRole::ActionRole);
	connect(infoPageButton, &QPushButton::released, this, &GeneralConfigurationWidget::openInfoPage);

	vl->addStretch(1);

	vl->addWidget(dbb);

	setLayout(vl);

	qRegisterMetaType<ManagementGeneralData>("ManagementGeneralData");

	connect(this, &GeneralConfigurationWidget::removeSpinner, [this]() {
		if (!_waitSpinner) return;

		_waitSpinner->deleteLater();
		_waitSpinner = nullptr;
	});

	connect(this, &GeneralConfigurationWidget::loadedData, this, &GeneralConfigurationWidget::updateData);
}

GeneralConfigurationWidget::~GeneralConfigurationWidget() {
	_futureWatcher.waitForFinished();
}

void GeneralConfigurationWidget::updateModList(QList<ManagementMod> modList) {
	_mods = modList;
}

void GeneralConfigurationWidget::selectedMod(int index) {
	_modIndex = index;
}

void GeneralConfigurationWidget::updateView() {
	if (_modIndex == -1 || _modIndex >= _mods.size()) return;
	
	delete _waitSpinner;
	_waitSpinner = new WaitSpinner(QApplication::tr("Updating"), this);

	QJsonObject requestData;
	requestData["Username"] = Config::Username;
	requestData["Password"] = Config::Password;
	requestData["ModID"] = _mods[_modIndex].id;
	
	const auto f = https::Https::postAsync(MANAGEMENTSERVER_PORT, "getGeneralConfiguration", QJsonDocument(requestData).toJson(QJsonDocument::Compact), [this](const QJsonObject & json, int statusCode) {
		if (statusCode != 200) {
			emit removeSpinner();
			return;
		}
		ManagementGeneralData content;
		content.read(json);
		
		emit loadedData(content);
		emit removeSpinner();
	});
	_futureWatcher.setFuture(f);
}

void GeneralConfigurationWidget::updateMod() {
	if (_modIndex == -1) return;

	ManagementGeneralData mgd;
	mgd.enabled = _enabledBox->isChecked();
	mgd.gothicVersion = static_cast<common::GameType>(_gothicVersionBox->currentIndex());
	mgd.modType = static_cast<common::ModType>(_typeBox->currentIndex());
	mgd.duration = _devDurationBox->value();
	mgd.releaseDate = _releaseDateEdit->date();
	mgd.feedbackMail = _feedbackMailEdit->text();
	mgd.discussionUrl = _discussionUrlEdit->text();
	mgd.keywords = _keywordsEdit->text();

	QJsonObject json;
	json["Username"] = Config::Username;
	json["Password"] = Config::Password;
	json["ModID"] = _mods[_modIndex].id;
	mgd.write(json);
	
	https::Https::postAsync(MANAGEMENTSERVER_PORT, "updateGeneralConfiguration", QJsonDocument(json).toJson(QJsonDocument::Compact), [](const QJsonObject &, int) {
		// maybe add error handling here
	});
}

void GeneralConfigurationWidget::openInfoPage() {
	if (_modIndex == -1) return;
	
	emit triggerInfoPage(_mods[_modIndex].id);
}

void GeneralConfigurationWidget::updateData(ManagementGeneralData content) {
	_enabledBox->setChecked(content.enabled);
	_gothicVersionBox->setCurrentIndex(static_cast<int>(content.gothicVersion));
	_typeBox->setCurrentIndex(static_cast<int>(content.modType));
	_devDurationBox->setValue(content.duration);
	_releaseDateEdit->setDate(content.releaseDate);
	_releaseDateEdit->setEnabled(!content.enabled);
	_feedbackMailEdit->setText(content.feedbackMail);
	_discussionUrlEdit->setText(content.discussionUrl.toString());
	_enabledBox->setVisible(!content.enabled);
	_keywordsEdit->setText(content.keywords);
}
