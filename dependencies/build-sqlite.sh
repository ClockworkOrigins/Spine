#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="sqlite-amalgamation-3150100.zip"
BUILD_DIR="${BUILD_ROOT}/sqlite-amalgamation-3150100"
PREFIX="${DEP_OUT_DIR}/sqlite/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile SQLite"

status "Extracting SQLite"

downloadAndUnpack ${ARCHIVE}

status "Building SQLite"

cd "${BUILD_DIR}"
${C_COMPILER} -O3 -DNDEBUG -o libsqlite3.so sqlite3.c -shared -lpthread -ldl -fPIC
${C_COMPILER} -O3 -DNDEBUG -o sqlite3 sqlite3.c shell.c -lpthread -ldl

status "Installing SQLite"

mkdir -p "${PREFIX}/include"
mkdir -p "${PREFIX}/bin"
mkdir -p "${PREFIX}/lib"

cp ${BUILD_DIR}/*.h "${PREFIX}/include"
cp ${BUILD_DIR}/*.so "${PREFIX}/lib"
cp sqlite3 "${PREFIX}/bin"

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

