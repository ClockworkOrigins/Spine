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
// Copyright 2019 Clockwork Origins

#pragma once

#include "launcher/Gothic1And2Launcher.h"

namespace spine {
namespace launcher {

	class Gothic1Launcher : public Gothic1And2Launcher {
		Q_OBJECT

	public:
		Gothic1Launcher();

		bool supports(common::GothicVersion gothic) const override;

		void setDirectory(const QString & directory) override;

	signals:
		void updatedPath();

	private slots:
		void patchCheck();

	private:
		common::GothicVersion getGothicVersion() const override {
			return common::GothicVersion::GOTHIC;
		};
	};

} /* namespace launcher */
} /* namespace spine */
