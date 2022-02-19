SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/utils)

SET(ResourceFile ${srcdir}/resources.qrc)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineUtilsSrc ${srcdir}/*.cpp)
FILE(GLOB SpineUtilsHeader ${includedir}/*.h)

ADD_LIBRARY(SpineUtils STATIC ${SpineUtilsSrc} ${SpineUtilsHeader} ${ResourceFile})

target_link_libraries(SpineUtils ${SQLITE3_LIBRARIES})

IF(WIN32)
	target_link_libraries(SpineUtils debug ${BOOST_DEBUG_BOOST_SYSTEM_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SYSTEM_LIBRARY})
	target_link_libraries(SpineUtils debug ${CLOCKUTILS_DEBUG_CLOCK_LOG_LIBRARY} optimized ${CLOCKUTILS_RELEASE_CLOCK_LOG_LIBRARY})
	target_link_libraries(SpineUtils debug ${ZIPPER_DEBUG_LIBRARIES} optimized ${ZIPPER_RELEASE_LIBRARIES})
	target_link_libraries(SpineUtils debug ${ZLIB_DEBUG_LIBRARIES} optimized ${ZLIB_RELEASE_LIBRARIES})
ELSE(UNIX)
	target_link_libraries(SpineUtils ${BOOST_LIBRARIES})
	target_link_libraries(SpineUtils ${CLOCKUTILS_LIBRARIES})
	target_link_libraries(SpineUtils ${ZIPPER_LIBRARIES})
	target_link_libraries(SpineUtils ${ZLIB_LIBRARIES})
ENDIF(WIN32)

set_target_properties(
	SpineUtils PROPERTIES FOLDER Libraries
)
