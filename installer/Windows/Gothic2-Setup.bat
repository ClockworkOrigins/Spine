@echo off
if "%OS%" == "Windows_NT" setlocal
if "%OS%" == "Windows_NT" setlocal enableextensions

rem --------------------------------------------------------------------------
rem [ script options ]

rem Verzeichnisse (leer = Standard, kein \ am Ende, . = aktuelles Verzeichnis)

rem --------------------------------------------------------------------------
rem [ script default ]

set COPYCMD=/Y
set DIR=.
set FOR=/F "delims=;"
if "%OS%" == "Windows_NT" goto WinNT
set DIR=nul
set FOR=
rem >>>  Windows 9x: G2TMP und G2DIR sollten nicht zu lang sein!  <<<
rem >>>  Sie k”nnen den Ordner danach umbenennen bzw verschieben  <<<
if "%G2DIR%" == "" set G2DIR=C:\Gothic II
if "%G2TMP%" == "" set G2TMP=C:\_g2setup_
:WinNT
if "%ProgramFiles%" == "" set ProgramFiles=C:\Programme
if "%G2DIR%" == "" if not exist "%ProgramFiles%\%DIR%" mkdir "%ProgramFiles%"
if "%G2DIR%" == "" if not exist "%ProgramFiles%\JoWooD\%DIR%" mkdir "%ProgramFiles%\JoWooD"
if "%G2DIR%" == "" set G2DIR=%ProgramFiles%\JoWooD\Gothic II
if "%G2TMP%" == "" set G2TMP=%TEMP%\$Gothic2-Setup$
if "%G2CDR%" == "" set G2CDR=.

rem --------------------------------------------------------------------------
rem [ display infos ]

echo.
echo Dieses Skript installiert Gothic II manuell von den CDs
echo nach: "%G2DIR%"
echo Um Gothic 2 in ein anderes Verzeichnis zu installieren,
echo brechen Sie diese Batch jetzt mit der Tastenkombination
echo [Strg+C] ab und „ndern die folgende Zeile: "set G2DIR="
echo.
echo Das Skript wurde mit den folgenden Versionen des Setups
echo (Gothic2-Setup.exe) unter Microsoft Windows XP getestet
echo.
echo        2002-11-09, 1.28, Deutsch (Org. Release)
echo        2003-11-19, 1.30, Deutsch (Gold Edition)
rem echo        2003-04-24, 1.31, English (Atari UK)
echo.
echo Diese Art der "Installation" sollte nur benutzt werden,
echo wenn innerhalb der normalen Installation die Orginal-CD
echo nicht als solche erkannt wird. Es werden keine Daten in
echo die Registrierungsdatenbank (Registry) geschrieben (die
echo Deinstallation muss ebenso manuell erfolgen); und weder
echo im Startmen noch auf dem Desktop werden Links erzeugt.
echo Gothic II wird dann mit der folgenden Datei gestartet:
echo.
echo   "%G2DIR%\System\Gothic2.exe"
echo.

if not exist "%G2DIR%\%DIR%" goto DetectCDR

rem --------------------------------------------------------------------------
rem [ detect cd-rom ]

:DetectCDR

rem --------------------------------------------------------------------------
rem [ extract setup ]

:ExtractCD
echo.
echo Starte %G2CDR%\Gothic2-Setup.exe...

start /wait %G2CDR%\Gothic2-Setup.exe /X %G2TMP%

echo ...fertig

rem --------------------------------------------------------------------------
rem [ prepare target ]

echo.
echo Zielverzeichnis wird vorbereitet...

if exist "%G2DIR%\%DIR%" attrib -h -r -s "%G2DIR%\*.*" /S

for %%i in ("%G2DIR%\*.dmp")        do del "%%i"
for %%i in ("%G2DIR%\Miles\*.m3d")  do del "%%i"
for %%i in ("%G2DIR%\System\*.asi") do del "%%i"
for %%i in ("%G2DIR%\System\*.flt") do del "%%i"
for %%i in ("%G2DIR%\System\*.m3d") do del "%%i"

