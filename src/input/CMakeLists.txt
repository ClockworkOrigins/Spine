SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/input)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineInputSrc ${srcdir}/*.cpp)
FILE(GLOB SpineInputHeader ${includedir}/*.h)

ADD_LIBRARY(SpineInput SHARED ${SpineInputSrc} ${SpineInputHeader})

target_link_libraries(SpineInput ${QT_LIBRARIES})

set_target_properties(
	SpineInput PROPERTIES FOLDER Libraries
)
