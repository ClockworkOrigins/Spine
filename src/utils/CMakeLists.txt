SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/utils)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineUtilsSrc ${srcdir}/*.cpp)
FILE(GLOB SpineUtilsHeader ${includedir}/*.h)

ADD_LIBRARY(SpineUtils STATIC ${SpineUtilsSrc} ${SpineUtilsHeader})

target_link_libraries(SpineUtils ${BOOST_LIBRARIES} ${CLOCKUTILS_LIBRARIES} ${SQLITE3_LIBRARIES})
target_link_libraries(SpineUtils debug ${ZIPPER_DEBUG_LIBRARIES} optimized ${ZIPPER_RELEASE_LIBRARIES})
target_link_libraries(SpineUtils debug ${ZLIB_DEBUG_LIBRARIES} optimized ${ZLIB_RELEASE_LIBRARIES})

set_target_properties(
	SpineUtils PROPERTIES FOLDER Libraries
)
