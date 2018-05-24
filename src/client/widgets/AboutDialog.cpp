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

#include "widgets/AboutDialog.h"

#include "SpineConfig.h"
#include "UpdateLanguage.h"

#include "widgets/GeneralSettingsWidget.h"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

namespace spine {
namespace widgets {

	AboutDialog::AboutDialog(GeneralSettingsWidget * generalSettingsWidget, QWidget * parent) : QDialog(parent, Qt::Popup) {
		QGridLayout * gridLayout = new QGridLayout(this);
		QVBoxLayout * layout = new QVBoxLayout();
		QFont f;
		f.setBold(true);
		setFont(f);
		layout->addWidget(new QLabel(QString("Spine ") + QString::fromStdString(VERSION_STRING), this));
		layout->addWidget(new QLabel(QString("Clockwork Origins"), this));
		QLabel * cwMail = new QLabel(QApplication::tr("Contact") + ": <a href=\"mailto:contact@clockwork-origins.de\">contact@clockwork-origins.de</a>", this);
		UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, cwMail, "Contact", ": <a href=\"mailto:contact@clockwork-origins.de\">contact@clockwork-origins.de</a>");
		cwMail->setOpenExternalLinks(true);
		layout->addWidget(cwMail);
		QLabel * cwLink = new QLabel(QApplication::tr("Homepage") + ": <a href=\"https://clockwork-origins.com/\">https://clockwork-origins.com/</a>", this);
		UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, cwLink, "Homepage", ": <a href=\"https://clockwork-origins.com/\">https://clockwork-origins.com/</a>");
		cwLink->setOpenExternalLinks(true);
		layout->addWidget(cwLink);
		layout->addWidget(new QLabel(QString(""), this));
		QLabel * buildForLabel = new QLabel(QApplication::tr("BuildForQt") + " " + QT_VERSION_STR, this);
		layout->addWidget(buildForLabel);
		UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, buildForLabel, "BuildForQt", " " + QT_VERSION_STR);
		QLabel * runningWithLabel = new QLabel(QApplication::tr("RunningWithQt") + " " + qVersion(), this);
		layout->addWidget(runningWithLabel);
		UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, runningWithLabel, "RunningWithQt", " " + qVersion());
		QDir dir(QApplication::applicationDirPath() + "/../copyright/");
		QLabel * licenseLabel = new QLabel(QApplication::tr("License") + ": " + dir.absolutePath() + "/Qt LICENSE", this);
		layout->addWidget(licenseLabel);
		UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, licenseLabel, "License", ": " + dir.absolutePath() + "/Qt LICENSE");
		QLabel * qtLink = new QLabel(QApplication::tr("Homepage") + ": <a href=\"https://www.qt.io/\">https://www.qt.io/</a>", this);
		UPDATELANGUAGESETTEXTEXT(generalSettingsWidget, qtLink, "Homepage", ": <a href=\"https://www.qt.io/\">https://www.qt.io/</a>");
		qtLink->setOpenExternalLinks(true);
		layout->addWidget(qtLink);
		QPixmap pixmap(":/clockworkLogo.png");
		pixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		QLabel * logoLabel = new QLabel(this);
		logoLabel->setPixmap(pixmap);
		gridLayout->addWidget(logoLabel, 0, 0);
		gridLayout->addLayout(layout, 0, 1);
		setLayout(gridLayout);
	}

	AboutDialog::~AboutDialog() {
	}

} /* namespace widgets */
} /* namespace spine */
