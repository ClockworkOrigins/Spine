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

#pragma once

#include "common/MessageStructs.h"

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QVBoxLayout;

namespace spine {
namespace translation {

	class TranslationRequestDialog : public QDialog {
		Q_OBJECT

	public:
		TranslationRequestDialog(QWidget * par);
		~TranslationRequestDialog();

	signals:
		void receivedRequestList(std::vector<spine::common::SendOwnProjectsMessage::Project>);
		void downloadedCsv();

	private slots:
		void openFileDialog();
		void parseScripts();
		void requestTranslation();
		void updateProgress(int current, int max);
		void checkParsePossible();
		void updateRequestList(std::vector<spine::common::SendOwnProjectsMessage::Project> projects);
		void openAccessDialog();
		void applyTranslation();
		void downloadCsv();

	private:
		QLineEdit * _pathEdit;
		QLineEdit * _projectNameEdit;
		QComboBox * _sourceLanguageBox;
		QComboBox * _destinationLanguageBox;
		QPushButton * _parseButton;
		QPushButton * _requestButton;
		common::TranslationModel * _model;
		QProgressDialog * _progressDialog;
		QLabel * _nameLabel;
		QLabel * _nameCountLabel;
		QLabel * _textLabel;
		QLabel * _textCountLabel;
		QLabel * _dialogLabel;
		QLabel * _dialogCountLabel;
		QLabel * _overallLabel;
		QLabel * _overallCountLabel;

		QVBoxLayout * _requestList;
		QList<QWidget *> _widgets;

		void restoreSettings();
		void saveSettings();
		void requestList();
	};

} /* namespace translation */
} /* namespace spine */
