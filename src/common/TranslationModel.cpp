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

#include "TranslationModel.h"

#include <algorithm>

using namespace spine::common;

TranslationModel::TranslationModel(const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage) : _projectName(projectName), _sourceLanguage(sourceLanguage), _destinationLanguage(destinationLanguage), _names(), _texts(), _dialogs(), _namesLock(), _textsLock(), _dialogsLock() {
}

void TranslationModel::addName(const std::string & name) {
	std::lock_guard<std::mutex> lg(_namesLock);
	if (std::find_if(_names.begin(), _names.end(), [name](const std::string & other) { return name == other; }) == _names.end()) {
		_names.push_back(name);
	}
}

void TranslationModel::addText(const std::string & text) {
	std::lock_guard<std::mutex> lg(_textsLock);
	if (std::find_if(_texts.begin(), _texts.end(), [text](const std::string & other) { return text == other; }) == _texts.end()) {
		_texts.push_back(text);
	}
}

void TranslationModel::addDialog(const std::vector<std::string> & dialog) {
	std::lock_guard<std::mutex> lg(_dialogsLock);
	if (std::find_if(_dialogs.begin(), _dialogs.end(), [dialog](const std::vector<std::string> & other) {
		bool equal = dialog.size() == other.size();
		if (equal) {
			for (size_t i = 0; i < dialog.size(); i++) {
				if (dialog[i] != other[i]) {
					equal = false;
					break;
				}
			}
		}
		return equal;
	}) == _dialogs.end()) {
		_dialogs.push_back(dialog);
	}
}

void TranslationModel::postProcess() {
	std::lock_guard<std::mutex> lg(_namesLock);
	std::lock_guard<std::mutex> lg2(_textsLock);
	std::lock_guard<std::mutex> lg3(_dialogsLock);
	std::vector<std::string> removes;
	for (std::string s : _texts) {
		if (std::find_if(_names.begin(), _names.end(), [s](const std::string & other) { return s == other; }) != _names.end()) {
			removes.push_back(s);
		}
	}
	for (std::string s : removes) {
		_texts.erase(std::remove_if(_texts.begin(), _texts.end(), [s](const std::string & other) { return s == other; }));
	}
}

int TranslationModel::getDialogTextCount() const {
	int count = 0;
	{
		std::lock_guard<std::mutex> lg(_dialogsLock);
		for (std::vector<std::string> vec : _dialogs) {
			count += int(vec.size());
		}
	}
	return count;
}

void TranslationModel::merge(TranslationModel * other) {
	{
		{
			std::lock_guard<std::mutex> lg3(other->_namesLock);
			std::vector<std::string> otherNames = other->getNames();
			for (std::string name : otherNames) {
				addName(name);
			}
		}
		{
			std::lock_guard<std::mutex> lg3(other->_textsLock);
			std::vector<std::string> otherTexts = other->getTexts();
			for (std::string text : otherTexts) {
				addText(text);
			}
		}
		{
			std::lock_guard<std::mutex> lg3(other->_dialogsLock);
			std::vector<std::vector<std::string>> otherDialogs = other->getDialogs();
			for (std::vector<std::string> dialog : otherDialogs) {
				addDialog(dialog);
			}
		}
	}
	postProcess();
}

bool TranslationModel::empty() const {
	std::lock_guard<std::mutex> lg(_namesLock);
	std::lock_guard<std::mutex> lg2(_textsLock);
	std::lock_guard<std::mutex> lg3(_dialogsLock);
	return _names.empty() && _texts.empty() && _dialogs.empty();
}

std::string TranslationModel::getRequestName() const {
	return _projectName;
}

std::string TranslationModel::getSourceLanguage() const {
	return _sourceLanguage;
}

std::string TranslationModel::getDestinationLanguage() const {
	return _destinationLanguage;
}

BOOST_CLASS_EXPORT_GUID(spine::common::TranslationModel, "TM")
BOOST_CLASS_IMPLEMENTATION(spine::common::TranslationModel, boost::serialization::object_serializable)
