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

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace clockUtils {
	enum class ClockError;
namespace sockets {
	class TcpSocket;
} /* namespace sockets */
} /* namespace clockUtils */

namespace std {
	class thread;
} /* namespace std */

namespace spine {
namespace common {
	struct ChangeTranslationRightsMessage;
	struct QueryTextToReviewMessage;
	struct QueryTextToTranslateMessage;
	struct RequestOwnProjectsMessage;
	struct RequestProjectsMessage;
	struct RequestTranslationDownloadMessage;
	struct RequestTranslationProgressMessage;
	struct RequestTranslatorsMessage;
	struct SendTranslationDraftMessage;
	struct SendTranslationReviewMessage;
	struct TranslationRequestMessage;
} /* namespace common */
namespace server {

	class TranslatorServer {
	public:
		TranslatorServer();
		~TranslatorServer();

		int run();

	private:
		clockUtils::sockets::TcpSocket * _listenClient;
		mutable std::mutex _lock;
		std::thread * _cleanupThread;
		std::thread * _deepLThread;
		bool _running;

		void accept(clockUtils::sockets::TcpSocket * sock);

		void receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error);

		void handleTranslationRequest(clockUtils::sockets::TcpSocket * sock, common::TranslationRequestMessage * msg);
		void handleQueryTextToReview(clockUtils::sockets::TcpSocket * sock, common::QueryTextToReviewMessage * msg);
		void handleQueryTextToTranslate(clockUtils::sockets::TcpSocket * sock, common::QueryTextToTranslateMessage * msg);
		void handleSendTranslationDraft(clockUtils::sockets::TcpSocket * sock, common::SendTranslationDraftMessage * msg);
		void handleRequestTranslationProgress(clockUtils::sockets::TcpSocket * sock, common::RequestTranslationProgressMessage * msg);
		void handleSendTranslationReview(clockUtils::sockets::TcpSocket * sock, common::SendTranslationReviewMessage * msg);
		void handleRequestProjects(clockUtils::sockets::TcpSocket * sock, common::RequestProjectsMessage * msg);
		void handleRequestOwnProjects(clockUtils::sockets::TcpSocket * sock, common::RequestOwnProjectsMessage * msg);
		void handleChangeTranslationRights(clockUtils::sockets::TcpSocket * sock, common::ChangeTranslationRightsMessage * msg);
		void handleRequestTranslators(clockUtils::sockets::TcpSocket * sock, common::RequestTranslatorsMessage * msg);
		void handleRequestTranslationDownload(clockUtils::sockets::TcpSocket * sock, common::RequestTranslationDownloadMessage * msg);

		/**
		 * \brief returns the id in the table for the username
		 */
		int getUserID(const std::string & username) const;
		std::string getUsername(int id) const;

		std::vector<std::pair<std::string, std::string>> getHints(const std::string & sourceLanguage, const std::string & destinationLanguage, const std::vector<std::string> & strings) const;
		std::string toLower(const std::string & s) const;
		std::vector<std::pair<std::string, std::string>> getHints(const std::string & sourceLanguage, const std::string & destinationLanguage, std::string s) const;
		void mergeHints(const std::vector<std::pair<std::string, std::string>> & strings, std::vector<std::pair<std::string, std::string>> & hints) const;
		bool equals(const std::string & translatedText, const std::string & newText) const;
		size_t levensthein(const std::string & a, const std::string & b) const;
		void determineProjectProgress(uint32_t requestID, const std::string & destinationLanguage, uint32_t & translated, uint32_t & toTranslate);
		std::string requestDeepL(const std::string & text, const std::string & sourceLanguage, const std::string & targetLanguage) const;

		void createFakeTranslation() const;

		void cleanup() const;
		void removeDialogs() const;
		void removeUnusedReferences() const;
		void removeNewlinesFromDeepL() const;
	};

} /* namespace server */
} /* namespace spine */
