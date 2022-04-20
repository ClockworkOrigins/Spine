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

#include "TranslatorServer.h"

#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <thread>

#include "MariaDBWrapper.h"
#include "ServerCommon.h"
#include "SpineServerConfig.h"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "common/MessageStructs.h"
#include "common/TranslationModel.h"

#include "clockUtils/sockets/TcpSocket.h"

#include "simple-web-server/client_https.hpp"

using namespace spine::common;
using namespace spine::server;

using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;

TranslatorServer::TranslatorServer() : _listenClient(new clockUtils::sockets::TcpSocket()), _cleanupThread(new std::thread(std::bind(&TranslatorServer::cleanup, this))), _deepLThread(new std::thread(std::bind(&TranslatorServer::createFakeTranslation, this))), _running(true) {
}

TranslatorServer::~TranslatorServer() {
	_running = false;
	delete _listenClient;
	_cleanupThread->join();
	delete _cleanupThread;
	_deepLThread->join();
	delete _deepLThread;
}

int TranslatorServer::run() {
	if (_listenClient->listen(TRANSLATORSERVER_PORT, 10, true, std::bind(&TranslatorServer::accept, this, std::placeholders::_1)) != clockUtils::ClockError::SUCCESS) {
		return 1;
	}

	return 0;
}

void TranslatorServer::accept(clockUtils::sockets::TcpSocket * sock) {
	sock->receiveCallback(std::bind(&TranslatorServer::receiveMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void TranslatorServer::receiveMessage(const std::vector<uint8_t> & message, clockUtils::sockets::TcpSocket * sock, clockUtils::ClockError error) {
	if (error != clockUtils::ClockError::SUCCESS) {
		std::thread([sock]() {
			sock->close();
			delete sock;
		}).detach();
	} else {
		try {
			Message * m = Message::DeserializePrivate(std::string(message.begin(), message.end()));
			if (!m) {
				sock->writePacket("Crash"); // it's a hack to stop hanging threads
				return;
			}
			if (m->type == MessageType::TRANSLATIONREQUEST) {
				auto * msg = dynamic_cast<TranslationRequestMessage *>(m);
				handleTranslationRequest(sock, msg);
			} else if (m->type == MessageType::QUERYTEXTTOTRANSLATE) {
				auto * msg = dynamic_cast<QueryTextToTranslateMessage *>(m);
				handleQueryTextToTranslate(sock, msg);
			} else if (m->type == MessageType::SENDTRANSLATIONDRAFT) {
				auto * msg = dynamic_cast<SendTranslationDraftMessage *>(m);
				handleSendTranslationDraft(sock, msg);
			} else if (m->type == MessageType::REQUESTTRANSLATIONPROGRESS) {
				auto * msg = dynamic_cast<RequestTranslationProgressMessage *>(m);
				handleRequestTranslationProgress(sock, msg);
			} else if (m->type == MessageType::QUERYTEXTTOREVIEW) {
				auto * msg = dynamic_cast<QueryTextToReviewMessage *>(m);
				handleQueryTextToReview(sock, msg);
			} else if (m->type == MessageType::SENDTRANSLATIONREVIEW) {
				auto * msg = dynamic_cast<SendTranslationReviewMessage *>(m);
				handleSendTranslationReview(sock, msg);
			} else if (m->type == MessageType::REQUESTPROJECTS) {
				auto * msg = dynamic_cast<RequestProjectsMessage *>(m);
				handleRequestProjects(sock, msg);
			} else if (m->type == MessageType::REQUESTOWNPROJECTS) {
				auto * msg = dynamic_cast<RequestOwnProjectsMessage *>(m);
				handleRequestOwnProjects(sock, msg);
			} else if (m->type == MessageType::REQUESTTRANSLATORS) {
				auto * msg = dynamic_cast<RequestTranslatorsMessage *>(m);
				handleRequestTranslators(sock, msg);
			} else if (m->type == MessageType::CHANGETRANSLATIONRIGHTS) {
				auto * msg = dynamic_cast<ChangeTranslationRightsMessage *>(m);
				handleChangeTranslationRights(sock, msg);
			} else if (m->type == MessageType::REQUESTTRANSLATIONDOWNLOAD) {
				auto * msg = dynamic_cast<RequestTranslationDownloadMessage *>(m);
				handleRequestTranslationDownload(sock, msg);
			} else if (m->type == MessageType::REQUESTCSVDOWNLOAD) {
				auto * msg = dynamic_cast<RequestCsvDownloadMessage *>(m);
				handleRequestCsvDownload(sock, msg);
			} else {
				std::cerr << "unexpected control message arrived: " << static_cast<int>(m->type) << std::endl;
				delete m;
				return;
			}

			delete m;
		} catch (...) {
			std::cerr << "deserialization not working" << std::endl;
			sock->writePacket("Crash"); // it's a hack to stop hanging threads
			return;
		}
	}
}

void TranslatorServer::handleTranslationRequest(clockUtils::sockets::TcpSocket * sock, TranslationRequestMessage * msg) {
	std::lock_guard<std::mutex> lg(_lock);
	TranslationRequestedMessage trm;
	trm.id = UINT32_MAX;
	do {
		if (msg->translationModel->empty()) {
			break;
		}
		int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE insertTranslationRequestStmt FROM \"INSERT INTO translationRequests (Name, SourceLanguage, DestinationLanguage, UserID) VALUES (CONVERT(? USING BINARY), CONVERT(? USING BINARY), CONVERT(? USING BINARY), ?)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDStmt FROM \"SELECT MAX(RequestID) FROM translationRequests\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAllNamesStmt FROM \"SELECT NameID, Name FROM names WHERE Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAllTextsStmt FROM \"SELECT TextID, String FROM texts WHERE Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAllDialogsStmt FROM \"SELECT DialogID FROM dialogs WHERE Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectAllDialogTextsStmt FROM \"SELECT DialogTextID, CAST(String AS BINARY) FROM dialogTexts WHERE DialogID = ? ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectReallyAllDialogTextsStmt FROM \"SELECT DialogID, CAST(String AS BINARY) FROM dialogTexts ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertNameRequestStmt FROM \"INSERT INTO nameRequests (RequestID, NameID) VALUES (?, ?)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertTextRequestStmt FROM \"INSERT INTO textRequests (RequestID, TextID) VALUES (?, ?)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogRequestStmt FROM \"INSERT INTO dialogRequests (RequestID, DialogID) VALUES (?, ?)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameDraftStmt FROM \"SELECT SourceID FROM nameDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextDraftStmt FROM \"SELECT SourceID FROM textDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogDraftStmt FROM \"SELECT SourceID FROM dialogDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertNameStmt FROM \"INSERT INTO names (Name, Language) VALUES (CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertTextStmt FROM \"INSERT INTO texts (String, Language) VALUES (CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogStmt FROM \"INSERT INTO dialogs (Language) VALUES (CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogTextStmt FROM \"INSERT INTO dialogTexts (DialogID, String) VALUES (?, CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE findNameIDStmt FROM \"SELECT NameID FROM names WHERE Name = CONVERT(? USING BINARY) AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE findTextIDStmt FROM \"SELECT TextID FROM texts WHERE String = CONVERT(? USING BINARY) AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE findDialogTextIDStmt FROM \"SELECT DialogTextID FROM dialogTexts WHERE String = CONVERT(? USING BINARY) AND DialogTextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameIDStmt FROM \"SELECT MAX(NameID) FROM names\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextIDStmt FROM \"SELECT MAX(TextID) FROM texts\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogIDStmt FROM \"SELECT MAX(DialogID) FROM dialogs\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameTranslationsStmt FROM \"SELECT SourceID FROM nameTranslations WHERE SourceID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextTranslationsStmt FROM \"SELECT SourceID FROM textTranslations WHERE SourceID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTranslationsStmt FROM \"SELECT SourceID FROM dialogTranslations WHERE SourceID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertNameDraftStmt FROM \"INSERT INTO nameDrafts (SourceID, Language, Translation, Timestamp, UserID, State) VALUES (?, CONVERT(? USING BINARY), '', 0, -1, 0)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertTextDraftStmt FROM \"INSERT INTO textDrafts (SourceID, Language, Translation, Timestamp, UserID, State) VALUES (?, CONVERT(? USING BINARY), '', 0, -1, 0)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogDraftStmt FROM \"INSERT INTO dialogDrafts (SourceID, Language, Timestamp, UserID, State) VALUES (?, CONVERT(? USING BINARY), 0, -1, 0)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogTextDraftStmt FROM \"INSERT INTO dialogTextDrafts (SourceID, Language, Translation) VALUES (?, CONVERT(? USING BINARY), '')\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramRequestName='" + msg->translationModel->getRequestName() + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramSourceLanguage='" + msg->translationModel->getSourceLanguage() + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + msg->translationModel->getDestinationLanguage() + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE insertTranslationRequestStmt USING @paramRequestName, @paramSourceLanguage, @paramDestinationLanguage, @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectRequestIDStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		std::vector<std::vector<std::string>> results = database.getResults<std::vector<std::string>>();
		if (results.empty()) {
			break;
		}
		std::string requestID = results[0][0];
		if (!database.query("SET @paramRequestID=" + requestID + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		std::vector<std::string> names = msg->translationModel->getNames();
		for (const std::string & name : names) {
			if (!database.query("SET @paramName='" + name + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE findNameIDStmt USING @paramName, @paramSourceLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			std::string nameID;
			if (!tmpResults.empty()) {
				nameID = tmpResults[0][0];
			}
			if (nameID.empty()) {
				if (!database.query("EXECUTE insertNameStmt USING @paramName, @paramSourceLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE selectNameIDStmt;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				tmpResults = database.getResults<std::vector<std::string>>();
				nameID = tmpResults[0][0];
			}
			if (!database.query("SET @paramNameID=" + nameID + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectNameTranslationsStmt USING @paramNameID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			if (tmpResults.empty()) {
				if (!database.query("EXECUTE selectNameDraftStmt USING @paramNameID, @paramDestinationLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				tmpResults = database.getResults<std::vector<std::string>>();
				if (tmpResults.empty()) {
					if (!database.query("EXECUTE insertNameDraftStmt USING @paramNameID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
				}
			}
			if (!database.query("EXECUTE insertNameRequestStmt USING @paramRequestID, @paramNameID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		}

		std::vector<std::string> texts = msg->translationModel->getTexts();
		for (const std::string & text : texts) {
			if (!database.query("SET @paramText='" + text + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE findTextIDStmt USING @paramText, @paramSourceLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			std::string textID;
			if (!tmpResults.empty()) {
				textID = tmpResults[0][0];
			}
			if (textID.empty()) {
				if (!database.query("EXECUTE insertTextStmt USING @paramText, @paramSourceLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE selectTextIDStmt;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				tmpResults = database.getResults<std::vector<std::string>>();
				textID = tmpResults[0][0];
			}
			if (!database.query("SET @paramTextID=" + textID + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectTextTranslationsStmt USING @paramTextID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			if (tmpResults.empty()) {
				if (!database.query("EXECUTE selectTextDraftStmt USING @paramTextID, @paramDestinationLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				tmpResults = database.getResults<std::vector<std::string>>();
				if (tmpResults.empty()) {
					if (!database.query("EXECUTE insertTextDraftStmt USING @paramTextID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
				}
			}
			if (!database.query("EXECUTE insertTextRequestStmt USING @paramRequestID, @paramTextID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		}

		if (!database.query("EXECUTE selectReallyAllDialogTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		std::map<int, std::vector<std::string>> dialogTexts;
		for (const auto & vec : results) {
			int dialogID = std::stoi(vec[0]);
			std::string dialogText = vec[1];
			dialogTexts[dialogID].push_back(dialogText);
		}
		std::vector<std::vector<std::string>> dialogs = msg->translationModel->getDialogs();
		for (std::vector<std::string> dialog : dialogs) {
			std::string dialogID;
			for (auto & dialogText : dialogTexts) {
				std::vector<std::string> realDialogTexts = dialogText.second;
				if (dialog.size() == realDialogTexts.size()) { // Dialog already exists, so reuse id
					bool same = true;
					for (size_t i = 0; i < dialog.size(); i++) {
						if (realDialogTexts[i] != dialog[i]) {
							same = false;
							break;
						}
					}
					if (same) {
						dialogID = std::to_string(dialogText.first);
						break;
					}
				}
			}
			if (dialogID.empty()) {
				if (!database.query("EXECUTE insertDialogStmt USING @paramSourceLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE selectDialogIDStmt;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto tmpResults = database.getResults<std::vector<std::string>>();
				dialogID = tmpResults[0][0];
				if (!database.query("SET @paramDialogID=" + dialogID + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				for (const std::string & dialogText : dialog) {
					if (!database.query("SET @paramText='" + dialogText + "';")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE insertDialogTextStmt USING @paramDialogID, @paramText;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
				}
			}
			if (!database.query("SET @paramDialogID=" + dialogID + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectDialogTranslationsStmt USING @paramDialogID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			if (tmpResults.empty()) {
				if (!database.query("EXECUTE selectDialogDraftStmt USING @paramDialogID, @paramDestinationLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				tmpResults = database.getResults<std::vector<std::string>>();
				if (tmpResults.empty()) {
					if (!database.query("EXECUTE insertDialogDraftStmt USING @paramDialogID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectAllDialogTextsStmt USING @paramDialogID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					tmpResults = database.getResults<std::vector<std::string>>();
					for (const auto & vec : tmpResults) {
						if (!database.query("SET @paramDialogTextID=" + vec[0] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE insertDialogTextDraftStmt USING @paramDialogTextID, @paramDestinationLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
					}
				}
			}
			if (!database.query("EXECUTE insertDialogRequestStmt USING @paramRequestID, @paramDialogID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		}
		trm.id = std::stoi(requestID);
	} while (false);
	std::string serialized = trm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleQueryTextToTranslate(clockUtils::sockets::TcpSocket * sock, QueryTextToTranslateMessage * msg) {
	std::lock_guard<std::mutex> lg(_lock);
	SendTextToTranslateMessage stttm;
	stttm.id = UINT32_MAX;
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDStmt FROM \"SELECT RequestID FROM translationRequests WHERE Name = CONVERT(? USING BINARY) AND SourceLanguage = CONVERT(? USING BINARY) AND DestinationLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameRequestsStmt FROM \"SELECT NameID FROM nameRequests WHERE RequestID = ? ORDER BY NameID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextRequestsStmt FROM \"SELECT TextID FROM textRequests WHERE RequestID = ? ORDER BY TextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogRequestsStmt FROM \"SELECT DialogID FROM dialogRequests WHERE RequestID = ? ORDER BY DialogID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameDraftsStmt FROM \"SELECT SourceID FROM nameDrafts WHERE Language = CONVERT(? USING BINARY) AND Timestamp < ? AND State = 0 ORDER BY SourceID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextDraftsStmt FROM \"SELECT SourceID FROM textDrafts WHERE Language = CONVERT(? USING BINARY) AND Timestamp < ? AND State = 0 ORDER BY SourceID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogDraftsStmt FROM \"SELECT SourceID FROM dialogDrafts WHERE Language = CONVERT(? USING BINARY) AND Timestamp < ? AND State = 0 ORDER BY SourceID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateNameDraftStmt FROM \"UPDATE nameDrafts SET Timestamp = ? WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateTextDraftStmt FROM \"UPDATE textDrafts SET Timestamp = ? WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateDialogDraftStmt FROM \"UPDATE dialogDrafts SET Timestamp = ? WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM names WHERE NameID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextStmt FROM \"SELECT CAST(String AS BINARY) FROM texts WHERE TextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogTextID, CAST(String AS BINARY) FROM dialogTexts WHERE DialogID = ? ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameDeepLStmt FROM \"SELECT CAST(Translation AS BINARY) FROM nameDeepLTranslations WHERE NameID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextDeepLStmt FROM \"SELECT CAST(Translation AS BINARY) FROM textDeepLTranslations WHERE TextID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextDeepLStmt FROM \"SELECT CAST(Translation AS BINARY) FROM dialogTextDeepLTranslations WHERE DialogTextID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramProjectName='" + msg->projectName + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramSourceLanguage='" + msg->sourceLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + msg->destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectRequestIDStmt USING @paramProjectName, @paramSourceLanguage, @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		std::vector<std::vector<std::string>> results = database.getResults<std::vector<std::string>>();
		if (results.empty()) {
			std::cout << "No request found" << std::endl;
			break;
		}
		std::string requestID = results[0][0];
		if (!database.query("SET @paramRequestID=" + requestID + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramTimestamp=" + std::to_string(time(nullptr)) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramWorkTimestamp=" + std::to_string(time(nullptr) + 60LL * 60) + ";")) { // 1 hour time per text?
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectNameRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::set<std::string> nameIDs;
			for (const auto & vec : results) {
				nameIDs.insert(vec[0]);
			}
			if (!database.query("EXECUTE selectNameDraftsStmt USING @paramDestinationLanguage, @paramTimestamp;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			std::vector<std::string> draftIDs(results.size());
			for (size_t i = 0; i < results.size(); i++) {
				draftIDs[i] = results[i][0];
			}
			for (const std::string & draftID : draftIDs) {
				if (nameIDs.find(draftID) != nameIDs.end()) {
					if (!database.query("SET @paramNameID=" + draftID + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE updateNameDraftStmt USING @paramWorkTimestamp, @paramNameID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectNameStmt USING @paramNameID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					auto tmpResults = database.getResults<std::vector<std::string>>();
					stttm.id = std::stoi(draftID);
					stttm.name = tmpResults[0][0];
					stttm.hints = getHints(msg->sourceLanguage, msg->destinationLanguage, stttm.name);
					if (!database.query("EXECUTE selectNameDeepLStmt USING @paramNameID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					tmpResults = database.getResults<std::vector<std::string>>();
					if (!tmpResults.empty()) {
						stttm.hints.emplace_back(stttm.name, tmpResults[0][0]);
					}
					break;
				}
			}
		}
		if (!stttm.name.empty()) {
			break;
		}

		if (!database.query("EXECUTE selectTextRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::set<std::string> textIDs;
			for (const auto & vec : results) {
				textIDs.insert(vec[0]);
			}
			if (!database.query("EXECUTE selectTextDraftsStmt USING @paramDestinationLanguage, @paramTimestamp;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			std::vector<std::string> draftIDs(results.size());
			for (size_t i = 0; i < results.size(); i++) {
				draftIDs[i] = results[i][0];
			}
			for (const std::string & draftID : draftIDs) {
				if (textIDs.find(draftID) != textIDs.end()) {
					if (!database.query("SET @paramTextID=" + draftID + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE updateTextDraftStmt USING @paramWorkTimestamp, @paramTextID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectTextStmt USING @paramTextID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					auto tmpResults = database.getResults<std::vector<std::string>>();
					stttm.id = std::stoi(draftID);
					stttm.text = tmpResults[0][0];
					stttm.hints = getHints(msg->sourceLanguage, msg->destinationLanguage, stttm.text);
					if (!database.query("EXECUTE selectTextDeepLStmt USING @paramTextID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					tmpResults = database.getResults<std::vector<std::string>>();
					if (!tmpResults.empty()) {
						stttm.hints.emplace_back(stttm.text, tmpResults[0][0]);
					}
					break;
				}
			}
		}
		if (!stttm.text.empty()) {
			break;
		}

		if (!database.query("EXECUTE selectDialogRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::set<std::string> dialogIDs;
			for (const auto & vec : results) {
				dialogIDs.insert(vec[0]);
			}
			if (!database.query("EXECUTE selectDialogDraftsStmt USING @paramDestinationLanguage, @paramTimestamp;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			std::vector<std::string> draftIDs(results.size());
			for (size_t i = 0; i < results.size(); i++) {
				draftIDs[i] = results[i][0];
			}
			for (const std::string & draftID : draftIDs) {
				if (dialogIDs.find(draftID) != dialogIDs.end()) {
					if (!database.query("SET @paramDialogID=" + draftID + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE updateDialogDraftStmt USING @paramWorkTimestamp, @paramDialogID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectDialogTextsStmt USING @paramDialogID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					auto tmpResults = database.getResults<std::vector<std::string>>();
					stttm.id = std::stoi(draftID);
					std::vector<std::pair<std::string, std::string>> hints;
					for (const auto & dt : tmpResults) {
						stttm.dialog.push_back(dt[1]);
						if (!database.query("SET @paramDialogTextID=" + dt[0] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE selectDialogTextDeepLStmt USING @paramDialogTextID, @paramDestinationLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						auto newTmpResults = database.getResults<std::vector<std::string>>();
						if (!newTmpResults.empty()) {
							hints.emplace_back(dt[1], newTmpResults[0][0]);
						}
					}
					stttm.hints = getHints(msg->sourceLanguage, msg->destinationLanguage, stttm.dialog);
					for (const auto & p : hints) {
						stttm.hints.push_back(p);
					}
					break;
				}
			}
		}
	} while (false);
	std::string serialized = stttm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleSendTranslationDraft(clockUtils::sockets::TcpSocket *, SendTranslationDraftMessage * msg) {
	std::lock_guard<std::mutex> lg(_lock);
	do {
		const int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE updateNameDraftStmt FROM \"UPDATE nameDrafts SET Translation = CONVERT(? USING BINARY), Timestamp = 0, UserID = ?, State = 1 WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateTextDraftStmt FROM \"UPDATE textDrafts SET Translation = CONVERT(? USING BINARY), Timestamp = 0, UserID = ?, State = 1 WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateDialogDraftStmt FROM \"UPDATE dialogDrafts SET Timestamp = 0, UserID = ?, State = 1 WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateDialogTextDraftStmt FROM \"UPDATE dialogTextDrafts SET Translation = CONVERT(? USING BINARY) WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogTextID FROM dialogTexts WHERE DialogID = ? ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + msg->destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!msg->name.empty()) {
			if (!database.query("SET @paramNameID=" + std::to_string(msg->id) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("SET @paramTranslation='" + msg->name + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE updateNameDraftStmt USING @paramTranslation, @paramUserID, @paramNameID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		}
		if (!msg->text.empty()) {
			if (!database.query("SET @paramTextID=" + std::to_string(msg->id) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("SET @paramTranslation='" + msg->text + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE updateTextDraftStmt USING @paramTranslation, @paramUserID, @paramTextID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		}
		if (!msg->dialog.empty()) {
			if (!database.query("SET @paramDialogID=" + std::to_string(msg->id) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE updateDialogDraftStmt USING @paramUserID, @paramDialogID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectDialogTextsStmt USING @paramDialogID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			const auto results = database.getResults<std::vector<std::string>>();
			if (results.empty()) {
				break;
			}
			if (results.size() != msg->dialog.size()) {
				break;
			}
			for (size_t i = 0; i < results.size(); i++) {
				if (!database.query("SET @paramTranslation='" + msg->dialog[i] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramDialogTextID=" + results[i][0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE updateDialogTextDraftStmt USING @paramTranslation, @paramDialogTextID, @paramDestinationLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
		}
	} while (false);
}

void TranslatorServer::handleRequestTranslationProgress(clockUtils::sockets::TcpSocket * sock, RequestTranslationProgressMessage * msg) {
	SendTranslationProgressMessage stpm;
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDStmt FROM \"SELECT RequestID FROM translationRequests WHERE Name = CONVERT(? USING BINARY) AND SourceLanguage = CONVERT(? USING BINARY) AND DestinationLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramProjectName='" + msg->projectName + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramSourceLanguage='" + msg->sourceLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + msg->destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectRequestIDStmt USING @paramProjectName, @paramSourceLanguage, @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		const auto results = database.getResults<std::vector<std::string>>();
		if (results.empty())
			break;

		determineProjectProgress(static_cast<uint32_t>(std::stoi(results[0][0])), msg->destinationLanguage, stpm.translated, stpm.toTranslate);
	} while (false);
	const std::string serialized = stpm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleQueryTextToReview(clockUtils::sockets::TcpSocket * sock, QueryTextToReviewMessage * msg) {
	std::lock_guard<std::mutex> lg(_lock);
	SendTextToReviewMessage sttrm;
	sttrm.id = UINT32_MAX;
	do {
		int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			std::cout << "Invalid user" << std::endl;
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDStmt FROM \"SELECT RequestID FROM translationRequests WHERE Name = CONVERT(? USING BINARY) AND SourceLanguage = CONVERT(? USING BINARY) AND DestinationLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameRequestsStmt FROM \"SELECT NameID FROM nameRequests WHERE RequestID = ? ORDER BY NameID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextRequestsStmt FROM \"SELECT TextID FROM textRequests WHERE RequestID = ? ORDER BY TextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogRequestsStmt FROM \"SELECT DialogID FROM dialogRequests WHERE RequestID = ? ORDER BY DialogID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameDraftsStmt FROM \"SELECT SourceID, CAST(Translation AS BINARY) FROM nameDrafts WHERE Language = CONVERT(? USING BINARY) AND Timestamp < ? AND State = 1 AND UserID != ? ORDER BY SourceID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextDraftsStmt FROM \"SELECT SourceID, CAST(Translation AS BINARY) FROM textDrafts WHERE Language = CONVERT(? USING BINARY) AND Timestamp < ? AND State = 1 AND UserID != ? ORDER BY SourceID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogDraftsStmt FROM \"SELECT SourceID FROM dialogDrafts WHERE Language = CONVERT(? USING BINARY) AND Timestamp < ? AND State = 1 AND UserID != ? ORDER BY SourceID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextDraftStmt FROM \"SELECT CAST(Translation AS BINARY) FROM dialogTextDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateNameDraftStmt FROM \"UPDATE nameDrafts SET Timestamp = ? WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateTextDraftStmt FROM \"UPDATE textDrafts SET Timestamp = ? WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateDialogDraftStmt FROM \"UPDATE dialogDrafts SET Timestamp = ? WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM names WHERE NameID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextStmt FROM \"SELECT CAST(String AS BINARY) FROM texts WHERE TextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogTextID, CAST(String AS BINARY) FROM dialogTexts WHERE DialogID = ? ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameDeepLStmt FROM \"SELECT CAST(Translation AS BINARY) FROM nameDeepLTranslations WHERE NameID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextDeepLStmt FROM \"SELECT CAST(Translation AS BINARY) FROM textDeepLTranslations WHERE TextID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextDeepLStmt FROM \"SELECT CAST(Translation AS BINARY) FROM dialogTextDeepLTranslations WHERE DialogTextID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramProjectName='" + msg->projectName + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramSourceLanguage='" + msg->sourceLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + msg->destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectRequestIDStmt USING @paramProjectName, @paramSourceLanguage, @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		std::vector<std::vector<std::string>> results = database.getResults<std::vector<std::string>>();
		if (results.empty()) {
			break;
		}
		std::string requestID = results[0][0];
		if (!database.query("SET @paramRequestID=" + requestID + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramTimestamp=" + std::to_string(time(nullptr)) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramWorkTimestamp=" + std::to_string(time(nullptr) + 60LL * 60) + ";")) { // 1 hour time per text?
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectNameRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::set<std::string> nameIDs;
			for (const auto & vec : results) {
				nameIDs.insert(vec[0]);
			}
			if (!database.query("EXECUTE selectNameDraftsStmt USING @paramDestinationLanguage, @paramTimestamp, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			std::vector<std::pair<std::string, std::string>> draftIDs(results.size());
			for (size_t i = 0; i < results.size(); i++) {
				draftIDs[i] = std::make_pair(results[i][0], results[i][1]);
			}
			for (const auto & draft : draftIDs) {
				if (nameIDs.find(draft.first) != nameIDs.end()) {
					if (!database.query("SET @paramNameID=" + draft.first + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE updateNameDraftStmt USING @paramWorkTimestamp, @paramNameID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectNameStmt USING @paramNameID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					auto tmpResults = database.getResults<std::vector<std::string>>();
					sttrm.id = std::stoi(draft.first);
					sttrm.name = std::make_pair(tmpResults[0][0], draft.second);
					sttrm.hints = getHints(msg->sourceLanguage, msg->destinationLanguage, sttrm.name.first);
					if (!database.query("EXECUTE selectNameDeepLStmt USING @paramNameID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					tmpResults = database.getResults<std::vector<std::string>>();
					if (!tmpResults.empty()) {
						sttrm.hints.emplace_back(sttrm.name.first, tmpResults[0][0]);
					}
					break;
				}
			}
		}
		if (!sttrm.name.first.empty()) {
			break;
		}

		if (!database.query("EXECUTE selectTextRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::set<std::string> textIDs;
			for (const auto & vec : results) {
				textIDs.insert(vec[0]);
			}
			if (!database.query("EXECUTE selectTextDraftsStmt USING @paramDestinationLanguage, @paramTimestamp, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			std::vector<std::pair<std::string, std::string>> draftIDs(results.size());
			for (size_t i = 0; i < results.size(); i++) {
				draftIDs[i] = std::make_pair(results[i][0], results[i][1]);
			}
			for (const auto & draft : draftIDs) {
				if (textIDs.find(draft.first) != textIDs.end()) {
					if (!database.query("SET @paramTextID=" + draft.first + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE updateTextDraftStmt USING @paramWorkTimestamp, @paramTextID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectTextStmt USING @paramTextID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					auto tmpResults = database.getResults<std::vector<std::string>>();
					sttrm.id = std::stoi(draft.first);
					sttrm.text = std::make_pair(tmpResults[0][0], draft.second);
					sttrm.hints = getHints(msg->sourceLanguage, msg->destinationLanguage, sttrm.text.first);
					if (!database.query("EXECUTE selectTextDeepLStmt USING @paramTextID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					tmpResults = database.getResults<std::vector<std::string>>();
					if (!tmpResults.empty()) {
						sttrm.hints.emplace_back(sttrm.text.first, tmpResults[0][0]);
					}
					break;
				}
			}
		}
		if (!sttrm.text.first.empty()) {
			break;
		}

		if (!database.query("EXECUTE selectDialogRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::set<std::string> dialogIDs;
			for (const auto & vec : results) {
				dialogIDs.insert(vec[0]);
			}
			if (!database.query("EXECUTE selectDialogDraftsStmt USING @paramDestinationLanguage, @paramTimestamp, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			std::vector<std::string> draftIDs(results.size());
			for (size_t i = 0; i < results.size(); i++) {
				draftIDs[i] = results[i][0];
			}
			for (const auto & draftID : draftIDs) {
				if (dialogIDs.find(draftID) != dialogIDs.end()) {
					if (!database.query("SET @paramDialogID=" + draftID + ";")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE updateDialogDraftStmt USING @paramWorkTimestamp, @paramDialogID, @paramDestinationLanguage;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					if (!database.query("EXECUTE selectDialogTextsStmt USING @paramDialogID;")) {
						std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
						break;
					}
					auto tmpResults = database.getResults<std::vector<std::string>>();
					sttrm.id = std::stoi(draftID);
					std::vector<std::string> sources;
					std::vector<std::string> drafts;
					std::vector<std::pair<std::string, std::string>> hints;
					for (auto dt : tmpResults) {
						if (!database.query("SET @paramDialogTextID=" + dt[0] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE selectDialogTextDraftStmt USING @paramDialogTextID, @paramDestinationLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						auto tmpResults2 = database.getResults<std::vector<std::string>>();
						sources.push_back(dt[1]);
						drafts.push_back(tmpResults2[0][0]);
						if (!database.query("EXECUTE selectDialogTextDeepLStmt USING @paramDialogTextID, @paramDestinationLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						auto newTmpResults = database.getResults<std::vector<std::string>>();
						if (!newTmpResults.empty()) {
							hints.emplace_back(dt[1], newTmpResults[0][0]);
						}
					}
					sttrm.dialog = std::make_pair(sources, drafts);
					sttrm.hints = getHints(msg->sourceLanguage, msg->destinationLanguage, sources);
					for (const auto & p : hints) {
						sttrm.hints.push_back(p);
					}
					break;
				}
			}
		}
	} while (false);
	std::string serialized = sttrm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleSendTranslationReview(clockUtils::sockets::TcpSocket * sock, SendTranslationReviewMessage * msg) {
	if (msg->changed) {
		SendTranslationDraftMessage stdm;
		stdm.destinationLanguage = msg->destinationLanguage;
		stdm.dialog = msg->dialog;
		stdm.id = msg->id;
		stdm.name = msg->name;
		stdm.text = msg->text;
		stdm.username = msg->username;
		handleSendTranslationDraft(sock, &stdm);
		return;
	}
	std::lock_guard<std::mutex> lg(_lock);
	do {
		int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE insertNameStmt FROM \"INSERT INTO names (Name, Language) VALUES (CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertTextStmt FROM \"INSERT INTO texts (String, Language) VALUES (CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogStmt FROM \"INSERT INTO dialogs (Language) VALUES (CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogTextStmt FROM \"INSERT INTO dialogTexts (String, DialogID) VALUES (CONVERT(? USING BINARY), ?)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameIDStmt FROM \"SELECT MAX(NameID) FROM names\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextIDStmt FROM \"SELECT MAX(TextID) FROM texts\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertNameTranslationStmt FROM \"INSERT INTO nameTranslations (SourceID, TargetID, TargetLanguage) VALUES (?, ?, CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertTextTranslationStmt FROM \"INSERT INTO textTranslations (SourceID, TargetID, TargetLanguage) VALUES (?, ?, CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE insertDialogTranslationStmt FROM \"INSERT INTO dialogTranslations (SourceID, TargetID, TargetLanguage) VALUES (?, ?, CONVERT(? USING BINARY))\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogIDStmt FROM \"SELECT MAX(DialogID) FROM dialogs\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteNameDraftStmt FROM \"DELETE FROM nameDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteTextDraftStmt FROM \"DELETE FROM textDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteDialogDraftStmt FROM \"DELETE FROM dialogDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteDialogTextDraftStmt FROM \"DELETE FROM dialogTextDrafts WHERE SourceID = ? AND Language = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogTextID FROM dialogTexts WHERE DialogID = ? ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameLanguageStmt FROM \"SELECT CAST(Language AS BINARY) FROM names WHERE NameID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextLanguageStmt FROM \"SELECT CAST(Language AS BINARY) FROM texts WHERE TextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogLanguageStmt FROM \"SELECT CAST(Language AS BINARY) FROM dialogs WHERE DialogID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameIDsStmt FROM \"SELECT TargetID, CAST(TargetLanguage AS BINARY) FROM nameTranslations WHERE SourceID = ? AND TargetID != ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextIDsStmt FROM \"SELECT TargetID, CAST(TargetLanguage AS BINARY) FROM textTranslations WHERE SourceID = ? AND TargetID != ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogIDsStmt FROM \"SELECT TargetID, CAST(TargetLanguage AS BINARY) FROM dialogTranslations WHERE SourceID = ? AND TargetID != ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + msg->destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!msg->name.empty()) {
			if (!database.query("SET @paramNameID=" + std::to_string(msg->id) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (msg->sourceLanguage.empty()) {
				if (!database.query("EXECUTE selectNameLanguageStmt USING @paramNameID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto tmpResults = database.getResults<std::vector<std::string>>();
				msg->sourceLanguage = tmpResults[0][0];
			}
			if (!database.query("SET @paramSourceLanguage='" + msg->sourceLanguage + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE deleteNameDraftStmt USING @paramNameID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("SET @paramTranslation='" + msg->name + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertNameStmt USING @paramTranslation, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectNameIDStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			if (!database.query("SET @paramTargetID=" + tmpResults[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertNameTranslationStmt USING @paramNameID, @paramTargetID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertNameTranslationStmt USING @paramTargetID, @paramNameID, @paramSourceLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}

			if (!database.query("EXECUTE selectNameIDsStmt USING @paramNameID, @paramTargetID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : tmpResults) {
				if (!database.query("SET @paramNewTargetID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramNewDestinationLanguage='" + vec[1] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				database.query("EXECUTE insertNameTranslationStmt USING @paramNameID, @paramNewTargetID, @paramNewDestinationLanguage;");
				database.query("EXECUTE insertNameTranslationStmt USING @paramNewTargetID, @paramNameID, @paramSourceLanguage;");
			}

			if (!database.query("EXECUTE selectNameIDsStmt USING @paramTargetID, @paramNameID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : tmpResults) {
				if (!database.query("SET @paramNewTargetID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramNewDestinationLanguage='" + vec[1] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				database.query("EXECUTE insertNameTranslationStmt USING @paramNameID, @paramNewTargetID, @paramNewDestinationLanguage;");
				database.query("EXECUTE insertNameTranslationStmt USING @paramNewTargetID, @paramNameID, @paramSourceLanguage;");
			}
		}
		if (!msg->text.empty()) {
			if (!database.query("SET @paramTextID=" + std::to_string(msg->id) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (msg->sourceLanguage.empty()) {
				if (!database.query("EXECUTE selectTextLanguageStmt USING @paramTextID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto tmpResults = database.getResults<std::vector<std::string>>();
				msg->sourceLanguage = tmpResults[0][0];
			}
			if (!database.query("SET @paramSourceLanguage='" + msg->sourceLanguage + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE deleteTextDraftStmt USING @paramTextID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("SET @paramTranslation='" + msg->text + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertTextStmt USING @paramTranslation, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectTextIDStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			if (!database.query("SET @paramTargetID=" + tmpResults[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertTextTranslationStmt USING @paramTextID, @paramTargetID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertTextTranslationStmt USING @paramTargetID, @paramTextID, @paramSourceLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}

			if (!database.query("EXECUTE selectTextIDsStmt USING @paramTextID, @paramTargetID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : tmpResults) {
				if (!database.query("SET @paramNewTargetID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramNewDestinationLanguage='" + vec[1] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				database.query("EXECUTE insertTextTranslationStmt USING @paramTextID, @paramNewTargetID, @paramNewDestinationLanguage;");
				database.query("EXECUTE insertTextTranslationStmt USING @paramNewTargetID, @paramTextID, @paramSourceLanguage;");
			}

			if (!database.query("EXECUTE selectTextIDsStmt USING @paramTargetID, @paramTextID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : tmpResults) {
				if (!database.query("SET @paramNewTargetID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramNewDestinationLanguage='" + vec[1] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				database.query("EXECUTE insertTextTranslationStmt USING @paramTextID, @paramNewTargetID, @paramNewDestinationLanguage;");
				database.query("EXECUTE insertTextTranslationStmt USING @paramNewTargetID, @paramTextID, @paramSourceLanguage;");
			}
		}
		if (!msg->dialog.empty()) {
			if (!database.query("SET @paramDialogID=" + std::to_string(msg->id) + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (msg->sourceLanguage.empty()) {
				if (!database.query("EXECUTE selectDialogLanguageStmt USING @paramDialogID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto tmpResults = database.getResults<std::vector<std::string>>();
				msg->sourceLanguage = tmpResults[0][0];
			}
			if (!database.query("SET @paramSourceLanguage='" + msg->sourceLanguage + "';")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE deleteDialogDraftStmt USING @paramDialogID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectDialogTextsStmt USING @paramDialogID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				if (!database.query("SET @paramDialogTextID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteDialogTextDraftStmt USING @paramDialogTextID, @paramDestinationLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
			if (!database.query("EXECUTE insertDialogStmt USING @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectDialogIDStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			if (!database.query("SET @paramTargetID=" + tmpResults[0][0] + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertDialogTranslationStmt USING @paramDialogID, @paramTargetID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE insertDialogTranslationStmt USING @paramTargetID, @paramDialogID, @paramSourceLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			for (const std::string & s : msg->dialog) {
				if (!database.query("SET @paramTranslation='" + s + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE insertDialogTextStmt USING @paramTranslation, @paramTargetID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}

			if (!database.query("EXECUTE selectDialogIDsStmt USING @paramDialogID, @paramTargetID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : tmpResults) {
				if (!database.query("SET @paramNewTargetID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramNewDestinationLanguage='" + vec[1] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto b = database.query("EXECUTE insertDialogTranslationStmt USING @paramDialogID, @paramNewTargetID, @paramNewDestinationLanguage;");
				b = database.query("EXECUTE insertDialogTranslationStmt USING @paramNewTargetID, @paramDialogID, @paramSourceLanguage;");
				static_cast<void>(b);
			}

			if (!database.query("EXECUTE selectDialogIDsStmt USING @paramTargetID, @paramDialogID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			tmpResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : tmpResults) {
				if (!database.query("SET @paramNewTargetID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramNewDestinationLanguage='" + vec[1] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				auto b = database.query("EXECUTE insertDialogTranslationStmt USING @paramDialogID, @paramNewTargetID, @paramNewDestinationLanguage;");
				b = database.query("EXECUTE insertDialogTranslationStmt USING @paramNewTargetID, @paramDialogID, @paramSourceLanguage;");
				static_cast<void>(b);
			}
		}
	} while (false);
}

void TranslatorServer::handleRequestProjects(clockUtils::sockets::TcpSocket * sock, RequestProjectsMessage * msg) {
	SendProjectsMessage spm;
	do {
		const int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDStmt FROM \"SELECT RequestID FROM requestPermissions WHERE UserID = ? AND RequestID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectProjectStmt FROM \"SELECT RequestID, CAST(Name AS BINARY), CAST(SourceLanguage AS BINARY), CAST(DestinationLanguage AS BINARY) FROM translationRequests\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectProjectStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		const auto results = database.getResults<std::vector<std::string>>();
		bool isTranslator = false;
		for (const auto & vec : results) {
			std::string requestID = vec[0];
			SendProjectsMessage::Project project;
			project.projectName = vec[1];
			project.sourceLanguage = vec[2];
			project.destinationLanguage = vec[3];
			if (!database.query("SET @paramRequestID=" + requestID + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectRequestIDStmt USING @paramUserID, @paramRequestID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			std::vector<std::vector<std::string>> tmpResults = database.getResults<std::vector<std::string>>();
			project.accessRights = !tmpResults.empty();
			isTranslator = isTranslator || project.accessRights;
			spm.projects.push_back(project);
		}
		if (!isTranslator) {
			spm.projects.clear();
		}
	} while (false);
	const std::string serialized = spm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleRequestOwnProjects(clockUtils::sockets::TcpSocket * sock, RequestOwnProjectsMessage * msg) {
	SendOwnProjectsMessage sopm;
	do {
		const int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDStmt FROM \"SELECT RequestID, CAST(Name AS BINARY), CAST(SourceLanguage AS BINARY), CAST(DestinationLanguage AS BINARY) FROM translationRequests WHERE UserID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectRequestIDStmt USING @paramUserID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		const auto results = database.getResults<std::vector<std::string>>();
		if (results.empty()) {
			break;
		}
		for (const auto & vec : results) {
			const auto requestID = static_cast<uint32_t>(std::stoi(vec[0]));
			SendOwnProjectsMessage::Project project;
			project.requestID = requestID;
			project.projectName = vec[1];
			project.sourceLanguage = vec[2];
			project.destinationLanguage = vec[3];
			determineProjectProgress(requestID, vec[3], project.translated, project.toTranslate);
			sopm.projects.push_back(project);
		}
	} while (false);
	const std::string serialized = sopm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleRequestTranslators(clockUtils::sockets::TcpSocket * sock, RequestTranslatorsMessage * msg) {
	SendTranslatorsMessage stm;
	do {
		std::vector<std::string> allUsers;
		{
			MariaDBWrapper database;
			if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "master", 0)) {
				std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				break;
			}
			if (!database.query("PREPARE selectStmt FROM \"SELECT Username FROM accounts ORDER BY Username ASC\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				break;
			}
			const auto lastResults = database.getResults<std::vector<std::string>>();
			for (const auto & vec : lastResults) {
				allUsers.push_back(vec[0]);
			}
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectUsersStmt FROM \"SELECT UserID FROM requestPermissions WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramRequestID=" + std::to_string(msg->requestID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectUsersStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		const auto results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			const auto userID = std::stoi(vec[0]);
			std::string username = ServerCommon::getUsername(userID);
			if (!username.empty()) {
				stm.translators.push_back(username);
				allUsers.erase(std::remove_if(allUsers.begin(), allUsers.end(), [username](const std::string & o) { return o == username; }), allUsers.end());
			}
		}
		stm.lockedUsers = allUsers;
	} while (false);
	const std::string serialized = stm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleChangeTranslationRights(clockUtils::sockets::TcpSocket *, ChangeTranslationRightsMessage * msg) {
	do {
		const int userID = ServerCommon::getUserID(msg->username);
		if (userID == -1) {
			break;
		}
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE insertPermissionStmt FROM \"INSERT INTO requestPermissions (RequestID, UserID) VALUES (?, ?)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deletePermissionStmt FROM \"DELETE FROM requestPermissions WHERE RequestID = ? AND UserID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramUserID=" + std::to_string(userID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramRequestID=" + std::to_string(msg->requestID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (msg->accessRights) {
			if (!database.query("EXECUTE insertPermissionStmt USING @paramRequestID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		} else {
			if (!database.query("EXECUTE deletePermissionStmt USING @paramRequestID, @paramUserID;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
		}
	} while (false);
}

void TranslatorServer::handleRequestTranslationDownload(clockUtils::sockets::TcpSocket * sock, RequestTranslationDownloadMessage * msg) {
	SendTranslationDownloadMessage stdm;
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectDestinationLanguageStmt FROM \"SELECT DestinationLanguage FROM translationRequests WHERE RequestID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameRequestsStmt FROM \"SELECT NameID FROM nameRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextRequestsStmt FROM \"SELECT TextID FROM textRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogRequestsStmt FROM \"SELECT DialogID FROM dialogRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNamesStmt FROM \"SELECT NameID, Name FROM names\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsStmt FROM \"SELECT TextID, String FROM texts\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogID, DialogTextID, String FROM dialogTexts ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameTranslationsStmt FROM \"SELECT SourceID, TargetID FROM nameTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextTranslationsStmt FROM \"SELECT SourceID, TargetID FROM textTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTranslationsStmt FROM \"SELECT SourceID, TargetID FROM dialogTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNamesDeepLStmt FROM \"SELECT NameID, CAST(Translation AS BINARY) FROM nameDeepLTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsDeepLStmt FROM \"SELECT TextID, CAST(Translation AS BINARY) FROM textDeepLTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsDeepLStmt FROM \"SELECT DialogTextID, CAST(Translation AS BINARY) FROM dialogTextDeepLTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramRequestID=" + std::to_string(msg->requestID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectDestinationLanguageStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		std::vector<std::vector<std::string>> results = database.getResults<std::vector<std::string>>();
		if (results.empty()) {
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + results[0][0] + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		std::set<std::string> nameIDs;
		std::set<std::string> textIDs;
		std::set<std::string> dialogIDs;
		if (!database.query("EXECUTE selectNameRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			nameIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectTextRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			textIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectDialogRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogIDs.insert(vec[0]);
		}

		std::map<std::string, std::string> names;
		std::map<std::string, std::string> texts;
		std::map<std::string, std::vector<std::pair<std::string, std::string>>> dialogs;
		if (!database.query("EXECUTE selectNamesStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			names.insert(std::make_pair(vec[0], vec[1]));
		}

		if (!database.query("EXECUTE selectTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			texts.insert(std::make_pair(vec[0], vec[1]));
		}

		if (!database.query("EXECUTE selectDialogTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogs[vec[0]].push_back(std::make_pair(vec[1], vec[2]));
		}

		std::map<std::string, std::string> namesDeepL;
		std::map<std::string, std::string> textsDeepL;
		std::map<std::string, std::string> dialogTextsDeepL;
		if (!database.query("EXECUTE selectNamesDeepLStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			namesDeepL[vec[0]] = vec[1];
		}
		if (!database.query("EXECUTE selectTextsDeepLStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			textsDeepL[vec[0]] = vec[1];
		}
		if (!database.query("EXECUTE selectDialogTextsDeepLStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogTextsDeepL[vec[0]] = vec[1];
		}

		std::map<std::string, std::string> nameMap;
		std::map<std::string, std::string> textMap;
		std::map<std::string, std::string> dialogMap;
		if (!database.query("EXECUTE selectNameTranslationsStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			nameMap.insert(std::make_pair(vec[0], vec[1]));
		}
		for (const std::string & nameID : nameIDs) {
			auto it = nameMap.find(nameID);
			if (it == nameMap.end()) {
				stdm.names.insert(std::make_pair(names[nameID], namesDeepL[nameID]));
			} else {
				stdm.names.insert(std::make_pair(names[nameID], names[it->second]));
			}
		}

		if (!database.query("EXECUTE selectTextTranslationsStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			textMap.insert(std::make_pair(vec[0], vec[1]));
		}
		for (const std::string & textID : textIDs) {
			auto it = textMap.find(textID);
			if (it == textMap.end()) {
				stdm.texts.insert(std::make_pair(texts[textID], textsDeepL[textID]));
			} else {
				stdm.texts.insert(std::make_pair(texts[textID], texts[it->second]));
			}
		}

		if (!database.query("EXECUTE selectDialogTranslationsStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogMap.insert(std::make_pair(vec[0], vec[1]));
		}
		for (const std::string & dialogID : dialogIDs) {
			auto it = dialogMap.find(dialogID);
			if (it == dialogMap.end()) {
				std::vector<std::string> originalDialogList;
				std::vector<std::string> dialogList;
				for (const auto & p : dialogs[dialogID]) {
					originalDialogList.push_back(p.second);
					dialogList.push_back(dialogTextsDeepL[p.first]);
				}
				stdm.dialogs.emplace_back(originalDialogList, dialogList);
			} else {
				std::vector<std::string> originalDialogList;
				for (const auto & p : dialogs[dialogID]) {
					originalDialogList.push_back(p.second);
				}
				std::vector<std::string> dialogList;
				for (const auto & p : dialogs[it->second]) {
					dialogList.push_back(p.second);
				}
				stdm.dialogs.emplace_back(originalDialogList, dialogList);
			}
		}
	} while (false);
	std::string serialized = stdm.SerializePrivate();
	sock->writePacket(serialized);
}

void TranslatorServer::handleRequestCsvDownload(clockUtils::sockets::TcpSocket * sock, RequestCsvDownloadMessage * msg) {
	SendCsvDownloadMessage scdm;
	scdm.csvString = "Original\tDeepL\tTranslation\n";
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectDestinationLanguageStmt FROM \"SELECT DestinationLanguage FROM translationRequests WHERE RequestID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameRequestsStmt FROM \"SELECT NameID FROM nameRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextRequestsStmt FROM \"SELECT TextID FROM textRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogRequestsStmt FROM \"SELECT DialogID FROM dialogRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNamesStmt FROM \"SELECT NameID, Name FROM names\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsStmt FROM \"SELECT TextID, String FROM texts\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogID, DialogTextID, String FROM dialogTexts ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameTranslationsStmt FROM \"SELECT SourceID, TargetID FROM nameTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextTranslationsStmt FROM \"SELECT SourceID, TargetID FROM textTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTranslationsStmt FROM \"SELECT SourceID, TargetID FROM dialogTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNamesDeepLStmt FROM \"SELECT NameID, CAST(Translation AS BINARY) FROM nameDeepLTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsDeepLStmt FROM \"SELECT TextID, CAST(Translation AS BINARY) FROM textDeepLTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsDeepLStmt FROM \"SELECT DialogTextID, CAST(Translation AS BINARY) FROM dialogTextDeepLTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramRequestID=" + std::to_string(msg->requestID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectDestinationLanguageStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		std::vector<std::vector<std::string>> results = database.getResults<std::vector<std::string>>();
		if (results.empty()) {
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + results[0][0] + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		std::set<std::string> nameIDs;
		std::set<std::string> textIDs;
		std::set<std::string> dialogIDs;
		if (!database.query("EXECUTE selectNameRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			nameIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectTextRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			textIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectDialogRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogIDs.insert(vec[0]);
		}

		std::map<std::string, std::string> names;
		std::map<std::string, std::string> texts;
		std::map<std::string, std::vector<std::pair<std::string, std::string>>> dialogs;
		if (!database.query("EXECUTE selectNamesStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			names.insert(std::make_pair(vec[0], vec[1]));
		}

		if (!database.query("EXECUTE selectTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			texts.insert(std::make_pair(vec[0], vec[1]));
		}

		if (!database.query("EXECUTE selectDialogTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogs[vec[0]].push_back(std::make_pair(vec[1], vec[2]));
		}

		std::map<std::string, std::string> namesDeepL;
		std::map<std::string, std::string> textsDeepL;
		std::map<std::string, std::string> dialogTextsDeepL;
		if (!database.query("EXECUTE selectNamesDeepLStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			namesDeepL[vec[0]] = vec[1];
		}
		if (!database.query("EXECUTE selectTextsDeepLStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			textsDeepL[vec[0]] = vec[1];
		}
		if (!database.query("EXECUTE selectDialogTextsDeepLStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogTextsDeepL[vec[0]] = vec[1];
		}

		std::map<std::string, std::string> nameMap;
		std::map<std::string, std::string> textMap;
		std::map<std::string, std::string> dialogMap;
		if (!database.query("EXECUTE selectNameTranslationsStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			nameMap.insert(std::make_pair(vec[0], vec[1]));
		}
		for (const std::string & nameID : nameIDs) {
			scdm.csvString += names[nameID] + "\t";
			scdm.csvString += namesDeepL[nameID] + "\t";

			auto it = nameMap.find(nameID);

			scdm.csvString = (it != nameMap.end() ? names[it->second] : "") + "\n";
		}

		if (!database.query("EXECUTE selectTextTranslationsStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			textMap.insert(std::make_pair(vec[0], vec[1]));
		}
		for (const std::string & textID : textIDs) {
			scdm.csvString += texts[textID] + "\t";
			scdm.csvString += textsDeepL[textID] + "\t";

			auto it = textMap.find(textID);

			scdm.csvString = (it != textMap.end() ? texts[it->second] : "") + "\n";
		}

		if (!database.query("EXECUTE selectDialogTranslationsStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			dialogMap.insert(std::make_pair(vec[0], vec[1]));
		}
		for (const std::string & dialogID : dialogIDs) {
			int i = 0;
			auto it = dialogMap.find(dialogID);

			for (const auto & p : dialogs[dialogID]) {
				scdm.csvString += p.second + "\t";
				scdm.csvString += dialogTextsDeepL[p.first] + "\t";

				scdm.csvString = (it != dialogMap.end() ? dialogs[it->second][i].second : "") + "\n";

				i++;
			}
		}
	} while (false);
	std::string serialized = scdm.SerializePrivate();
	sock->writePacket(serialized);
}

std::vector<std::pair<std::string, std::string>> TranslatorServer::getHints(const std::string & sourceLanguage, const std::string & destinationLanguage, const std::vector<std::string> & strings) const {
	std::vector<std::pair<std::string, std::string>> hints;
	for (const auto & s : strings) {
		mergeHints(getHints(sourceLanguage, destinationLanguage, s), hints);
	}
	return hints;
}

std::string TranslatorServer::toLower(const std::string & s) const {
	std::string result;
	for (const char c : s) {
		result += static_cast<char>(::tolower(c));
	}
	return result;
}

std::vector<std::pair<std::string, std::string>> TranslatorServer::getHints(const std::string & sourceLanguage, const std::string & destinationLanguage, std::string s) const {
	s = toLower(s);

	std::vector<std::pair<std::string, std::string>> hints;

	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameTranslatedStmt FROM \"SELECT SourceID, TargetID FROM nameTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextTranslatedStmt FROM \"SELECT SourceID, TargetID FROM textTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTranslatedStmt FROM \"SELECT SourceID, TargetID FROM dialogTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNamesStmt FROM \"SELECT NameID, CAST(Name AS BINARY) FROM names WHERE Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsStmt FROM \"SELECT TextID, CAST(String AS BINARY) FROM texts WHERE Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogsStmt FROM \"SELECT DialogID FROM dialogs WHERE Language = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogID, CAST(String AS BINARY) FROM dialogTexts ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameStmt FROM \"SELECT CAST(Name AS BINARY) FROM names WHERE NameID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextStmt FROM \"SELECT CAST(String AS BINARY) FROM texts WHERE TextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramSourceLanguage='" + sourceLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectNamesStmt USING @paramSourceLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::map<int, std::string> names;
			for (const auto & vec : results) {
				names.insert(std::make_pair(std::stoi(vec[0]), vec[1]));
			}

			if (!database.query("EXECUTE selectNameTranslatedStmt USING @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				int sourceID = std::stoi(vec[0]);
				auto it = names.find(sourceID);
				if (it != names.end()) {
					std::string text = toLower(it->second);
					if (equals(s, text)) {
						if (!database.query("SET @paramNameID=" + vec[1] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE selectNameStmt USING @paramNameID;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						auto tmpResults = database.getResults<std::vector<std::string>>();
						hints.emplace_back(it->second, tmpResults[0][0]);
					}
				}
			}
		}

		if (!database.query("EXECUTE selectTextsStmt USING @paramSourceLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::map<int, std::string> texts;
			for (const auto & vec : results) {
				texts.insert(std::make_pair(std::stoi(vec[0]), vec[1]));
			}

			if (!database.query("EXECUTE selectTextTranslatedStmt USING @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				int sourceID = std::stoi(vec[0]);
				auto it = texts.find(sourceID);
				if (it != texts.end()) {
					std::string text = toLower(it->second);
					if (equals(s, text)) {
						if (!database.query("SET @paramTextID=" + vec[1] + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE selectTextStmt USING @paramTextID;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						auto tmpResults = database.getResults<std::vector<std::string>>();
						hints.emplace_back(it->second, tmpResults[0][0]);
					}
				}
			}
		}

		if (!database.query("EXECUTE selectDialogTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		if (!results.empty()) {
			std::map<int, std::vector<std::string>> dialogTexts;
			for (const auto & vec : results) {
				dialogTexts[std::stoi(vec[0])].push_back(vec[1]);
			}

			if (!database.query("EXECUTE selectDialogTranslatedStmt USING @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				int sourceID = std::stoi(vec[0]);
				auto it = dialogTexts.find(sourceID);
				if (it != dialogTexts.end()) {
					for (size_t i = 0; i < it->second.size(); i++) {
						std::string text = toLower(it->second[i]);
						if (equals(s, text)) {
							auto it2 = dialogTexts.find(std::stoi(vec[1]));
							if (it2 != dialogTexts.end() && it2->second.size() == it->second.size()) {
								hints.emplace_back(it->second[i], it2->second[i]);
							}
						}
					}
				}
			}
		}
	} while (false);

	return hints;
}

void TranslatorServer::mergeHints(const std::vector<std::pair<std::string, std::string>> & strings, std::vector<std::pair<std::string, std::string>> & hints) const {
	for (const auto & s : strings) {
		if (std::find_if(hints.begin(), hints.end(), [s](const std::pair<std::string, std::string> & s2) { return s.first == s2.first; }) == hints.end()) {
			hints.push_back(s);
		}
	}
}

bool TranslatorServer::equals(const std::string & translatedText, const std::string & newText) const {
	bool equal = false;
	if (newText.find(translatedText) != std::string::npos || translatedText.find(newText) != std::string::npos) {
		equal = true;
	} else {
		const size_t diff = levensthein(translatedText, newText);
		if (diff <= newText.length() / 4u) { // at least 25% of the text are the same?
			equal = true;
		}
	}
	return equal;
}

size_t TranslatorServer::levensthein(const std::string & a, const std::string & b) const {
	std::vector<std::vector<size_t>> D(a.length() + 1, std::vector<size_t>(b.length() + 1, 0));
	for (size_t i = 0; i < a.length() + 1; i++) {
		for (size_t j = 0; j < b.length() + 1; j++) {
			if (i == 0 && j == 0) {
				D[i][j] = 0;
			} else if (j == 0) {
				D[i][0] = i;
			} else if (i == 0) {
				D[0][j] = j;
			} else {
				unsigned int add = 0;
				if (a.at(i - 1) != b.at(j - 1)) {
					add = 1;
				}
				D[i][j] = std::min(std::min(D[i - 1][j - 1] + add, D[i][j - 1] + 1), D[i - 1][j] + 1);
			}
		}
	}
	return D[a.length()][b.length()];
}

void TranslatorServer::determineProjectProgress(uint32_t requestID, const std::string & destinationLanguage, uint32_t & translated, uint32_t & toTranslate) {
	translated = 0;
	toTranslate = 0;
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameRequestsStmt FROM \"SELECT NameID FROM nameRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextRequestsStmt FROM \"SELECT TextID FROM textRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogRequestsStmt FROM \"SELECT DialogID FROM dialogRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameTranslatedStmt FROM \"SELECT SourceID FROM nameTranslations WHERE SourceID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextTranslatedStmt FROM \"SELECT SourceID FROM textTranslations WHERE SourceID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTranslatedStmt FROM \"SELECT SourceID FROM dialogTranslations WHERE TargetLanguage = CONVERT(? USING BINARY)\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextCountStmt FROM \"SELECT COUNT(DialogTextID) FROM dialogTexts WHERE DialogID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogID FROM dialogTexts ORDER BY DialogTextID ASC\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramRequestID=" + std::to_string(requestID) + ";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("SET @paramDestinationLanguage='" + destinationLanguage + "';")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("EXECUTE selectNameRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();
		toTranslate += static_cast<uint32_t>(results.size());

		for (const auto & vec : results) {
			std::string nameID = vec[0];
			if (!database.query("SET @paramNameID=" + nameID + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectNameTranslatedStmt USING @paramNameID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			translated += static_cast<uint32_t>(tmpResults.size());
		}

		if (!database.query("EXECUTE selectTextRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		toTranslate += static_cast<uint32_t>(results.size());

		for (const auto & vec : results) {
			std::string textID = vec[0];
			if (!database.query("SET @paramTextID=" + textID + ";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("EXECUTE selectTextTranslatedStmt USING @paramTextID, @paramDestinationLanguage;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto tmpResults = database.getResults<std::vector<std::string>>();
			translated += static_cast<uint32_t>(tmpResults.size());
		}

		if (!database.query("EXECUTE selectDialogRequestsStmt USING @paramRequestID;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		size_t count = results.size();
		std::vector<int> dialogRequests(count);
		for (size_t i = 0; i < count; i++) {
			dialogRequests[i] = std::stoi(results[i][0]);
		}

		if (!database.query("EXECUTE selectDialogTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		std::map<int, int> dialogCounts;
		for (auto & vec : results) {
			int dialogID = std::stoi(vec[0]);
			dialogCounts[dialogID]++;
		}

		if (!database.query("EXECUTE selectDialogTranslatedStmt USING @paramDestinationLanguage;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		count = results.size();
		for (size_t i = 0; i < count; i++) {
			int dialogID = std::stoi(results[i][0]);
			translated += dialogCounts[dialogID];
		}

		for (int dialogID : dialogRequests) {
			toTranslate += dialogCounts[dialogID];
		}
		if (translated > toTranslate) {
			translated = toTranslate;
		}
	} while (false);
}

std::string TranslatorServer::requestDeepL(const std::string & text, const std::string & sourceLanguage, const std::string & targetLanguage) const {
	std::string result;
	std::string escapedText = std::regex_replace(text, std::regex("´"), "'");
	escapedText = std::regex_replace(escapedText, std::regex("`"), "'");

	static std::map<std::string, std::string> languageMap = {
		{ "Deutsch", "DE" },
		{ "English", "EN" },
		{ "Polish", "PL" },
		{ "Russian", "RU" }
	};

	const std::string sl = languageMap[sourceLanguage];
	const std::string tl = languageMap[targetLanguage];

	auto * client = new HttpsClient("api-free.deepl.com", false);

	std::string function = "/v2/translate?auth_key=";
	function += DEEPLKEY;
	function += "&source_lang=" + sl;
	function += "&target_lang=" + tl;
	function += "&text=text";

	client->request("POST", function, [&result](std::shared_ptr<HttpsClient::Response> response, const SimpleWeb::error_code &) {
		const std::string content = response->content.string();

		std::cout << "Received result: " << content << std::endl;

		if (response->status_code.find("200") != std::string::npos) {
			std::stringstream ss(content);

			boost::property_tree::ptree pt;
			read_json(ss, pt);

			if (pt.count("translations") == 0)
				return;

			const auto tChild = pt.get_child("translations");
			result = tChild.get<std::string>("text");
		} else {
			result = "";
		}
	});
	client->io_service->run();

	return result;
}

void TranslatorServer::createFakeTranslation() const {
	constexpr time_t COOLDOWN = 60LL * 60 * 24;
	time_t startTime = time(nullptr) - COOLDOWN;
	do {
		const time_t currentTime = time(nullptr);
		if (currentTime - startTime >= COOLDOWN) {
			MariaDBWrapper database;
			if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
				std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
				break;
			}
			if (!database.query("PREPARE selectNamesDeepLStmt FROM \"SELECT NameID, CAST(TargetLanguage AS BINARY) FROM nameDeepLTranslations\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectTextsDeepLStmt FROM \"SELECT TextID, CAST(TargetLanguage AS BINARY) FROM textDeepLTranslations\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectDialogTextsDeepLStmt FROM \"SELECT DialogTextID, CAST(TargetLanguage AS BINARY) FROM dialogTextDeepLTranslations\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectNamesStmt FROM \"SELECT NameID, CAST(Name AS BINARY), CAST(Language AS BINARY) FROM names\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectTextsStmt FROM \"SELECT TextID, CAST(String AS BINARY), CAST(Language AS BINARY) FROM texts\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogID, DialogTextID, CAST(String AS BINARY) FROM dialogTexts\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE selectDialogsStmt FROM \"SELECT DialogID, CAST(Language AS BINARY) FROM dialogs\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE insertNameStmt FROM \"INSERT INTO nameDeepLTranslations (NameID, Translation, TargetLanguage) VALUES (?, CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE insertTextStmt FROM \"INSERT INTO textDeepLTranslations (TextID, Translation, TargetLanguage) VALUES (?, CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			if (!database.query("PREPARE insertDialogTextStmt FROM \"INSERT INTO dialogTextDeepLTranslations (DialogTextID, Translation, TargetLanguage) VALUES (?, CONVERT(? USING BINARY), CONVERT(? USING BINARY))\";")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}

			std::map<int32_t, std::pair<std::string, std::string>> names;
			std::map<int32_t, std::pair<std::string, std::string>> texts;
			std::map<int32_t, std::string> dialogs;
			std::map<int32_t, std::pair<std::string, std::string>> dialogTexts;

			std::set<std::pair<int32_t, std::string>> namesDeepL;
			std::set<std::pair<int32_t, std::string>> textsDeepL;
			std::set<std::pair<int32_t, std::string>> dialogTextsDeepL;

			if (!database.query("EXECUTE selectNamesStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			auto results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				names.insert(std::make_pair(std::stoi(vec[0]), std::make_pair(vec[1], vec[2])));
			}

			if (!database.query("EXECUTE selectTextsStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				texts.insert(std::make_pair(std::stoi(vec[0]), std::make_pair(vec[1], vec[2])));
			}

			if (!database.query("EXECUTE selectDialogsStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				dialogs.insert(std::make_pair(std::stoi(vec[0]), vec[1]));
			}

			if (!database.query("EXECUTE selectDialogTextsStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				dialogTexts.insert(std::make_pair(std::stoi(vec[1]), std::make_pair(vec[2], dialogs[std::stoi(vec[0])])));
			}

			if (!database.query("EXECUTE selectNamesDeepLStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				namesDeepL.insert(std::make_pair(std::stoi(vec[0]), vec[1]));
			}

			if (!database.query("EXECUTE selectTextsDeepLStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				textsDeepL.insert(std::make_pair(std::stoi(vec[0]), vec[1]));
			}

			if (!database.query("EXECUTE selectDialogTextsDeepLStmt;")) {
				std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
				break;
			}
			results = database.getResults<std::vector<std::string>>();
			for (const auto & vec : results) {
				dialogTextsDeepL.insert(std::make_pair(std::stoi(vec[0]), vec[1]));
			}

			std::vector<std::string> languages = { "Deutsch", "English", "Polish", "Russian" };
			for (const std::string & language : languages) {
				if (!database.query("SET @paramTargetLanguage='" + language + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				for (const auto & p : names) {
					if (p.second.second == language) {
						continue;
					}
					size_t count = namesDeepL.count(std::make_pair(p.first, language));
					if (count == 0) {
						std::string name = p.second.first;
						name = std::regex_replace(name, std::regex("&apos;"), "'");

						std::string translatedName = requestDeepL(name, p.second.second, language);
						translatedName = std::regex_replace(translatedName, std::regex("'"), "&apos;");
						if (translatedName.empty()) {
							continue;
						}
						if (!database.query("SET @paramNameID=" + std::to_string(p.first) + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("SET @paramTranslation='" + translatedName + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE insertNameStmt USING @paramNameID, @paramTranslation, @paramTargetLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
					}
				}
				for (const auto & p : texts) {
					if (p.second.second == language) {
						continue;
					}
					size_t count = textsDeepL.count(std::make_pair(p.first, language));
					if (count == 0) {
						std::string text = p.second.first;
						text = std::regex_replace(text, std::regex("&apos;"), "'");

						std::string translatedText = requestDeepL(text, p.second.second, language);
						translatedText = std::regex_replace(translatedText, std::regex("'"), "&apos;");
						if (translatedText.empty()) {
							continue;
						}
						if (!database.query("SET @paramTextID=" + std::to_string(p.first) + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("SET @paramTranslation='" + translatedText + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE insertTextStmt USING @paramTextID, @paramTranslation, @paramTargetLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
					}
				}
				for (const auto & p : dialogTexts) {
					if (p.second.second == language) {
						continue;
					}
					size_t count = dialogTextsDeepL.count(std::make_pair(p.first, language));
					if (count == 0) {
						std::string dialogText = p.second.first;
						dialogText = std::regex_replace(dialogText, std::regex("&apos;"), "'");

						std::string translatedDialogText = requestDeepL(dialogText, p.second.second, language);
						translatedDialogText = std::regex_replace(translatedDialogText, std::regex("'"), "&apos;");
						if (translatedDialogText.empty()) {
							continue;
						}
						if (!database.query("SET @paramDialogTextID=" + std::to_string(p.first) + ";")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("SET @paramTranslation='" + translatedDialogText + "';")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
						if (!database.query("EXECUTE insertDialogTextStmt USING @paramDialogTextID, @paramTranslation, @paramTargetLanguage;")) {
							std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
							break;
						}
					}
				}
			}

			startTime += COOLDOWN;
		}
		std::this_thread::sleep_for(std::chrono::minutes(1));
	} while (_running);
}

void TranslatorServer::cleanup() const {
	constexpr time_t COOLDOWN = 60LL * 60 * 24;
	time_t startTime = time(nullptr) - COOLDOWN;
	do {
		const time_t currentTime = time(nullptr);
		if (currentTime - startTime >= COOLDOWN) {
			std::lock_guard<std::mutex> lg(_lock);
			removeDialogs();
			removeUnusedReferences();
			removeNewlinesFromDeepL();

			startTime += COOLDOWN;
		}
		std::this_thread::sleep_for(std::chrono::minutes(1));
	} while (_running);
}

void TranslatorServer::removeDialogs() const {
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsStmt FROM \"SELECT TextID, String FROM texts\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteTextStmt FROM \"DELETE FROM texts WHERE TextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteTextDraftStmt FROM \"DELETE FROM textDrafts WHERE SourceID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteTextRequestsStmt FROM \"DELETE FROM textRequests WHERE TextID = ? LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		const auto results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			std::string textID = vec[0];
			std::string text = vec[1];
			const size_t foundSpaces = std::count(text.begin(), text.end(), ' ');
			const size_t foundUnderScores = std::count(text.begin(), text.end(), '_');
			if (foundSpaces == 0 && foundUnderScores >= 2) {
				std::cout << text << std::endl;
				if (!database.query("SET @paramTextID=" + textID + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteTextStmt USING @paramTextID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteTextDraftStmt USING @paramTextID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteTextRequestsStmt USING @paramTextID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
		}
	} while (false);
}

void TranslatorServer::removeUnusedReferences() const {
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectRequestIDsStmt FROM \"SELECT RequestID FROM translationRequests\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectNameRequestIDsStmt FROM \"SELECT DISTINCT RequestID FROM nameRequests\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextRequestIDsStmt FROM \"SELECT DISTINCT RequestID FROM textRequests\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogRequestIDsStmt FROM \"SELECT DISTINCT RequestID FROM dialogRequests\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteNameRequestsStmt FROM \"DELETE FROM nameRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteTextRequestsStmt FROM \"DELETE FROM textRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE deleteDialogeRequestsStmt FROM \"DELETE FROM dialogRequests WHERE RequestID = ?\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectRequestIDsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();
		std::set<std::string> existingRequestIDs;
		std::set<std::string> foundRequestIDs;

		for (const auto & vec : results) {
			existingRequestIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectNameRequestIDsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			foundRequestIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectTextRequestIDsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			foundRequestIDs.insert(vec[0]);
		}

		if (!database.query("EXECUTE selectDialogRequestIDsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			foundRequestIDs.insert(vec[0]);
		}

		for (const std::string & requestID : foundRequestIDs) {
			if (existingRequestIDs.count(requestID) == 0) {
				if (!database.query("SET @paramRequestID=" + requestID + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteNameRequestsStmt USING @paramRequestID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteTextRequestsStmt USING @paramRequestID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE deleteDialogeRequestsStmt USING @paramRequestID;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
		}
	} while (false);
}

void TranslatorServer::removeNewlinesFromDeepL() const {
	do {
		MariaDBWrapper database;
		if (!database.connect("localhost", DATABASEUSER, DATABASEPASSWORD, "translator", 0)) {
			std::cout << "Couldn't connect to database: " << __LINE__ << /*" " << database.getLastError() <<*/ std::endl;
			break;
		}
		if (!database.query("PREPARE selectNamesStmt FROM \"SELECT NameID, CAST(Translation AS BINARY), CAST(TargetLanguage AS BINARY) FROM nameDeepLTranslations\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectTextsStmt FROM \"SELECT TextID, CAST(Translation AS BINARY), CAST(TargetLanguage AS BINARY) FROM textDeepLTranslations\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE selectDialogTextsStmt FROM \"SELECT DialogTextID, CAST(Translation AS BINARY), CAST(TargetLanguage AS BINARY) FROM dialogTextDeepLTranslations\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateNameStmt FROM \"UPDATE nameDeepLTranslations SET Translation = CONVERT(? USING BINARY) WHERE NameID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateTextStmt FROM \"UPDATE textDeepLTranslations SET Translation = CONVERT(? USING BINARY) WHERE TextID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		if (!database.query("PREPARE updateDialogTextStmt FROM \"UPDATE dialogTextDeepLTranslations SET Translation = CONVERT(? USING BINARY) WHERE DialogTextID = ? AND TargetLanguage = CONVERT(? USING BINARY) LIMIT 1\";")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}

		if (!database.query("EXECUTE selectNamesStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		auto results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			std::string translation = vec[1];
			bool changed = false;
			while (translation.back() == '\n') {
				translation = translation.substr(0, translation.size() - 1);
				changed = true;
			}
			if (changed) {
				if (!database.query("SET @paramNameID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramName='" + translation + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramLanguage='" + vec[2] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE updateNameStmt USING @paramNameID, @paramName, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
		}

		if (!database.query("EXECUTE selectTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			std::string translation = vec[1];
			bool changed = false;
			while (translation.back() == '\n') {
				translation = translation.substr(0, translation.size() - 1);
				changed = true;
			}
			if (changed) {
				if (!database.query("SET @paramTextID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramText='" + translation + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramLanguage='" + vec[2] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE updateTextStmt USING @paramTextID, @paramText, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
		}

		if (!database.query("EXECUTE selectDialogTextsStmt;")) {
			std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
			break;
		}
		results = database.getResults<std::vector<std::string>>();
		for (const auto & vec : results) {
			std::string translation = vec[1];
			bool changed = false;
			while (translation.back() == '\n') {
				translation = translation.substr(0, translation.size() - 1);
				changed = true;
			}
			if (changed) {
				if (!database.query("SET @paramDialogTextID=" + vec[0] + ";")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramDialogText='" + translation + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("SET @paramLanguage='" + vec[2] + "';")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
				if (!database.query("EXECUTE updateDialogTextStmt USING @paramDialogTextID, @paramDialogText, @paramLanguage;")) {
					std::cout << "Query couldn't be started: " << __LINE__ << std::endl;
					break;
				}
			}
		}
	} while (false);
}
