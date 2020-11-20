SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ArgumentPrinterSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ArgumentPrinter ${ArgumentPrinterSrc})

IF(UNIX)
	target_link_libraries(ArgumentPrinter pthread)
ENDIF(UNIX)

set_target_properties(
	ArgumentPrinter PROPERTIES FOLDER Tools
)
