SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(UnusedIDFinderSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(UnusedIDFinder ${UnusedIDFinderSrc})

target_link_libraries(UnusedIDFinder ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(UnusedIDFinder pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET UnusedIDFinder POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/UnusedIDFinder.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	UnusedIDFinder PROPERTIES FOLDER Tools
)
