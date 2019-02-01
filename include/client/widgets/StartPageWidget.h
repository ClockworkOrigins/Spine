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

#ifndef __SPINE_WIDGETS_STARTPAGEWIDGET_H__
#define __SPINE_WIDGETS_STARTPAGEWIDGET_H__

#include "common/MessageStructs.h"

#include <QWidget>

class QMainWindow;
class QPushButton;
class QScrollArea;
class QStandardItemModel;
class QTableView;
class QVBoxLayout;

namespace spine {
namespace widgets {

	class GeneralSettingsWidget;
	class NewsWidget;

	class StartPageWidget : public QWidget {
		Q_OBJECT

	public:
		StartPageWidget(bool onlineMode, QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par);

		void requestNewsUpdate();

	signals:
		void receivedNews();
		void tryInstallMod(int, int);
		void finishedInstallation(int, int, bool);
		void triggerModStart(int, QString);

	public slots:
		void setLanguage(QString language);
		void setUsername(QString username, QString password);

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
		QString _language;
		std::vector<NewsWidget *> _news;
		QMainWindow * _mainWindow;
		QPushButton * _writeNewsButton;
		QTableView * _newsTicker;
		QStandardItemModel * _newsTickerModel;
		GeneralSettingsWidget * _generalSettingsWidget;
		QString _username;
		QString _password;
		bool _onlineMode;
		QPushButton * _startModButton;

		void showEvent(QShowEvent* evt) override;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_STARTPAGEWIDGET_H__ */
