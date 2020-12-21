@echo OFF

IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting zlib"

Set ARCHIVE=zlib1211.zip
Set BUILD_DIR=%TMP_DIR%/zlib-1.2.11

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://zlib.net/

cd %DEP_DIR%

call build-zlib.bat build Debug
call build-zlib.bat build Release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%/%ARCH_DIR%/zlib/%2

IF EXIST %PREFIX% EXIT /B

echo "Compile zlib"

echo "Configuring zlib"

IF EXIST "%BUILD_DIR%\spineBuild" RD /S / Q "%BUILD_DIR%\spineBuild"

mkdir "%BUILD_DIR%\spineBuild"

pushd "%BUILD_DIR%\spineBuild"

cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" ..

echo "Building zlib"

cmake --build . --config %2

echo "Installing zlib"

cmake --build . --config %2 --target install

echo "Cleaning up"

popd

