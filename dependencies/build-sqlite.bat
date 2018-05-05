@echo OFF

call build-common.bat %1 %2

Set ARCHIVE=sqlite-amalgamation-3150100.zip
Set BUILD_DIR=%TMP_DIR%/sqlite-amalgamation-3150100
Set PREFIX=%DEP_DIR%/%ARCH_DIR%/sqlite

IF EXIST %PREFIX% EXIT /B

echo "Compile SQLite"

echo "Extracting SQLite"

call build-common.bat downloadAndUnpack %ARCHIVE% %BUILD_DIR%

echo "Building SQLite"

cd %BUILD_DIR%
cl /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SQLITE_EXPORTS" /DSQLITE_API=__declspec(dllexport) /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /EHsc /nologo /D_USRDLL sqlite3.c /link /DLL /OUT:sqlite3.dll /IMPLIB:sqlite3.lib
if %errorlevel% gtr 0 exit /b

echo "Installing SQLite"

mkdir "%PREFIX%"
mkdir "%PREFIX%/bin"
mkdir "%PREFIX%/lib"
mkdir "%PREFIX%/include"

move sqlite3.h %PREFIX%/include/sqlite3.h
move sqlite3ext.h %PREFIX%/include/sqlite3ext.h
move sqlite3.dll %PREFIX%/bin/sqlite3.dll
move sqlite3.lib %PREFIX%/lib/sqlite3.lib

echo "Cleaning up"

cd %DEP_DIR%
RD /S /Q "%TMP_DIR%"

