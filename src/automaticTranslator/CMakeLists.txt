SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(AutomaticTranslatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(AutomaticTranslator ${AutomaticTranslatorSrc})

target_link_libraries(AutomaticTranslator ${QT_LIBRARIES} ${TRANSLATOR_LIBRARIES})

IF(UNIX)
	target_link_libraries(AutomaticTranslator pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET AutomaticTranslator POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/AutomaticTranslator.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	AutomaticTranslator PROPERTIES FOLDER Tools
)
