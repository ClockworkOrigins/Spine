@echo OFF

IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting GoogleMock with GoogleTest"

Set ARCHIVE=release-1.10.0.zip
Set BUILD_DIR=%TMP_DIR%/googletest-release-1.10.0

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://github.com/google/googletest/archive/

cd %DEP_DIR%

call build-gmock.bat build Debug
call build-gmock.bat build Release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%/%ARCH_DIR%/gmock/%2

IF EXIST %PREFIX% EXIT /B

echo "Compile GoogleMock with GoogleTest"

echo "Configuring GoogleMock with GoogleTest"

IF EXIST "%BUILD_DIR%\spineBuild" RD /S / Q "%BUILD_DIR%\spineBuild"

mkdir "%BUILD_DIR%\spineBuild"

pushd "%BUILD_DIR%\spineBuild"

cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -DBUILD_SHARED_LIBS=ON -G "%VSCOMPILER%" -A "%VSARCH%" ..

echo "Building GoogleMock with GoogleTest"

cmake --build . --config %2

echo "Installing GoogleMock with GoogleTest"

cmake --build . --config %2 --target install

echo "Cleaning up"

popd