if not exist "%G2DIR%\%DIR%"                                     mkdir "%G2DIR%"
if not exist "%G2DIR%\Data\%DIR%"                                mkdir "%G2DIR%\Data"
if not exist "%G2DIR%\Miles\%DIR%"                               mkdir "%G2DIR%\Miles"
if not exist "%G2DIR%\Saves\%DIR%"                               mkdir "%G2DIR%\Saves"
if not exist "%G2DIR%\Saves\_current\%DIR%"                      mkdir "%G2DIR%\Saves\_current"
if not exist "%G2DIR%\System\%DIR%"                              mkdir "%G2DIR%\System"
if not exist "%G2DIR%\_work\%DIR%"                               mkdir "%G2DIR%\_work"
if not exist "%G2DIR%\_work\Data\%DIR%"                          mkdir "%G2DIR%\_work\Data"
if not exist "%G2DIR%\_work\Data\Anims\%DIR%"                    mkdir "%G2DIR%\_work\Data\Anims"
if not exist "%G2DIR%\_work\Data\Anims\MDS_Mobsi\%DIR%"          mkdir "%G2DIR%\_work\Data\Anims\MDS_Mobsi"
if not exist "%G2DIR%\_work\Data\Anims\MDS_Overlay\%DIR%"        mkdir "%G2DIR%\_work\Data\Anims\MDS_Overlay"
if not exist "%G2DIR%\_work\Data\Anims\_compiled\%DIR%"          mkdir "%G2DIR%\_work\Data\Anims\_compiled"
if not exist "%G2DIR%\_work\Data\Meshes\%DIR%"                   mkdir "%G2DIR%\_work\Data\Meshes"
if not exist "%G2DIR%\_work\Data\Meshes\_compiled\%DIR%"         mkdir "%G2DIR%\_work\Data\Meshes\_compiled"
if not exist "%G2DIR%\_work\Data\Music\%DIR%"                    mkdir "%G2DIR%\_work\Data\Music"
if not exist "%G2DIR%\_work\Data\Music\NewWorld\%DIR%"           mkdir "%G2DIR%\_work\Data\Music\NewWorld"
if not exist "%G2DIR%\_work\Data\Presets\%DIR%"                  mkdir "%G2DIR%\_work\Data\Presets"
if not exist "%G2DIR%\_work\Data\Scripts\%DIR%"                  mkdir "%G2DIR%\_work\Data\Scripts"
if not exist "%G2DIR%\_work\Data\Scripts\_compiled\%DIR%"        mkdir "%G2DIR%\_work\Data\Scripts\_compiled"
if not exist "%G2DIR%\_work\Data\Scripts\Content\%DIR%"          mkdir "%G2DIR%\_work\Data\Scripts\Content"
if not exist "%G2DIR%\_work\Data\Scripts\Content\Cutscene\%DIR%" mkdir "%G2DIR%\_work\Data\Scripts\Content\Cutscene"
if not exist "%G2DIR%\_work\Data\Scripts\System\%DIR%"           mkdir "%G2DIR%\_work\Data\Scripts\System"
if not exist "%G2DIR%\_work\Data\Scripts\System\Camera\%DIR%"    mkdir "%G2DIR%\_work\Data\Scripts\System\Camera"
if not exist "%G2DIR%\_work\Data\Scripts\System\Menu\%DIR%"      mkdir "%G2DIR%\_work\Data\Scripts\System\Menu"
if not exist "%G2DIR%\_work\Data\Scripts\System\Music\%DIR%"     mkdir "%G2DIR%\_work\Data\Scripts\System\Music"
if not exist "%G2DIR%\_work\Data\Scripts\System\PFX\%DIR%"       mkdir "%G2DIR%\_work\Data\Scripts\System\PFX"
if not exist "%G2DIR%\_work\Data\Scripts\System\SFX\%DIR%"       mkdir "%G2DIR%\_work\Data\Scripts\System\SFX"
if not exist "%G2DIR%\_work\Data\Scripts\System\VisualFX\%DIR%"  mkdir "%G2DIR%\_work\Data\Scripts\System\VisualFX"
if not exist "%G2DIR%\_work\Data\Scripts\System\_intern\%DIR%"   mkdir "%G2DIR%\_work\Data\Scripts\System\_intern"
if not exist "%G2DIR%\_work\Data\Textures\%DIR%"                 mkdir "%G2DIR%\_work\Data\Textures"
if not exist "%G2DIR%\_work\Data\Textures\_compiled\%DIR%"       mkdir "%G2DIR%\_work\Data\Textures\_compiled"
if not exist "%G2DIR%\_work\Data\Video\%DIR%"                    mkdir "%G2DIR%\_work\Data\Video"

echo ...fertig

REM --------------------------------------------------------------------------
REM [ move to target ]

echo.
echo Verschiebe Dateien in Zielordner...

