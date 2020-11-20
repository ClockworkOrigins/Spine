SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(TTSCreatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(TTSCreator ${TTSCreatorSrc})

target_link_libraries(TTSCreator ${QT_LIBRARIES} ${TINYXML_LIBRARIES})

IF(UNIX)
	target_link_libraries(TTSCreator pthread)
ENDIF(UNIX)

set_target_properties(
	TTSCreator PROPERTIES FOLDER Tools
)
