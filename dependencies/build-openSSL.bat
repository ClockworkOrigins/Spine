@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=openssl-1.1.1g.tar.gz
Set BUILD_DIR=%TMP_DIR%\openssl-1.1.1g
Set PREFIX=%DEP_DIR%\%ARCH_DIR%\openSSL\

IF EXIST %PREFIX% EXIT /B

echo "Compile OpenSSL"

echo "Extracting OpenSSL"

REM call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://www.openssl.org/source/

echo "Configuring OpenSSL"

REM cd %BUILD_DIR%
IF [%BOOSTARCH%] == [32] (
	REM perl Configure VC-WIN32 no-asm shared --prefix=%PREFIX%
)
IF [%BOOSTARCH%] == [64] (
	REM perl Configure VC-WIN64A no-asm shared --prefix=%PREFIX%
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
	REM call ms\do_win64a.bat
	
	REM do this programatically in the future instead of using a fix file
	REM xcopy %DEP_DIR%\..\ext\build.info %BUILD_DIR%\build.info* /Y
	
	REM nmake
	
	powershell -ExecutionPolicy RemoteSigned -File %DEP_DIR%\build.ps1 -openssl_release 1.1.1d -vs_version 140 -config release -platform x64 -library shared

	mkdir %PREFIX%
	mkdir %PREFIX%\bin
	mkdir %PREFIX%\lib

	xcopy /I /S /Y %DEP_DIR%\VS_140\include %PREFIX%\include
	xcopy /S /Y %DEP_DIR%\VS_140\win64\bin\release\*.dll %PREFIX%\bin\
	xcopy /S /Y %DEP_DIR%\VS_140\win64\bin\release\*.lib %PREFIX%\lib\

	echo "Installing OpenSSL"
	
	REM nmake install_sw
	
	

	echo "Cleaning up"

	cd %DEP_DIR%
	RD /S /Q "%TMP_DIR%"
)

