SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ReferenceFinderSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ReferenceFinder ${ReferenceFinderSrc})

target_link_libraries(ReferenceFinder ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(ReferenceFinder pthread)
ENDIF(UNIX)

set_target_properties(
	ReferenceFinder PROPERTIES FOLDER Tools
)
