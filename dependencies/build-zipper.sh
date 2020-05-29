#!/bin/bash

cd "$(readlink "$(dirname "${0}")")"

. ./build-common.sh ${1}

BUILD_DIR="${BUILD_ROOT}/zipper"
PREFIX="${DEP_OUT_DIR}/zipper/"

if [ -d "${PREFIX}" ]; then
	exit 0
fi

title "Compile zipper"

status "Cloning zipper"

if [ ! -d "${BUILD_ROOT}" ]; then
	mkdir ${BUILD_ROOT}
fi

pushd ${BUILD_ROOT}

git clone --recursive https://github.com/sebastiandev/zipper.git zipper

popd

status "Configuring zipper"

cd "${BUILD_DIR}"
cmake \
	-DBUILD_SHARED_VERSION=OFF \
	-DBUILD_STATIC_VERSION=ON \
	-DBUILD_TEST=OFF \
	-DBUILD_TESTING=OFF \
	-DLIBZ_INCLUDE_DIR=${DEP_OUT_DIR}/zlib/include \
	-DLIBZ_LIBRARY=${DEP_OUT_DIR}/zlib/lib/libz.so \
	-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=${C_COMPILER} \
	-DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
	.

status "Building zipper"

cmake --build . --config Release -- -j

status "Installing zipper"

cmake --build . --config Release --target install

status "Cleaning up"

cd "${DEP_DIR}"
rm -rf "${BUILD_ROOT}"

