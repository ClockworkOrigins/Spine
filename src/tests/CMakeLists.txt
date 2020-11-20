SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/client)

INCLUDE_DIRECTORIES(${includedir})

SET(UnitTesterSrc
	${srcdir}/main.cpp
	
	${srcdir}/test_GothicParser.cpp
)

ADD_EXECUTABLE(UnitTester ${UnitTesterSrc} ${UnitTesterGuiHeader})

target_link_libraries(UnitTester ${GTEST_LIBRARIES} ${QT_LIBRARIES} ${TRANSLATOR_LIBRARIES})

IF(WITH_TRANSLATOR)
	target_link_libraries(UnitTester SpineTranslator)
ENDIF(WITH_TRANSLATOR)

IF(UNIX)
	target_link_libraries(UnitTester pthread)
ENDIF(UNIX)

IF(WIN32)
	target_link_libraries(UnitTester ws2_32)
ENDIF(WIN32)

LINK_TO_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/Debug/tests")
LINK_TO_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/tests")
LINK_TO_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/Release/tests")

set_target_properties(
	UnitTester PROPERTIES FOLDER Tests
)
