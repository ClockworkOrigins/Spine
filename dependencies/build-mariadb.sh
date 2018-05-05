#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="mariadb-connector-c-2.3.1.tar.gz"
BUILD_DIR="${BUILD_ROOT}/mariadb-connector-c-2.3.1"
PREFIX="${DEP_OUT_DIR}/mariadb"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile MariaDB"

status "Extracting MariaDB"

downloadAndUnpack ${ARCHIVE}

status "Configuring MariaDB"

cd "${BUILD_DIR}"
cmake \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
.

status "Building MariaDB"

make -j ${CPU_CORES}

status "Installing MariaDB"

make install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"
cd "${PREFIX}"
cp lib/mariadb/* lib/
cd "${DEP_DIR}"

