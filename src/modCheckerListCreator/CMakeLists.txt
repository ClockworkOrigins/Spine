SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ModCheckerListCreatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ModCheckerListCreator ${ModCheckerListCreatorSrc})

target_link_libraries(ModCheckerListCreator SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(ModCheckerListCreator pthread)
ENDIF(UNIX)

set_target_properties(
	ModCheckerListCreator PROPERTIES FOLDER Tools
)
