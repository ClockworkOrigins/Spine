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

ADD_EXECUTABLE(Spine ${GUI_TYPE} ${SpineSrc} ${SpineHeader} ${QT_TRANSLATION_FILES} ${ResourceFile} ${WINDOWS_RC_FILES})

IF(WITH_API)
	add_dependencies(Spine SpineAPI${ARCH_POSTFIX})
ENDIF(WITH_API)

target_link_libraries(Spine SpineCommon SpineDiscord SpineGui SpineHttps SpineSecurity SpineUtils ${DISCORD_LIBRARIES} ${OPENSSL_LIBRARIES} ${QT_LIBRARIES} ${SQLITE3_LIBRARIES})
#target_link_libraries(Spine debug ${BOOST_DEBUG_BOOST_FILESYSTEM_LIBRARY} optimized ${BOOST_RELEASE_BOOST_FILESYSTEM_LIBRARY})
target_link_libraries(Spine debug ${BOOST_DEBUG_BOOST_IOSTREAMS_LIBRARY} optimized ${BOOST_RELEASE_BOOST_IOSTREAMS_LIBRARY})
target_link_libraries(Spine debug ${BOOST_DEBUG_BOOST_SERIALIZATION_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SERIALIZATION_LIBRARY})
target_link_libraries(Spine debug ${BOOST_DEBUG_BOOST_SYSTEM_LIBRARY} optimized ${BOOST_RELEASE_BOOST_SYSTEM_LIBRARY})
target_link_libraries(Spine debug ${CLOCKUTILS_DEBUG_CLOCK_COMPRESSION_LIBRARY} optimized ${CLOCKUTILS_RELEASE_CLOCK_COMPRESSION_LIBRARY})
target_link_libraries(Spine debug ${CLOCKUTILS_DEBUG_CLOCK_LOG_LIBRARY} optimized ${CLOCKUTILS_RELEASE_CLOCK_LOG_LIBRARY})
target_link_libraries(Spine debug ${CLOCKUTILS_DEBUG_CLOCK_SOCKETS_LIBRARY} optimized ${CLOCKUTILS_RELEASE_CLOCK_SOCKETS_LIBRARY})
target_link_libraries(Spine debug ${TRANSLATOR_DEBUG_LIBRARIES} optimized ${TRANSLATOR_RELEASE_LIBRARIES})
target_link_libraries(Spine debug ${ZIPPER_DEBUG_LIBRARIES} optimized ${ZIPPER_RELEASE_LIBRARIES})
target_link_libraries(Spine debug ${ZLIB_DEBUG_LIBRARIES} optimized ${ZLIB_RELEASE_LIBRARIES})

IF(WITH_TRANSLATOR)
	target_link_libraries(Spine SpineTranslator)
ENDIF(WITH_TRANSLATOR)

IF(UNIX)
	target_link_libraries(Spine pthread)
ENDIF(UNIX)

add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/de_DE.qm $<TARGET_FILE:Spine>/../)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/en_US.qm $<TARGET_FILE:Spine>/../)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/es_ES.qm $<TARGET_FILE:Spine>/../)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/po_PO.qm $<TARGET_FILE:Spine>/../)
add_custom_command(TARGET Spine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/ru_RU.qm $<TARGET_FILE:Spine>/../)

IF(WIN32)
	target_link_libraries(Spine version ws2_32)
	IF(WIN32 AND NOT WITH_CONSOLE)
		target_link_libraries(Spine Qt5::WinMain)
	ENDIF(WIN32 AND NOT WITH_CONSOLE)
ENDIF(WIN32)
