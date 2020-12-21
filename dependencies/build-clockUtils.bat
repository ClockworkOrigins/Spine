@echo OFF

IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting clockUtils"

Set ARCHIVE=clockutils-1.2.1-src.zip
Set BUILD_DIR=%TMP_DIR%/clockutils-1.2.1-src

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% http://clockwork-origins.de/clockUtils/downloads/

cd %DEP_DIR%

call build-clockUtils.bat build Debug
call build-clockUtils.bat build Release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%/%ARCH_DIR%/clockUtils/%2

IF EXIST %PREFIX% EXIT /B

echo "Compile clockUtils"

echo "Configuring clockUtils"

IF EXIST "%BUILD_DIR%\spineBuild" RD /S / Q "%BUILD_DIR%\spineBuild"

mkdir "%BUILD_DIR%\spineBuild"

pushd "%BUILD_DIR%\spineBuild"

cmake -DWITH_TESTING=OFF -DWITH_LIBRARY_ARGPARSER=OFF  -DWITH_LIBRARY_COMPRESSION=ON  -DWITH_LIBRARY_CONTAINER=OFF  -DWITH_LIBRARY_INIPARSER=OFF  -DWITH_LIBRARY_LOG=ON  -DWITH_LIBRARY_SOCKETS=ON  -DCLOCKUTILS_BUILD_SHARED=OFF -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" ..

echo "Building clockUtils"

cmake --build . --config %2

echo "Installing clockUtils"

cmake --build . --config %2 --target install

echo "Cleaning up"

popd
