@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=release-1.10.0.zip
Set BUILD_DIR=%TMP_DIR%/googletest-release-1.10.0
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/gmock

IF EXIST %PREFIX% EXIT /B

echo "Compile GoogleMock with GoogleTest"

echo "Extracting GoogleMock with GoogleTest"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://github.com/google/googletest/archive/

echo "Configuring GoogleMock with GoogleTest"

cd %BUILD_DIR%
cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -DBUILD_SHARED_LIBS=ON -G "%VSCOMPILER%" -A "%VSARCH%" .

echo "Building GoogleMock with GoogleTest"

cmake --build . --config Release

echo "Installing GoogleMock with GoogleTest"

cmake --build . --config Release --target install

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

