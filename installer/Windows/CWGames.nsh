;===============================================================================
;
;               CW Games Setup Macros
;               Copyright (c) 2016 Clockwork Origins
;
;                  System:   NSIS 2.0  http://nsis.sf.net/
;                  Editor:   HMNE 2.0  http://hmne.sf.net/
;
;===============================================================================


!ifndef CWGames_NSH_INCLUDED
!define CWGames_NSH_INCLUDED

;   Install
;


Function CWGames_GetInstallLocation
  Push $R0
  Push $R1
  Push $R2
  StrCpy $R1 "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Spine"
  ReadRegStr $R0 HKLM $R1 "InstallLocation"
  StrCmp $R0 "" "" done
  ReadRegStr $R0 HKLM $R1 "UninstallString"
  StrCmp $R0 "" done
  Push $R0
  Push "\UNWISE.EXE"
  Call CWGames_StrStr
  Pop $R1
  StrCmp $R1 "" "" +2
  StrCpy $R1 $R0
  StrLen $R2 $R1
  StrLen $R1 $R0
  IntOp $R1 $R1 - $R2
  StrCpy $R0 $R0 $R1
  GetFullPathName $R0 $R0
  done:
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd


Function CWGames_GetInstallVersion
  Exch $R0
  Push $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
  GetDLLVersion "$R0\Spine.exe" $R0 $R1
  IntOp $R2 $R0 / 0x00010000
  IntOp $R3 $R0 & 0x0000FFFF
  IntOp $R4 $R1 / 0x00010000
  IntOp $R5 $R1 & 0x0000FFFF
  StrCpy $R0 "$R2.$R3.$R4.$R5"
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

!macro CWGames_IfInstallVersion ROOT VERS JUMP
  Push $R0
  Push "${ROOT}"
  Call CWGames_GetInstallVersion
  Pop $R0
  StrCmp $R0 "${VERS}" +3
  Pop $R0
  Goto +3
  Pop $R0
  Goto "${JUMP}"
!macroend


Function CWGames_GetInstallVersionBase
  Exch $R0
  Push $R1
  Push $R2
  Push $R3
  GetDLLVersion "$R0\Spine.exe" $R0 $R1
  IntOp $R2 $R0 / 0x00010000
  IntOp $R3 $R0 & 0x0000FFFF
  StrCpy $R0 "$R2.$R3"
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

!macro CWGames_IfInstallVersionBase ROOT BASE JUMP
  Push $R0
  Push "${ROOT}"
  Call CWGames_GetInstallVersionBase
  Pop $R0
  StrCmp $R0 "${BASE}" +3
  Pop $R0
  Goto +3
  Pop $R0
  Goto "${JUMP}"
!macroend


Function CWGames_GetInstallVersionCode
  Exch $R0
  Push $R1
  Push $R2
  Push $R3
  ClearErrors
  GetDLLVersion "$R0\Spine.exe" $R0 $R1
  IntOp $R0 0 - 1
  IfErrors done
  IntOp $R0 $R1 & 0x0000FFFF
  done:
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

!macro CWGames_IfInstallVersionCode ROOT CODE JUMP
  Push $R0
  Push "${ROOT}"
  Call CWGames_GetInstallVersionCode
  Pop $R0
  IntCmp $R0 ${CODE} +3
  Pop $R0
  Goto +3
  Pop $R0
  Goto "${JUMP}"
!macroend


!macro CWGames_SetOutPath FILEPATH
  StrCmp "$OUTDIR" "${FILEPATH}" +2
  SetOutPath "${FILEPATH}"
!macroend


;===============================================================================
;
;   Directories
;


!macro CWGames_CreateDirectory FILEPATH
  IfFileExists "${FILEPATH}" +2
  CreateDirectory "${FILEPATH}"
!macroend


!macro CWGames_RemoveDirectory FILEPATH
  IfFileExists "${FILEPATH}\*.*" "" +2
  RMDir "${FILEPATH}"
!macroend


;===============================================================================
;
;   Files
;


!macro CWGames_DeleteFile FILENAME
  IfFileExists "${FILENAME}" "" +2
  Delete "${FILENAME}"
!macroend


!macro CWGames_SetFilename FILENAME
  IfFileExists "${FILENAME}" "" +2
  Rename "${FILENAME}" "${FILENAME}"
!macroend

;===============================================================================
;
;   Utilities
;


Function CWGames_StrStr
  Exch $R1
  Exch
  Exch $R2
  Push $R3
  Push $R4
  Push $R5
  StrLen $R3 $R1
  StrCpy $R4 0
  loop:
  StrCpy $R5 $R2 $R3 $R4
  StrCmp $R5 $R1 done
  StrCmp $R5 "" done
  IntOp $R4 $R4 + 1
  Goto loop
  done:
  StrCpy $R1 $R2 "" $R4
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Exch $R1
FunctionEnd


;===============================================================================


!endif ;CWGames_NSH_INCLUDED

