#!/bin/bash

mkdir build
cd build
cmake -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_TOOLS=ON -DWITH_CONSOLE=OFF -DWITH_TRANSLATOR=ON -DTEST_CONFIG=ON -DCMAKE_BUILD_TYPE=Release ..
make
cd bin
./UnitTester
