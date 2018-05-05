#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="translator.rar"
BUILD_DIR="${BUILD_ROOT}/translator"
PREFIX="${DEP_OUT_DIR}/translator/"

if [ -d ${PREFIX} ]; then
	exit 0
fi

title "Compile Translator"

status "Extracting Translator"

downloadAndUnpack ${ARCHIVE}

status "Configuring Translator"

cd "${BUILD_DIR}"

cp "${DEP_DIR}/../ext/RSAKey.h" include/translator/common/

cmake \
	-DWITH_CLIENT=OFF \
	-DWITH_SERVER=OFF \
	-DTRANSLATOR_DEP_DIR="${DEP_OUT_DIR}" \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
	.

status "Building Translator"

make -j ${CPU_CORES}

status "Installing Translator"

make install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

