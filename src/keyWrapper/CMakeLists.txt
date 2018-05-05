SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(KeyWrapperSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(KeyWrapper ${KeyWrapperSrc})

IF(UNIX)
	target_link_libraries(KeyWrapper pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET KeyWrapper POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/KeyWrapper.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
