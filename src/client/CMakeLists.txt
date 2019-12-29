SET(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
SET(includedir ${CMAKE_SOURCE_DIR}/include/client)

INCLUDE_DIRECTORIES(${includedir})

SET(ResourceFile ${srcdir}/resources/resources.qrc)

# Translation files
SET(TRANS
	${CMAKE_SOURCE_DIR}/languages/de_DE.ts
	${CMAKE_SOURCE_DIR}/languages/en_US.ts
	${CMAKE_SOURCE_DIR}/languages/es_ES.ts
	${CMAKE_SOURCE_DIR}/languages/po_PO.ts
	${CMAKE_SOURCE_DIR}/languages/ru_RU.ts
)

FILE(GLOB_RECURSE SpineSrc ${srcdir}/*.cpp)
FILE(GLOB_RECURSE SpineHeader ${includedir}/*.h)

SOURCE_GROUP(launcher /launcher/*)
SOURCE_GROUP(models /models/*)
SOURCE_GROUP(resources /resources/*)
SOURCE_GROUP(widgets /widgets/*)
SOURCE_GROUP(widgets\\gothicIniPages /widgets/gothicIniPages/*)
SOURCE_GROUP(widgets\\gothic2IniPages /widgets/gothic2IniPages/*)
SOURCE_GROUP(widgets\\management /widgets/management/*)
SOURCE_GROUP(widgets\\systempackIniPages /widgets/systempackIniPages/*)
SOURCE_GROUP(moc /moc_*)

# add resources ...
QT5_ADD_RESOURCES(QT_RESOURCE_FILES ${ResourceFile})

# add translations ...
QT5_ADD_TRANSLATION(QT_TRANSLATION_FILES ${TRANS})

IF(WIN32 AND NOT WITH_CONSOLE)
	SET(GUI_TYPE WIN32)
ENDIF(WIN32 AND NOT WITH_CONSOLE)

IF(WIN32)
   set(WINDOWS_RC_FILES
         ${CMAKE_SOURCE_DIR}/config/icon.rc
         )
ENDIF(WIN32)

ADD_EXECUTABLE(Spine ${GUI_TYPE} ${SpineSrc} ${SpineHeader} ${QT_TRANSLATION_FILES} ${QT_RESOURCE_FILES} ${WINDOWS_RC_FILES})
add_dependencies(Spine SpineAPI)

target_link_libraries(Spine SpineCommon SpineHttps SpineSecurity SpineUtils ${BOOST_LIBRARIES} ${CLOCKUTILS_LIBRARIES} ${CLOCKWORK_LIBRARIES} ${OPENSSL_LIBRARIES} ${QT_LIBRARIES} ${SQLITE3_LIBRARIES} ${TRANSLATOR_LIBRARIES} ${ZLIB_LIBRARIES})

IF(WITH_TRANSLATOR)
	target_link_libraries(Spine SpineTranslator)
ENDIF(WITH_TRANSLATOR)

IF(WIN32)
	TARGET_LINK_LIBRARIES(Spine SpineGamepad)
ENDIF(WIN32)

IF(UNIX)
	target_link_libraries(Spine pthread)
ENDIF(UNIX)

add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/de_DE.qm ${CMAKE_BINARY_DIR}/bin)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/en_US.qm ${CMAKE_BINARY_DIR}/bin)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/es_ES.qm ${CMAKE_BINARY_DIR}/bin)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/po_PO.qm ${CMAKE_BINARY_DIR}/bin)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/ru_RU.qm ${CMAKE_BINARY_DIR}/bin)

IF(WIN32)
	target_link_libraries(Spine version ws2_32)
	IF(WIN32 AND NOT WITH_CONSOLE)
		target_link_libraries(Spine Qt5::WinMain)
	ENDIF(WIN32 AND NOT WITH_CONSOLE)
	add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/Spine.exe ${CMAKE_BINARY_DIR}/bin)
ENDIF(WIN32)
