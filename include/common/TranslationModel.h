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
// Copyright 2022 Clockwork Origins

#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/export.hpp"
#include "boost/serialization/vector.hpp"

namespace spine {
namespace common {

	class TranslationModel {
		friend class boost::serialization::access;

	public:
		TranslationModel(const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage);

		void addName(const std::string & name);
		void addText(const std::string & text);
		void addDialog(const std::vector<std::string> & dialogTexts);

		void postProcess();

		std::vector<std::string> getNames() const {
			return _names;
		}

		std::vector<std::string> getTexts() const {
			return _texts;
		}

		std::vector<std::vector<std::string>> getDialogs() const {
			return _dialogs;
		}

		int getDialogTextCount() const;

		void merge(TranslationModel * other);

		bool empty() const;

		std::string getRequestName() const;
		std::string getSourceLanguage() const;
		std::string getDestinationLanguage() const;

	private:
		std::string _projectName;
		std::string _sourceLanguage;
		std::string _destinationLanguage;
		std::vector<std::string> _names;
		std::vector<std::string> _texts;
		std::vector<std::vector<std::string>> _dialogs;
		mutable std::mutex _namesLock;
		mutable std::mutex _textsLock;
		mutable std::mutex _dialogsLock;

		TranslationModel() {}

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & _projectName;
			ar & _sourceLanguage;
			ar & _destinationLanguage;
			ar & _names;
			ar & _texts;
			ar & _dialogs;
		}
	};

} /* namespace common */
} /* namespace spine */
