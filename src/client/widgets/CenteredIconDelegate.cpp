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

#include "widgets/CenteredIconDelegate.h"

#include "DatabaseFilterModel.h"

#include <QApplication>
#include <QPainter>

using namespace spine;
using namespace spine::widgets;

CenteredIconDelegate::CenteredIconDelegate(QWidget * par) : QStyledItemDelegate(par) {}

void CenteredIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.column() == DatabaseColumn::Languages) {
		Q_ASSERT(index.isValid());

	    QStyleOptionViewItem opt = option;
	    initStyleOption(&opt, index);
	    // disable default icon
	    opt.icon = QIcon();
	    // draw default item
	    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, 0);

	    const QRect r = option.rect;

	    // get pixmap
	    const QPixmap pix = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));

	    // draw pixmap at center of item
	    const QPoint p = QPoint((r.width() - pix.width())/2, (r.height() - pix.height())/2);
	    painter->drawPixmap(r.topLeft() + p, pix);
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

QSize CenteredIconDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.column() == DatabaseColumn::Languages) {
		const QPixmap pix = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
		return pix.size();
	}
	
	return QStyledItemDelegate::sizeHint(option, index);
}

void CenteredIconDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QStyledItemDelegate::setModelData(editor, model, index);
}
