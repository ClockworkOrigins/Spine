SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(VariableCheckerSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(VariableChecker ${VariableCheckerSrc})

target_link_libraries(VariableChecker SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(VariableChecker pthread)
ENDIF(UNIX)

set_target_properties(
	VariableChecker PROPERTIES FOLDER Tools
)
