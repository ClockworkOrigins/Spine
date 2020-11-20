SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(ZipperSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(Zipper ${ZipperSrc})

target_link_libraries(Zipper SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZIPPER_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(Zipper pthread)
ENDIF(UNIX)

set_target_properties(
	Zipper PROPERTIES FOLDER Tools
)
