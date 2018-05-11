@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=Simple-Web-Server-3.0.0-rc2.tar.gz
Set BUILD_DIR=%TMP_DIR%/Simple-Web-Server-3.0.0-rc2
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/SimpleWebServer

IF EXIST %PREFIX% EXIT /B

echo "Compile SimpleWebServer"

echo "Extracting SimpleWebServer"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring SimpleWebServer"

cd %BUILD_DIR%
cmake -DBOOST_ROOT=%DEP_DIR%/%ARCH_DIR%/boost -DOPENSSL_ROOT_DIR=%DEP_DIR%/%ARCH_DIR%/openssl -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building SimpleWebServer"

MSBuild.exe Simple-Web-Server.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Installing SimpleWebServer"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

