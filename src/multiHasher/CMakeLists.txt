SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

SET(MultiHasherSrc
	${srcdir}/main.cpp
)

ADD_EXECUTABLE(MultiHasher ${MultiHasherSrc})

target_link_libraries(MultiHasher SpineUtils ${BOOST_LIBRARIES} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(UNIX)
	target_link_libraries(MultiHasher pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET MultiHasher POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/MultiHasher.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	MultiHasher PROPERTIES FOLDER Tools
)
