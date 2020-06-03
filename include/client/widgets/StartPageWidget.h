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

#include <QWidget>

class QPushButton;
class QScrollArea;
class QStandardItemModel;
class QTableView;
class QVBoxLayout;

namespace spine {
namespace client {
	enum class InstallMode;
}
namespace widgets {

	class NewsWidget;

	class StartPageWidget : public QWidget {
		Q_OBJECT

	public:
		StartPageWidget(QWidget * par);

		void requestNewsUpdate();

	signals:
		void receivedNews();
		void tryInstallMod(int, int, client::InstallMode);
		void finishedInstallation(int, int, bool);
		void triggerModStart(int, QString);
		void showInfoPage(int);

	public slots:
		void loginChanged();
		void setLanguage(QString);

	private slots:
		void updateNews();
		void selectedNews(const QModelIndex & index);
		void executeNewsWriter();
		void refresh();
		void startMod();

	private:
		QScrollArea * _scrollArea;
		QWidget * _newsContainer;
		QVBoxLayout * _newsLayout;
		QList<NewsWidget *> _news;
		QPushButton * _writeNewsButton;
		QPushButton * _startModButton;
		QVBoxLayout * _newsTickerLayout;
		QList<QWidget *> _newsTickerWidgets;
		QList<common::SendAllNewsMessage::NewsTicker> _newsTickers;

		void showEvent(QShowEvent* evt) override;
	};

} /* namespace widgets */
} /* namespace spine */
