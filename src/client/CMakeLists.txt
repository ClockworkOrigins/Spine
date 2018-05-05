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

SET(SpineSrc
	${srcdir}/main.cpp
	
	${srcdir}/Config.cpp
	${srcdir}/Database.cpp
	${srcdir}/DatabaseFilterModel.cpp
	${srcdir}/DirValidator.cpp
	${srcdir}/FileDownloader.cpp
	${srcdir}/ImpressionUpdater.cpp
	${srcdir}/LibraryFilterModel.cpp
	${srcdir}/MultiFileDownloader.cpp
	${srcdir}/ReportGenerator.cpp
	${srcdir}/RtfToHtmlConverter.cpp
	${srcdir}/SavegameManager.cpp
	${srcdir}/ScreenshotManager.cpp
	${srcdir}/Uninstaller.cpp
	
	${srcdir}/models/CustomStatisticsModel.cpp
	${srcdir}/models/SpineEditorModel.cpp
	
	${srcdir}/security/Hash.cpp
	
	${srcdir}/translator/GothicParser.cpp
	${srcdir}/translator/TranslationApplier.cpp
	
	${srcdir}/widgets/AboutDialog.cpp
	${srcdir}/widgets/AchievementInfoView.cpp
	${srcdir}/widgets/AchievementOrientationPreview.cpp
	${srcdir}/widgets/AchievementSpineSettingsWidget.cpp
	${srcdir}/widgets/AchievementView.cpp
	${srcdir}/widgets/AddFriendDialog.cpp
	${srcdir}/widgets/AutoUpdateDialog.cpp
	${srcdir}/widgets/ChangelogDialog.cpp
	${srcdir}/widgets/DeveloperSettingsWidget.cpp
	${srcdir}/widgets/DownloadProgressDialog.cpp
	${srcdir}/widgets/ExportDialog.cpp
	${srcdir}/widgets/FAQDialog.cpp
	${srcdir}/widgets/FAQEntry.cpp
	${srcdir}/widgets/FeedbackDialog.cpp
	${srcdir}/widgets/FriendRequestView.cpp
	${srcdir}/widgets/FriendsView.cpp
	${srcdir}/widgets/FullscreenPreview.cpp
	${srcdir}/widgets/GamepadSpineSettingsWidget.cpp
	${srcdir}/widgets/GameSettingsWidget.cpp
	${srcdir}/widgets/GeneralSettingsWidget.cpp
	${srcdir}/widgets/GeneralSpineSettingsWidget.cpp
	${srcdir}/widgets/Gothic1IniWidget.cpp
	${srcdir}/widgets/Gothic2IniWidget.cpp
	${srcdir}/widgets/HiddenAchievementsView.cpp
	${srcdir}/widgets/ImportDialog.cpp
	${srcdir}/widgets/IniConfigurator.cpp
	${srcdir}/widgets/IntegrityCheckDialog.cpp
	${srcdir}/widgets/LeGoSpineSettingsWidget.cpp
	${srcdir}/widgets/LibraryListView.cpp
	${srcdir}/widgets/LocationSettingsWidget.cpp
	${srcdir}/widgets/LoginDialog.cpp
	${srcdir}/widgets/MainWindow.cpp
	${srcdir}/widgets/ManagementDialog.cpp
	${srcdir}/widgets/ModDatabaseView.cpp
	${srcdir}/widgets/ModInfoPage.cpp
	${srcdir}/widgets/ModInfoView.cpp
	${srcdir}/widgets/ModUpdateDialog.cpp
	${srcdir}/widgets/NewCombinationDialog.cpp
	${srcdir}/widgets/NewsWidget.cpp
	${srcdir}/widgets/NewsWriterDialog.cpp
	${srcdir}/widgets/ProfileModView.cpp
	${srcdir}/widgets/ProfileView.cpp
	${srcdir}/widgets/RatingWidget.cpp
	${srcdir}/widgets/SavegameDialog.cpp
	${srcdir}/widgets/ScoreSpineSettingsWidget.cpp
	${srcdir}/widgets/SettingsDialog.cpp
	${srcdir}/widgets/SpineEditor.cpp
	${srcdir}/widgets/StartPageWidget.cpp
	${srcdir}/widgets/SubmitCompatibilityDialog.cpp
	${srcdir}/widgets/SystempackIniWidget.cpp
	${srcdir}/widgets/UninstallDialog.cpp
	${srcdir}/widgets/WaitSpinner.cpp
	
	${srcdir}/widgets/gothicIniPages/ControlsPage.cpp
	${srcdir}/widgets/gothicIniPages/EnginePage.cpp
	${srcdir}/widgets/gothicIniPages/GamePage.cpp
	${srcdir}/widgets/gothicIniPages/PerformancePage.cpp
	${srcdir}/widgets/gothicIniPages/SoundPage.cpp
	${srcdir}/widgets/gothicIniPages/VideoPage.cpp
	${srcdir}/widgets/gothicIniPages/VisualizationPage.cpp
	
	${srcdir}/widgets/gothic2IniPages/ControlsPage.cpp
	${srcdir}/widgets/gothic2IniPages/EnginePage.cpp
	${srcdir}/widgets/gothic2IniPages/GamePage.cpp
	${srcdir}/widgets/gothic2IniPages/PerformancePage.cpp
	${srcdir}/widgets/gothic2IniPages/SoundPage.cpp
	${srcdir}/widgets/gothic2IniPages/VideoPage.cpp
	${srcdir}/widgets/gothic2IniPages/VisualizationPage.cpp
	
	${srcdir}/widgets/management/AchievementWidget.cpp
	${srcdir}/widgets/management/AchievementsWidget.cpp
	${srcdir}/widgets/management/CustomStatisticsWidget.cpp
	${srcdir}/widgets/management/GeneralConfigurationWidget.cpp
	${srcdir}/widgets/management/ModFilesWidget.cpp
	${srcdir}/widgets/management/ScoresWidget.cpp
	${srcdir}/widgets/management/StatisticsWidget.cpp
	${srcdir}/widgets/management/UserManagementWidget.cpp
	
	${srcdir}/widgets/systempackIniPages/GamePage.cpp
	${srcdir}/widgets/systempackIniPages/SystemPage.cpp
)

