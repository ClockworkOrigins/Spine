SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ReferenceFinderSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ReferenceFinder ${ReferenceFinderSrc})

target_link_libraries(ReferenceFinder ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(ReferenceFinder pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET ReferenceFinder POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/ReferenceFinder.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	ReferenceFinder PROPERTIES FOLDER Tools
)
