SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(HashEvaluatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(HashEvaluator ${HashEvaluatorSrc})

target_link_libraries(HashEvaluator SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(HashEvaluator pthread)
ENDIF(UNIX)

set_target_properties(
	HashEvaluator PROPERTIES FOLDER Tools
)
