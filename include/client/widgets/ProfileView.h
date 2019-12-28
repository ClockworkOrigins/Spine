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

#ifndef __SPINE_WIDGETS_PROFILEVIEW_H__
#define __SPINE_WIDGETS_PROFILEVIEW_H__

#include "common/MessageStructs.h"

#include <QWidget>

class QCheckBox;
class QLabel;
class QMainWindow;
class QPushButton;
class QScrollArea;
class QTabWidget;
class QVBoxLayout;

namespace spine {
namespace widgets {

	class AchievementView;
	class GeneralSettingsWidget;
	class HiddenAchievementsView;
	class ProfileModView;
	class WaitSpinner;

	class ProfileView : public QWidget {
		Q_OBJECT

	public:
		ProfileView(QMainWindow * mainWindow, GeneralSettingsWidget * generalSettingsWidget, QWidget * par);
		~ProfileView();

		void setGothicDirectory(QString path);
		void setGothic2Directory(QString path);

	signals:
		void receivedMods(std::vector<common::ModStats>);
		void receivedUserLevel(uint32_t, uint32_t, uint32_t);
		void receivedAchievementStats(int32_t, std::vector<common::SendAllAchievementStatsMessage::AchievementStats>);
		void receivedScoreStats(std::vector<common::SendAllScoreStatsMessage::ScoreStats>);
		void triggerLogout();

	public slots:
		void updateList();
		void reset();
		void loginChanged();
		void openAchievementView(int32_t modID, QString modName);
		void openScoreView(int32_t modID, QString modName);

	private slots:
		void updateModList(std::vector<common::ModStats> mods);
		void updateUserLevel(uint32_t level, uint32_t currentXP, uint32_t nextXP);
		void updateAchievements(int32_t modID, std::vector<common::SendAllAchievementStatsMessage::AchievementStats> achievementStats);
		void updateScores(std::vector<common::SendAllScoreStatsMessage::ScoreStats> scoreStats);
		void toggledHidePatchesAndTools();

	private:
		QLabel * _nameLabel;
		QLabel * _timeLabel;
		QPushButton * _backToProfileButton;
		QLabel * _modnameLabel;
		QLabel * _achievmentDescriptionLabel;
		QScrollArea * _scrollArea;
		QWidget * _mainWidget;
		QVBoxLayout * _scrollLayout;
		QScrollArea * _achievementScrollArea;
		QWidget * _achievementsWidget;
		QVBoxLayout * _achievementsLayout;
		QTabWidget * _scoresWidget;
		QList<ProfileModView *> _mods;
		QList<AchievementView *> _achievements;
		QString _gothicDirectory;
		QString _gothic2Directory;
		bool _specialPage;
		QPushButton * _logoutButton;
		QCheckBox* _hidePatchesAndToolsBox;
		QMainWindow * _mainWindow;
		WaitSpinner * _waitSpinner;
		HiddenAchievementsView * _hiddenAchievementsView;
	};

} /* namespace widgets */
} /* namespace spine */

#endif /* __SPINE_WIDGETS_PROFILEVIEW_H__ */
