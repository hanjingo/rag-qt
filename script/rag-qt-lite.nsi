; rag-qt NSIS installer script
; Build command example:
;   makensis rag-qt.nsi

Unicode true

!include "MUI2.nsh"
!include "FileFunc.nsh"

!define APP_NAME "rag-qt"
!define APP_VERSION "0.0.1"
!define APP_PUBLISHER "hanjingo"
!define APP_EXE "rag-qt.exe"

; This script assumes it is compiled from repository root.
!define SOURCE_DIR "..\bin\Release-Lite"
!define INSTALL_ROOT "C:"
!define INSTALL_DIR "${INSTALL_ROOT}\${APP_NAME}"
!define STARTMENU_DIR "$SMPROGRAMS\${APP_NAME}"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "..\bin\${APP_NAME}-lite-${APP_VERSION}-setup.exe"
InstallDir "${INSTALL_DIR}"
InstallDirRegKey HKLM "${UNINST_KEY}" "InstallLocation"
RequestExecutionLevel admin

VIProductVersion "0.0.1.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey "FileDescription" "${APP_NAME} Installer"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "LegalCopyright" "Copyright (C) ${APP_PUBLISHER}"



!define MUI_ABORTWARNING
!define MUI_ICON "..\res\icons\logo.ico"
!define MUI_UNICON "..\res\icons\logo.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

Section "Install" SEC_MAIN
	SetOutPath "$INSTDIR"

	; Copy Release-Lite runtime recursively, skipping data directories from bulk copy.
	File /r \
		/x "*.ilk" \
		/x "*.pdb" \
		/x "*.dmp" \
		/x "*.exp" \
		/x "*.lib" \
		/x "default*.log" \
		/x "rag-core_test.exe" \
		/x "configs" \
		/x "configs\*" \
		/x "plugins" \
		/x "plugins\*" \
		/x "log" \
		/x "log\*" \
		/x "models" \
		/x "models\*" \
		/x "tmp" \
		/x "tmp\*" \
		"${SOURCE_DIR}\*"

	; Conditional installation for subdirectories: if already exists, skip; otherwise copy from package.
	IfFileExists "$INSTDIR\configs" +2
	File /r "${SOURCE_DIR}\configs"

	IfFileExists "$INSTDIR\plugins" +2
	File /r "${SOURCE_DIR}\plugins"

	; Ensure log directory exists
	IfFileExists "$INSTDIR\log" +2
	CreateDirectory "$INSTDIR\log"

	WriteUninstaller "$INSTDIR\Uninstall.exe"

	CreateDirectory "${STARTMENU_DIR}"
	CreateShortCut "${STARTMENU_DIR}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
	CreateShortCut "${STARTMENU_DIR}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"
	CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"

	WriteRegStr HKLM "${UNINST_KEY}" "DisplayName" "${APP_NAME} ${APP_VERSION}"
	WriteRegStr HKLM "${UNINST_KEY}" "DisplayVersion" "${APP_VERSION}"
	WriteRegStr HKLM "${UNINST_KEY}" "Publisher" "${APP_PUBLISHER}"
	WriteRegStr HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "${UNINST_KEY}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
	WriteRegStr HKLM "${UNINST_KEY}" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
	WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify" 1
	WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair" 1
SectionEnd

Section "Uninstall"
	Delete "$DESKTOP\${APP_NAME}.lnk"
	Delete "${STARTMENU_DIR}\${APP_NAME}.lnk"
	Delete "${STARTMENU_DIR}\Uninstall ${APP_NAME}.lnk"
	RMDir "${STARTMENU_DIR}"

	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\*.*"

	FindFirst $0 $1 "$INSTDIR\*"
	loop:
		StrCmp $1 "" done
		StrCmp $1 "." next
		StrCmp $1 ".." next
		StrCmp $1 "configs" next
		StrCmp $1 "models" next
		StrCmp $1 "log" next
		StrCmp $1 "plugins" next
		IfFileExists "$INSTDIR\$1\*.*" 0 next
		RMDir /r "$INSTDIR\$1"
	next:
		FindNext $0 $1
		Goto loop
	done:
		FindClose $0
	
	RMDir "$INSTDIR"

	DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd

Function .onInstSuccess
	DetailPrint "Install completed: $INSTDIR"
FunctionEnd

Function un.onUninstSuccess
	HideWindow
	MessageBox MB_ICONINFORMATION|MB_OK "${APP_NAME} was successfully removed. (configs, log and plugins directories preserved)"
FunctionEnd