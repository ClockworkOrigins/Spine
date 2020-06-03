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
// Copyright 2020 Clockwork Origins

#include "gui/Spoiler.h"

#include "utils/Config.h"
#include "utils/Database.h"

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QToolButton>

using namespace spine::gui;

Spoiler::Spoiler(const QString & title, QWidget * parent) : QWidget(parent), _animationDuration(300) {
	_toggleButton = new QToolButton(this);
	
    _toggleButton->setStyleSheet("QToolButton { border: none; }");
    _toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _toggleButton->setArrowType(Qt::ArrowType::RightArrow);
    _toggleButton->setText(title);
    _toggleButton->setCheckable(true);
    _toggleButton->setChecked(false);

	_headerLine = new QFrame(this);
    _headerLine->setFrameShape(QFrame::HLine);
    _headerLine->setFrameShadow(QFrame::Sunken);
    _headerLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	_contentArea = new QScrollArea(this);
    _contentArea->setStyleSheet("QScrollArea { background-color: white; border: none; }");
    _contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // start out collapsed
    _contentArea->setMaximumHeight(0);
    _contentArea->setMinimumHeight(0);
	
    // let the entire widget grow and shrink with its content
	_toggleAnimation = new QParallelAnimationGroup(this);
    _toggleAnimation->addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    _toggleAnimation->addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    _toggleAnimation->addAnimation(new QPropertyAnimation(_contentArea, "maximumHeight"));
	
    // don't waste space
	_mainLayout = new QGridLayout();
    _mainLayout->setVerticalSpacing(0);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    int row = 0;
    _mainLayout->addWidget(_toggleButton, row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout->addWidget(_headerLine, row++, 2, 1, 1);
    _mainLayout->addWidget(_contentArea, row, 0, 1, 3);
	
    setLayout(_mainLayout);
	
    QObject::connect(_toggleButton, &QToolButton::clicked, [this](const bool checked) {
        _toggleButton->setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
        _toggleAnimation->setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        _toggleAnimation->start();
    });
}

void Spoiler::setAnimationDuration(int animationDuration) {
	_animationDuration = animationDuration;
}

void Spoiler::setContentLayout(QLayout * contentLayout) {
    delete _contentArea->layout();
    _contentArea->setLayout(contentLayout);
	
    const auto collapsedHeight = sizeHint().height() - _contentArea->maximumHeight();
	const auto contentHeight = contentLayout->sizeHint().height();
    for (int i = 0; i < _toggleAnimation->animationCount() - 1; ++i) {
        QPropertyAnimation * spoilerAnimation = dynamic_cast<QPropertyAnimation *>(_toggleAnimation->animationAt(i));
        spoilerAnimation->setDuration(_animationDuration);
        spoilerAnimation->setStartValue(collapsedHeight);
        spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
    }
    QPropertyAnimation * contentAnimation = dynamic_cast<QPropertyAnimation *>(_toggleAnimation->animationAt(_toggleAnimation->animationCount() - 1));
    contentAnimation->setDuration(_animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}
