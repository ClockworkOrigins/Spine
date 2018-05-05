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

#ifndef __SPINE_LIBRARYFILTERMODEL_H__
#define __SPINE_LIBRARYFILTERMODEL_H__

#include <QSortFilterProxyModel>

class QSettings;

namespace spine {

	class LibraryFilterModel : public QSortFilterProxyModel {
		Q_OBJECT

	public:
		enum DataRole {
			GothicRole = Qt::UserRole,
			TypeRole,
			IniFileRole,
			InstalledRole,
			ModIDRole,
			HiddenRole
		};
		LibraryFilterModel(QSettings * iniParser, QObject * par);

		bool isGothicActive() const {
			return _gothicActive;
		}

		bool isGothic2Active() const {
			return _gothic2Active;
		}

		bool isGothicAndGothic2Active() const {
			return _gothicAndGothic2Active;
		}

		bool isShowHiddenActive() const {
			return _showHidden;
		}

	public slots:
		void gothicChanged(int state);
		void gothic2Changed(int state);
		void gothicAndGothic2Changed(int state);
		void showHidden(int state);

	private:
		QSettings * _iniParser;
		bool _gothicActive;
		bool _gothic2Active;
		bool _gothicAndGothic2Active;
		bool _showHidden;

		bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override;
	};

} /* namespace spine */

#endif /* __SPINE_LIBRARYFILTERMODEL_H__ */
