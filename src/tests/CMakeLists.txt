SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/client)

INCLUDE_DIRECTORIES(${includedir})

SET(UnitTesterSrc
	${srcdir}/main.cpp
	
	${srcdir}/test_GothicParser.cpp

	${CMAKE_SOURCE_DIR}/src/client/translator/GothicParser.cpp
)

QT5_WRAP_CPP(UnitTesterGuiHeader
	${includedir}/translator/GothicParser.h
)

ADD_EXECUTABLE(UnitTester ${UnitTesterSrc} ${UnitTesterGuiHeader})

target_link_libraries(UnitTester ${GTEST_LIBRARIES} ${QT_LIBRARIES} ${TRANSLATOR_LIBRARIES})

IF(UNIX)
	target_link_libraries(UnitTester pthread)
ENDIF(UNIX)

IF(WIN32)
	target_link_libraries(UnitTester ws2_32)
	add_custom_command(TARGET UnitTester POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/UnitTester.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test1/test.d ${CMAKE_BINARY_DIR}/bin/tests/test1/test.d COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test1/names.txt ${CMAKE_BINARY_DIR}/bin/tests/test1/names.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test1/texts.txt ${CMAKE_BINARY_DIR}/bin/tests/test1/texts.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test1/dialogs.txt ${CMAKE_BINARY_DIR}/bin/tests/test1/dialogs.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test2/test.d ${CMAKE_BINARY_DIR}/bin/tests/test2/test.d COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test2/names.txt ${CMAKE_BINARY_DIR}/bin/tests/test2/names.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test2/texts.txt ${CMAKE_BINARY_DIR}/bin/tests/test2/texts.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test2/dialogs.txt ${CMAKE_BINARY_DIR}/bin/tests/test2/dialogs.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test3/test.d ${CMAKE_BINARY_DIR}/bin/tests/test3/test.d COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test3/names.txt ${CMAKE_BINARY_DIR}/bin/tests/test3/names.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test3/texts.txt ${CMAKE_BINARY_DIR}/bin/tests/test3/texts.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test3/dialogs.txt ${CMAKE_BINARY_DIR}/bin/tests/test3/dialogs.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test4/test.d ${CMAKE_BINARY_DIR}/bin/tests/test4/test.d COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test4/names.txt ${CMAKE_BINARY_DIR}/bin/tests/test4/names.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test4/texts.txt ${CMAKE_BINARY_DIR}/bin/tests/test4/texts.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test4/dialogs.txt ${CMAKE_BINARY_DIR}/bin/tests/test4/dialogs.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test5/test.d ${CMAKE_BINARY_DIR}/bin/tests/test5/test.d COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test5/names.txt ${CMAKE_BINARY_DIR}/bin/tests/test5/names.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test5/texts.txt ${CMAKE_BINARY_DIR}/bin/tests/test5/texts.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/test5/dialogs.txt ${CMAKE_BINARY_DIR}/bin/tests/test5/dialogs.txt COPYONLY)
