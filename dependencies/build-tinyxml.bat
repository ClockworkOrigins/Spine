@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=tinyxml2-4.0.1.zip
Set BUILD_DIR=%TMP_DIR%/tinyxml2-4.0.1
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/tinyxml2

IF EXIST %PREFIX% EXIT /B

echo "Compile tinyxml2"

echo "Extracting tinyxml2"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring tinyxml2"

cd %BUILD_DIR%
cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" .

echo "Building tinyxml2"

MSBuild.exe tinyxml2.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release

echo "Installing tinyxml2"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

