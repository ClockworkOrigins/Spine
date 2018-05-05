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

ADD_LIBRARY(SpineAPI SHARED ${CommonSrc} ${APIHeader})
set_target_properties(SpineAPI PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(SpineAPI SpineCommon ${CLOCKUTILS_LIBRARIES})

IF(WIN32)
	target_link_libraries(SpineAPI ws2_32)
	add_custom_command(TARGET SpineAPI POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/SpineAPI.dll ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
