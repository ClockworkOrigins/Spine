SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/server)

SET(DatabaseAdderSrc
	${srcdir}/main.cpp
	${srcdir}/../server/MariaDBWrapper.cpp
)

ADD_EXECUTABLE(DatabaseAdder ${DatabaseAdderSrc})

target_link_libraries(DatabaseAdder ${CLOCKWORK_LIBRARIES} ${MARIADB_LIBRARIES})

IF(UNIX)
	target_link_libraries(DatabaseAdder pthread)
ENDIF(UNIX)

IF(WIN32)
	add_custom_command(TARGET DatabaseAdder POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/DatabaseAdder.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

set_target_properties(
	DatabaseAdder PROPERTIES FOLDER Tools
)
