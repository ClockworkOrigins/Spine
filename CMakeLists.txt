cmake_minimum_required (VERSION 2.8.12)

#########################################################################
#########################################################################
#
# Global Build Configuration Options
#
#########################################################################

OPTION(WITH_CLIENT "builds with client" OFF)
OPTION(WITH_SERVER "builds with server" OFF)
OPTION(WITH_CONSOLE "build with console" OFF)
OPTION(WITH_TOOLS "build with helper tools" ON)
OPTION(WITH_TRANSLATOR "build with translator" OFF)
OPTION(TEST_CONFIG "build with test server" OFF)
SET(DATABASEUSER "foobar" CACHE STRING "user used on the server for database access")
SET(DATABASEPASSWORD "blafoo" CACHE STRING "password used on the server for database access")
SET(ACCOUNTSDATABASE "accounts" CACHE STRING "database used for general account stuff")
SET(SPINEDATABASE "spine" CACHE STRING "database for all Spine related stuff")
SET(TRANSLATOR_RSA_HEADER "config/RSAKey.h" CACHE STRING "header containing the RSA key used for the translator API")
SET(SPINE_RSA_HEADER "config/RSAKey.h" CACHE STRING "header containing the RSA key used for the server API")
SET(SPINE_PRIVATE_KEY "config/rsa.pem" CACHE STRING "private RSA key used for the server")
SET(VCREDIST_PATH "" CACHE STRING "path to the vcredist for the used Visual Studio version")
SET(DXREDIST_PATH "" CACHE STRING "path to the DirectX redistributable")

project(Spine CXX)

SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 14)
SET(VERSION_PATCH 11)

INCLUDE(cmake/variables.cmake)

IF(DXPATH)
	INCLUDE_DIRECTORIES(${DXPATH}/include)
	LINK_DIRECTORIES(${DXPATH}/lib/x86)
ENDIF(DXPATH)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin)

IF(TEST_CONFIG)
	SET(SERVER_PORT 19184)
	SET(UPLOADSERVER_PORT 19189)
	ADD_DEFINITIONS(-DTEST_CONFIG)
ELSE()
	SET(SERVER_PORT 19180)
	SET(UPLOADSERVER_PORT 19188)
ENDIF()

IF(WIN32)
	SET(CMAKE_CXX_FLAGS_DEBUG		"${CMAKE_CXX_FLAGS_DEBUG} /bigobj /D_WIN32_WINNT=0x0501")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE} /DSPINE_RELEASE /bigobj /D_WIN32_WINNT=0x0501")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO		"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj /DQT_FORCE_ASSERTS /D_WIN32_WINNT=0x0501")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CMAKE_CXX_FLAGS_MINSIZEREL} /bigobj /D_WIN32_WINNT=0x0501")
ENDIF(WIN32)

ADD_DEFINITIONS(-DSPINE_PRIVATE_KEY="${SPINE_PRIVATE_KEY}")
IF(WITH_TRANSLATOR)
	ADD_DEFINITIONS(-DWITH_TRANSLATOR)
ENDIF(WITH_TRANSLATOR)

#########################################################################
# Library Section
#########################################################################

configure_file(${TRANSLATOR_RSA_HEADER} ${CMAKE_SOURCE_DIR}/ext/RSAKey.h COPYONLY)

INCLUDE(cmake/searchDependencies.cmake)

configure_file(${CMAKE_SOURCE_DIR}/config/Config.h.in ${CMAKE_BINARY_DIR}/include/SpineConfig.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/ServerConfig.h.in ${CMAKE_BINARY_DIR}/include/SpineServerConfig.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/changelog.xml ${CMAKE_BINARY_DIR}/bin/changelog.xml COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/Spine_Version.xml ${CMAKE_BINARY_DIR}/bin/Spine_Version.xml COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/makeSymlink.bat ${CMAKE_BINARY_DIR}/makeSymlink.bat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/removeSymlink.bat ${CMAKE_BINARY_DIR}/removeSymlink.bat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Gothic2-Setup.bat ${CMAKE_BINARY_DIR}/Gothic2-Setup.bat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Gothic2-Addon-Setup.bat ${CMAKE_BINARY_DIR}/Gothic2-Addon-Setup.bat COPYONLY)

configure_file(${SPINE_RSA_HEADER} ${CMAKE_BINARY_DIR}/include/common/RSAKey.h COPYONLY)

INCLUDE(cmake/addToBinDir.cmake)

INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(src)
IF(EXISTS "${VCREDIST_PATH}" AND EXISTS "${DXREDIST_PATH}" AND WIN32)
	ADD_SUBDIRECTORY(installer)
ENDIF()
ADD_SUBDIRECTORY(scriptApi)
ADD_SUBDIRECTORY(tutorials)