set MOVEDIR=
for %%i in (EULA.txt)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic2.url)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %FOR% %%i in ("JoWooD Homepage.url")       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ReadMe.txt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ReadMe-1.30.txt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Register.url)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (VDFS.CFG)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=Data
for %%i in (Anims.vdf)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Meshes.vdf)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Sounds.vdf)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Sounds_Bird_01.vdf)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech1.vdf)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech2.vdf)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_English_Demo.vdf)           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_HeYou_CityGde_engl.vdf)     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_Parlan_engl.vdf)            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_Wegelagerer_deutsch.vdf)    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Textures.vdf)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Textures_Fonts_Apostroph.vdf)      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Textures_Multilingual_Jowood.vdf)  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Worlds.vdf)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=Miles
for %%i in (MssA3D.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssA3D2.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDolby.m3d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDs3D.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDs3Dh.m3d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDs3Ds.m3d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDx7.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDx7sh.m3d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDx7sl.m3d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDx7sn.m3d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssEAX.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssEAX2.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssFast.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssSoft.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssRSX.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=System
for %%i in (ar.exe)                            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BinkW32.dll)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BugslayerUtil.dll)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (GeDialogs.dll)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic.ini)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic2.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic2.rpt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ImageHl2.dll)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (KillHelp.exe)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MallocWin32Debug.dll)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MsDBI.dll)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Mss32.dll)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Paths.d)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SHW32.dll)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32e.dll)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32e.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32g.dll)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32g.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (AutoPan.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BandPass.flt)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Capture.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Chorus.flt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Compress.flt)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Flange.flt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (HighPass.flt)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (LagInter.flt)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (LowPass.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MDelay.flt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MP3Dec.asi)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssV12.asi)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssV24.asi)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssV29.asi)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ParmEq.flt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Reverb1.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Reverb2.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Reverb3.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SDelay.flt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ShelfEq.flt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Anims
for %%i in (Bloodfly.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Crawler.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Demon.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Draconian.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Dragon.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DragonSnapper.mds)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Dragon_Rock.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Giant_Bug.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Giant_Rat.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gobbo.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Golem.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Harpie.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Irrlicht.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Lurker.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Meatbug.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Molerat.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Orc.mds)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Scavenger.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Shadow.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Sheep.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Snapper.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Swampshark.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Troll.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Waran.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Wolf.mds)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Zombie.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Anims\MDS_Mobsi
for %%i in (BarBQ_NW_Misc_Sheep_01.mds)        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BarBQ_Scav.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Book_Blue.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BsAnvil_OC.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BsCool_OC.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BsFire_OC.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BsSharp_OC.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_NW_Normal_Locked.mds)     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_NW_Normal_Open.mds)       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_NW_Rich_Locked.mds)       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_NW_Rich_Open.mds)         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_OcChestLarge.mds)         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_OcChestLargeLocked.mds)   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_OcChestMedium.mds)        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_OcChestMediumLocked.mds)  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_OcCrateLarge.mds)         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_OcCrateLargeLocked.mds)   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestSmall_NW_Poor_Locked.mds)     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestSmall_NW_Poor_Open.mds)       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestSmall_OcChestSmall.mds)       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestSmall_OcChestSmallLocked.mds) do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestSmall_OcCrateSmall.mds)       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestSmall_OcCrateSmallLocked.mds) do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_NW_City_01.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_NW_DragonIsle_01.mds)         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_NW_DragonIsle_02.mds)         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_NW_Normal_01.mds)             do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_NW_Poor_01.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_NW_Rich_01.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Door_Wooden.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Fireplace_Ground2.mds)             do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Fireplace_High2.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Grave_Orc_1.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Grave_Orc_2.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Grave_Orc_3.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Grave_Orc_4.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Herb_Psi.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Lever_1_OC.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Pan_OC.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (RMaker_1.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Smoke_WaterPipe.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TouchPlate_Stone.mds)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TurnSwitch_Block.mds)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (VWheel_1_OC.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Anims\MDS_Overlay
for %%i in (FireWaran.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Golem_FireGolem.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Golem_IceGolem.mds)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_1hST1.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_1hST2.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_2hST1.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_2hST2.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Acrobatic.mds)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Arrogance.mds)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Babe.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_BowT1.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_BowT2.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_CBowT1.mds)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_CBowT2.mds)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Flee.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Mage.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Militia.mds)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Relaxed.mds)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Skeleton.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Skeleton_Fly.mds)           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Sprint.mds)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Swim.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Tired.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Torch.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Orc_Torch.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Music\NewWorld
for %%i in (BAN_DayFgt.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BAN_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Default.sgt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DI_DayStd.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLC_DayFgt.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLC_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Alpenhorn.dls)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Bass.dls)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Brass.dls)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Daduk.dls)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_DragonIsland.dls)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Flute.dls)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Guitar.dls)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Harp.dls)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Metronom.dls)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Organ.dls)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Percussions.dls)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Piano.dls)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Rare.dls)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DLS_Strings.dls)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (FRI_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gamestart.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (KAS_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Khorinis.sty)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (KH_DayFgt.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (KH_DayStd.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (LEU_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (LOB_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MAY_DayFgt.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MAY_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MI_DayStd.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MO_DayStd.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NewWorld.sty)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NW_DayFgt.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NW_DayStd.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NW_DayStd_A0.sgt)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NW_DayStd_A1.sgt)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NW_DayThr.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NW_MonoTest.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OWD_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OWP_DayFgt.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OWP_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (PIE_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (WOO_DayFgt.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (WOO_DayStd.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (WOO_DayThr.sgt)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %FOR% %%i in ("Xardas Tower.sty")          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (XT_DayStd.sgt)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Banditen.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_DragonIsland.sty)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_DragonLocationFGT.sty)            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_DragonLocationStd.sty)            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Earth.sty)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Friedhof.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Gamestart.sty)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Graveyard.sty)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Idylle.sty)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Kaserne.sty)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Khorinis.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Leuchtturm.sty)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_LobardsHof.sty)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Love.sty)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Mayatempel.sty)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Monastry.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Monastry_Indoor.sty)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_NewWorld.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Oldcamp.sty)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_OldWorld.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Orcs.sty)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_OW_DragonLocation.sty)            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_OW_Path.sty)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Pier.sty)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Taverne.sty)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Wood.sty)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %FOR% %%i in ("_Xardas Tower.sty")         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Presets
for %%i in (DialogCams.zen)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Lensflare.zen)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (LightPresets.zen)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\Content\Cutscene
for %%i in (OU.bin)                            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OU.csl)                            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System
for %%i in (Camera.src)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu.src)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Music.src)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ParticleFX.src)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SFX.src)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (VisualFX.src)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\Camera
for %%i in (CamInst.d)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\Menu
for %%i in (Menu_Defines.d)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Log.d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Main.d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Misc.d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt.d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt_Audio.d)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt_Controls.d)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt_Ext.d)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt_Game.d)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt_Graphics.d)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Opt_Video.d)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Savegame.d)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu_Status.d)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\Music
for %%i in (MusicInst.d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\PFX
for %%i in (ParticleFxEngine.d)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (PFX.d)                             do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (PfxMagic.d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\SFX
for %%i in (SfxInst.d)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SfxInstSpeech.d)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\VisualFX
for %%i in (VisualFxInst.d)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\_intern
for %%i in (Camera.d)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Classes.d)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Constants.d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Fight.d)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu.d)                            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Music.d)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ParticleFxDef.d)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SFX.d)                             do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (VisualFX.d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\_compiled
for %%i in (Camera.dat)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Fight.dat)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic.dat)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu.dat)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Music.dat)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OuInfo.inf)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ParticleFX.dat)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SFX.dat)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (VisualFX.dat)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Video
for %%i in (Credits.bik)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Credits2.bik)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Credits_Extro.bik)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DragonAttack.bik)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Extro_AllesWirdGut.bik)            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Extro_DJG.bik)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Extro_KDF.bik)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Extro_PAL.bik)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Extro_Xardas.bik)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (FishFood.bik)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Intro.bik)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Logo0.bik)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Logo1.bik)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Logo2.bik)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (LoveScene.bik)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OrcAttack.bik)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Ship.bik)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul

echo ...fertig

REM --------------------------------------------------------------------------
REM [ cleanup ]

echo.
echo Abschliessende Aktionen...

if exist "%G2DIR%\System\Gothic2.rpt" type nul > "%G2DIR%\System\Gothic2.rpt"

for %%i in (%%UNINSTALL_LANG%%) do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (DelSaves.exe)       do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (G2Setup.dll)        do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (G2Setup.exe)        do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (Gothic2.plc)        do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (Math64.dll)         do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (UnWise.exe)         do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (VssVer.scc)         do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"

if exist "%G2TMP%\%DIR%" rmdir "%G2TMP%"

echo ...fertig

if not exist "%G2TMP%\%DIR%" goto SkriptEnde
echo.
echo DER TEMPORŽRE ORDNER EXISTIERT NOCH!
echo BITTE šBERPRšFEN SIE DEN INHALT VON:
echo "%G2TMP%"

:SkriptEnde
if "%OS%" == "Windows_NT" endlocal
echo.
exit 0
