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

namespace spine {
namespace common {
	class TranslationModel;
	struct SendProjectsMessage;
	struct SendTextToReviewMessage;
	struct SendTextToTranslateMessage;
	struct SendTranslationDownloadMessage;
	struct SendTranslatorsMessage;
	struct SendOwnProjectsMessage;
}
namespace translation {

	class TranslatorAPI {
	public:
		static common::SendProjectsMessage * getProjects(const std::string & username);

		static common::TranslationModel * createModel(const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage);

		static uint32_t requestTranslation(const std::string & username, common::TranslationModel * model);

		static common::SendTextToTranslateMessage * requestTextToTranslate(const std::string & username, const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage);

		static bool sendTranslationDraft(const std::string & username, const std::string & destinationLanguage, const std::string & name, const std::string & text, const std::vector<std::string> & dialog, uint32_t id);

		static common::SendTextToReviewMessage * requestTextToReview(const std::string & username, const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage);

		static bool sendTranslationReview(const std::string & username, const std::string & sourceLanguage, const std::string & destinationLanguage, const std::string & name, const std::string & text, const std::vector<std::string> & dialog, uint32_t id, bool changed);

		static std::pair<uint32_t, uint32_t> requestTranslationProgress(const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage);

		static common::SendOwnProjectsMessage * requestOwnProjects(const std::string & username);

		static common::SendTranslatorsMessage * requestTranslators(uint32_t requestID);

		static void changeTranslatorRights(uint32_t requestID, const std::string & username, bool accessRights);

		static common::SendTranslationDownloadMessage * requestTranslationDownload(uint32_t requestID);
	};

} /* namespace translation */
} /* namespace spine */
