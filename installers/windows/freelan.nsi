; FreeLAN installer - NSIS script
;
; Author: Julien Kauffmann <julien.kauffmann@freelan.org>

; Include the configuration
!include "config.nsh"

; This enables the new GUI style
!include "MUI.nsh"
;
; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "resources\icons\freelan.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

Name "${PRODUCT_NAME} - ${PRODUCT_VERSION}"
OutFile "freelan_${PRODUCT_VERSION}-setup_${ARCH}.exe"
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" MainSection
SetOutPath "$INSTDIR"
SetOverwrite ifnewer
File "..\..\freelan\bin\freelan.exe"
CreateDirectory "$SMPROGRAMS\FreeLAN"
SectionEnd

Section -Post
WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd

Section Uninstall
RMDir "$SMPROGRAMS\FreeLAN"
SectionEnd
