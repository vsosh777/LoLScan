; LoLScan NSIS installer script.
; Build from repo root:
;   makensis installer\LoLScan.nsi

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

!define APP_NAME "LoLScan"
!define APP_EXE "AppImgui.exe"
!define APP_VERSION "1.0.0"
!define APP_PUBLISHER "LoLScan"
!define DRIVER_SERVICE "Driver"

!define ROOT_DIR ".."
!define BUILD_DIR "${ROOT_DIR}\AppImgui\x64\Release"
!define DRIVER_DIR "${ROOT_DIR}\Driver\x64\Debug"
!define ICON_FILE "${ROOT_DIR}\AppImgui\AppImgui\icon.ico"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

!if /FileExists "${BUILD_DIR}\${APP_EXE}"
!else
  !error "Missing app binary: ${BUILD_DIR}\${APP_EXE}"
!endif

!if /FileExists "${BUILD_DIR}\fonts\JetBrainsMono-Regular.ttf"
!else
  !error "Missing font files in: ${BUILD_DIR}\fonts"
!endif

!if /FileExists "${DRIVER_DIR}\Driver.inf"
!else
  !error "Missing driver INF: ${DRIVER_DIR}\Driver.inf"
!endif

!if /FileExists "${DRIVER_DIR}\Driver.sys"
!else
  !error "Missing driver SYS: ${DRIVER_DIR}\Driver.sys"
!endif

Name "${APP_NAME}"
OutFile "${ROOT_DIR}\LoLScan-Setup-x64.exe"
Unicode True
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${UNINSTALL_KEY}" "InstallLocation"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
ShowInstDetails show
ShowUninstDetails show

!define MUI_ABORTWARNING
!define MUI_ICON "${ICON_FILE}"
!define MUI_UNICON "${ICON_FILE}"
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APP_NAME}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP|MB_OK "${APP_NAME} installer supports only 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd

Function un.onInit
  SetRegView 64
FunctionEnd

Section "LoLScan App" SEC_APP
  SetShellVarContext all
  SetOutPath "$INSTDIR"

  ; Stop running instance before replacing files.
  nsExec::ExecToLog '"$SYSDIR\taskkill.exe" /F /IM ${APP_EXE}'
  Pop $0

  File "${BUILD_DIR}\${APP_EXE}"
  SetOutPath "$INSTDIR\fonts"
  File /r "${BUILD_DIR}\fonts\*.*"

  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

  WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayVersion" "${APP_VERSION}"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "Publisher" "${APP_PUBLISHER}"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "${UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair" 1

  ; Enable app autostart by default (matches in-app Settings toggle state).
  nsExec::ExecToLog '"$SYSDIR\schtasks.exe" /Create /F /SC ONLOGON /RL HIGHEST /IT /TN "LoLScan Autostart" /TR "\$\"$INSTDIR\${APP_EXE}\$\" --autostart"'
  Pop $0
  ${If} $0 != 0
    MessageBox MB_ICONEXCLAMATION|MB_OK "Failed to create autostart task (exit code: $0). You can enable it later in Settings."
  ${EndIf}

  ; Remove legacy task name to avoid duplicates.
  nsExec::ExecToLog '"$SYSDIR\schtasks.exe" /Delete /F /TN "LoLScan Elevated Autostart"'
  Pop $0
SectionEnd

Section "Driver (x64 Debug)" SEC_DRIVER
  SetOutPath "$INSTDIR\driver"
  File "${DRIVER_DIR}\Driver.sys"
  File "${DRIVER_DIR}\Driver.inf"
  File /nonfatal "${DRIVER_DIR}\Driver.cer"

  ; Install/update as a legacy kernel service from Driver.sys (test-mode friendly).
  nsExec::ExecToLog '"$SYSDIR\sc.exe" query ${DRIVER_SERVICE}'
  Pop $0

  ${If} $0 == 0
    nsExec::ExecToLog '"$SYSDIR\sc.exe" stop ${DRIVER_SERVICE}'
    Pop $0
    Sleep 500
    nsExec::ExecToLog '"$SYSDIR\sc.exe" config ${DRIVER_SERVICE} type= kernel start= auto error= normal binPath= "$INSTDIR\driver\Driver.sys" DisplayName= "LoLScan Driver"'
    Pop $0
    ${If} $0 != 0
      MessageBox MB_ICONSTOP|MB_OK "Driver service reconfiguration failed (sc exit code: $0)."
      Abort
    ${EndIf}
  ${Else}
    nsExec::ExecToLog '"$SYSDIR\sc.exe" create ${DRIVER_SERVICE} type= kernel start= auto error= normal binPath= "$INSTDIR\driver\Driver.sys" DisplayName= "LoLScan Driver"'
    Pop $0
    ${If} $0 != 0
      MessageBox MB_ICONSTOP|MB_OK "Driver service creation failed (sc exit code: $0)."
      Abort
    ${EndIf}
  ${EndIf}

  ; Ensure the service is started (INF sets demand-start).
  nsExec::ExecToLog '"$SYSDIR\sc.exe" start ${DRIVER_SERVICE}'
  Pop $0
  ${If} $0 != 0
  ${AndIf} $0 != 1056
    MessageBox MB_ICONSTOP|MB_OK "Driver start failed (sc exit code: $0)."
    Abort
  ${EndIf}
SectionEnd

Section "Uninstall"
  SetShellVarContext all

  nsExec::ExecToLog '"$SYSDIR\taskkill.exe" /F /IM ${APP_EXE}'
  Pop $0

  ; Remove scheduled tasks created by the app (if present).
  nsExec::ExecToLog '"$SYSDIR\schtasks.exe" /Delete /F /TN "LoLScan Autostart"'
  Pop $0
  nsExec::ExecToLog '"$SYSDIR\schtasks.exe" /Delete /F /TN "LoLScan Elevated Autostart"'
  Pop $0

  ; Stop/remove the installed kernel service (best effort).
  nsExec::ExecToLog '"$SYSDIR\sc.exe" stop ${DRIVER_SERVICE}'
  Pop $0
  nsExec::ExecToLog '"$SYSDIR\sc.exe" delete ${DRIVER_SERVICE}'
  Pop $0

  Delete "$DESKTOP\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk"
  RMDir "$SMPROGRAMS\${APP_NAME}"

  Delete "$INSTDIR\${APP_EXE}"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR\fonts"
  RMDir /r "$INSTDIR\driver"
  RMDir "$INSTDIR"

  ; Remove per-user settings for current user.
  RMDir /r "$APPDATA\LoLScan"

  DeleteRegKey HKLM "${UNINSTALL_KEY}"
SectionEnd
