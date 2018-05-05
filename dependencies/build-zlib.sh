#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="zlib-1.2.10.tar.xz"
BUILD_DIR="${BUILD_ROOT}/zlib-1.2.10"
PREFIX="${DEP_OUT_DIR}/zlib/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile zlib"

status "Extracting zlib"

downloadAndUnpack ${ARCHIVE}

status "Configuring zlib"

cd "${BUILD_DIR}"
cmake \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
.

status "Building zlib"

make -j ${CPU_CORES}

status "Installing zlib"

make install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

