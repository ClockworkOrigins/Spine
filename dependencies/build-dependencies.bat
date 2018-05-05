@echo OFF

call build-clockUtils.bat %1 %2
call build-gmock.bat %1 %2
call build-mariadb.bat %1 %2
call build-openSSL.bat %1 %2
call build-sqlite.bat %1 %2
call build-tinyxml.bat %1 %2
call build-zlib.bat %1 %2
call build-boost.bat %1 %2
call build-simplewebserver.bat %1 %2
