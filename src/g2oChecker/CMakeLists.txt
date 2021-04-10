SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(G2OCheckerSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(G2OChecker ${G2OCheckerSrc})

target_link_libraries(G2OChecker SpineHttps ${QT_LIBRARIES})
target_link_libraries(G2OChecker debug ${BOOST_DEBUG_BOOST_SYSTEM_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SYSTEM_LIBRARY})
target_link_libraries(G2OChecker debug ${ZIPPER_DEBUG_LIBRARIES} optimized ${ZIPPER_RELEASE_LIBRARIES})
target_link_libraries(G2OChecker debug ${ZLIB_DEBUG_LIBRARIES} optimized ${ZLIB_RELEASE_LIBRARIES})

IF(UNIX)
	target_link_libraries(G2OChecker pthread)
ENDIF(UNIX)

set_target_properties(
	G2OChecker PROPERTIES FOLDER Tools
)
