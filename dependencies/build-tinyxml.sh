#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="tinyxml2-4.0.1.zip"
BUILD_DIR="${BUILD_ROOT}/tinyxml2-4.0.1"
PREFIX="${DEP_OUT_DIR}/tinyxml2/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile tinyxml2"

status "Extracting tinyxml2"

downloadAndUnpack ${ARCHIVE}

status "Configuring tinyxml2"

cd "${BUILD_DIR}"
cmake \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_STATIC_LIBS=OFF \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
	.

status "Building tinyxml2"

make -j ${CPU_CORES}

status "Installing tinyxml2"

make install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

