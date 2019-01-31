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

#include "widgets/GeneralSettingsWidget.h"

#include "Config.h"
#include "Database.h"
#include "SpineConfig.h"
#include "UpdateLanguage.h"

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

namespace spine {
namespace widgets {

	int GeneralSettingsWidget::downloadRate = 5120;
	bool GeneralSettingsWidget::extendedLogging = false;
	bool GeneralSettingsWidget::skipExitCheckbox = false;

	GeneralSettingsWidget::GeneralSettingsWidget(QSettings * iniParser, QWidget * par) : QWidget(par), _iniParser(iniParser), _languageComboBox(nullptr), _styleComboBox(nullptr), _autoUpdateBox(nullptr), _hideIncompatibleCheckBox(nullptr), _extendedLoggingCheckBox(nullptr) {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();

			const QString language = _iniParser->value("MISC/language", "English").toString();

			QLabel * languageLabel = new QLabel(QApplication::tr("Language"), this);
			_languageComboBox = new QComboBox(this);
			_languageComboBox->setEditable(false);
			_languageComboBox->addItem("Deutsch");
			_languageComboBox->addItem("English");
			_languageComboBox->addItem("Polish");
			_languageComboBox->addItem("Russian");
			_languageComboBox->setCurrentText(language);
			hl->addWidget(languageLabel);
			hl->addWidget(_languageComboBox);

			hl->setAlignment(Qt::AlignLeft | Qt::AlignTop);

			l->addLayout(hl);

			UPDATELANGUAGESETTEXT(this, languageLabel, "Language");

			l->addSpacing(10);
		}
		{
			QHBoxLayout * hl = new QHBoxLayout();

			const QString style = _iniParser->value("MISC/style", "Default").toString();

			QLabel * styleLabel = new QLabel(QApplication::tr("Style"), this);
			_styleComboBox = new QComboBox(this);
			_styleComboBox->setEditable(false);
			_styleComboBox->addItem("Default");
			//_styleComboBox->addItem("Dark Theme By Elgcahlxukuth");
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

			UPDATELANGUAGESETTEXT(this, styleLabel, "Style");

			l->addSpacing(10);

			connect(_styleComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changedStyle(QString)));
		}
		{
			_autoUpdateBox = new QCheckBox(QApplication::tr("AutoUpdateCheck"), this);;

			const bool checkForUpdates = _iniParser->value("MISC/checkForUpdates", true).toBool();
			_autoUpdateBox->setChecked(checkForUpdates);

			l->addWidget(_autoUpdateBox);

			UPDATELANGUAGESETTEXT(this, _autoUpdateBox, "AutoUpdateCheck");

			l->addSpacing(10);
		}
		{
			QPushButton * pb = new QPushButton(QApplication::tr("ReactivateModUpdates"), this);
			UPDATELANGUAGESETTEXT(this, pb, "ReactivateModUpdates");
			connect(pb, SIGNAL(released()), this, SLOT(reactivateModUpdates()));

			l->addWidget(pb);
		}
		{
			QHBoxLayout * hl = new QHBoxLayout();

			downloadRate = _iniParser->value("MISC/kbps", 5120).toInt();
			if (downloadRate > 5120) {
				downloadRate = 5120;
			}
			QLabel * kbpsLabel = new QLabel(QApplication::tr("DownloadRate"), this);
			UPDATELANGUAGESETTEXT(this, kbpsLabel, "DownloadRate");
			kbpsLabel->setToolTip(QApplication::tr("DownloadRateTooltip"));
			UPDATELANGUAGESETTOOLTIP(this, kbpsLabel, "DownloadRateTooltip");
			_downloadRateSpinBox = new QSpinBox(this);
			_downloadRateSpinBox->setMaximum(5120);
			_downloadRateSpinBox->setMinimum(1);
			_downloadRateSpinBox->setValue(downloadRate);
			_downloadRateSpinBox->setToolTip(QApplication::tr("DownloadRateTooltip"));
			UPDATELANGUAGESETTOOLTIP(this, _downloadRateSpinBox, "DownloadRateTooltip");
			hl->addWidget(kbpsLabel);
			hl->addWidget(_downloadRateSpinBox);

			hl->setAlignment(Qt::AlignLeft | Qt::AlignTop);

			l->addLayout(hl);
		}
		{
			const bool hideIncompatible = _iniParser->value("MISC/hideIncompatible", true).toBool();;
			_hideIncompatibleCheckBox = new QCheckBox(QApplication::tr("HideIncompatiblePatches"), this);
			UPDATELANGUAGESETTEXT(this, _hideIncompatibleCheckBox, "HideIncompatiblePatches");
			_hideIncompatibleCheckBox->setChecked(hideIncompatible);

			l->addWidget(_hideIncompatibleCheckBox);
		}
		{
			extendedLogging = _iniParser->value("MISC/extendedLogging", false).toBool();
			_extendedLoggingCheckBox = new QCheckBox(QApplication::tr("ExtendedLogging"), this);
			UPDATELANGUAGESETTEXT(this, _extendedLoggingCheckBox, "ExtendedLogging");
			_extendedLoggingCheckBox->setToolTip(QApplication::tr("ExtendedLoggingTooltip"));
			UPDATELANGUAGESETTOOLTIP(this, _extendedLoggingCheckBox, "ExtendedLoggingTooltip");
			_extendedLoggingCheckBox->setChecked(extendedLogging);

			l->addWidget(_extendedLoggingCheckBox);
		}
		{
			skipExitCheckbox = _iniParser->value("MISC/skipExitCheckbox", false).toBool();
			_skipExitCheckBox = new QCheckBox(QApplication::tr("SkipExitCheckbox"), this);
			UPDATELANGUAGESETTEXT(this, _skipExitCheckBox, "SkipExitCheckbox");
			_skipExitCheckBox->setToolTip(QApplication::tr("SkipExitCheckboxTooltip"));
			UPDATELANGUAGESETTOOLTIP(this, _skipExitCheckBox, "SkipExitCheckboxTooltip");
			_skipExitCheckBox->setChecked(skipExitCheckbox);

			l->addWidget(_skipExitCheckBox);
		}

