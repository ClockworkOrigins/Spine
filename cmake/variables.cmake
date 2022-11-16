IF(UNIX)
	IF(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
		SET(DEP_DIR_BUILD "gcc")
		SET(UNIX_COMPILER "gcc")
		IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.8")
			SET(UNIX_COMPILER "gcc-4.7")
			SET(DEP_DIR_BUILD "gcc-4.7")
		ENDIF()
		SET(Qt5_Platform gcc_64)
		SET(Qt5_DIR ${Qt5_BaseDir}/${Qt5_Platform})
		SET(Qt5_libDir "${Qt5_DIR}/lib" CACHE STRING "Directory containing the qt libs")
	ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
		SET(DEP_DIR_BUILD "clang")
		SET(UNIX_COMPILER "clang")
		SET(Qt5_DIR ${Qt5_BaseDir}/${Qt5_Platform})
		SET(Qt5_libDir "${Qt5_DIR}/lib" CACHE STRING "Directory containing the qt libs")
	ENDIF()
ELSEIF(WIN32)
	IF(CMAKE_GENERATOR MATCHES "Visual Studio 12 2013")
		SET(DEP_DIR_BUILD "msvc13")
		SET(VS_TOOLCHAIN "msvc13")
		SET(VSENV VS12)
		SET(Qt5_Platform msvc2013)
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 14 2015")
		SET(DEP_DIR_BUILD "msvc15")
		SET(VS_TOOLCHAIN "msvc15")
		SET(VSENV VS15)
		SET(Qt5_Platform msvc2015)
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 15 2017")
		SET(DEP_DIR_BUILD "msvc17")
		SET(VS_TOOLCHAIN "msvc17")
		SET(VSENV VS17)
		SET(Qt5_Platform msvc2017)
	ELSEIF(CMAKE_GENERATOR MATCHES "Visual Studio 16 2019")
		SET(DEP_DIR_BUILD "msvc19")
		SET(VS_TOOLCHAIN "msvc19")
		SET(VSENV VS19)
		SET(Qt5_Platform msvc2019)
		SET(SUPPORTSWEBENGINE 1)
	ENDIF()

	IF(CMAKE_GENERATOR_PLATFORM MATCHES "x64")
		SET(VS_ARCH "64")
		SET(VSSCRIPTARCH "amd64")
		SET(Qt5_Platform ${Qt5_Platform}_64)
		SET(DEP_DIR_BUILD ${DEP_DIR_BUILD}_64)
	ELSEIF(CMAKE_GENERATOR_PLATFORM MATCHES "x86" OR CMAKE_GENERATOR_PLATFORM MATCHES "Win32")
		SET(VS_ARCH "32")
		SET(VSSCRIPTARCH "x86")
		SET(DEP_DIR_BUILD ${DEP_DIR_BUILD}_32)
	ELSE()
	ENDIF()

	SET(Qt5_DIR ${Qt5_BaseDir}/${Qt5_Platform})
	SET(Qt5_libDir "${Qt5_BaseDir}/lib" CACHE STRING "Directory containing the qt libs")
ENDIF()

SET(ENV{Qt5_DIR} ${Qt5_DIR})
SET(Qt5_DIR_Backup ${Qt5_DIR})

SET(SPINE_DEP_DIR "${CMAKE_SOURCE_DIR}/dependencies/${DEP_DIR_BUILD}")

SET(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)

IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

#########################################################################
# Global Macros and Definitions
#########################################################################

# Sets appropriate Compilerflags

SET(CXX_FLAGS "-pedantic -Wall -Wextra -Woverloaded-virtual -Wnon-virtual-dtor -Wformat=2 -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wredundant-decls -Wctor-dtor-privacy -Wno-sign-conversion -Wno-unused-parameter -Wno-long-long -fPIC")
SET(CLANG_FLAGS "-Wstring-plus-int")

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.7)
		SET(CXX_FLAGS "${CXX_FLAGS} -Wold-style-cast")
	ENDIF()
	IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
		SET(CXX_FLAGS "${CXX_FLAGS} -Wno-old-style-cast")
	ENDIF()
	SET(CMAKE_CXX_FLAGS_DEBUG		"${CXX_FLAGS} -g")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CXX_FLAGS} -0s -DNDEBUG")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CXX_FLAGS} -O3 -DNDEBUG")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CXX_FLAGS} -O2 -g")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CMAKE_CXX_FLAGS_MINSIZEREL}")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE}")
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	SET(CXX_FLAGS "${CXX_FLAGS} -Wold-style-cast")
	SET(CMAKE_CXX_FLAGS_DEBUG		"${CXX_FLAGS} ${CLANG_FLAGS} -g")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CXX_FLAGS} ${CLANG_FLAGS} -0s -DNDEBUG")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CXX_FLAGS} ${CLANG_FLAGS} -O3 -DNDEBUG")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CXX_FLAGS} ${CLANG_FLAGS} -O2 -g")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CMAKE_CXX_FLAGS_MINSIZEREL}")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE}")
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	# 4996: usage of deprecated code
	# 4458: hiding member of same name
	# 4100: unreferenced formal parameter
	# 4715: not all code paths return a value
	SET(CXX_FLAGS "/MP /W4 /wd4127 /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS /wd4714 /we4996 /we4458 /we4100 /we4715")

	IF(${MSVC_VERSION} GREATER_EQUAL 1920)
		SET(CXX_FLAGS "${CXX_FLAGS} /Zc:__cplusplus")
	ENDIF()

	SET(CMAKE_CXX_FLAGS_DEBUG		"${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS}")
	SET(CMAKE_CXX_FLAGS_MINSIZEREL		"${CMAKE_CXX_FLAGS_MINSIZEREL} ${CXX_FLAGS}")
	SET(CMAKE_CXX_FLAGS_RELEASE		"${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS}")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO	"${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CXX_FLAGS}")
ELSE()
	MESSAGE(SEND_FATAL "Unknown C++ compiler \"${CMAKE_CXX_COMPILER_ID}\".")
ENDIF()

#########################################################################
# Global Directory Definitions
#########################################################################

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

IF(${VS_ARCH} AND ${VS_ARCH} STREQUAL "64")
	SET(ARCH_POSTFIX "64")
ENDIF(${VS_ARCH} AND ${VS_ARCH} STREQUAL "64")
