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

#include "widgets/TranslationRequestDialog.h"

#include "Conversion.h"

#include "translator/GothicParser.h"

#include "widgets/AccessRightsDialog.h"
#include "widgets/ApplyTranslationDialog.h"
#include "widgets/WaitSpinner.h"

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

#include "translator/api/TranslatorAPI.h"
#include "translator/common/TranslationModel.h"

namespace spine {
namespace widgets {

	TranslationRequestDialog::TranslationRequestDialog(QSettings * iniParser, QString username, QWidget * par) : QDialog(par), _iniParser(iniParser), _username(username), _model(nullptr), _progressDialog(nullptr), _widgets() {
		QVBoxLayout * l = new QVBoxLayout();
		l->setAlignment(Qt::AlignTop);

		{
			QHBoxLayout * hl = new QHBoxLayout();

			QLabel * pathLabel = new QLabel(QApplication::tr("ParsePath"), this);
			_pathEdit = new QLineEdit(this);
			_pathEdit->setReadOnly(true);
			_pathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
			QPushButton * pathPushButton = new QPushButton("...", this);
			hl->addWidget(pathLabel);
			hl->addWidget(_pathEdit);
			hl->addWidget(pathPushButton);
			connect(pathPushButton, &QPushButton::released, this, &TranslationRequestDialog::openFileDialog);

			hl->setAlignment(Qt::AlignLeft);

			l->addLayout(hl);
		}

		{
			QHBoxLayout * hl = new QHBoxLayout();

			hl->setAlignment(Qt::AlignLeft);

			QStringList languages;
			languages << "Deutsch" << "English" << "Polish" << "Russian";

			QLabel * projectNameLabel = new QLabel(QApplication::tr("ProjectName"), this);
			_projectNameEdit = new QLineEdit(this);

			QLabel * sourceLanguageLabel = new QLabel(QApplication::tr("SourceLanguage"), this);
			_sourceLanguageBox = new QComboBox(this);
			_sourceLanguageBox->addItems(languages);

			QLabel * destinationLanguageLabel = new QLabel(QApplication::tr("DestinationLanguage"), this);
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
			QHBoxLayout * hl = new QHBoxLayout();

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
			QGridLayout * gl = new QGridLayout();

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

		_requestList = new QVBoxLayout();
		_requestList->setAlignment(Qt::AlignTop);

		l->addLayout(_requestList);

		setLayout(l);

		setWindowTitle(QApplication::tr("RequestTranslation"));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		restoreSettings();

		qRegisterMetaType<std::vector<translator::common::SendOwnProjectsMessage::Project>>("std::vector<translator::common::SendOwnProjectsMessage::Project>");
		connect(this, &TranslationRequestDialog::receivedRequestList, this, &TranslationRequestDialog::updateRequestList);

		requestList();
	}

	TranslationRequestDialog::~TranslationRequestDialog() {
		delete _model;
		saveSettings();
	}

	void TranslationRequestDialog::openFileDialog() {
		QString path = QFileDialog::getExistingDirectory(this, QApplication::tr("SelectGothicDir"), _pathEdit->text());
		if (!path.isEmpty()) {
			if (_pathEdit->text() != path) {
				_pathEdit->setText(path);
			}
		}
		checkParsePossible();
	}

	void TranslationRequestDialog::parseScripts() {
		delete _model;

		translation::GothicParser gp(this);

		_model = translator::api::TranslatorAPI::createModel(q2s(_projectNameEdit->text()), q2s(_sourceLanguageBox->currentText()), q2s(_destinationLanguageBox->currentText()));
		_progressDialog = new QProgressDialog(QApplication::tr("ParseScripts"), "", 0, 100, this);
		_progressDialog->setCancelButton(nullptr);
		_progressDialog->setWindowFlags(_progressDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
		connect(&gp, &translation::GothicParser::updatedProgress, this, &TranslationRequestDialog::updateProgress);
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
		WaitSpinner ws(QApplication::tr("RequestingTranslationSpinner"), this);
		QEventLoop loop;
		QFutureWatcher<void> watcher;
		connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
		const QFuture<void> future = QtConcurrent::run([this]() {
			translator::api::TranslatorAPI::requestTranslation(q2s(_username), _model);
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
		_parseButton->setEnabled(!_username.isEmpty() && !_pathEdit->text().isEmpty() && !_projectNameEdit->text().isEmpty() && _sourceLanguageBox->currentIndex() != _destinationLanguageBox->currentIndex());
	}

	void TranslationRequestDialog::updateRequestList(std::vector<translator::common::SendOwnProjectsMessage::Project> projects) {
		for (QWidget * w : _widgets) {
			w->deleteLater();
		}
		_widgets.clear();

		for (auto project : projects) {
			QHBoxLayout * hl = new QHBoxLayout();
			QLabel * l = new QLabel(s2q(project.projectName) + " (" + s2q(project.sourceLanguage) + " => " + s2q(project.destinationLanguage) + ")", this);
			QProgressBar * pb = new QProgressBar(this);
			pb->setMinimum(0);
			pb->setMaximum(project.toTranslate);
			pb->setValue(project.translated);
			pb->setFormat("%v / %m (%p %)");
			pb->setAlignment(Qt::AlignCenter);

			QPushButton * accessButton = new QPushButton(QApplication::tr("UserManagement"), this);
			accessButton->setProperty("requestID", int(project.requestID));
			accessButton->setProperty("title", l->text());
			connect(accessButton, &QPushButton::released, this, &TranslationRequestDialog::openAccessDialog);

			QPushButton * applyTranslationButton = new QPushButton(QApplication::tr("ApplyTranslation"), this);
			applyTranslationButton->setProperty("requestID", int(project.requestID));
			applyTranslationButton->setProperty("title", l->text());
			connect(applyTranslationButton, &QPushButton::released, this, &TranslationRequestDialog::applyTranslation);

			hl->addWidget(l, 1);
			hl->addWidget(pb, 1);
			hl->addWidget(accessButton, 0);
			hl->addWidget(applyTranslationButton, 0);

			_requestList->addLayout(hl);

			_widgets.append(l);
			_widgets.append(pb);
			_widgets.append(accessButton);
			_widgets.append(applyTranslationButton);
		}
	}

	void TranslationRequestDialog::openAccessDialog() {
		Q_ASSERT(sender());
		const int requestID = sender()->property("requestID").toInt();
		const QString title = sender()->property("title").toString();
		AccessRightsDialog dlg(requestID, title, _iniParser, this);
		dlg.exec();
	}

	void TranslationRequestDialog::applyTranslation() {
		Q_ASSERT(sender());
		const int requestID = sender()->property("requestID").toInt();
		const QString title = sender()->property("title").toString();
		ApplyTranslationDialog dlg(requestID, title, _iniParser, this);
		dlg.exec();
	}

	void TranslationRequestDialog::restoreSettings() {
		const QByteArray arr = _iniParser->value("WINDOWGEOMETRY/TranslationRequestDialogGeometry", QByteArray()).toByteArray();
		if (!restoreGeometry(arr)) {
			_iniParser->remove("WINDOWGEOMETRY/TranslationRequestDialogGeometry");
		}
	}

	void TranslationRequestDialog::saveSettings() {
		_iniParser->setValue("WINDOWGEOMETRY/TranslationRequestDialogGeometry", saveGeometry());
	}

	void TranslationRequestDialog::requestList() {
		QtConcurrent::run([this]() {
			translator::common::SendOwnProjectsMessage * sopm = translator::api::TranslatorAPI::requestOwnProjects(q2s(_username));
			if (sopm) {
				emit receivedRequestList(sopm->projects);
			}
			delete sopm;
		});
	}

} /* namespace widgets */
} /* namespace spine */
