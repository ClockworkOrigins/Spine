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

#include "translator/TranslationRequestDialog.h"

#include "common/TranslationModel.h"

#include "gui/WaitSpinner.h"

#include "translator/GothicParser.h"

#include "utils/Config.h"
#include "utils/Conversion.h"
#include "utils/LanguageConverter.h"

#include "translator/AccessRightsDialog.h"
#include "translator/ApplyTranslationDialog.h"
#include "translator/TranslatorAPI.h"

#include <QApplication>
#include <QComboBox>
#include <QtConcurrentRun>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>

using namespace spine::common;
using namespace spine::gui;
using namespace spine::translation;
using namespace spine::utils;

TranslationRequestDialog::TranslationRequestDialog(QWidget * par) : QDialog(par), _model(nullptr), _progressDialog(nullptr), _requestList(new QVBoxLayout()) {
	auto * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		auto * hl = new QHBoxLayout();

		auto * pathLabel = new QLabel(QApplication::tr("ParsePath"), this);
		_pathEdit = new QLineEdit(this);
		_pathEdit->setReadOnly(true);
		_pathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		auto * pathPushButton = new QPushButton("...", this);
		hl->addWidget(pathLabel);
		hl->addWidget(_pathEdit);
		hl->addWidget(pathPushButton);
		connect(pathPushButton, &QPushButton::released, this, &TranslationRequestDialog::openFileDialog);

		hl->setAlignment(Qt::AlignLeft);

		l->addLayout(hl);
	}

	{
		auto * hl = new QHBoxLayout();

		hl->setAlignment(Qt::AlignLeft);

		const QStringList languages = LanguageConverter::getLanguages();

		auto * projectNameLabel = new QLabel(QApplication::tr("ProjectName"), this);
		_projectNameEdit = new QLineEdit(this);

		auto * sourceLanguageLabel = new QLabel(QApplication::tr("SourceLanguage"), this);
		_sourceLanguageBox = new QComboBox(this);
		_sourceLanguageBox->addItems(languages);

		auto * destinationLanguageLabel = new QLabel(QApplication::tr("DestinationLanguage"), this);
		_destinationLanguageBox = new QComboBox(this);
		_destinationLanguageBox->addItems(languages);

		hl->addWidget(projectNameLabel);
		hl->addWidget(_projectNameEdit);
		hl->addWidget(sourceLanguageLabel);
		hl->addWidget(_sourceLanguageBox);
		hl->addWidget(destinationLanguageLabel);
		hl->addWidget(_destinationLanguageBox);

		l->addLayout(hl);

		connect(_projectNameEdit, &QLineEdit::textChanged, this, &TranslationRequestDialog::checkParsePossible);
		connect(_sourceLanguageBox, &QComboBox::currentTextChanged, this, &TranslationRequestDialog::checkParsePossible);
		connect(_destinationLanguageBox, &QComboBox::currentTextChanged, this, &TranslationRequestDialog::checkParsePossible);
	}

	{
		auto * hl = new QHBoxLayout();

		hl->setAlignment(Qt::AlignLeft);

		_parseButton = new QPushButton(QApplication::tr("ParseScripts"), this);
		_parseButton->setDisabled(true);
		connect(_parseButton, &QPushButton::released, this, &TranslationRequestDialog::parseScripts);

		_requestButton = new QPushButton(QApplication::tr("RequestTranslation"), this);
		_requestButton->setDisabled(true);
		connect(_requestButton, &QPushButton::released, this, &TranslationRequestDialog::requestTranslation);

		hl->addWidget(_parseButton);
		hl->addWidget(_requestButton);

		l->addLayout(hl);
	}

	{
		auto * gl = new QGridLayout();

		gl->setAlignment(Qt::AlignLeft);

		_nameLabel = new QLabel(QApplication::tr("Names"), this);
		_nameLabel->hide();
		_nameCountLabel = new QLabel(this);
		_nameCountLabel->hide();

		_textLabel = new QLabel(QApplication::tr("Texts"), this);
		_textLabel->hide();
		_textCountLabel = new QLabel(this);
		_textCountLabel->hide();

		_dialogLabel = new QLabel(QApplication::tr("Dialogs/DialogLines"), this);
		_dialogLabel->hide();
		_dialogCountLabel = new QLabel(this);
		_dialogCountLabel->hide();

		_overallLabel = new QLabel(QApplication::tr("Overall"), this);
		_overallLabel->hide();
		_overallCountLabel = new QLabel(this);
		_overallCountLabel->hide();

		gl->addWidget(_nameLabel, 0, 0);
		gl->addWidget(_nameCountLabel, 0, 1);
		gl->addWidget(_textLabel, 1, 0);
		gl->addWidget(_textCountLabel, 1, 1);
		gl->addWidget(_dialogLabel, 2, 0);
		gl->addWidget(_dialogCountLabel, 2, 1);
		gl->addWidget(_overallLabel, 3, 0);
		gl->addWidget(_overallCountLabel, 3, 1);

		l->addLayout(gl);
	}

	_requestList->setAlignment(Qt::AlignTop);

	l->addLayout(_requestList);

	setLayout(l);

	setWindowTitle(QApplication::tr("RequestTranslation"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	restoreSettings();

	qRegisterMetaType<std::vector<SendOwnProjectsMessage::Project>>("std::vector<spine::common::SendOwnProjectsMessage::Project>");
	connect(this, &TranslationRequestDialog::receivedRequestList, this, &TranslationRequestDialog::updateRequestList);

	requestList();
}

TranslationRequestDialog::~TranslationRequestDialog() {
	delete _model;
	saveSettings();
}

void TranslationRequestDialog::openFileDialog() {
	const QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothicDir"), _pathEdit->text());
	if (!path.isEmpty()) {
		if (_pathEdit->text() != path) {
			_pathEdit->setText(path);
		}
	}
	checkParsePossible();
}

void TranslationRequestDialog::parseScripts() {
	delete _model;

	GothicParser gp(this);

	_model = TranslatorAPI::createModel(q2s(_projectNameEdit->text()), q2s(_sourceLanguageBox->currentText()), q2s(_destinationLanguageBox->currentText()));
	_progressDialog = new QProgressDialog(QApplication::tr("ParseScripts"), "", 0, 100, this);
	_progressDialog->setCancelButton(nullptr);
	_progressDialog->setWindowFlags(_progressDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	connect(&gp, &GothicParser::updatedProgress, this, &TranslationRequestDialog::updateProgress);
	gp.parseTexts(_pathEdit->text(), _model);
	delete _progressDialog;
	const bool condition = _model && _model->getNames().size() + _model->getTexts().size() + _model->getDialogTextCount() > 0;
	if (condition) {
		_nameCountLabel->setText(QString::number(_model->getNames().size()));
		_textCountLabel->setText(QString::number(_model->getTexts().size()));
		_dialogCountLabel->setText(QString::number(_model->getDialogs().size()) + " / " + QString::number(_model->getDialogTextCount()));
		_overallCountLabel->setText(QString::number(_model->getNames().size() + _model->getTexts().size() + _model->getDialogTextCount()));

		_nameLabel->setVisible(true);
		_nameCountLabel->setVisible(true);
		_textLabel->setVisible(true);
		_textCountLabel->setVisible(true);
		_dialogLabel->setVisible(true);
		_dialogCountLabel->setVisible(true);
		_overallLabel->setVisible(true);
		_overallCountLabel->setVisible(true);
	}
	_requestButton->setEnabled(condition);
}

void TranslationRequestDialog::requestTranslation() {
	gui::WaitSpinner ws(QApplication::tr("RequestingTranslationSpinner"), this);
	QEventLoop loop;
	QFutureWatcher<void> watcher;
	connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
	const QFuture<void> future = QtConcurrent::run([this]() {
		TranslatorAPI::requestTranslation(q2s(Config::Username), _model);
		delete _model;
		_model = nullptr;
	});
	watcher.setFuture(future);
	loop.exec();

	requestList();
	_requestButton->setEnabled(false);
}

void TranslationRequestDialog::updateProgress(int current, int max) {
	Q_ASSERT(_progressDialog);
	_progressDialog->setMaximum(max);
	_progressDialog->setValue(current);
}

void TranslationRequestDialog::checkParsePossible() {
	_parseButton->setEnabled(!Config::Username.isEmpty() && !_pathEdit->text().isEmpty() && !_projectNameEdit->text().isEmpty() && _sourceLanguageBox->currentIndex() != _destinationLanguageBox->currentIndex());
}

void TranslationRequestDialog::updateRequestList(std::vector<SendOwnProjectsMessage::Project> projects) {
	for (QWidget * w : _widgets) {
		w->deleteLater();
	}
	_widgets.clear();

	for (const auto & project : projects) {
		auto * hl = new QHBoxLayout();
		auto * l = new QLabel(s2q(project.projectName) + " (" + s2q(project.sourceLanguage) + " => " + s2q(project.destinationLanguage) + ")", this);
		auto * pb = new QProgressBar(this);
		pb->setMinimum(0);
		pb->setMaximum(static_cast<int>(project.toTranslate));
		pb->setValue(static_cast<int>(project.translated));
		pb->setFormat("%v / %m (%p %)");
		pb->setAlignment(Qt::AlignCenter);

		auto * accessButton = new QPushButton(QApplication::tr("UserManagement"), this);
		accessButton->setProperty("requestID", static_cast<int>(project.requestID));
		accessButton->setProperty("title", l->text());
		connect(accessButton, &QPushButton::released, this, &TranslationRequestDialog::openAccessDialog);

		auto * applyTranslationButton = new QPushButton(QApplication::tr("ApplyTranslation"), this);
		applyTranslationButton->setProperty("requestID", static_cast<int>(project.requestID));
		applyTranslationButton->setProperty("title", l->text());
		connect(applyTranslationButton, &QPushButton::released, this, &TranslationRequestDialog::applyTranslation);

		auto * downloadCsvButton = new QPushButton(QApplication::tr("DownloadCsv"), this);
		downloadCsvButton->setProperty("requestID", static_cast<int>(project.requestID));
		downloadCsvButton->setProperty("title", l->text());
		connect(downloadCsvButton, &QPushButton::released, this, &TranslationRequestDialog::downloadCsv);

		hl->addWidget(l, 1);
		hl->addWidget(pb, 1);
		hl->addWidget(accessButton, 0);
		hl->addWidget(applyTranslationButton, 0);
		hl->addWidget(downloadCsvButton, 0);

		_requestList->addLayout(hl);

		_widgets.append(l);
		_widgets.append(pb);
		_widgets.append(accessButton);
		_widgets.append(applyTranslationButton);
		_widgets.append(downloadCsvButton);
	}
}

void TranslationRequestDialog::openAccessDialog() {
	Q_ASSERT(sender());
	const int requestID = sender()->property("requestID").toInt();
	const QString title = sender()->property("title").toString();
	AccessRightsDialog dlg(requestID, title, this);
	dlg.exec();
}

void TranslationRequestDialog::applyTranslation() {
	Q_ASSERT(sender());
	const int requestID = sender()->property("requestID").toInt();
	const QString title = sender()->property("title").toString();
	ApplyTranslationDialog dlg(requestID, title, this);
	dlg.exec();
}

void TranslationRequestDialog::downloadCsv() {
	Q_ASSERT(sender());
	const int requestID = sender()->property("requestID").toInt();
	const QString title = sender()->property("title").toString();

	const auto * spinner = new WaitSpinner(QApplication::tr("Downloading"), this);

	connect(this, &TranslationRequestDialog::downloadedCsv, spinner, &WaitSpinner::deleteLater, Qt::QueuedConnection);

	QtConcurrent::run([this, requestID, title] {
		auto title2 = title;
		const auto csvString = TranslatorAPI::requestCsvDownload(requestID);
		title2 = title2.replace(QRegExp("[" + QRegExp::escape("\\/:*?\"<>|") + "]"), QString("_"));
		const auto path = QString("%1/%2.csv").arg(qApp->applicationDirPath()).arg(title2);
		QFile f(path);
		if (f.open(QIODevice::WriteOnly)) {
			QTextStream ts(&f);
			ts << csvString;
		}

		emit downloadedCsv();
	});
}

void TranslationRequestDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/TranslationRequestDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/TranslationRequestDialogGeometry");
	}
}

void TranslationRequestDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/TranslationRequestDialogGeometry", saveGeometry());
}

void TranslationRequestDialog::requestList() {
	const auto * spinner = new WaitSpinner(QApplication::tr("Updating"), this);
	connect(this, &TranslationRequestDialog::receivedRequestList, spinner, &WaitSpinner::deleteLater);

	QtConcurrent::run([this] {
		SendOwnProjectsMessage * sopm = TranslatorAPI::requestOwnProjects(q2s(Config::Username));
		if (sopm) {
			emit receivedRequestList(sopm->projects);
		}
		delete sopm;
	});
}
