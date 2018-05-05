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

#ifndef __SPINE_DIRVALIDATOR_H__
#define __SPINE_DIRVALIDATOR_H__

#include <QValidator>

namespace spine {

	class DirValidator : public QValidator {
	public:
		DirValidator();

		State validate(QString & input, int & pos) const override;

		bool isValid(QString string) const {
			int pos = string.length();
			return validate(string, pos) == State::Acceptable;
		}
	};

} /* namespace spine */

#endif /* __SPINE_DIRVALIDATOR_H__ */
