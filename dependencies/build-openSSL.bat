@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=openssl-1.0.2j.tar.gz
Set BUILD_DIR=%TMP_DIR%\openssl-1.0.2j
Set PREFIX=%DEP_DIR%\%ARCH_DIR%\openSSL\

IF EXIST %PREFIX% EXIT /B

echo "Compile OpenSSL"

echo "Extracting OpenSSL"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Configuring OpenSSL"

cd %BUILD_DIR%
IF [%BOOSTARCH%] == [32] (
	perl Configure VC-WIN32 no-asm --prefix=%PREFIX%
)
IF [%BOOSTARCH%] == [64] (
	perl Configure VC-WIN64A no-asm --prefix=%PREFIX%
)

echo "Building OpenSSL"

IF [%BOOSTARCH%] == [32] (
	call ms\do_ms.bat
	nmake -f ms\ntdll.mak

	echo "Installing OpenSSL"

	xcopy /I /S /Y %BUILD_DIR%\inc32 %PREFIX%\include
	xcopy /S /Y %BUILD_DIR%\out32dll\*.dll %PREFIX%\bin\
	xcopy /S /Y %BUILD_DIR%\out32dll\*.lib %PREFIX%\lib\

	echo "Cleaning up"

	cd %DEP_DIR%
	RD /S /Q "%TMP_DIR%"
)
IF [%BOOSTARCH%] == [64] (
	call ms\do_win64a.bat
	nmake -f ms\ntdll.mak

	echo "Installing OpenSSL"

	xcopy /I /S /Y %BUILD_DIR%\inc32 %PREFIX%\include
	xcopy /S /Y %BUILD_DIR%\out32dll\*.dll %PREFIX%\bin\
	xcopy /S /Y %BUILD_DIR%\out32dll\*.lib %PREFIX%\lib\

	echo "Cleaning up"

	cd %DEP_DIR%
	RD /S /Q "%TMP_DIR%"
)

