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

#include "widgets/GeneralSettingsWidget.h"

#include "SpineConfig.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include "widgets/UpdateLanguage.h"

#include "clockUtils/iniParser/iniParser.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDirIterator>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTranslator>
#include <QVBoxLayout>

using namespace spine;
using namespace spine::utils;
using namespace spine::widgets;

bool GeneralSettingsWidget::skipExitCheckbox = false;
GeneralSettingsWidget * GeneralSettingsWidget::instance = nullptr;

GeneralSettingsWidget::GeneralSettingsWidget(QWidget * par) : QWidget(par), _languageComboBox(nullptr), _styleComboBox(nullptr), _hideIncompatibleCheckBox(nullptr), _extendedLoggingCheckBox(nullptr) {
	instance = this;
	
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * hl = new QHBoxLayout();

		Config::Language = Config::IniParser->value("MISC/language", "English").toString();

		auto * languageLabel = new QLabel(QApplication::tr("Language"), this);
		_languageComboBox = new QComboBox(this);
		_languageComboBox->setEditable(false);
		_languageComboBox->addItem("Deutsch");
		_languageComboBox->addItem("English");
		_languageComboBox->addItem("Polish");
		_languageComboBox->addItem("Russian");
		_languageComboBox->setCurrentText(Config::Language);
		hl->addWidget(languageLabel);
		hl->addWidget(_languageComboBox);

		hl->setAlignment(Qt::AlignLeft | Qt::AlignTop);

		l->addLayout(hl);

		UPDATELANGUAGESETTEXT(languageLabel, "Language");

		l->addSpacing(10);
	}
	{
		auto * hl = new QHBoxLayout();

		const QString style = Config::IniParser->value("MISC/style", "Default").toString();

		auto * styleLabel = new QLabel(QApplication::tr("Style"), this);
		_styleComboBox = new QComboBox(this);
		_styleComboBox->setEditable(false);
		_styleComboBox->addItem("Default");
		_styleComboBox->addItem("Dark Theme By Elgcahlxukuth");
		_styleComboBox->addItem("Dark Theme By Milky-Way"); // not yet

		QDirIterator it(Config::STYLESDIR, QStringList() << "*.css", QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			it.next();
			_styleComboBox->addItem(QFileInfo(it.fileName()).baseName());
		}

		_styleComboBox->addItem("...");
		_styleComboBox->setCurrentText(style);
		hl->addWidget(styleLabel);
		hl->addWidget(_styleComboBox);

		hl->setAlignment(Qt::AlignLeft | Qt::AlignTop);

		l->addLayout(hl);

		UPDATELANGUAGESETTEXT(styleLabel, "Style");

		l->addSpacing(10);

		connect(_styleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
			changedStyle(_styleComboBox->itemText(idx));
		});
	}
	{
		auto * pb = new QPushButton(QApplication::tr("ReactivateModUpdates"), this);
		UPDATELANGUAGESETTEXT(pb, "ReactivateModUpdates");
		connect(pb, &QPushButton::released, this, &GeneralSettingsWidget::reactivateModUpdates);

		l->addWidget(pb);
	}
	{
		auto * hl = new QHBoxLayout();

		Config::downloadRate = Config::IniParser->value("MISC/kbps", 5120).toInt();
		if (Config::downloadRate > 5120) {
			Config::downloadRate = 1024;
		}
		auto * kbpsLabel = new QLabel(QApplication::tr("DownloadRate"), this);
		UPDATELANGUAGESETTEXT(kbpsLabel, "DownloadRate");
		kbpsLabel->setToolTip(QApplication::tr("DownloadRateTooltip"));
		UPDATELANGUAGESETTOOLTIP(kbpsLabel, "DownloadRateTooltip");
		_downloadRateSpinBox = new QSpinBox(this);
		_downloadRateSpinBox->setMaximum(5120);
		_downloadRateSpinBox->setMinimum(1);
		_downloadRateSpinBox->setValue(Config::downloadRate);
		_downloadRateSpinBox->setToolTip(QApplication::tr("DownloadRateTooltip"));
		UPDATELANGUAGESETTOOLTIP(_downloadRateSpinBox, "DownloadRateTooltip");
		hl->addWidget(kbpsLabel);
		hl->addWidget(_downloadRateSpinBox);

		hl->setAlignment(Qt::AlignLeft | Qt::AlignTop);

		l->addLayout(hl);
	}
	{
		const bool hideIncompatible = Config::IniParser->value("MISC/hideIncompatible", true).toBool();
		_hideIncompatibleCheckBox = new QCheckBox(QApplication::tr("HideIncompatiblePatches"), this);
		UPDATELANGUAGESETTEXT(_hideIncompatibleCheckBox, "HideIncompatiblePatches");
		_hideIncompatibleCheckBox->setChecked(hideIncompatible);

		l->addWidget(_hideIncompatibleCheckBox);
	}
	{
		Config::extendedLogging = Config::IniParser->value("MISC/extendedLogging", false).toBool();
		_extendedLoggingCheckBox = new QCheckBox(QApplication::tr("ExtendedLogging"), this);
		UPDATELANGUAGESETTEXT(_extendedLoggingCheckBox, "ExtendedLogging");
		_extendedLoggingCheckBox->setToolTip(QApplication::tr("ExtendedLoggingTooltip"));
		UPDATELANGUAGESETTOOLTIP(_extendedLoggingCheckBox, "ExtendedLoggingTooltip");
		_extendedLoggingCheckBox->setChecked(Config::extendedLogging);

		l->addWidget(_extendedLoggingCheckBox);
	}
	{
		skipExitCheckbox = Config::IniParser->value("MISC/skipExitCheckbox", false).toBool();
		_skipExitCheckBox = new QCheckBox(QApplication::tr("SkipExitCheckbox"), this);
		UPDATELANGUAGESETTEXT(_skipExitCheckBox, "SkipExitCheckbox");
		_skipExitCheckBox->setToolTip(QApplication::tr("SkipExitCheckboxTooltip"));
		UPDATELANGUAGESETTOOLTIP(_skipExitCheckBox, "SkipExitCheckboxTooltip");
		_skipExitCheckBox->setChecked(skipExitCheckbox);

		l->addWidget(_skipExitCheckBox);
	}
	{
		_devModeActive = Config::IniParser->value("DEVELOPER/Enabled", false).toBool();
		_developerModeCheckBox = new QCheckBox(QApplication::tr("ActivateDeveloperMode"), this);
		UPDATELANGUAGESETTEXT(_developerModeCheckBox, "ActivateDeveloperMode");
		_developerModeCheckBox->setToolTip(QApplication::tr("ActivateDeveloperModeTooltip"));
		UPDATELANGUAGESETTOOLTIP(_developerModeCheckBox, "ActivateDeveloperModeTooltip");
		_developerModeCheckBox->setChecked(_devModeActive);

		l->addWidget(_developerModeCheckBox);
	}

	l->addStretch(1);

	setLayout(l);
}

