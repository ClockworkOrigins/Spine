SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ImageResizerSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ImageResizer ${ImageResizerSrc})

target_link_libraries(ImageResizer ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(ImageResizer pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET ImageResizer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/ImageResizer.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	ImageResizer PROPERTIES FOLDER Tools
)
