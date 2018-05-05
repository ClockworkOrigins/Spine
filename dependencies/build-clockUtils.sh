#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="clockutils-1.2.0-src.zip"
BUILD_DIR="${BUILD_ROOT}/clockutils-1.2.0-src"
PREFIX="${DEP_OUT_DIR}/clockUtils/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile clockUtils"

status "Extracting clockUtils"

downloadAndUnpack ${ARCHIVE} http://clockwork-origins.de/clockUtils/downloads/

status "Configuring clockUtils"

cd "${BUILD_DIR}"
cmake \
	-DWITH_TESTING=OFF \
	-DWITH_LIBRARY_ARGPARSER=OFF \
	-DWITH_LIBRARY_COMPRESSION=ON \
	-DWITH_LIBRARY_CONTAINER=OFF \
	-DWITH_LIBRARY_INIPARSER=OFF \
	-DWITH_LIBRARY_LOG=ON \
	-DWITH_LIBRARY_SOCKETS=ON \
	-DCLOCKUTILS_BUILD_SHARED=OFF \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
	.

status "Building clockUtils"

make -j ${CPU_CORES}

status "Installing clockUtils"

make install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

