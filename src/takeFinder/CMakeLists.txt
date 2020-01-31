SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(TakeFinderSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(TakeFinder ${TakeFinderSrc})

target_link_libraries(TakeFinder ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(TakeFinder pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET TakeFinder POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/TakeFinder.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	TakeFinder PROPERTIES FOLDER Tools
)
