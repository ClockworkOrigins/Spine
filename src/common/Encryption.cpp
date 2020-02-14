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

#include "common/Encryption.h"

#include <algorithm>
#include <mutex>
#include <sstream>

#include "common/RSAKey.h"

#ifdef WIN32
	#include "openssl/applink.c"
#endif
#include "openssl/pem.h"

using namespace common;
using namespace spine::common;

bool Encryption::encryptPublic(const std::string & in, std::string & out) {
	int completeLength = int(in.length());

	if (completeLength == 0) {
		return false;
	}
	RSA * publicKey = nullptr;
	BIO * bufio = BIO_new_mem_buf(reinterpret_cast<void *>(RSA_PUB_KEY), -1);
	PEM_read_bio_RSA_PUBKEY(bufio, &publicKey, nullptr, nullptr);
	BIO_free(bufio);

	int length = 0;
	const int packetSize = RSA_size(publicKey) - 42;
	const int determinedSize = static_cast<int>(((in.length() - 1) / packetSize + 1) * RSA_size(publicKey));
	unsigned char * arr = new unsigned char[determinedSize];
	for (size_t i = 0; i < (in.length() - 1) / packetSize + 1; i++) {
		length += RSA_public_encrypt(std::min(packetSize, completeLength), reinterpret_cast<const unsigned char *>(in.c_str()) + i * packetSize, arr + length, publicKey, RSA_PKCS1_OAEP_PADDING);
		completeLength -= std::min(packetSize, completeLength);
		if (length == -1) {
			delete[] arr;
			RSA_free(publicKey);
			return false;
		}
	}
	if (length == 0) {
		delete[] arr;
		RSA_free(publicKey);
		return false;
	}
	out = std::string(reinterpret_cast<char *>(arr), length);
	delete[] arr;
	RSA_free(publicKey);
	return true;
}

bool Encryption::encryptPrivate(const std::string & in, std::string & out) {
	int completeLength = int(in.length());

	if (completeLength == 0) {
		return false;
	}
	RSA * privateKey = nullptr;
	FILE * f = fopen(SPINE_PRIVATE_KEY, "r");
	if (f != nullptr) {
		PEM_read_RSAPrivateKey(f, &privateKey, nullptr, nullptr);
		fclose(f);
	}

	int length = 0;
	const int packetSize = RSA_size(privateKey) - 12;
	const int determinedSize = static_cast<int>(((in.length() - 1) / packetSize + 1) * RSA_size(privateKey));
	unsigned char * arr = new unsigned char[determinedSize];
	for (size_t i = 0; i < (in.length() - 1) / packetSize + 1; i++) {
		length += RSA_private_encrypt(std::min(packetSize, completeLength), reinterpret_cast<const unsigned char *>(in.c_str()) + i * packetSize, arr + length, privateKey, RSA_PKCS1_PADDING);
		completeLength -= std::min(packetSize, completeLength);
		if (length == -1) {
			delete[] arr;
			RSA_free(privateKey);
			return false;
		}
	}
	if (length == 0) {
		delete[] arr;
		RSA_free(privateKey);
		return false;
	}
	out = std::string(reinterpret_cast<char *>(arr), length);
	delete[] arr;
	RSA_free(privateKey);
	return true;
}

bool Encryption::decryptPublic(const std::string & in, std::string & out) {
	int completeLength = int(in.length());

	if (completeLength == 0) {
		return false;
	}
	RSA * publicKey = nullptr;
	BIO * bufio = BIO_new_mem_buf(reinterpret_cast<void *>(RSA_PUB_KEY), -1);
	PEM_read_bio_RSA_PUBKEY(bufio, &publicKey, nullptr, nullptr);
	BIO_free(bufio);

	int length = 0;
	const int packetSize = RSA_size(publicKey);
	if (packetSize == 0 || completeLength % packetSize != 0) {
		RSA_free(publicKey);
		return false;
	}
	const int determinedSize = static_cast<int>(((in.length() - 1) / packetSize + 1) * RSA_size(publicKey));
	unsigned char * arr = new unsigned char[determinedSize];
	for (size_t i = 0; i < (in.length() - 1) / packetSize + 1; i++) {
		length += RSA_public_decrypt(std::min(packetSize, completeLength), reinterpret_cast<const unsigned char *>(in.c_str()) + i * packetSize, arr + length, publicKey, RSA_PKCS1_PADDING);
		completeLength -= std::min(packetSize, completeLength);
		if (length == -1) {
			delete[] arr;
			RSA_free(publicKey);
			return false;
		}
	}
	if (length == 0) {
		delete[] arr;
		RSA_free(publicKey);
		return false;
	}
	out = std::string(reinterpret_cast<char *>(arr), length);
	delete[] arr;
	RSA_free(publicKey);
	return true;
}

bool Encryption::decryptPrivate(const std::string & in, std::string & out) {
	int completeLength = int(in.length());

	if (completeLength == 0) {
		return false;
	}

	RSA * privateKey = nullptr;
	FILE * f = fopen(SPINE_PRIVATE_KEY, "r");
	if (f != nullptr) {
		PEM_read_RSAPrivateKey(f, &privateKey, nullptr, nullptr);
		fclose(f);
	}
	int length = 0;
	const int packetSize = RSA_size(privateKey);
	if (packetSize == 0 || completeLength % packetSize != 0) {
		RSA_free(privateKey);
		return false;
	}
	const int determinedSize = static_cast<int>(((in.length() - 1) / packetSize + 1) * RSA_size(privateKey));
	unsigned char * arr = new unsigned char[determinedSize];
	for (size_t i = 0; i < (in.length() - 1) / packetSize + 1; i++) {
		length += RSA_private_decrypt(std::min(packetSize, completeLength), reinterpret_cast<const unsigned char *>(in.c_str()) + i * packetSize, arr + length, privateKey, RSA_PKCS1_OAEP_PADDING);
		completeLength -= std::min(packetSize, completeLength);
		if (length == -1) {
			delete[] arr;
			RSA_free(privateKey);
			return false;
		}
	}
	if (length == 0) {
		delete[] arr;
		RSA_free(privateKey);
		return false;
	}
	out = std::string(reinterpret_cast<char *>(arr), length);
	delete[] arr;
	RSA_free(privateKey);
	return true;
}
