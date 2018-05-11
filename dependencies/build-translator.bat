@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=translator.rar
Set BUILD_DIR=%TMP_DIR%/translator
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/translator

IF EXIST %PREFIX% EXIT /B

echo "Compile translator"

echo "Extracting translator"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring translator"

cd %BUILD_DIR%

xcopy /F "%DEP_DIR%/../ext/RSAKey.h" "%BUILD_DIR%/include/translator/common/RSAKey.h*"

cmake -DWITH_SERVER=OFF -DWITH_CLIENT=OFF -DTRANSLATOR_DEP_DIR=%DEP_DIR%\%ARCH_DIR% -DCMAKE_INSTALL_PREFIX=%PREFIX% -G "%VSCOMPILER%%VSARCH%" .

echo "Building translator"

MSBuild.exe Translator.sln /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Installing translator"

MSBuild.exe INSTALL.vcxproj /p:Configuration=Release /p:Platform=%VSSOLUTIONARCH%

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

