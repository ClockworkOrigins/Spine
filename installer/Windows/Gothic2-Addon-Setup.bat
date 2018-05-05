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
if "%G2TMP%" == "" set G2TMP=C:\_gxsetup_
:WinNT
if "%ProgramFiles%" == "" set ProgramFiles=C:\Programme
if "%G2DIR%" == "" if not exist "%ProgramFiles%\%DIR%" mkdir "%ProgramFiles%"
if "%G2DIR%" == "" if not exist "%ProgramFiles%\JoWooD\%DIR%" mkdir "%ProgramFiles%\JoWooD"
if "%G2DIR%" == "" set G2DIR=%ProgramFiles%\JoWooD\Gothic II
if "%G2TMP%" == "" set G2TMP=%TEMP%\$G2Addon-Setup$
if "%G2CDR%" == "" set G2CDR=.

rem --------------------------------------------------------------------------
rem [ display infos ]

echo.
echo  Dieses Skript installiert die offizielle Erweiterung
echo  'Gothic II - Die Nacht des Raben' manuell von der CD
echo  nach: "%G2DIR%"
echo  Um die Erweiterung in ein anderes Verzeichnis zu in-
echo  stallieren, brechen Sie die Batch mit der Tastenkom-
echo  bination [Strg+C] ab und passen die G2DIR-Option an.
echo.
echo Das Skript wurde mit den folgenden Versionen des Setups
echo (Gothic2-Addon-Setup.exe) unter MS Windows XP getestet:
echo.
echo        2003-08-08, 2.50, Deutsch (Org. Release)
echo        2003-11-14, 2.60, Deutsch (Gold Edition)
echo.
echo Diese Art der "Installation" sollte nur benutzt werden,
echo wenn innerhalb der normalen Installation die Orginal-CD
echo nicht als solche erkannt wird. Es werden keine Daten in
echo die Registrierungsdatenbank (Registry) geschrieben (die
echo Deinstallation muss ebenso manuell erfolgen); und weder
echo im Startmen noch auf dem Desktop werden Links erzeugt.
echo Gothic II wird dann mit der folgenden Datei gestartet:
echo   "%G2DIR%\System\Gothic2.exe"
echo.

if exist "%G2DIR%\System\Gothic2.exe" goto DetectCDR

rem --------------------------------------------------------------------------
rem [ detect cd-rom ]

:DetectCDR

rem --------------------------------------------------------------------------
rem [ extract setup ]

:ExtractCD
echo.
echo Starte %G2CDR%\Gothic2-Addon-Setup.exe...

start /wait %G2CDR%\Gothic2-Addon-Setup.exe /X %G2TMP%

echo ...fertig

rem --------------------------------------------------------------------------
rem [ prepare target ]

echo.
echo Zielverzeichnis wird vorbereitet...

if exist "%G2DIR%\%DIR%" attrib -h -r -s "%G2DIR%\*.*" /S

