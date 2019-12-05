SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/utils)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineUtilsSrc ${srcdir}/*.cpp)
FILE(GLOB SpineUtilsHeader ${includedir}/*.h)

### No cpp files yet
#ADD_LIBRARY(SpineUtils STATIC ${SpineUtilsSrc} ${SpineUtilsHeader})

#target_link_libraries(SpineUtils ${BOOST_LIBRARIES})
