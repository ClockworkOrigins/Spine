SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(AutomaticTranslatorSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(AutomaticTranslator ${AutomaticTranslatorSrc})

target_link_libraries(AutomaticTranslator ${QT_LIBRARIES} ${TRANSLATOR_LIBRARIES})

IF(UNIX)
	target_link_libraries(AutomaticTranslator pthread)
ENDIF(UNIX)

set_target_properties(
	AutomaticTranslator PROPERTIES FOLDER Tools
)
