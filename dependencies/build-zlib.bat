@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=zlib-1.2.10.tar.xz
Set BUILD_DIR=%TMP_DIR%/zlib-1.2.10
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/zlib

IF EXIST %PREFIX% EXIT /B

echo "Compile zlib"

echo "Extracting zlib"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring zlib"

cd %BUILD_DIR%
cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building zlib"

MSBuild.exe zlib.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release

echo "Installing zlib"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

