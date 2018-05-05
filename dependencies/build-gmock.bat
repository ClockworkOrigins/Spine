@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=gmock-1.7.0.zip
Set BUILD_DIR=%TMP_DIR%/gmock-1.7.0
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/gmock

IF EXIST %PREFIX% EXIT /B

echo "Compile GoogleMock with GoogleTest"

echo "Extracting GoogleMock with GoogleTest"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring GoogleMock with GoogleTest"

cd %BUILD_DIR%
cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -G "%VSCOMPILER%%VSARCH%" .

echo "Building GoogleMock with GoogleTest"

MSBuild.exe gmock.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release

echo "Installing GoogleMock with GoogleTest"

mkdir "%PREFIX%"
mkdir "%PREFIX%/include"
mkdir "%PREFIX%/lib"
xcopy /S /Y "%BUILD_DIR%/gtest/include" "%PREFIX%/include"
xcopy /S /Y "%BUILD_DIR%/include" "%PREFIX%/include"
xcopy /S /Y "%BUILD_DIR%/gtest/Release" "%PREFIX%/lib"
xcopy /S /Y "%BUILD_DIR%/Release" "%PREFIX%/lib"

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

