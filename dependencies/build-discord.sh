#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="discord_game_sdk.zip"
BUILD_DIR="${BUILD_ROOT}/"
PREFIX="${DEP_OUT_DIR}/Discord/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile Discord"

status "Extracting Discord"

rm -rf "${BUILD_DIR}"

downloadAndUnpack ${ARCHIVE} https://dl-game-sdk.discordapp.net/latest

mkdir -p "${PREFIX}"
mkdir -p "${PREFIX}/include"
mkdir -p "${PREFIX}/lib"

cp ${BUILD_DIR}cpp/*.h "${PREFIX}/include/"
cp ${BUILD_DIR}cpp/*.cpp "${PREFIX}/src/"
cp ${BUILD_DIR}lib/x86_64/discord_game_sdk.so "${PREFIX}/lib/"

status "Cleaning up"

cd "${DEP_DIR}"
#rm -rf "${BUILD_ROOT}"

