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

#include "launcher/LauncherFactory.h"

#include "launcher/Gothic1Launcher.h"
#include "launcher/Gothic2Launcher.h"

namespace spine {
namespace launcher {

	LauncherFactory * LauncherFactory::getInstance() {
		static LauncherFactory factory;
		return &factory;
	}

	LauncherFactory::LauncherFactory() {
		_launchers.append(createLauncher(common::GothicVersion::GOTHIC));
		_launchers.append(createLauncher(common::GothicVersion::GOTHIC2));
	}

	ILauncherPtr LauncherFactory::getLauncher(common::GothicVersion gothic) const {
		for (const auto & l : _launchers) {
			if (!l->supports(gothic)) continue;

			return l;
		}

		return ILauncherPtr();
	}

	ILauncherPtr LauncherFactory::createLauncher(common::GothicVersion gothic) const {
		ILauncherPtr launcher;
		switch (gothic) {
		case common::GothicVersion::GOTHIC: {
			launcher = QSharedPointer<Gothic1Launcher>(new Gothic1Launcher());
			break;
		}
		case common::GothicVersion::GOTHIC2: {
			launcher = QSharedPointer<Gothic2Launcher>(new Gothic2Launcher());
			break;
		}
		case common::GothicVersion::GOTHICINGOTHIC2: {
			// not supported as launcher
			break;
		}
		case common::GothicVersion::Gothic1And2: {
			// not supported as launcher
			break;
		}
		default: {
			break;
		}
		}

		if (launcher) {
			connect(launcher.dynamicCast<QObject>().data(), SIGNAL(restartAsAdmin()), this, SIGNAL(restartAsAdmin()));
		}

		return launcher;
	}
 
} /* namespace launcher */
} /* namespace spine */
