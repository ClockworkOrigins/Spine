SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/client)

INCLUDE_DIRECTORIES(${includedir})

SET(UnitTesterSrc
	${srcdir}/main.cpp

	${srcdir}/test_GothicParser.cpp
	${srcdir}/test_GothicVdf.cpp
)

ADD_EXECUTABLE(UnitTester ${UnitTesterSrc} ${UnitTesterGuiHeader})

target_link_libraries(UnitTester debug ${GTEST_DEBUG_LIBRARIES} optimized ${GTEST_RELEASE_LIBRARIES})
target_link_libraries(UnitTester debug ${BOOST_DEBUG_BOOST_IOSTREAMS_LIBRARY} optimized ${BOOST_RELEASE_BOOST_IOSTREAMS_LIBRARY})
target_link_libraries(UnitTester ${QT_LIBRARIES})

IF(WITH_TRANSLATOR)
	target_link_libraries(UnitTester debug ${TRANSLATOR_DEBUG_LIBRARIES} optimized ${TRANSLATOR_RELEASE_LIBRARIES})
	target_link_libraries(UnitTester SpineTranslator)
ENDIF(WITH_TRANSLATOR)

IF(UNIX)
	target_link_libraries(UnitTester pthread)
ENDIF(UNIX)

IF(WIN32)
	target_link_libraries(UnitTester ws2_32)
ENDIF(WIN32)

ADD_DEFINITIONS(-DVDF_PATH="${CMAKE_SOURCE_DIR}/media/Spine.vdf")
ADD_DEFINITIONS(-DTEST_RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources")

LINK_TO_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/Debug/tests")
LINK_TO_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/tests")
LINK_TO_FOLDER("${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/bin/Release/tests")

set_target_properties(
	UnitTester PROPERTIES FOLDER Tests
)

ADD_TEST(NAME TestUnitTests COMMAND $<TARGET_FILE:UnitTester> WORKING_DIRECTORY $<TARGET_FILE:UnitTester>/..)
