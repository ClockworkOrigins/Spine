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

#include "TranslatorAPI.h"

#include "SpineServerConfig.h"

#include "common/MessageStructs.h"
#include "common/TranslationModel.h"

#include "clockUtils/sockets/TcpSocket.h"

using namespace spine::common;
using namespace spine::translation;

SendProjectsMessage * TranslatorAPI::getProjects(const std::string & username) {
	RequestProjectsMessage rpm;
	rpm.username = username;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = rpm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDPROJECTS) {
					auto * spm = dynamic_cast<SendProjectsMessage *>(msg);
					return spm;
				} else {
					return nullptr;
				}
			} catch (...) {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

TranslationModel * TranslatorAPI::createModel(const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage) {
	return new TranslationModel(projectName, sourceLanguage, destinationLanguage);
}

uint32_t TranslatorAPI::requestTranslation(const std::string & username, TranslationModel * model) {
	TranslationRequestMessage trm;
	trm.username = username;
	trm.translationModel = model;
	clockUtils::sockets::TcpSocket sock;
	uint32_t id = UINT32_MAX;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = trm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::TRANSLATIONREQUESTED) {
					const auto * trqm = dynamic_cast<TranslationRequestedMessage *>(msg);
					id = trqm->id;
					delete trqm;
				}
			} catch (...) {
			}
		}
	}
	return id;
}

SendTextToTranslateMessage * TranslatorAPI::requestTextToTranslate(const std::string & username, const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage) {
	QueryTextToTranslateMessage qtttm;
	qtttm.username = username;
	qtttm.projectName = projectName;
	qtttm.sourceLanguage = sourceLanguage;
	qtttm.destinationLanguage = destinationLanguage;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = qtttm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDTEXTTOTRANSLATE) {
					auto * stttm = dynamic_cast<SendTextToTranslateMessage *>(msg);
					return stttm;
				} else {
					return nullptr;
				}
			} catch (...) {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

bool TranslatorAPI::sendTranslationDraft(const std::string & username, const std::string & destinationLanguage, const std::string & name, const std::string & text, const std::vector<std::string> & dialog, uint32_t id) {
	SendTranslationDraftMessage stdm;
	stdm.username = username;
	stdm.destinationLanguage = destinationLanguage;
	stdm.name = name;
	stdm.text = text;
	stdm.dialog = dialog;
	stdm.id = id;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = stdm.SerializePublic();
		return sock.writePacket(serialized) == clockUtils::ClockError::SUCCESS;
	}
	return false;
}

SendTextToReviewMessage * TranslatorAPI::requestTextToReview(const std::string & username, const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage) {
	QueryTextToReviewMessage qttrm;
	qttrm.username = username;
	qttrm.projectName = projectName;
	qttrm.sourceLanguage = sourceLanguage;
	qttrm.destinationLanguage = destinationLanguage;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = qttrm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDTEXTTOREVIEW) {
					auto * sttrm = dynamic_cast<SendTextToReviewMessage *>(msg);
					return sttrm;
				} else {
					return nullptr;
				}
			} catch (...) {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

bool TranslatorAPI::sendTranslationReview(const std::string & username, const std::string & sourceLanguage, const std::string & destinationLanguage, const std::string & name, const std::string & text, const std::vector<std::string> & dialog, uint32_t id, bool changed) {
	SendTranslationReviewMessage strm;
	strm.username = username;
	strm.destinationLanguage = sourceLanguage;
	strm.destinationLanguage = destinationLanguage;
	strm.name = name;
	strm.text = text;
	strm.dialog = dialog;
	strm.id = id;
	strm.changed = changed;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		const std::string serialized = strm.SerializePublic();
		return sock.writePacket(serialized) == clockUtils::ClockError::SUCCESS;
	}
	return false;
}

std::pair<uint32_t, uint32_t> TranslatorAPI::requestTranslationProgress(const std::string & projectName, const std::string & sourceLanguage, const std::string & destinationLanguage) {
	RequestTranslationProgressMessage rtpm;
	rtpm.projectName = projectName;
	rtpm.sourceLanguage = sourceLanguage;
	rtpm.destinationLanguage = destinationLanguage;
	std::pair<uint32_t, uint32_t> progress;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = rtpm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDTRANSLATIONPROGRESS) {
					auto * stpm = dynamic_cast<SendTranslationProgressMessage *>(msg);
					progress = std::make_pair(stpm->translated, stpm->toTranslate);
					delete stpm;
				}
			} catch (...) {
			}
		}
	}
	return progress;
}

SendOwnProjectsMessage * TranslatorAPI::requestOwnProjects(const std::string & username) {
	RequestOwnProjectsMessage ropm;
	ropm.username = username;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = ropm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDOWNPROJECTS) {
					auto * sopm = dynamic_cast<SendOwnProjectsMessage *>(msg);
					return sopm;
				} else {
					return nullptr;
				}
			} catch (...) {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

SendTranslatorsMessage * TranslatorAPI::requestTranslators(uint32_t requestID) {
	RequestTranslatorsMessage rtm;
	rtm.requestID = requestID;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = rtm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDTRANSLATORS) {
					auto * stm = dynamic_cast<SendTranslatorsMessage *>(msg);
					return stm;
				} else {
					return nullptr;
				}
			} catch (...) {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}

void TranslatorAPI::changeTranslatorRights(uint32_t requestID, const std::string & username, bool accessRights) {
	ChangeTranslationRightsMessage ctrm;
	ctrm.username = username;
	ctrm.requestID = requestID;
	ctrm.accessRights = accessRights;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		const std::string serialized = ctrm.SerializePublic();
		sock.writePacket(serialized);
	}
}

SendTranslationDownloadMessage * TranslatorAPI::requestTranslationDownload(uint32_t requestID) {
	RequestTranslationDownloadMessage rtdm;
	rtdm.requestID = requestID;
	clockUtils::sockets::TcpSocket sock;
	if (clockUtils::ClockError::SUCCESS == sock.connectToHostname("clockwork-origins.de", TRANSLATORSERVER_PORT, 10000)) {
		std::string serialized = rtdm.SerializePublic();
		sock.writePacket(serialized);
		if (clockUtils::ClockError::SUCCESS == sock.receivePacket(serialized)) {
			try {
				Message * msg = Message::DeserializePublic(serialized);
				if (msg && msg->type == MessageType::SENDTRANSLATIONDOWNLOAD) {
					auto * stdm = dynamic_cast<SendTranslationDownloadMessage *>(msg);
					return stdm;
				} else {
					return nullptr;
				}
			} catch (...) {
				return nullptr;
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}
