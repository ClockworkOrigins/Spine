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

#ifndef __SPINE_WIDGETS_NEWSWIDGET_H__
#define __SPINE_WIDGETS_NEWSWIDGET_H__

#include "common/MessageStructs.h"

#include <QWidget>

class QLabel;
class QPushButton;
class QTextBrowser;

namespace spine {
namespace widgets {

	class NewsWriterDialog;

	class NewsWidget : public QWidget {
		Q_OBJECT

	public:
		NewsWidget(common::SendAllNewsMessage::News news, bool onlineMode, QWidget * par);

	signals:
		void tryInstallMod(int, int);

	public slots:
		void finishedInstallation(int modID, int packageID, bool success);

	private slots:
		void urlClicked(const QUrl & link);
		void installMod();

	private:
		friend class NewsWriterDialog;

		QLabel * _titleLabel;
		QTextBrowser * _textBrowser;
		QLabel * _timestampLabel;
		int32_t _newsID;

		QList<QPushButton *> _installButtons;

		void paintEvent(QPaintEvent* evt) override;
		void update(common::SendAllNewsMessage::News news);
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_NEWSWIDGET_H__ */
