SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(KeyWrapperSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(KeyWrapper ${KeyWrapperSrc})

IF(UNIX)
	target_link_libraries(KeyWrapper pthread)
ENDIF(UNIX)

set_target_properties(
	KeyWrapper PROPERTIES FOLDER Tools
)