		l->addStretch(1);

		setLayout(l);
	}

	GeneralSettingsWidget::~GeneralSettingsWidget() {
	}

	void GeneralSettingsWidget::saveSettings() {
		_iniParser->beginGroup("MISC");
		const QString language = _iniParser->value("language", "English").toString();
		if (language != _languageComboBox->currentText()) {
			_iniParser->setValue("language", _languageComboBox->currentText());
			QTranslator * translator = new QTranslator(qApp);
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
			emit languageChanged(_languageComboBox->currentText());
		}
		{
			const QString style = _iniParser->value("style", "Default").toString();
			if (style != _styleComboBox->currentText()) {
				_iniParser->setValue("style", _styleComboBox->currentText());
				QString cssFile = ":styles.css";
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

		_iniParser->setValue("checkForUpdates", _autoUpdateBox->isChecked());
		{
			downloadRate = _downloadRateSpinBox->value();
			_iniParser->setValue("kbps", downloadRate);
		}
		const bool hideIncompatible = _iniParser->value("hideIncompatible", true).toBool();
		_iniParser->setValue("hideIncompatible", _hideIncompatibleCheckBox->isChecked());
		if (hideIncompatible != _hideIncompatibleCheckBox->isChecked()) {
			emit changedHideIncompatible(_hideIncompatibleCheckBox->isChecked());
		}
		_iniParser->setValue("extendedLogging", _extendedLoggingCheckBox->isChecked());
		extendedLogging = _extendedLoggingCheckBox->isChecked();

		_iniParser->setValue("skipExitCheckbox", _skipExitCheckBox->isChecked());
		skipExitCheckbox = _skipExitCheckBox->isChecked();
		_iniParser->endGroup();
	}

	void GeneralSettingsWidget::rejectSettings() {
		_iniParser->beginGroup("MISC");
		const QString language = _iniParser->value("language", "English").toString();
		_languageComboBox->setCurrentText(language);

		const QString style = _iniParser->value("style", "Default").toString();
		_styleComboBox->setCurrentText(style);

		const bool checkForUpdates = _iniParser->value("checkForUpdates", true).toBool();
		_autoUpdateBox->setChecked(checkForUpdates);
		{
			int kbps = _iniParser->value("kbps", 5120).toInt();
			if (kbps > 5120) {
				kbps = 5120;
			}
			downloadRate = kbps;
		}
		const bool hideIncompatible = _iniParser->value("hideIncompatible", true).toBool();
		_hideIncompatibleCheckBox->setChecked(hideIncompatible);
		extendedLogging = _iniParser->value("extendedLogging", false).toBool();
		_extendedLoggingCheckBox->setChecked(extendedLogging);
		skipExitCheckbox = _iniParser->value("skipExitCheckbox", false).toBool();
		_skipExitCheckBox->setChecked(skipExitCheckbox);
		_iniParser->endGroup();
	}

	QString GeneralSettingsWidget::getLanguage() const {
		return _languageComboBox->currentText();
	}

	bool GeneralSettingsWidget::getHideIncompatible() const {
		return _hideIncompatibleCheckBox->isChecked();
	}

	void GeneralSettingsWidget::changedStyle(QString styleName) {
		if (styleName == "...") {
			QString path = QFileDialog::getOpenFileName(this, QApplication::tr("SelectStyle"), Config::STYLESDIR, "*.css");
			if (!path.isEmpty() && !path.contains(Config::STYLESDIR)) {
				QFile(path).copy(Config::STYLESDIR + "/" + QFileInfo(path).fileName());
				_styleComboBox->clear();

				const QString style = _iniParser->value("MISC/style", "Default").toString();
				_styleComboBox->addItem("Default");

				QDirIterator it(Config::STYLESDIR, QStringList() << "*.css", QDir::Files, QDirIterator::Subdirectories);
				while (it.hasNext()) {
					it.next();
					_styleComboBox->addItem(QFileInfo(it.fileName()).completeBaseName());
				}

				_styleComboBox->addItem("...");
				_styleComboBox->setCurrentText(style);
			}
		}
	}

	void GeneralSettingsWidget::reactivateModUpdates() {
		Database::DBError err;
		Database::execute(Config::BASEDIR.toStdString() + "/" + UPDATES_DATABASE, "DELETE FROM updates;", err);

		emit resetModUpdates();
	}

} /* namespace widgets */
} /* namespace spine */
