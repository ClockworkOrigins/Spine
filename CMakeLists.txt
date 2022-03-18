cmake_minimum_required (VERSION 3.7.0)

#########################################################################
#########################################################################
#
# Global Build Configuration Options
#
#########################################################################

OPTION(WITH_CLIENT "builds with client" OFF)
OPTION(WITH_SERVER "builds with server" OFF)
OPTION(WITH_API "builds with API" OFF)
OPTION(WITH_CONSOLE "build with console" OFF)
OPTION(WITH_TOOLS "build with helper tools" ON)
OPTION(WITH_G2OCHECKER "build with G2O Checker" OFF)
OPTION(TEST_CONFIG "build with test server" OFF)
SET(DATABASEUSER "foobar" CACHE STRING "user used on the server for database access")
SET(DATABASEPASSWORD "blafoo" CACHE STRING "password used on the server for database access")
SET(ACCOUNTSDATABASE "accounts" CACHE STRING "database used for general account stuff")
SET(SPINEDATABASE "spine" CACHE STRING "database for all Spine related stuff")
SET(EWDATABASE "Elemental War" CACHE STRING "database for all Elemental War related stuff")
SET(EW2DATABASE "Elemental War 2" CACHE STRING "database for all Elemental War 2 related stuff")
SET(TRI6DATABASE "Tri6" CACHE STRING "database for all Tri6 related stuff")
SET(TRANSLATOR_RSA_HEADER "config/RSAKey.h" CACHE STRING "header containing the RSA key used for the translator API")
SET(SPINE_RSA_HEADER "config/RSAKey.h" CACHE STRING "header containing the RSA key used for the server API")
SET(SPINE_PRIVATE_KEY "config/rsa.pem" CACHE STRING "private RSA key used for the server")
SET(VCREDIST_PATH "" CACHE STRING "path to the vcredist for the used Visual Studio version")
SET(SSLCHAINPATH "" CACHE STRING "path to the fullchain path of the SSL certificate")
SET(SSLPRIVKEYNPATH "" CACHE STRING "path to the private key of the SSL certificate")
SET(DISCORDKEY "12345" CACHE STRING "Discord API Key")

project(Spine CXX)

IF(NOT MSVC_VERSION)
	SET(MSVC_VERSION 0)
ENDIF()

IF(NOT WIN32 OR ${MSVC_VERSION} GREATER_EQUAL 1920)
	SET(CMAKE_CXX_STANDARD 17)
ELSE()
	SET(CMAKE_CXX_STANDARD 14)
ENDIF()

SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 39)
SET(VERSION_PATCH 0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

INCLUDE(cmake/common.cmake)
INCLUDE(cmake/variables.cmake)

IF(DXPATH)
	INCLUDE_DIRECTORIES(${DXPATH}/include)
	LINK_DIRECTORIES(${DXPATH}/lib/x86)
ENDIF(DXPATH)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Debug)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/Release)

IF(TEST_CONFIG)
	SET(SERVER_PORT 19184)
	SET(UPLOADSERVER_PORT 19189)
	SET(DATABASESERVER_PORT 19191)
	SET(MANAGEMENTSERVER_PORT 19192)
	ADD_DEFINITIONS(-DTEST_CONFIG)
ELSE()
	SET(SERVER_PORT 19180)
	SET(UPLOADSERVER_PORT 19188)
	SET(DATABASESERVER_PORT 19181)
	SET(MANAGEMENTSERVER_PORT 19182)
ENDIF()

IF(G2OFOLDER)
	SET(WITH_G2OCHECKER ON)
	ADD_DEFINITIONS(-DG2OFOLDER="${G2OFOLDER}" -DWITH_G2OCHECKER)
ENDIF(G2OFOLDER)

IF(WIN32)
	SET(CMAKE_CXX_FLAGS_DEBUG		"${CMAKE_CXX_FLAGS_DEBUG} /bigobj /D_WIN32_WINNT=0x0501")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE} /DSPINE_RELEASE /bigobj /D_WIN32_WINNT=0x0501")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO		"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj /DQT_FORCE_ASSERTS /D_WIN32_WINNT=0x0501")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CMAKE_CXX_FLAGS_MINSIZEREL} /bigobj /D_WIN32_WINNT=0x0501")
ENDIF(WIN32)

ADD_DEFINITIONS(-DSPINE_PRIVATE_KEY="${SPINE_PRIVATE_KEY}")

#########################################################################
# Library Section
#########################################################################

configure_file(${TRANSLATOR_RSA_HEADER} ${CMAKE_SOURCE_DIR}/ext/RSAKey.h COPYONLY)

INCLUDE(cmake/searchDependencies.cmake)

configure_file(${CMAKE_SOURCE_DIR}/config/Config.h.in ${CMAKE_BINARY_DIR}/include/SpineConfig.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/ServerConfig.h.in ${CMAKE_BINARY_DIR}/include/SpineServerConfig.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/changelog.xml ${CMAKE_BINARY_DIR}/bin/Debug/changelog.xml COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/changelog.xml ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/changelog.xml COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/changelog.xml ${CMAKE_BINARY_DIR}/bin/Release/changelog.xml COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/config/Spine_Version.xml ${CMAKE_BINARY_DIR}/bin/Spine_Version.xml COPYONLY) # Linux only, so can stay in bin
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/makeSymlink.bat ${CMAKE_BINARY_DIR}/makeSymlink.bat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/removeSymlink.bat ${CMAKE_BINARY_DIR}/removeSymlink.bat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Gothic2-Setup.bat ${CMAKE_BINARY_DIR}/Gothic2-Setup.bat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Gothic2-Addon-Setup.bat ${CMAKE_BINARY_DIR}/Gothic2-Addon-Setup.bat COPYONLY)

configure_file(${SPINE_RSA_HEADER} ${CMAKE_BINARY_DIR}/include/common/RSAKey.h COPYONLY)

INCLUDE(cmake/addToBinDir.cmake)

INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)

ENABLE_TESTING()

ADD_SUBDIRECTORY(src)
IF(EXISTS "${VCREDIST_PATH}" AND WIN32)
	ADD_SUBDIRECTORY(installer)
ENDIF()
ADD_SUBDIRECTORY(scriptApi)
ADD_SUBDIRECTORY(tutorials)
