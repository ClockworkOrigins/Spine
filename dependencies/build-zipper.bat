@echo OFF

call build-common.bat %1 %2

Set BUILD_DIR=%TMP_DIR%/zipper
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/zipper

IF EXIST %PREFIX% EXIT /B

echo "Compile zipper"

echo "Cloning zipper"

IF NOT EXIST %TMP_DIR% MKDIR %TMP_DIR%

pushd %TMP_DIR%

git clone --recursive https://github.com/sebastiandev/zipper.git zipper

popd

echo "Configuring zipper"

cd %BUILD_DIR%
cmake -DBUILD_SHARED_VERSION=OFF -DBUILD_STATIC_VERSION=ON -DBUILD_TEST=OFF -DBUILD_TESTING=OFF -DLIBZ_INCLUDE_DIR=%DEP_DIR%/%ARCH_DIR%/zlib/include -DLIBZ_LIBRARY=%DEP_DIR%/%ARCH_DIR%/zlib/lib/zlibstatic.lib -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building zipper"

cmake --build . --config Release

echo "Installing zipper"

cmake --build . --config Release --target install

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

