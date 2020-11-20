SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/api)

INCLUDE_DIRECTORIES(${includedir})

SET(CommonSrc
	${srcdir}/API.cpp
	${srcdir}/APIMessage.cpp
	${srcdir}/Friends.cpp
	${srcdir}/Gamepad.cpp
	${srcdir}/Multiplayer.cpp
	${srcdir}/Statistics.cpp
	${srcdir}/zString.cpp
)

FILE(GLOB APIHeader ${includedir}/*.h)

configure_file(${CMAKE_SOURCE_DIR}/media/Spine.vdf ${CMAKE_BINARY_DIR}/media/Spine.vdf COPYONLY)

ADD_LIBRARY(SpineAPI${ARCH_POSTFIX} SHARED ${CommonSrc} ${APIHeader})
set_target_properties(SpineAPI${ARCH_POSTFIX} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(SpineAPI${ARCH_POSTFIX} SpineCommon ${CLOCKUTILS_LIBRARIES})

IF(WIN32)
	target_link_libraries(SpineAPI${ARCH_POSTFIX} ws2_32)
ENDIF(WIN32)

set_target_properties(
	SpineAPI${ARCH_POSTFIX} PROPERTIES FOLDER Client
)
