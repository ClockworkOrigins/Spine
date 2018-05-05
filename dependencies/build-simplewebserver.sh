#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="Simple-Web-Server-3.0.0-rc2.tar.gz"
BUILD_DIR="${BUILD_ROOT}/Simple-Web-Server-3.0.0-rc2"
PREFIX="${DEP_OUT_DIR}/SimpleWebServer/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile SimpleWebServer"

status "Extracting SimpleWebServer"

downloadAndUnpack ${ARCHIVE}

status "Configuring SimpleWebServer"

cd "${BUILD_DIR}"
cmake \
	-DBoost_DIR="${DEP_OUT_DIR}/boost" \
	-DBoost_INCLUDE_DIR="${DEP_OUT_DIR}/boost/include" \
	-DOPENSSL_ROOT_DIR="${DEP_OUT_DIR}/openssl" \
	-DOPENSSL_INCLUDE_DIR="${DEP_OUT_DIR}/openssl/include" \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
	.

status "Building SimpleWebServer"

make -j ${CPU_CORES}

status "Installing SimpleWebServer"

make install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

