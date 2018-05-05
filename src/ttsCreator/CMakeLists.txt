SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(TTSCreatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(TTSCreator ${TTSCreatorSrc})

target_link_libraries(TTSCreator ${QT_LIBRARIES} ${TINYXML_LIBRARIES})

IF(UNIX)
	target_link_libraries(TTSCreator pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET TTSCreator POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/TTSCreator.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
