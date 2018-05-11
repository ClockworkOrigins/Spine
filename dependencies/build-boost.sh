#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

ARCHIVE="boost_1_62_0.tar.bz2"
BUILD_DIR="${BUILD_ROOT}/boost_1_62_0"
PREFIX="${DEP_OUT_DIR}/boost/"
PREFIX=$(printf %q "$PREFIX")

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile Boost"

status "Extracting Boost"

downloadAndUnpack ${ARCHIVE}

status "Configuring Boost"

cd "${BUILD_DIR}"
./bootstrap.sh  --prefix="${PREFIX}" --with-libraries=date_time,filesystem,iostreams,regex,serialization,system,thread

status "Building & Installing Boost"

./bjam -d2 \
	--toolset=${C_COMPILER} \
	cxxflags=-fPIC \
	-j ${CPU_CORES} \
	variant=release \
	--layout=system \
	threading=multi \
	link=static \
	install >/dev/null

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

