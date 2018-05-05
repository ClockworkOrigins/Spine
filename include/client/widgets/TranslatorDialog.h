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

#ifndef __SPINE_WIDGETS_TRANSLATORDIALOG_H__
#define __SPINE_WIDGETS_TRANSLATORDIALOG_H__

#include <QDialog>

#include "translator/common/MessageStructs.h"

class QComboBox;
class QProgressBar;
class QSettings;
class QTextBrowser;
class QVBoxLayout;

namespace spine {
namespace widgets {

	class TranslationWidget;
	class WaitSpinner;

	class TranslatorDialog : public QDialog {
		Q_OBJECT

	public:
		TranslatorDialog(QSettings * iniParser, QString username, QWidget * par);
		~TranslatorDialog();

	signals:
		void receivedProjects(std::vector<translator::common::SendProjectsMessage::Project>);
		void receivedTextToTranslate();
		void receivedTextToReview();
		void receivedProgress(std::pair<uint32_t, uint32_t>);

	private slots:
		void submit();
		void discard();
		void updateProjects(std::vector<translator::common::SendProjectsMessage::Project> projects);
		void requestText();
		void requestReview();
		void updateTextToTranslate();
		void updateTextToReview();
		void changedProject();
		void updateProgress(std::pair<uint32_t, uint32_t> progress);

	private:
		QSettings * _iniParser;
		QString _username;
		QComboBox * _projectsComboBox;
		QProgressBar * _progressBar;
		std::vector<::translator::common::SendProjectsMessage::Project> _projects;
		QPushButton * _requestTextButton;
		QPushButton * _requestReviewButton;
		::translator::common::SendProjectsMessage::Project _activeProject;
		::translator::common::SendTextToTranslateMessage * _textToTranslateMsg;
		::translator::common::SendTextToReviewMessage * _textToReviewMsg;
		QTextBrowser * _sourcePreview;
		QTextBrowser * _destinationPreview;
		QTextBrowser * _hintPreview;
		QVBoxLayout * _translationsLayout;
		QList<TranslationWidget *> _translationWidgets;
		QComboBox * _hintsBox;
		WaitSpinner * _waitSpinner;

		void closeEvent(QCloseEvent * evt) override;
		void restoreSettings();
		void saveSettings();
		void queryAllProjects();
		void requestProjectProgress();
		void reset();
		bool eventFilter(QObject * o, QEvent * e) override;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_TRANSLATORDIALOG_H__ */
