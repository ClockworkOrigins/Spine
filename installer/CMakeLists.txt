IF(WIN32)
	SET(VCREDIST "vcredist_x64.exe")

	configure_file(${CMAKE_SOURCE_DIR}/config/setup.nsi.in ${CMAKE_BINARY_DIR}/installer/Windows/setup.nsi @ONLY)

	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/CWGames.nsh ${CMAKE_BINARY_DIR}/installer/Windows/CWGames.nsh COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/English.nsh ${CMAKE_BINARY_DIR}/installer/Windows/English.nsh COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/German.nsh ${CMAKE_BINARY_DIR}/installer/Windows/German.nsh COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Polish.nsh ${CMAKE_BINARY_DIR}/installer/Windows/Polish.nsh COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/MoveFileFolder.nsh ${CMAKE_BINARY_DIR}/installer/Windows/MoveFileFolder.nsh COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/setup.ini ${CMAKE_BINARY_DIR}/installer/Windows/setup.ini COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/makeSymlink.bat ${CMAKE_BINARY_DIR}/installer/Windows/makeSymlink.bat COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/removeSymlink.bat ${CMAKE_BINARY_DIR}/installer/Windows/removeSymlink.bat COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Gothic2-Setup.bat ${CMAKE_BINARY_DIR}/installer/Windows/Gothic2-Setup.bat COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/installer/Windows/Gothic2-Addon-Setup.bat ${CMAKE_BINARY_DIR}/installer/Windows/Gothic2-Addon-Setup.bat COPYONLY)
	configure_file(${VCREDIST_PATH} ${CMAKE_BINARY_DIR}/installer/Windows/${VCREDIST} COPYONLY)

	configure_file($ENV{Qt5_DIR}/bin/libEGL.dll ${CMAKE_BINARY_DIR}/installer/Qt/libEGL.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/libGLESv2.dll ${CMAKE_BINARY_DIR}/installer/Qt/libGLESv2.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Concurrent.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5Concurrent.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Core.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5Core.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Gui.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5Gui.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Network.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5Network.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Svg.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5Svg.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5Widgets.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5Widgets.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/bin/Qt5WinExtras.dll ${CMAKE_BINARY_DIR}/installer/Qt/Qt5WinExtras.dll COPYONLY)
	
	configure_file($ENV{Qt5_DIR}/plugins/imageformats/qico.dll ${CMAKE_BINARY_DIR}/installer/Qt/plugins/imageformats/qico.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/plugins/imageformats/qjpeg.dll ${CMAKE_BINARY_DIR}/installer/Qt/plugins/imageformats/qjpeg.dll COPYONLY)
	configure_file($ENV{Qt5_DIR}/plugins/imageformats/qsvg.dll ${CMAKE_BINARY_DIR}/installer/Qt/plugins/imageformats/qsvg.dll COPYONLY)

	configure_file($ENV{Qt5_DIR}/plugins/platforms/qwindows.dll ${CMAKE_BINARY_DIR}/installer/Qt/plugins/platforms/qwindows.dll COPYONLY)

	# Windows installer with NSIS
	add_custom_target(WindowsInstaller COMMAND makensis.exe ${CMAKE_BINARY_DIR}/installer/Windows/setup.nsi WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/installer/Windows DEPENDS Spine SpineAPI cleanup)
ENDIF(WIN32)
