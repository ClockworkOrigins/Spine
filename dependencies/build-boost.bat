@echo OFF

setlocal EnableDelayedExpansion

call build-common.bat %1 %2

Set ARCHIVE=boost_1_62_0.tar.gz
Set BUILD_DIR=%TMP_DIR%/boost_1_62_0
Set PREFIX=%DEP_DIR%\%ARCH_DIR%\boost

Set ZLIBARCHIVE=zlib1211.zip
Set ZLIBBUILD_DIR=%TMP_DIR%/zlib-1.2.11

IF EXIST %PREFIX% EXIT /B

echo "Compile Boost"

echo "Extracting Boost"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://sourceforge.net/projects/boost/files/boost/1.62.0/

cd %DEP_DIR%

echo "Extracting zlib"

call build-common.bat downloadAndUnpack %ZLIBARCHIVE% %ZLIBBUILD_DIR% https://zlib.net/

echo "Configuring Boost"

cd %BUILD_DIR%
call bootstrap.bat > log.txt

echo "Building Boost"
b2 toolset=%BOOSTCOMPILER% address-model=%BOOSTARCH% --with-date_time --with-filesystem --with-iostreams -sNO_COMPRESSION=0 -sNO_ZLIB=0 -sZLIB_SOURCE=%BUILD_DIR%\..\zlib-1.2.11 --with-regex --with-serialization --with-system --with-thread link=static threading=multi --layout=system -j %NUMBER_OF_PROCESSORS% variant=release install --prefix=%PREFIX% stage > log2.txt

echo "Installing Boost"

echo #define BOOST_ALL_NO_LIB >> "%PREFIX%\include\boost\config\user.hpp"
echo #define BOOST_SYMBOL_EXPORT >> "%PREFIX%\include\boost\config\user.hpp"
for /f %%a IN ('dir /b %PREFIX%\lib\libboost_*.lib') do (
	SET str=%%a
	SET str=!str:libboost_=boost_!
	ren %PREFIX%\lib\%%a !str!
)

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"
