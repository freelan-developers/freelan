# FreeLAN installer - NSIS script
#
# Author: Julien Kauffmann <julien.kauffmann@freelan.org>

name "${NAME} ${VERSION}"
outFile "${NAME}-${VERSION}-${ARCH}-install.exe"
showInstDetails show
showUnInstDetails show

; To enable ${If}-like constructs
!include "LogicLib.nsh"

Function .onInit
; To pick the appropriate installation folder
${If} ${ARCH} == "amd64"
StrCpy $INSTDIR "$PROGRAMFILES64\FreeLAN"
${Else}
StrCpy $INSTDIR "$PROGRAMFILES\FreeLAN"
${EndIf}
FunctionEnd

; Enables Modern User Interface 2
!include "MUI2.nsh"

; MUI2 Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"

# Installer pages
; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

# Uninstaller pages
; Welcome page
!insertmacro MUI_UNPAGE_WELCOME
; Confirm page
!insertmacro MUI_UNPAGE_CONFIRM
# Instfiles page
!insertmacro MUI_UNPAGE_INSTFILES
# Finish page
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"

section
setOutPath $INSTDIR
writeUninstaller $INSTDIR\uninstall.exe
sectionEnd

section "Binaries" section_Binaries
setOverwrite ifnewer

setOutPath $INSTDIR\bin
file ..\..\freelan\bin\freelan.exe
sectionEnd
langString DESC_section_Binaries ${LANG_ENGLISH} "The FreeLAN binary files."

section "Sample configuration" section_Sample_configuration
setOverwrite ifnewer

setOutPath $INSTDIR\config
file ..\..\freelan\config\freelan.cfg
sectionEnd
langString DESC_section_Sample_configuration ${LANG_ENGLISH} "A sample configuration file."

section "Uninstall"
delete $INSTDIR\uninstall.exe
;delete $INSTDIR\config\freelan.cfg
delete $INSTDIR\bin\freelan.exe
rmDir $INSTDIR\config
rmDir $INSTDIR\bin
sectionEnd

# MUI2 components descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${section_Binaries} $(DESC_section_Binaries)
!insertmacro MUI_DESCRIPTION_TEXT ${section_Sample_configuration} $(DESC_section_Sample_configuration)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
