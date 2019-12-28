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

#include "widgets/TranslationWidget.h"

#include <QHBoxLayout>
#include <QLineEdit>

namespace spine {
namespace widgets {

	TranslationWidget::TranslationWidget(QString sourceText, QString targetText, QWidget * par) : QWidget(par), _sourceEdit(nullptr), _translationEdit(nullptr) {
		QHBoxLayout * l = new QHBoxLayout();
		
		_sourceEdit = new QLineEdit(sourceText, this);
		_sourceEdit->setReadOnly(true);
		_translationEdit = new QLineEdit(targetText, this);

		l->addWidget(_sourceEdit);
		l->addWidget(_translationEdit);

		setLayout(l);

		connect(_translationEdit, &QLineEdit::cursorPositionChanged, this, &TranslationWidget::focusChanged);
		connect(_translationEdit, &QLineEdit::textChanged, this, &TranslationWidget::focusChanged);
	}

	TranslationWidget::~TranslationWidget() {
	}

	QString TranslationWidget::getTranslation() const {
		return _translationEdit->text();
	}

	void TranslationWidget::setFocusToTranslation() {
		_translationEdit->setFocus();
		focusChanged();
	}

	void TranslationWidget::focusChanged() {
		emit selectedSource(_sourceEdit->text());
		emit selectedDestination(_translationEdit->text());
	}

} /* namespace widgets */
} /* namespace spine */
