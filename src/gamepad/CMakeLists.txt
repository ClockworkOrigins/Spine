SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/gamepad)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineGamepadSrc ${srcdir}/*.cpp)
FILE(GLOB SpineGamepadHeader ${includedir}/*.h)

ADD_LIBRARY(SpineGamepad STATIC ${SpineGamepadSrc} ${SpineGamepadHeader})

#target_link_libraries(SpineGamepad ${BOOST_LIBRARIES})
