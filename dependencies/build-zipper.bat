@echo OFF

IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting zipper"

Set BUILD_DIR=%TMP_DIR%/zipper

IF NOT EXIST %TMP_DIR% MKDIR %TMP_DIR%

pushd %TMP_DIR%

git clone --branch v1.0.1 --recursive https://github.com/sebastiandev/zipper.git zipper

popd

cd %DEP_DIR%

call build-zipper.bat build Debug d
call build-zipper.bat build Release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%/%ARCH_DIR%/zipper/%2

IF EXIST %PREFIX% EXIT /B

echo "Compile zipper"

echo "Configuring zipper"

IF EXIST "%BUILD_DIR%\spineBuild" RD /S / Q "%BUILD_DIR%\spineBuild"

mkdir "%BUILD_DIR%\spineBuild"

pushd "%BUILD_DIR%\spineBuild"

cmake -DBUILD_SHARED_VERSION=OFF -DBUILD_STATIC_VERSION=ON -DBUILD_TEST=OFF -DLIBZ_INCLUDE_DIR=%DEP_DIR%/%ARCH_DIR%/zlib/%2/include -DLIBZ_LIBRARY=%DEP_DIR%/%ARCH_DIR%/zlib/lib/%2/zlibstatic%3.lib -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" ..

echo "Building zipper"

cmake --build . --config %2

echo "Installing zipper"

cmake --build . --config %2 --target install

echo "Cleaning up"

popd