for %%i in ("%G2DIR%\*.dmp")                                 do del "%%i"
for %%i in ("%G2DIR%\Miles\*.m3d")                           do del "%%i"
for %%i in ("%G2DIR%\System\*.asi")                          do del "%%i"
for %%i in ("%G2DIR%\System\*.flt")                          do del "%%i"
for %%i in ("%G2DIR%\System\*.m3d")                          do del "%%i"
for %%i in ("%G2DIR%\_work\Data\Scripts\System\PFX\*.d")     do del "%%i"
for %%i in ("%G2DIR%\_work\Data\Scripts\System\_intern\*.d") do del "%%i"

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
if not exist "%G2DIR%\_work\Data\Music\AddonWorld\%DIR%"         mkdir "%G2DIR%\_work\Data\Music\AddonWorld"
if not exist "%G2DIR%\_work\Data\Music\NewWorld\%DIR%"           mkdir "%G2DIR%\_work\Data\Music\NewWorld"
if not exist "%G2DIR%\_work\Data\Presets\%DIR%"                  mkdir "%G2DIR%\_work\Data\Presets"
if not exist "%G2DIR%\_work\Data\Scripts\%DIR%"                  mkdir "%G2DIR%\_work\Data\Scripts"
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
if not exist "%G2DIR%\_work\Data\Scripts\_compiled\%DIR%"        mkdir "%G2DIR%\_work\Data\Scripts\_compiled"
if not exist "%G2DIR%\_work\Data\Textures\%DIR%"                 mkdir "%G2DIR%\_work\Data\Textures"
if not exist "%G2DIR%\_work\Data\Textures\_compiled\%DIR%"       mkdir "%G2DIR%\_work\Data\Textures\_compiled"
if not exist "%G2DIR%\_work\Data\Video\%DIR%"                    mkdir "%G2DIR%\_work\Data\Video"
if not exist "%G2DIR%\_work\Tools\%DIR%"                         mkdir "%G2DIR%\_work\Tools"
if not exist "%G2DIR%\_work\Tools\Data\%DIR%"                    mkdir "%G2DIR%\_work\Tools\Data"
if not exist "%G2DIR%\_work\Tools\Data\ObjPresets\%DIR%"         mkdir "%G2DIR%\_work\Tools\Data\ObjPresets"
if not exist "%G2DIR%\_work\Tools\Max42\%DIR%"                   mkdir "%G2DIR%\_work\Tools\Max42"
if not exist "%G2DIR%\_work\Tools\UlraEdit\%DIR%"                mkdir "%G2DIR%\_work\Tools\UlraEdit"
if not exist "%G2DIR%\_work\Tools\VDFS\%DIR%"                    mkdir "%G2DIR%\_work\Tools\VDFS"
if not exist "%G2DIR%\_work\Tools\zSpy\%DIR%"                    mkdir "%G2DIR%\_work\Tools\zSpy"

echo ...fertig

REM --------------------------------------------------------------------------
REM [ move to target ]

echo.
echo Verschiebe Dateien in Zielordner...

