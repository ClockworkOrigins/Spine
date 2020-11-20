SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(MultiHasherSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(MultiHasher ${MultiHasherSrc})

target_link_libraries(MultiHasher SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(MultiHasher pthread)
ENDIF(UNIX)

set_target_properties(
	MultiHasher PROPERTIES FOLDER Tools
)
