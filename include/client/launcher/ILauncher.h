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
// Copyright 2019 Clockwork Origins

#pragma once

#include "common/GothicVersion.h"

#include <QSharedPointer>
#include <QtPlugin>

namespace spine {
namespace launcher {

	class ILauncher {
	public:
		virtual ~ILauncher() {}

		virtual bool supports(common::GothicVersion gothic) = 0;

	signals:
		virtual void restartAsAdmin() = 0;
	};
	typedef QSharedPointer<ILauncher> ILauncherPtr;

} /* namespace launcher */
} /* namespace spine */

Q_DECLARE_INTERFACE(spine::launcher::ILauncher, "spine::launcher::ILauncher");
