@echo OFF

IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting GoogleMock with GoogleTest"

Set ARCHIVE=translator.zip
Set BUILD_DIR=%TMP_DIR%/translator

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

xcopy /F "%DEP_DIR%/../ext/RSAKey.h" "%BUILD_DIR%/include/translator/common/RSAKey.h*"

cd %DEP_DIR%

call build-translator.bat build Debug
call build-translator.bat build Release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%/%ARCH_DIR%/translator/%2

IF EXIST %PREFIX% EXIT /B

echo "Compile translator"

echo "Configuring translator"

IF EXIST "%BUILD_DIR%\spineBuild" RD /S / Q "%BUILD_DIR%\spineBuild"

mkdir "%BUILD_DIR%\spineBuild"

pushd "%BUILD_DIR%\spineBuild"

cmake -DWITH_SERVER=OFF -DWITH_CLIENT=OFF -DTRANSLATOR_DEP_DIR=%DEP_DIR%\%ARCH_DIR% -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" ..

echo "Building translator"

cmake --build . --config %2

echo "Installing translator"

cmake --build . --config %2 --target install

echo "Cleaning up"

popd

