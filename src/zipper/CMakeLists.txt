SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ZipperSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(Zipper ${ZipperSrc})

target_link_libraries(Zipper SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZIPPER_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(Zipper pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET Zipper POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/Zipper.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	Zipper PROPERTIES FOLDER Tools
)
