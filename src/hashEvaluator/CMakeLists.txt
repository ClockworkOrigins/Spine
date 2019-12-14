SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(HashEvaluatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(HashEvaluator ${HashEvaluatorSrc})

target_link_libraries(HashEvaluator SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(HashEvaluator pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET HashEvaluator POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/HashEvaluator.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
