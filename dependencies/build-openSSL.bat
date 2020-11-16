@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=openssl-1.1.1g.tar.gz
Set BUILD_DIR=%TMP_DIR%\openssl-1.1.1g
Set PREFIX=%DEP_DIR%\%ARCH_DIR%\openSSL\

IF EXIST %PREFIX% EXIT /B

echo "Compile OpenSSL"

echo "Extracting OpenSSL"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://www.openssl.org/source/

echo "Configuring OpenSSL"

pushd %BUILD_DIR%

IF [%BOOSTARCH%] == [32] (
	perl Configure VC-WIN32 no-asm shared --prefix=%PREFIX%
)
IF [%BOOSTARCH%] == [64] (
	perl Configure VC-WIN64A no-asm shared --prefix=%PREFIX%
)

echo "Building OpenSSL"

nmake
nmake install

echo "Installing OpenSSL"

echo "Cleaning up"

popd

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

