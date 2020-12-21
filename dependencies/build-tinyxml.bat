@echo OFF

IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting tinyxml2"

Set ARCHIVE=tinyxml2-4.0.1.zip
Set BUILD_DIR=%TMP_DIR%/tinyxml2-4.0.1

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

cd %DEP_DIR%

call build-tinyxml.bat build Debug
call build-tinyxml.bat build Release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%/%ARCH_DIR%/tinyxml2/%2

IF EXIST %PREFIX% EXIT /B

echo "Compile tinyxml2"

echo "Configuring tinyxml2"

IF EXIST "%BUILD_DIR%\spineBuild" RD /S / Q "%BUILD_DIR%\spineBuild"

mkdir "%BUILD_DIR%\spineBuild"

pushd "%BUILD_DIR%\spineBuild"

cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" ..

echo "Building tinyxml2"

cmake --build . --config %2

echo "Installing tinyxml2"

cmake --build . --config %2 --target install

echo "Cleaning up"

popd

