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

#include "translator/TranslatorDialog.h"

#include "gui/WaitSpinner.h"

#include "utils/Config.h"
#include "utils/Conversion.h"

#include "translator/TranslationWidget.h"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QtConcurrentRun>
#include <QDialogButtonBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "translator/api/TranslatorAPI.h"

using namespace spine::translator;
using namespace spine::utils;

TranslatorDialog::TranslatorDialog(QWidget * par) : QDialog(par), _projectsComboBox(nullptr), _progressBar(nullptr), _projects(), _requestTextButton(nullptr), _requestReviewButton(nullptr), _activeProject(), _textToTranslateMsg(nullptr), _textToReviewMsg(nullptr), _sourcePreview(nullptr), _destinationPreview(nullptr), _hintPreview(nullptr), _translationsLayout(nullptr), _translationWidgets(), _hintsBox(nullptr), _waitSpinner(nullptr) {
	QVBoxLayout * l = new QVBoxLayout();
	l->setAlignment(Qt::AlignTop);

	{
		QHBoxLayout * hl = new QHBoxLayout();
		_projectsComboBox = new QComboBox(this);
		_progressBar = new QProgressBar(this);
		_progressBar->hide();
		_progressBar->setFormat("%v / %m (%p %)");
		_progressBar->setAlignment(Qt::AlignCenter);
		hl->addWidget(_projectsComboBox, 1);
		hl->addWidget(_progressBar, 1);

		l->addLayout(hl);
	}
	{
		QHBoxLayout * hl = new QHBoxLayout();
		_requestTextButton = new QPushButton(QApplication::tr("RequestTextToTranslate"), this);
		_requestTextButton->setEnabled(false);
		_requestTextButton->installEventFilter(this);
		connect(_requestTextButton, &QPushButton::released, this, &TranslatorDialog::requestText);
		_requestReviewButton = new QPushButton(QApplication::tr("RequestTextToReview"), this);
		_requestReviewButton->setEnabled(false);
		_requestReviewButton->installEventFilter(this);
		connect(_requestReviewButton, &QPushButton::released, this, &TranslatorDialog::requestReview);

		hl->addWidget(_requestTextButton);
		hl->addWidget(_requestReviewButton);

		l->addLayout(hl);
	}
	{
		QGridLayout * gl = new QGridLayout();

		QHBoxLayout * hl = new QHBoxLayout();
		_sourcePreview = new QTextBrowser(this);
		_destinationPreview = new QTextBrowser(this);

		hl->addWidget(_sourcePreview);
		hl->addWidget(_destinationPreview);

		_hintPreview = new QTextBrowser(this);

		gl->addLayout(hl, 0, 0);
		gl->addWidget(_hintPreview, 0, 1);

		QScrollArea * scrollArea = new QScrollArea(this);
		QWidget * mainWidget = new QWidget(this);
		scrollArea->setWidgetResizable(true);
		_translationsLayout = new QVBoxLayout();
		_translationsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		mainWidget->setLayout(_translationsLayout);
		mainWidget->setProperty("default", true);
		scrollArea->setWidget(mainWidget);
		_hintsBox = new QComboBox(this);

		gl->addWidget(scrollArea, 1, 0);
		gl->addWidget(_hintsBox, 1, 1);

		l->addLayout(gl);
	}

	QDialogButtonBox * dbb = new QDialogButtonBox(QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::StandardButton::Discard, Qt::Orientation::Horizontal, this);
	l->addWidget(dbb);

	setLayout(l);

	QPushButton * b = dbb->button(QDialogButtonBox::StandardButton::Apply);
	b->setText(QApplication::tr("Submit"));
	b->setShortcut(QKeySequence(Qt::Key::Key_Return));

	connect(b, &QPushButton::released, this, &TranslatorDialog::submit);

	b = dbb->button(QDialogButtonBox::StandardButton::Discard);
	b->setText(QApplication::tr("Discard"));
	b->installEventFilter(this);

	connect(b, &QPushButton::released, this, &TranslatorDialog::discard);

	setWindowTitle(QApplication::tr("Translator"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	restoreSettings();

	qRegisterMetaType<std::vector<::translator::common::SendProjectsMessage::Project>>("std::vector<translator::common::SendProjectsMessage::Project>");
	qRegisterMetaType<std::pair<uint32_t, uint32_t>>("std::pair<uint32_t, uint32_t>");
	connect(this, &TranslatorDialog::receivedProjects, this, &TranslatorDialog::updateProjects);
	connect(this, &TranslatorDialog::receivedTextToTranslate, this, &TranslatorDialog::updateTextToTranslate);
	connect(this, &TranslatorDialog::receivedTextToReview, this, &TranslatorDialog::updateTextToReview);
	connect(_projectsComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &TranslatorDialog::changedProject);
	connect(this, &TranslatorDialog::receivedProgress, this, &TranslatorDialog::updateProgress);
	connect(_hintsBox, &QComboBox::currentTextChanged, _hintPreview, &QTextBrowser::setText);

	queryAllProjects();
}

TranslatorDialog::~TranslatorDialog() {
	delete _textToTranslateMsg;
	delete _textToReviewMsg;
	saveSettings();
}

void TranslatorDialog::submit() {
	bool newRequestTranslate = false;
	if (_textToTranslateMsg) {
		const std::string name = _textToTranslateMsg->name.empty() ? "" : q2s(_translationWidgets[0]->getTranslation());
		const std::string text = _textToTranslateMsg->text.empty() ? "" : q2s(_translationWidgets[0]->getTranslation());
		std::vector<std::string> dialog;
		if (!_textToTranslateMsg->dialog.empty()) {
			for (TranslationWidget * tw : _translationWidgets) {
				dialog.push_back(q2s(tw->getTranslation()));
			}
		}
		::translator::api::TranslatorAPI::sendTranslationDraft(q2s(Config::Username), _activeProject.destinationLanguage, name, text, dialog, _textToTranslateMsg->id);
		newRequestTranslate = true;
	}
	bool newRequestReview = false;
	if (_textToReviewMsg) {
		bool changed = false;
		const std::string name = _textToReviewMsg->name.first.empty() ? "" : q2s(_translationWidgets[0]->getTranslation());
		changed |= name != _textToReviewMsg->name.second;
		const std::string text = _textToReviewMsg->text.first.empty() ? "" : q2s(_translationWidgets[0]->getTranslation());
		changed |= text != _textToReviewMsg->text.second;
		std::vector<std::string> dialog;
		if (!_textToReviewMsg->dialog.first.empty()) {
			for (TranslationWidget * tw : _translationWidgets) {
				dialog.push_back(q2s(tw->getTranslation()));
				changed |= dialog[dialog.size() - 1] != _textToReviewMsg->dialog.second[dialog.size() - 1];
			}
		}
		::translator::api::TranslatorAPI::sendTranslationReview(q2s(Config::Username), _activeProject.sourceLanguage, _activeProject.destinationLanguage, name, text, dialog, _textToReviewMsg->id, changed);
		newRequestReview = true;
		if (!changed) {
			_progressBar->setValue(_progressBar->value() + 1);
		}
	}
	reset();
	if (newRequestTranslate) {
		requestText();
	} else if (newRequestReview) {
		requestReview();
	}
}

void TranslatorDialog::discard() {
	reset();
}

void TranslatorDialog::updateProjects(std::vector<::translator::common::SendProjectsMessage::Project> projects) {
	delete _waitSpinner;
	_waitSpinner = nullptr;
	_projects = projects;
	QStringList options;
	for (auto p : projects) {
		options << QString("%1 (%2 => %3)").arg(s2q(p.projectName), s2q(p.sourceLanguage), s2q(p.destinationLanguage));
	}
	_projectsComboBox->clear();
	_projectsComboBox->addItems(options);
	_requestTextButton->setEnabled(!projects.empty());
	_requestReviewButton->setEnabled(!projects.empty());
	changedProject();
}

void TranslatorDialog::requestText() {
	if (!_activeProject.accessRights) {
		return;
	}
	delete _waitSpinner;
	_waitSpinner = new gui::WaitSpinner(QApplication::tr("Updating"), this);
	if (_textToTranslateMsg) {
		delete _textToTranslateMsg;
		_textToTranslateMsg = nullptr;
	}
	if (_textToReviewMsg) {
		delete _textToReviewMsg;
		_textToReviewMsg = nullptr;
	}
	QtConcurrent::run([this]() {
		_textToTranslateMsg = ::translator::api::TranslatorAPI::requestTextToTranslate(q2s(Config::Username), _activeProject.projectName, _activeProject.sourceLanguage, _activeProject.destinationLanguage);
		emit receivedTextToTranslate();
	});
}

void TranslatorDialog::requestReview() {
	if (!_activeProject.accessRights) {
		return;
	}
	delete _waitSpinner;
	_waitSpinner = new gui::WaitSpinner(QApplication::tr("Updating"), this);
	if (_textToTranslateMsg) {
		delete _textToTranslateMsg;
		_textToTranslateMsg = nullptr;
	}
	if (_textToReviewMsg) {
		delete _textToReviewMsg;
		_textToReviewMsg = nullptr;
	}
	QtConcurrent::run([this]() {
		_textToReviewMsg = ::translator::api::TranslatorAPI::requestTextToReview(q2s(Config::Username), _activeProject.projectName, _activeProject.sourceLanguage, _activeProject.destinationLanguage);
		emit receivedTextToReview();
	});

}

void TranslatorDialog::updateTextToTranslate() {
	delete _waitSpinner;
	_waitSpinner = nullptr;
	if (!_textToTranslateMsg) {
		return;
	}
	for (TranslationWidget * tw : _translationWidgets) {
		tw->deleteLater();
	}
	_translationWidgets.clear();
	if (!_textToTranslateMsg->name.empty()) {
		TranslationWidget * tw = new TranslationWidget(s2q(_textToTranslateMsg->name).trimmed(), "", this);
		_translationsLayout->addWidget(tw);
		_translationWidgets.append(tw);
		connect(tw, &TranslationWidget::selectedSource, _sourcePreview, &QTextBrowser::setText);
		connect(tw, &TranslationWidget::selectedDestination, _destinationPreview, &QTextBrowser::setText);
	}
	if (!_textToTranslateMsg->text.empty()) {
		TranslationWidget * tw = new TranslationWidget(s2q(_textToTranslateMsg->text).trimmed(), "", this);
		_translationsLayout->addWidget(tw);
		_translationWidgets.append(tw);
		connect(tw, &TranslationWidget::selectedSource, _sourcePreview, &QTextBrowser::setText);
		connect(tw, &TranslationWidget::selectedDestination, _destinationPreview, &QTextBrowser::setText);
	}
	if (!_textToTranslateMsg->dialog.empty()) {
		for (const std::string & s : _textToTranslateMsg->dialog) {
			TranslationWidget * tw = new TranslationWidget(s2q(s).trimmed(), "", this);
			_translationsLayout->addWidget(tw);
			_translationWidgets.append(tw);
			connect(tw, &TranslationWidget::selectedSource, _sourcePreview, &QTextBrowser::setText);
			connect(tw, &TranslationWidget::selectedDestination, _destinationPreview, &QTextBrowser::setText);
		}
	}
	if (!_translationWidgets.empty()) {
		_translationWidgets.front()->setFocusToTranslation();
	}
	QStringList options;
	for (const std::pair<std::string, std::string> s : _textToTranslateMsg->hints) {
		options << QString("\"%1\" => \"%2\"").arg(s2q(s.first).trimmed(), s2q(s.second).trimmed());
	}
	_hintsBox->clear();
	_hintsBox->addItems(options);
}

void TranslatorDialog::updateTextToReview() {
	delete _waitSpinner;
	_waitSpinner = nullptr;
	if (!_textToReviewMsg) {
		return;
	}
	for (TranslationWidget * tw : _translationWidgets) {
		tw->deleteLater();
	}
	_translationWidgets.clear();
	if (!_textToReviewMsg->name.first.empty()) {
		TranslationWidget * tw = new TranslationWidget(s2q(_textToReviewMsg->name.first).trimmed(), s2q(_textToReviewMsg->name.second).trimmed(), this);
		_translationsLayout->addWidget(tw);
		_translationWidgets.append(tw);
		connect(tw, &TranslationWidget::selectedSource, _sourcePreview, &QTextBrowser::setText);
		connect(tw, &TranslationWidget::selectedDestination, _destinationPreview, &QTextBrowser::setText);
	}
	if (!_textToReviewMsg->text.first.empty()) {
		TranslationWidget * tw = new TranslationWidget(s2q(_textToReviewMsg->text.first).trimmed(), s2q(_textToReviewMsg->text.second).trimmed(), this);
		_translationsLayout->addWidget(tw);
		_translationWidgets.append(tw);
		connect(tw, &TranslationWidget::selectedSource, _sourcePreview, &QTextBrowser::setText);
		connect(tw, &TranslationWidget::selectedDestination, _destinationPreview, &QTextBrowser::setText);
	}
	if (!_textToReviewMsg->dialog.first.empty()) {
		for (size_t i = 0; i < _textToReviewMsg->dialog.first.size(); i++) {
			TranslationWidget * tw = new TranslationWidget(s2q(_textToReviewMsg->dialog.first[i]).trimmed(), s2q(_textToReviewMsg->dialog.second[i]).trimmed(), this);
			_translationsLayout->addWidget(tw);
			_translationWidgets.append(tw);
			connect(tw, &TranslationWidget::selectedSource, _sourcePreview, &QTextBrowser::setText);
			connect(tw, &TranslationWidget::selectedDestination, _destinationPreview, &QTextBrowser::setText);
		}
	}
	if (!_translationWidgets.empty()) {
		_translationWidgets.front()->setFocusToTranslation();
	}
	QStringList options;
	for (const std::pair<std::string, std::string> s : _textToReviewMsg->hints) {
		options << QString("\"%1\" => \"%2\"").arg(s2q(s.first).trimmed(), s2q(s.second).trimmed());
	}
	_hintsBox->clear();
	_hintsBox->addItems(options);
}

void TranslatorDialog::changedProject() {
	reset();
	if (_projects.empty()) {
		_progressBar->hide();
		return;
	}
	const int index = _projectsComboBox->currentIndex();
	_activeProject = _projects[index];

	_requestReviewButton->setEnabled(_activeProject.accessRights);
	_requestTextButton->setEnabled(_activeProject.accessRights);

	requestProjectProgress();
}

void TranslatorDialog::updateProgress(std::pair<uint32_t, uint32_t> progress) {
	delete _waitSpinner;
	_waitSpinner = nullptr;
	_progressBar->show();
	_progressBar->setMinimum(0);
	_progressBar->setMaximum(progress.second);
	_progressBar->setValue(progress.first);
}

void TranslatorDialog::closeEvent(QCloseEvent * evt) {
	QDialog::closeEvent(evt);
	evt->accept();
	reject();
}

void TranslatorDialog::restoreSettings() {
	const QByteArray arr = Config::IniParser->value("WINDOWGEOMETRY/TranslatorDialogGeometry", QByteArray()).toByteArray();
	if (!restoreGeometry(arr)) {
		Config::IniParser->remove("WINDOWGEOMETRY/TranslatorDialogGeometry");
	}
}

void TranslatorDialog::saveSettings() {
	Config::IniParser->setValue("WINDOWGEOMETRY/TranslatorDialogGeometry", saveGeometry());
}

void TranslatorDialog::queryAllProjects() {
	if (Config::Username.isEmpty()) {
		return;
	}
	delete _waitSpinner;
	_waitSpinner = new gui::WaitSpinner(QApplication::tr("Updating"), this);
	QtConcurrent::run([this]() {
		::translator::common::SendProjectsMessage * msg = ::translator::api::TranslatorAPI::getProjects(q2s(Config::Username));
		if (msg) {
			emit receivedProjects(msg->projects);
			delete msg;
		} else {
			emit receivedProjects(std::vector<::translator::common::SendProjectsMessage::Project>());
		}
	});
}

void TranslatorDialog::requestProjectProgress() {
	delete _waitSpinner;
	_waitSpinner = new gui::WaitSpinner(QApplication::tr("Updating"), this);
	QtConcurrent::run([this]() {
		const auto progress = ::translator::api::TranslatorAPI::requestTranslationProgress(_activeProject.projectName, _activeProject.sourceLanguage, _activeProject.destinationLanguage);
		emit receivedProgress(progress);
	});
}

void TranslatorDialog::reset() {
	delete _textToTranslateMsg;
	_textToTranslateMsg = nullptr;
	delete _textToReviewMsg;
	_textToReviewMsg = nullptr;

	for (TranslationWidget * tw : _translationWidgets) {
		tw->deleteLater();
	}
	_translationWidgets.clear();

	_hintsBox->clear();

	_sourcePreview->setText("");
	_destinationPreview->setText("");
	_hintPreview->setText("");
}

bool TranslatorDialog::eventFilter(QObject * o, QEvent * evt) {
	if (evt->type() == QEvent::KeyPress) {
		QKeyEvent * keyEvent = dynamic_cast<QKeyEvent *>(evt);
		if (keyEvent->key() == Qt::Key_Return) {
			evt->ignore();
			return false;
		}
	}
	return QWidget::eventFilter(o, evt);
}
