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

#ifndef __SPINE_MODELS_SPINEEDITORMODEL_H__
#define __SPINE_MODELS_SPINEEDITORMODEL_H__

#include <cstdint>

#include "common/SpineModules.h"

#include <QObject>

namespace spine {
namespace common {
	enum class GothicVersion;
} /* namespace common */
namespace models {

	enum AchievementOrientation {
		TopLeft = 0,
		TopRight,
		BottomLeft,
		BottomRight
	};

	enum LeGoModules {
		PrintS			= 1 << 0,
		HookEngine		= 1 << 1,
		AI_Function		= 1 << 2,
		Trialoge		= 1 << 3,
		Dialoggestures	= 1 << 4,
		FrameFunctions	= 1 << 5,
		Cursor			= 1 << 6,
		Focusnames		= 1 << 7,
		Random			= 1 << 8,
		Bloodsplats		= 1 << 9,
		Saves			= 1 << 10,
		PermMem			= 1 << 11,
		Anim8			= 1 << 12,
		View			= 1 << 13,
		Interface		= 1 << 14,
		Bars			= 1 << 15,
		Buttons			= 1 << 16,
		Timer			= 1 << 17,
		EventHandler	= 1 << 18,
		Gamestate		= 1 << 19,
		Sprite			= 1 << 20,
		Names			= 1 << 21,
		ConsoleCommands	= 1 << 22,
		Buffs			= 1 << 23,
		Render			= 1 << 24,

		All				= (1 << 23) - 1
	};

	struct AchievementModel {
		QString name;
		QString description;
		QString lockedImage;
		QString unlockedImage;
		bool hidden;
		int maxProgress;

		AchievementModel() : name(), description(), lockedImage(), unlockedImage(), hidden(false), maxProgress(0) {
		}
	};

	struct ScoreModel {
		QString name;

		ScoreModel() : name() {
		}
	};

	class SpineEditorModel : public QObject {
		Q_OBJECT

	public:
		SpineEditorModel(QObject * par);

		void setPath(QString path);
		QString getPath() const;

		void setGothicVersion(common::GothicVersion gothicVersion);
		common::GothicVersion getGothicVersion() const;

		void load();
		void save();

		void setModules(int32_t modules);
		int32_t getModules() const;

		void setAchievementOrientation(AchievementOrientation achievementOrientation);
		AchievementOrientation getAchievementOrientation() const;

		void setAchievementDisplayDuration(int duration);
		int getAchievementDisplayDuration() const;

		void setAchievements(QList<AchievementModel> achievements);
		QList<AchievementModel> getAchievements() const;

		void setScores(QList<ScoreModel> scores);
		QList<ScoreModel> getScores() const;

		void setEarthquakeVibration(bool enabled);
		bool getEarthquakeVibration() const;

		void setLeGoModules(int32_t modules);
		int32_t getLeGoModules() const;

		void setModName(QString modName);
		QString getModName() const;

	signals:
		void changed();

	private:
		QString _path;
		int32_t _modules;
		AchievementOrientation _achievementOrientation;
		int _achievementDisplayDuration;
		QList<AchievementModel> _achievements;
		QList<ScoreModel> _scores;
		common::GothicVersion _gothicVersion;
		bool _earthquakeVibration;
		int32_t _legoModules;
		QString _modName;
		QString _achievmentBackground;

		QString getSpineInitString() const;
		QString getLeGoInitString() const;
	};

} /* namespace models */
} /* namespace spine */

#endif /* __SPINE_MODELS_SPINEEDITORMODEL_H__ */