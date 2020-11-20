SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(UnusedIDFinderSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(UnusedIDFinder ${UnusedIDFinderSrc})

target_link_libraries(UnusedIDFinder ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(UnusedIDFinder pthread)
ENDIF(UNIX)

set_target_properties(
	UnusedIDFinder PROPERTIES FOLDER Tools
)
