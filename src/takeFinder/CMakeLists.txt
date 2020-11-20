SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(TakeFinderSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(TakeFinder ${TakeFinderSrc})

target_link_libraries(TakeFinder ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(TakeFinder pthread)
ENDIF(UNIX)

set_target_properties(
	TakeFinder PROPERTIES FOLDER Tools
)
