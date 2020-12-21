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

#include "widgets/ChangelogDialog.h"

#include <map>

#include "utils/Config.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QPushButton>
#include <QSettings>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QXmlStreamReader>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

ChangelogDialog::ChangelogDialog(QWidget * par) : QDialog(par), _changelogBrowser(nullptr) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	_changelogBrowser = new QTextBrowser(this);
	l->addWidget(_changelogBrowser);

	const bool dsa = Config::IniParser->value("CHANGELOGDIALOG/DontShowAgain", false).toBool();
	auto * cb = new QCheckBox(QApplication::tr("DontShowAgain"), this);
	cb->setChecked(dsa);
	l->addWidget(cb);
	connect(cb, &QCheckBox::stateChanged, this, &ChangelogDialog::changedShowState);

	auto * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Close, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Close);
	b->setText(QApplication::tr("Close"));

	connect(b, &QPushButton::released, this, &ChangelogDialog::accepted);
	connect(b, &QPushButton::released, this, &ChangelogDialog::accept);
	connect(b, &QPushButton::released, this, &ChangelogDialog::hide);

	setMinimumWidth(500);
	setMinimumHeight(500);

	setWindowTitle(QApplication::tr("Changelog"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	restoreSettings();
}

ChangelogDialog::~ChangelogDialog() {
	saveSettings();
}

int ChangelogDialog::execStartup() {
	const bool dsa = Config::IniParser->value("CHANGELOGDIALOG/DontShowAgain", false).toBool();
	
	if (dsa) return Accepted;
	
	return exec();
}

int ChangelogDialog::exec() {
	QFile xmlFile(qApp->applicationDirPath() + "/changelog.xml");

	if (!xmlFile.open(QIODevice::ReadOnly)) return Rejected;
	
	QXmlStreamReader xml(&xmlFile);
	
	const auto language = Config::Language;

	std::map<uint32_t, std::pair<QStringList, QStringList>, std::greater<uint32_t>> versions;

	uint32_t currentVersion = 0;

	while (!xml.atEnd()) {
		if (!xml.readNextStartElement()) break;
		
		const auto name = xml.name();

		if (name == "Version") {
			auto attributes = xml.attributes();

			if (attributes.count() != 3) continue;

			if (!attributes.hasAttribute("majorVersion")) continue;
			
			if (!attributes.hasAttribute("minorVersion")) continue;
			
			if (!attributes.hasAttribute("patchVersion")) continue;

			const uint8_t majorVersion = static_cast<uint8_t>(attributes.value("majorVersion").toInt());
			const uint8_t minorVersion = static_cast<uint8_t>(attributes.value("minorVersion").toInt());
			const uint8_t patchVersion = static_cast<uint8_t>(attributes.value("patchVersion").toInt());

			currentVersion = (majorVersion << 16) + (minorVersion << 8) + patchVersion;
			versions.insert(std::make_pair(currentVersion, std::make_pair(QStringList(), QStringList())));

			while (!xml.atEnd()) {
				if (!xml.readNextStartElement()) break;

				const auto childName = xml.name();

				if (childName == "Enhancement") {
					attributes = xml.attributes();

					if (!attributes.hasAttribute("language")) continue;

					const auto l = attributes.value("language");

					if (l != language) {
						xml.skipCurrentElement();
						continue;
					}

					const auto text = xml.readElementText();

					versions[currentVersion].first.push_back(text);
				} else if (childName == "Bug") {
					attributes = xml.attributes();

					if (!attributes.hasAttribute("language")) continue;

					const auto l = attributes.value("language");

					if (l != language) {
						xml.skipCurrentElement();
						continue;
					}

					const auto text = xml.readElementText();

					versions[currentVersion].second.push_back(text);
				}
			}
		}
	}

	QString html;

	for (auto & p : versions) {
		const int majorVersion = (p.first / 256 / 256) % 256;
		const int minorVersion = (p.first / 256) % 256;
		const int patchVersion = p.first % 256;
		html += "<h1>Version " + QString::number(majorVersion) + "." + QString::number(minorVersion) + "." + QString::number(patchVersion) + "</h1>";

		if (!p.second.first.empty()) {
			html += "<h2>" + QApplication::tr("Enhancements") + ":</h2><ul>";

			for (const QString & en : p.second.first) {
				html += "<li>" + en;
			}

			html += "</ul>";
		}
		if (!p.second.second.empty()) {
			html += "<h2>" + QApplication::tr("Bugs") + ":</h2><ul>";

			for (const QString & b : p.second.second) {
				html += "<li>" + b;
			}

			html += "</ul>";
		}
	}
	_changelogBrowser->setHtml(html);

	return QDialog::exec();
}

void ChangelogDialog::changedShowState(int state) {
	Config::IniParser->setValue("CHANGELOGDIALOG/DontShowAgain", state == Qt::Checked);
}

void ChangelogDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/ChangelogDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/ChangelogDialogGeometry");
	}
}

void ChangelogDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/ChangelogDialogGeometry", saveGeometry());
}
