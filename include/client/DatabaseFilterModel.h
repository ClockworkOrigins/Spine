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

#include <QSortFilterProxyModel>

namespace spine {

	enum DatabaseColumn {
		ModID,
		Name,
		Author,
		Type,
		Game,
		DevDuration,
		AvgDuration,
		Release,
		Update,
		Version,
		Size,
		Install
	};

	enum DatabaseRole {
		SortRole = Qt::UserRole,
		FilterRole,
		PackageIDRole
	};

	class DatabaseFilterModel : public QSortFilterProxyModel {
		Q_OBJECT

	public:
		DatabaseFilterModel(QObject * par);

		bool isGamesActive() const {
			return _gamesActive;
		}

		bool isDemosActive() const {
			return _demosActive;
		}

		bool isFullVersionsActive() const {
			return _fullVersionsActive;
		}

		bool isPlayTestingActive() const {
			return _playTestingActive;
		}

		bool isGothicActive() const {
			return _gothicActive;
		}

		bool isGothic2Active() const {
			return _gothic2Active;
		}

		bool isGothicAndGothic2Active() const {
			return _gothicAndGothic2Active;
		}

		bool isTotalConversionActive() const {
			return _totalConversionActive;
		}

		bool isEnhancementActive() const {
			return _enhancementActive;
		}

		bool isPathActive() const {
			return _patchActive;
		}

		bool isToolActive() const {
			return _toolActive;
		}

		bool isOriginalActive() const {
			return _originalActive;
		}

		bool isGMPActive() const {
			return _gmpActive;
		}

		int getMinDuration() const {
			return _minDuration;
		}

		int getMaxDuration() const {
			return _maxDuration;
		}

		void setRendererAllowed(bool allowed) {
			_rendererAllowed = allowed;
		}

	public slots:
		void gamesChanged(int state);
		void demosChanged(int state);
		void fullVersionsChanged(int state);
		void playTestingChanged(int state);
		void gothicChanged(int state);
		void gothic2Changed(int state);
		void gothicAndGothic2Changed(int state);
		void totalConversionChanged(int state);
		void enhancementChanged(int state);
		void patchChanged(int state);
		void toolChanged(int state);
		void originalChanged(int state);
		void gmpChanged(int state);
		void minDurationChanged(int);
		void maxDurationChanged(int);

	private:
		bool _gamesActive;
		bool _demosActive;
		bool _fullVersionsActive;
		bool _playTestingActive;
		bool _gothicActive;
		bool _gothic2Active;
		bool _gothicAndGothic2Active;
		bool _totalConversionActive;
		bool _enhancementActive;
		bool _patchActive;
		bool _toolActive;
		bool _originalActive;
		bool _gmpActive;
		int _minDuration;
		int _maxDuration;
		bool _rendererAllowed;

		bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;
	};

} /* namespace spine */