set MOVEDIR=
for %%i in (Gothic2.url)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %FOR% %%i in ("JoWooD Homepage.url")       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ReadMe-Addon.txt)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (VDFS.CFG)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=Data
for %%i in (Anims_Addon.vdf)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Meshes_Addon.vdf)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Sounds_Addon.vdf)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Sounds_Bird_01.vdf)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_Addon.vdf)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_HeYou_CityGde_engl.vdf)     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_Parlan_engl.vdf)            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Speech_Wegelagerer_deutsch.vdf)    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Textures_Addon.vdf)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Textures_Fonts_Apostroph.vdf)      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Textures_Multilingual_Jowood.vdf)  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Worlds_Addon.vdf)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=Miles
for %%i in (MssA3D.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDs3D.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDx7.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssEAX.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssRSX.m3d)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssSoft.m3d)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=System
for %%i in (ar.exe)                            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BinkW32.dll)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic2.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gothic2.rpt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Mss32.dll)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MssDsp.flt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Paths.d)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Spacer2.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Spacer2.rpt)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32g.dll)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32g.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32e.dll)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Vdfs32e.exe)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Anims
for %%i in (Alligator.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BlattCrawler.mds)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Bloodfly.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Crawler.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Demon.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Draconian.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Dragon.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (DragonSnapper.mds)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (FireShadow.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Giant_Bug.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Giant_Rat.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Gobbo.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Golem.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Harpie.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Irrlicht.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Keiler.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Lurker.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Meatbug.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Molerat.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Orc.mds)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Razor.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Scavenger.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ScavengerGL.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Shadow.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Snapper.mds)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (StoneGuardian.mds)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (StonePuma.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SwampDrone.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SwampGolem.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SwampRat.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SwampShark.mds)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SwampZombie.mds)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Swarm.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Troll.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Waran.mds)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Wolf.mds)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Zombie.mds)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Anims\MDS_Mobsi
for %%i in (ChestBig_Add_Stone_Locked.mds)     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ChestBig_Add_Stone_Open.mds)       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TouchPlate_Seq_Stone_Key_01.mds)   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TouchPlate_Seq_Stone_Men_01.mds)   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TouchPlate_Seq_Stone_Storm_01.mds) do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TouchPlate_Seq_Stone_Sun_01.mds)   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (TouchPlate_Seq_Stone_Trap_01.mds)  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Anims\MDS_Overlay
for %%i in (FireWaran.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Skeleton.mds)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Humans_Skeleton_Fly.mds)           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Orc_Torch.mds)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OrcBiter.mds)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Music\AddonWorld
for %%i in (ADT_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ADW_Day_Fgt.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ADW_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (AWC_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BDT_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (BIB_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (CAN_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (PIR_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SHO_Day_Fgt.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SHO_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (STO_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SWP_Day_Std.sgt)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_AdanosTemple.sty)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_AddonWorld.sty)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_AddonWorldEntrance.sty)           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_BanditsCamp.sty)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_BibCanyon.sty)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Canyon.sty)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_PiratesCamp.sty)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_ShowDown.sty)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_StonePlates.sty)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_Swamp.sty)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Accordion.dls)               do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Bass.dls)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Celli.dls)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Fiddle.dls)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Guitar.dls)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Harp.dls)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Horn.dls)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_MonoPerc.dls)                do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_RareIngame.dls)              do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (__DLS_Violins.dls)                 do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Music\NewWorld
for %FOR% %%i in ("_Xardas Tower.sty")         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Presets
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
for %%i in (PfxInst.d)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (PfxInstEngine.d)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (PfxInstMagic.d)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\SFX
for %%i in (SfxInst.d)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (SfxInstSpeech.d)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\VisualFX
for %%i in (VisualFxInst.d)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Data\Scripts\System\_intern
for %%i in (Camera.d)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Menu.d)                            do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Music.d)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ParticleFx.d)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
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
for %%i in (Addon_Title.bik)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Credits.bik)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Credits2.bik)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Extro_Raven.bik)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Intro_Addon.bik)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OreHeap.bik)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Portal_Raven.bik)                  do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools
for %%i in (Macros.zmf)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools\Data
for %%i in (Addon.pml)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Effects.pml)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (MatLib.ini)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (NewWorld.pml)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OldCamp.pml)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (OldWorld.pml)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Portale.pml)                       do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Temp.pml)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Testlevel.pml)                     do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Tmp.pml)                           do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (Water.pml)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (_work.pml)                         do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools\Data\ObjPresets
for %%i in (zCCsCamera.opf)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (zCZoneZFogDefault.opf)             do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools\Max42
for %%i in (3dsExp.dle)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (3dsExp.txt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (3dsImp.dli)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (3dsImp.txt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ZenExp.dle)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (ZenExp.txt)                        do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools\UlraEdit
for %%i in (WordFile.txt)                      do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools\VDFS
for %%i in (GothicVDFS.exe)                    do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
set MOVEDIR=_work\Tools\zSpy
for %%i in (zSpy.exe)                          do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (zSpyDefault.cfg)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul
for %%i in (zSpyDefault.log)                   do if exist "%G2TMP%\%%i" move "%G2TMP%\%%i" "%G2DIR%\%MOVEDIR%\%%i" > nul

echo ...fertig

REM --------------------------------------------------------------------------
REM [ cleanup ]

echo.
echo Abschliessende Aktionen...

if exist "%G2DIR%\System\Gothic2.rpt" type nul > "%G2DIR%\System\Gothic2.rpt"
if exist "%G2DIR%\System\Spacer2.rpt" type nul > "%G2DIR%\System\Spacer2.rpt"
if exist "%G2DIR%\_work\Tools\zSpy\zSpyDefault.log" type nul > "%G2DIR%\_work\Tools\zSpy\zSpyDefault.log"

for %%i in (%%UNINSTALL_LANG%%) do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
for %%i in (CsSetup.exe)        do if exist "%G2TMP%\%%i" del "%G2TMP%\%%i"
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
