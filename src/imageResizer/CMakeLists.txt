SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ImageResizerSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(ImageResizer ${ImageResizerSrc})

target_link_libraries(ImageResizer ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(ImageResizer pthread)
ENDIF(UNIX)

set_target_properties(
	ImageResizer PROPERTIES FOLDER Tools
)