GeneralSettingsWidget * GeneralSettingsWidget::getInstance() {
	return instance;
}

void GeneralSettingsWidget::saveSettings() {
	Config::IniParser->beginGroup("MISC");
	const QString language = Config::IniParser->value("language", "English").toString();
	if (language != _languageComboBox->currentText()) {
		Config::IniParser->setValue("language", _languageComboBox->currentText());
		auto * translator = new QTranslator(qApp);
		if (_languageComboBox->currentText() == "Deutsch") {
			QLocale::setDefault(QLocale("de_DE"));
			translator->load(qApp->applicationDirPath() + "/de_DE");
		} else if (_languageComboBox->currentText() == "Polish") {
			QLocale::setDefault(QLocale(QLocale::Language::Polish, QLocale::Country::Poland));
			translator->load(qApp->applicationDirPath() + "/po_PO");
		} else if (_languageComboBox->currentText() == "Russian") {
			QLocale::setDefault(QLocale(QLocale::Language::Russian, QLocale::Country::Russia));
			translator->load(qApp->applicationDirPath() + "/ru_RU");
		} else if (_languageComboBox->currentText() == "Spanish") {
			QLocale::setDefault(QLocale(QLocale::Language::Spanish, QLocale::Country::Spain));
			translator->load(qApp->applicationDirPath() + "/es_ES");
		} else {
			QLocale::setDefault(QLocale("en_US"));
			translator->load(qApp->applicationDirPath() + "/en_US");
		}
		qApp->installTranslator(translator);
		Config::Language = _languageComboBox->currentText();
		emit languageChanged(_languageComboBox->currentText());
	}
	{
		const QString style = Config::IniParser->value("style", "Default").toString();
		if (style != _styleComboBox->currentText()) {
			Config::IniParser->setValue("style", _styleComboBox->currentText());
			QString cssFile;
			if (_styleComboBox->currentText() == "Default") {
				cssFile = ":styles.css";
			} else if (_styleComboBox->currentText() == "Dark Theme By Elgcahlxukuth") {
				cssFile = ":dark_theme_for_spine_app.css";
			} else if (_styleComboBox->currentText() == "Dark Theme By Milky-Way") {
				cssFile = ":monokai.css";
			} else if (_styleComboBox->currentText() == "...") {
				cssFile = ":styles.css";
			} else {
				cssFile = Config::STYLESDIR + "/" + _styleComboBox->currentText() + ".css";
			}
			QFile f(cssFile);
			if (f.open(QIODevice::ReadOnly)) {
				const QString s(f.readAll());
				qApp->setStyleSheet(s);
			}
		}
	}

	{
		Config::downloadRate = _downloadRateSpinBox->value();
		Config::IniParser->setValue("kbps", Config::downloadRate);
	}
	
	const bool hideIncompatible = Config::IniParser->value("hideIncompatible", true).toBool();
	Config::IniParser->setValue("hideIncompatible", _hideIncompatibleCheckBox->isChecked());
	if (hideIncompatible != _hideIncompatibleCheckBox->isChecked()) {
		emit changedHideIncompatible(_hideIncompatibleCheckBox->isChecked());
	}
	Config::IniParser->setValue("extendedLogging", _extendedLoggingCheckBox->isChecked());
	Config::extendedLogging = _extendedLoggingCheckBox->isChecked();

	Config::IniParser->setValue("skipExitCheckbox", _skipExitCheckBox->isChecked());
	skipExitCheckbox = _skipExitCheckBox->isChecked();
	Config::IniParser->endGroup();

	Config::IniParser->setValue("DEVELOPER/Enabled", _developerModeCheckBox->isChecked());
	
}

