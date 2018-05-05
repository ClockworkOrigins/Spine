#!/bin/bash

set -e

. ./build-clockUtils.sh ${1}
. ./build-gmock.sh ${1}
. ./build-mariadb.sh ${1}
. ./build-sqlite.sh ${1}
. ./build-tinyxml.sh ${1}
. ./build-zlib.sh ${1}
. ./build-boost.sh ${1}
. ./build-simplewebserver.sh ${1}
