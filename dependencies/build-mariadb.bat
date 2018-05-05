@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=mariadb-connector-c-2.3.1.tar.gz
Set BUILD_DIR=%TMP_DIR%/mariadb-connector-c-2.3.1
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/mariadb

IF EXIST %PREFIX% EXIT /B

echo "Compile MariaDB"

echo "Extracting MariaDB"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring MariaDB"

cd %BUILD_DIR%
cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building MariaDB"

MSBuild.exe mariadb-client.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release

echo "Installing MariaDB"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release
mkdir "%PREFIX%/include"
mkdir "%PREFIX%/include/mariadb"
mkdir "%PREFIX%/lib"
xcopy /S /Y "%BUILD_DIR%\include" "%PREFIX%\include\mariadb"
xcopy /S /Y "%BUILD_DIR%\libmariadb\Release" "%PREFIX%\lib"

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

