SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/gui)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineGuiSrc ${srcdir}/*.cpp)
FILE(GLOB SpineGuiHeader ${includedir}/*.h)

ADD_LIBRARY(SpineGui STATIC ${SpineGuiSrc} ${SpineGuiHeader})

target_link_libraries(SpineGui ${BOOST_LIBRARIES})
