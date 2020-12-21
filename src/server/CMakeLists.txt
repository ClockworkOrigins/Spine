SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/server)

INCLUDE_DIRECTORIES(${includedir})

FILE(GLOB SpineServerSrc ${srcdir}/*.cpp)
FILE(GLOB ServerHeader ${includedir}/*.h)

ADD_EXECUTABLE(SpineServer ${SpineServerSrc} ${ServerHeader})

target_link_libraries(SpineServer SpineCommon ${MARIADB_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(SpineServer debug ${BOOST_DEBUG_BOOST_FILESYSTEM_LIBRARY} optimized ${BOOST_RELEASE_BOOST_FILESYSTEM_LIBRARY})
target_link_libraries(SpineServer debug ${BOOST_DEBUG_BOOST_SERIALIZATION_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SERIALIZATION_LIBRARY})
target_link_libraries(SpineServer debug ${BOOST_DEBUG_BOOST_SYSTEM_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SYSTEM_LIBRARY})
target_link_libraries(SpineServer debug ${CLOCKUTILS_DEBUG_CLOCK_SOCKETS_LIBRARY} optimized ${CLOCKUTILS_RELEASE_CLOCK_SOCKETS_LIBRARY})
target_link_libraries(SpineServer debug ${TINYXML_DEBUG_LIBRARIES} optimized ${TINYXML_RELEASE_LIBRARIES})

IF(UNIX)
	target_link_libraries(SpineServer pthread)
ENDIF(UNIX)

IF(WIN32)
	target_link_libraries(SpineServer ws2_32)
ENDIF(WIN32)

set_target_properties(
	SpineServer PROPERTIES FOLDER Server
)
