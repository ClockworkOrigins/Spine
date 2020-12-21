SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(HashEvaluatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(HashEvaluator ${HashEvaluatorSrc})

target_link_libraries(HashEvaluator SpineUtils ${QT_LIBRARIES})
target_link_libraries(HashEvaluator debug ${BOOST_DEBUG_BOOST_IOSTREAMS_LIBRARY} optimized ${BOOST_RELEASE_BOOST_IOSTREAMS_LIBRARY})
target_link_libraries(HashEvaluator debug ${ZLIB_DEBUG_LIBRARIES} optimized ${ZLIB_RELEASE_LIBRARIES})

IF(UNIX)
	target_link_libraries(HashEvaluator pthread)
ENDIF(UNIX)

set_target_properties(
	HashEvaluator PROPERTIES FOLDER Tools
)
