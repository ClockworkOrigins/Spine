@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=clockutils-1.2.1-src.zip
Set BUILD_DIR=%TMP_DIR%/clockutils-1.2.1-src
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/clockUtils

IF EXIST %PREFIX% EXIT /B

echo "Compile clockUtils"

echo "Extracting clockUtils"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% http://clockwork-origins.de/clockUtils/downloads/

echo "Configuring clockUtils"

cd %BUILD_DIR%
cmake -DWITH_TESTING=OFF -DWITH_LIBRARY_ARGPARSER=OFF  -DWITH_LIBRARY_COMPRESSION=ON  -DWITH_LIBRARY_CONTAINER=OFF  -DWITH_LIBRARY_INIPARSER=OFF  -DWITH_LIBRARY_LOG=ON  -DWITH_LIBRARY_SOCKETS=ON  -DCLOCKUTILS_BUILD_SHARED=OFF -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building clockUtils"

MSBuild.exe clockUtils.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Installing clockUtils"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

