#!/bin/bash

set -e

# make sure this doesn't get called directly
if [ "`basename "${0}"`" == "build-common.sh" ]; then
	echo "This script should not be called directly. Please execute build-dependencies.sh instead"
	exit 1
fi

# shared variables
DEP_DIR="${PWD}"
PATCH_DIR="${PWD}/../ext/patches"
BUILD_ROOT="/tmp/`whoami`/Spine"
CPU_CORES=`grep -c processor /proc/cpuinfo`

mkdir -p "${BUILD_ROOT}"

if [ "${1}" = "gcc" ] ; then
	C_COMPILER=gcc
	CXX_COMPILER=g++
	DEP_OUT_DIR=${DEP_DIR}/gcc
fi
if [ "${1}" = "gcc-4.7" ] ; then
	C_COMPILER=gcc-4.7
	CXX_COMPILER=g++-4.7
	DEP_OUT_DIR=${DEP_DIR}/gcc-4.7
fi
if [ "${1}" = "clang" ] ; then
	C_COMPILER=clang
	CXX_COMPILER=clang++
	DEP_OUT_DIR=${DEP_DIR}/clang
fi

# print titles
title() {
	text="$1"
	echo
	echo
	echo "${text}"
	echo
	echo
}

# print status text
status() {
	text="${1}"
	echo "	${text}"
}

# downloads and unpacks a dependency
downloadAndUnpack() {
	FILE=${1}
	URL=http://www.clockwork-origins.de/dependencies/
	if [ -n "$2" ]; then
		URL=${2}
	fi

	if ! [ -f "${BUILD_ROOT}/${FILE}" ]; then
		wget ${URL}/${FILE} -P ${BUILD_ROOT}
	fi
	FILENAME=$(basename "${FILE}")
	EXTENSION="${FILENAME##*.}"
	
	cd "${BUILD_ROOT}"
	if [ "${EXTENSION}" == "zip" ]; then
		unzip "${FILE}"
	fi
	if [ "${EXTENSION}" == "gz" ]; then
		tar xfz "${FILE}"
	fi
	if [ "${EXTENSION}" == "bz2" ]; then
		tar xfj "${FILE}"
	fi
	if [ "${EXTENSION}" == "xz" ]; then
		tar xJf "${FILE}"
	fi
	if [ "${EXTENSION}" == "rar" ]; then
		unrar x "${FILE}"
	fi
}

