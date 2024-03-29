@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=v3.2.5.zip
Set BUILD_DIR=%TMP_DIR%/mariadb-connector-c-3.2.5
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/mariadb

IF EXIST %PREFIX% EXIT /B

echo "Compile MariaDB"

echo "Extracting MariaDB"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/

echo "Configuring MariaDB"

cd %BUILD_DIR%
cmake -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%" -A "%VSARCH%" .

echo "Building MariaDB"

MSBuild.exe mariadb-connector-c.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Installing MariaDB"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%
mkdir "%PREFIX%/include"
mkdir "%PREFIX%/include/mariadb"
mkdir "%PREFIX%/lib"
xcopy /S /Y "%BUILD_DIR%\include" "%PREFIX%\include\mariadb"
xcopy /S /Y "%BUILD_DIR%\libmariadb\Release" "%PREFIX%\lib"

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"