void GeneralSettingsWidget::rejectSettings() {
	Config::IniParser->beginGroup("MISC");
	const QString language = Config::IniParser->value("language", "English").toString();
	_languageComboBox->setCurrentText(language);

	const QString style = Config::IniParser->value("style", "Default").toString();
	_styleComboBox->setCurrentText(style);

	{
		int kbps = Config::IniParser->value("kbps", 5120).toInt();
		if (kbps > 5120) {
			kbps = 5120;
		}
		Config::downloadRate = kbps;
	}
	const bool hideIncompatible = Config::IniParser->value("hideIncompatible", true).toBool();
	_hideIncompatibleCheckBox->setChecked(hideIncompatible);
	Config::extendedLogging = Config::IniParser->value("extendedLogging", false).toBool();
	_extendedLoggingCheckBox->setChecked(Config::extendedLogging);
	skipExitCheckbox = Config::IniParser->value("skipExitCheckbox", false).toBool();
	_skipExitCheckBox->setChecked(skipExitCheckbox);
	Config::IniParser->endGroup();

	Config::IniParser->setValue("DEVELOPER/Enabled", _devModeActive);
}

bool GeneralSettingsWidget::getHideIncompatible() const {
	return _hideIncompatibleCheckBox->isChecked();
}

QString GeneralSettingsWidget::getLanguage() const {
	return _languageComboBox->currentText();
}

void GeneralSettingsWidget::changedStyle(QString styleName) {
	if (styleName != "...") return;
	
	const QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectStyle"), Config::STYLESDIR, "*.css");
	
	if (path.isEmpty() || path.contains(Config::STYLESDIR)) return;
	
	QFile(path).copy(Config::STYLESDIR + "/" + QFileInfo(path).fileName());
	_styleComboBox->clear();

	const QString style = Config::IniParser->value("MISC/style", "Default").toString();
	_styleComboBox->addItem("Default");

	QDirIterator it(Config::STYLESDIR, QStringList() << "*.css", QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		it.next();
		_styleComboBox->addItem(QFileInfo(it.fileName()).completeBaseName());
	}

	_styleComboBox->addItem("...");
	_styleComboBox->setCurrentText(style);
}

void GeneralSettingsWidget::reactivateModUpdates() {
	Database::DBError err;
	Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "DELETE FROM updates;", err);

	emit resetModUpdates();
}
