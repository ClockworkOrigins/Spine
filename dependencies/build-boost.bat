@echo OFF

setlocal EnableDelayedExpansion
IF "%1" == "build" (goto build)

call build-common.bat %1 %2

echo "Extracting Boost"
Set ARCHIVE=boost_1_78_0.tar.gz
Set BUILD_DIR=%TMP_DIR%/boost_1_78_0

Set ZLIBARCHIVE=zlib1213.zip
Set ZLIBBUILD_DIR=%TMP_DIR%/zlib-1.2.13

echo "Extracting Boost"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR% https://sourceforge.net/projects/boost/files/boost/1.78.0/

cd %DEP_DIR%

echo "Extracting zlib"

call build-common.bat downloadAndUnpack %ZLIBARCHIVE% %ZLIBBUILD_DIR% https://zlib.net/

cd %DEP_DIR%

call build-boost.bat build Debug debug
call build-boost.bat build Release release

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

EXIT /B

:build

Set PREFIX=%DEP_DIR%\%ARCH_DIR%\boost\%2

IF EXIST %PREFIX% EXIT /B

echo "Compile Boost"

echo "Configuring Boost"

pushd "%BUILD_DIR%"

call bootstrap.bat > log.txt

echo "Building Boost"
b2 toolset=%BOOSTCOMPILER% address-model=%BOOSTARCH% --with-date_time --with-filesystem --with-iostreams -sNO_COMPRESSION=0 -sNO_ZLIB=0 -sZLIB_SOURCE=%BUILD_DIR%\..\zlib-1.2.13 --with-regex --with-serialization --with-system --with-thread link=static threading=multi --layout=system -j %NUMBER_OF_PROCESSORS% variant=%3 install --prefix=%PREFIX% stage > %DEP_DIR%\log2.txt

echo "Installing Boost"

echo #define BOOST_ALL_NO_LIB >> "%PREFIX%\include\boost\config\user.hpp"
echo #define BOOST_SYMBOL_EXPORT >> "%PREFIX%\include\boost\config\user.hpp"
echo #define BOOST_USE_WINDOWS_H >> "%PREFIX%\include\boost\config\user.hpp"
for /f %%a IN ('dir /b %PREFIX%\lib\libboost_*.lib') do (
	SET str=%%a
	SET str=!str:libboost_=boost_!
	ren %PREFIX%\lib\%%a !str!
)

echo "Cleaning up"

popd
