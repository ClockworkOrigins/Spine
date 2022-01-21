SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ModCheckerSrc
	${srcdir}/main.cpp
)

SET(ResourceFile ${srcdir}/resources.qrc)

ADD_EXECUTABLE(ModChecker ${ModCheckerSrc} ${ResourceFile})

target_link_libraries(ModChecker SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(ModChecker pthread)
ENDIF(UNIX)

set_target_properties(
	ModChecker PROPERTIES FOLDER Tools
)