QT5_WRAP_CPP(SpineGuiHeader
	${includedir}/DatabaseFilterModel.h
	${includedir}/FileDownloader.h
	${includedir}/LibraryFilterModel.h
	${includedir}/MultiFileDownloader.h
	${includedir}/ReportGenerator.h
	${includedir}/ScreenshotManager.h
	
	${includedir}/models/CustomStatisticsModel.h
	${includedir}/models/SpineEditorModel.h
	
	${includedir}/translator/GothicParser.h
	${includedir}/translator/TranslationApplier.h
	
	${includedir}/widgets/AboutDialog.h
	${includedir}/widgets/AchievementInfoView.h
	${includedir}/widgets/AchievementSpineSettingsWidget.h
	${includedir}/widgets/AchievementView.h
	${includedir}/widgets/AddFriendDialog.h
	${includedir}/widgets/AutoUpdateDialog.h
	${includedir}/widgets/ChangelogDialog.h
	${includedir}/widgets/DeveloperSettingsWidget.h
	${includedir}/widgets/DownloadProgressDialog.h
	${includedir}/widgets/ExportDialog.h
	${includedir}/widgets/FeedbackDialog.h
	${includedir}/widgets/FriendRequestView.h
	${includedir}/widgets/FriendsView.h
	${includedir}/widgets/GamepadSpineSettingsWidget.h
	${includedir}/widgets/GameSettingsWidget.h
	${includedir}/widgets/GeneralSettingsWidget.h
	${includedir}/widgets/GeneralSpineSettingsWidget.h
	${includedir}/widgets/Gothic1IniWidget.h
	${includedir}/widgets/Gothic2IniWidget.h
	${includedir}/widgets/ImportDialog.h
	${includedir}/widgets/IniConfigurator.h
	${includedir}/widgets/IntegrityCheckDialog.h
	${includedir}/widgets/LeGoSpineSettingsWidget.h
	${includedir}/widgets/LibraryListView.h
	${includedir}/widgets/LocationSettingsWidget.h
	${includedir}/widgets/LoginDialog.h
	${includedir}/widgets/MainWindow.h
	${includedir}/widgets/ManagementDialog.h
	${includedir}/widgets/ModDatabaseView.h
	${includedir}/widgets/ModInfoPage.h
	${includedir}/widgets/ModInfoView.h
	${includedir}/widgets/ModUpdateDialog.h
	${includedir}/widgets/NewCombinationDialog.h
	${includedir}/widgets/NewsWidget.h
	${includedir}/widgets/NewsWriterDialog.h
	${includedir}/widgets/ProfileModView.h
	${includedir}/widgets/ProfileView.h
	${includedir}/widgets/RatingWidget.h
	${includedir}/widgets/SavegameDialog.h
	${includedir}/widgets/ScoreSpineSettingsWidget.h
	${includedir}/widgets/SettingsDialog.h
	${includedir}/widgets/SpineEditor.h
	${includedir}/widgets/StartPageWidget.h
	${includedir}/widgets/SubmitCompatibilityDialog.h
	${includedir}/widgets/SystempackIniWidget.h
	${includedir}/widgets/UninstallDialog.h
	${includedir}/widgets/WaitSpinner.h
	
	${includedir}/widgets/gothicIniPages/ControlsPage.h
	${includedir}/widgets/gothicIniPages/EnginePage.h
	${includedir}/widgets/gothicIniPages/GamePage.h
	${includedir}/widgets/gothicIniPages/PerformancePage.h
	${includedir}/widgets/gothicIniPages/SoundPage.h
	${includedir}/widgets/gothicIniPages/VideoPage.h
	${includedir}/widgets/gothicIniPages/VisualizationPage.h
	
	${includedir}/widgets/gothic2IniPages/ControlsPage.h
	${includedir}/widgets/gothic2IniPages/EnginePage.h
	${includedir}/widgets/gothic2IniPages/GamePage.h
	${includedir}/widgets/gothic2IniPages/PerformancePage.h
	${includedir}/widgets/gothic2IniPages/SoundPage.h
	${includedir}/widgets/gothic2IniPages/VideoPage.h
	${includedir}/widgets/gothic2IniPages/VisualizationPage.h
	
	${includedir}/widgets/management/AchievementWidget.h
	${includedir}/widgets/management/AchievementsWidget.h
	${includedir}/widgets/management/CustomStatisticsWidget.h
	${includedir}/widgets/management/GeneralConfigurationWidget.h
	${includedir}/widgets/management/ModFilesWidget.h
	${includedir}/widgets/management/ScoresWidget.h
	${includedir}/widgets/management/StatisticsWidget.h
	${includedir}/widgets/management/UserManagementWidget.h
	
	${includedir}/widgets/systempackIniPages/GamePage.h
	${includedir}/widgets/systempackIniPages/SystemPage.h
)

