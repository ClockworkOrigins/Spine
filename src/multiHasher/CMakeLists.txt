SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(MultiHasherSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(MultiHasher ${MultiHasherSrc})

target_link_libraries(MultiHasher SpineUtils ${QT_LIBRARIES})
target_link_libraries(MultiHasher debug ${BOOST_DEBUG_BOOST_IOSTREAMS_LIBRARY} optimized ${BOOST_RELEASE_BOOST_IOSTREAMS_LIBRARY})
target_link_libraries(MultiHasher debug ${ZLIB_DEBUG_LIBRARIES} optimized ${ZLIB_RELEASE_LIBRARIES})

IF(UNIX)
	target_link_libraries(MultiHasher pthread)
ENDIF(UNIX)

set_target_properties(
	MultiHasher PROPERTIES FOLDER Tools
)
