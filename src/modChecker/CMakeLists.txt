SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ModCheckerSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ModChecker ${ModCheckerSrc})

target_link_libraries(ModChecker SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(ModChecker pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET ModChecker POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/ModChecker.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
