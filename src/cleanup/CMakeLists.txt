INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/client)

SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB CleanupSrc ${srcdir}/*.cpp)

SET(ResourceFile ${srcdir}/resources/resources.qrc)

SOURCE_GROUP(resources /resources/*)

# add resources ...
QT5_ADD_RESOURCES(QT_RESOURCE_FILES ${ResourceFile})

ADD_EXECUTABLE(cleanup ${CleanupSrc} ${QT_RESOURCE_FILES})

target_link_libraries(cleanup SpineUtils ${CLOCKUTILS_LIBRARIES} ${QT_LIBRARIES})

IF(UNIX)
	target_link_libraries(cleanup pthread)
ENDIF(UNIX)

set_target_properties(
	cleanup PROPERTIES FOLDER Client
)
