SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ArgumentPrinterSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ArgumentPrinter ${ArgumentPrinterSrc})

IF(UNIX)
	target_link_libraries(ArgumentPrinter pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET ArgumentPrinter POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/ArgumentPrinter.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
