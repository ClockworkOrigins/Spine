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
// Copyright 2018 Clockwork Origins

#include "common/MessageStructs.h"

#include <sstream>

#include "common/Encryption.h"

using namespace spine::common;

std::string Message::SerializeBlank() const {
	std::stringstream ss;
	boost::archive::text_oarchive arch(ss);
	auto * m = const_cast<Message *>(this);
	try {
		arch << m;
	} catch (...) {
	}
	return ss.str();
}

Message * Message::DeserializeBlank(const std::string & s) {
	Message * m = nullptr;
	std::stringstream ss(s);
	try {
		boost::archive::text_iarchive arch(ss);
		arch >> m;
	} catch (...) {
	}
	return m;
}

std::string Message::SerializePublic() const {
	std::stringstream ss;
	boost::archive::text_oarchive arch(ss);
	auto * m = const_cast<Message *>(this);
	try {
		arch << m;
	} catch (...) {
	}
	std::string encrypted;
	Encryption::encryptPublic(ss.str(), encrypted);
	return encrypted;
}

Message * Message::DeserializePublic(const std::string & s) {
	Message * m = nullptr;
	std::string decrypted;
	if (Encryption::decryptPublic(s, decrypted)) {
		std::stringstream ss(decrypted);
		try {
			boost::archive::text_iarchive arch(ss);
			arch >> m;
		} catch (...) {
		}
	}
	return m;
}

std::string Message::SerializePrivate() const {
	std::stringstream ss;
	boost::archive::text_oarchive arch(ss);
	auto * m = const_cast<Message *>(this);
	try {
		arch << m;
	} catch (...) {
	}
	std::string encrypted;
	Encryption::encryptPrivate(ss.str(), encrypted);
	return encrypted;
}

Message * Message::DeserializePrivate(const std::string & s) {
	Message * m = nullptr;
	std::string decrypted;
	if (Encryption::decryptPrivate(s, decrypted)) {
		std::stringstream ss(decrypted);
		try {
			boost::archive::text_iarchive arch(ss);
			arch >> m;
		} catch (...) {
		}
	}
	return m;
}

BOOST_CLASS_EXPORT_GUID(spine::common::Message, "0")
BOOST_CLASS_IMPLEMENTATION(spine::common::Message, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateRequestMessage, "1")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateRequestMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateFileCountMessage, "2")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateFileCountMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateFileHeaderMessage, "3")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateFileHeaderMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateFileMessage, "4")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateFileMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestAllModsMessage, "5")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestAllModsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateAllModsMessage, "6")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateAllModsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestModFilesMessage, "7")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestModFilesMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::ListModFilesMessage, "8")
BOOST_CLASS_IMPLEMENTATION(spine::common::ListModFilesMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::DownloadSucceededMessage, "9")
BOOST_CLASS_IMPLEMENTATION(spine::common::DownloadSucceededMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateDownloadSizesMessage, "11")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateDownloadSizesMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestPlayTimeMessage, "12")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestPlayTimeMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendPlayTimeMessage, "13")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendPlayTimeMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestUsernameMessage, "14")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestUsernameMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendUsernameMessage, "15")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendUsernameMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestScoresMessage, "16")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestScoresMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendScoresMessage, "17")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendScoresMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateScoreMessage, "18")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateScoreMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestAchievementsMessage, "19")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestAchievementsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendAchievementsMessage, "20")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendAchievementsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UnlockAchievementMessage, "21")
BOOST_CLASS_IMPLEMENTATION(spine::common::UnlockAchievementMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SearchMatchMessage, "24")
BOOST_CLASS_IMPLEMENTATION(spine::common::SearchMatchMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::FoundMatchMessage, "25")
BOOST_CLASS_IMPLEMENTATION(spine::common::FoundMatchMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateFilesMessage, "29")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateFilesMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestPackageFilesMessage, "31")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestPackageFilesMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdatePackageListMessage, "32")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdatePackageListMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::PackageDownloadSucceededMessage, "33")
BOOST_CLASS_IMPLEMENTATION(spine::common::PackageDownloadSucceededMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestAllNewsMessage, "42")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestAllNewsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendAllNewsMessage, "43")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendAllNewsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestOverallSavePathMessage, "44")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestOverallSavePathMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendOverallSavePathMessage, "45")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendOverallSavePathMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::AckMessage, "52")
BOOST_CLASS_IMPLEMENTATION(spine::common::AckMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateAchievementProgressMessage, "56")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateAchievementProgressMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestOverallSaveDataMessage, "66")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestOverallSaveDataMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendOverallSaveDataMessage, "67")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendOverallSaveDataMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateOverallSaveDataMessage, "68")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateOverallSaveDataMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::VibrateGamepadMessage, "69")
BOOST_CLASS_IMPLEMENTATION(spine::common::VibrateGamepadMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::GamepadEnabledMessage, "70")
BOOST_CLASS_IMPLEMENTATION(spine::common::GamepadEnabledMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::GamepadActiveMessage, "71")
BOOST_CLASS_IMPLEMENTATION(spine::common::GamepadActiveMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::GamepadButtonStateMessage, "72")
BOOST_CLASS_IMPLEMENTATION(spine::common::GamepadButtonStateMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::GamepadTriggerStateMessage, "73")
BOOST_CLASS_IMPLEMENTATION(spine::common::GamepadTriggerStateMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::GamepadStickStateMessage, "74")
BOOST_CLASS_IMPLEMENTATION(spine::common::GamepadStickStateMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::GamepadRawModeMessage, "75")
BOOST_CLASS_IMPLEMENTATION(spine::common::GamepadRawModeMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UploadModfilesMessage, "78")
BOOST_CLASS_IMPLEMENTATION(spine::common::UploadModfilesMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::RequestAllFriendsMessage, "88")
BOOST_CLASS_IMPLEMENTATION(spine::common::RequestAllFriendsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendAllFriendsMessage, "89")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendAllFriendsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UpdateChapterStatsMessage, "99")
BOOST_CLASS_IMPLEMENTATION(spine::common::UpdateChapterStatsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::IsAchievementUnlockedMessage, "101")
BOOST_CLASS_IMPLEMENTATION(spine::common::IsAchievementUnlockedMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::SendAchievementUnlockedMessage, "102")
BOOST_CLASS_IMPLEMENTATION(spine::common::SendAchievementUnlockedMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UploadAchievementIconsMessage, "103")
BOOST_CLASS_IMPLEMENTATION(spine::common::UploadAchievementIconsMessage, boost::serialization::object_serializable)
BOOST_CLASS_EXPORT_GUID(spine::common::UploadScreenshotsMessage, "104")
BOOST_CLASS_IMPLEMENTATION(spine::common::UploadScreenshotsMessage, boost::serialization::object_serializable)