IF(WIN32)
	SET(SpineSrc
		${SpineSrc}
		${srcdir}/DirectXVersionCheck.cpp
		${srcdir}/WindowsExtensions.cpp
	
		${srcdir}/gamepad/KeyMapping.cpp
		${srcdir}/gamepad/XBoxController.cpp

		${srcdir}/widgets/GamepadSettingsWidget.cpp
		${srcdir}/widgets/InstallGothic2FromCDDialog.cpp
	)

	QT5_WRAP_CPP(SpineGuiHeaderWindows
		${includedir}/widgets/GamepadSettingsWidget.h
		${includedir}/widgets/InstallGothic2FromCDDialog.h
	)
ENDIF(WIN32)

IF(WITH_TRANSLATOR)
	SET(SpineSrc
		${SpineSrc}
		${srcdir}/widgets/AccessRightsDialog.cpp
		${srcdir}/widgets/ApplyTranslationDialog.cpp
		${srcdir}/widgets/TranslationRequestDialog.cpp
		${srcdir}/widgets/TranslationWidget.cpp
		${srcdir}/widgets/TranslatorDialog.cpp
	)

	QT5_WRAP_CPP(SpineGuiHeaderTranslator
		${includedir}/widgets/AccessRightsDialog.h
		${includedir}/widgets/ApplyTranslationDialog.h
		${includedir}/widgets/TranslationRequestDialog.h
		${includedir}/widgets/TranslationWidget.h
		${includedir}/widgets/TranslatorDialog.h
	)
ENDIF(WITH_TRANSLATOR)

SOURCE_GROUP(gamepad /gamepad/*)
SOURCE_GROUP(models /models/*)
SOURCE_GROUP(resources /resources/*)
SOURCE_GROUP(security /security/*)
SOURCE_GROUP(translator /translator/*)
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

ADD_EXECUTABLE(Spine ${GUI_TYPE} ${SpineSrc} ${SpineGuiHeader} ${SpineGuiHeaderWindows} ${SpineGuiHeaderTranslator} ${QT_TRANSLATION_FILES} ${QT_RESOURCE_FILES} ${WINDOWS_RC_FILES})
add_dependencies(Spine SpineAPI)

target_link_libraries(Spine SpineCommon ${BOOST_LIBRARIES} ${CLOCKUTILS_LIBRARIES} ${CLOCKWORK_LIBRARIES} ${OPENSSL_LIBRARIES} ${QT_LIBRARIES} ${SQLITE3_LIBRARIES} ${TRANSLATOR_LIBRARIES} ${ZLIB_LIBRARIES})

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
