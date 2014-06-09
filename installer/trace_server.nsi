# This is install script for NSIS installer for Trace Server (win x64)

# where to create install.exe
OutFile "c:\_builds\trace_server\install_trace_server.exe"

# good for debugging
ShowInstDetails Show

#!include LogicLib.nsh
#!include WinVer.nsh
#!include nsDialogs.nsh
#!include x64.nsh
#!include Sections.nsh
#!addplugindir .

# Set the text to prompt user to enter a directory
DirText "This will install trace server program on your computer. Choose a directory"

Name "Trace Server 4 Windows"
RequestExecutionLevel admin
AddBrandingImage left 256

Page Custom brandimage "" ": Brand Image"
Page License
Page Directory
Page Components
Page InstFiles

Function brandimage
  SetOutPath "$TEMP"
  SetFileAttributes installer.bmp temporary
  File installer.bmp
  SetBrandingImage "$TEMP\installer.bmp" /resizetofit
FunctionEnd

### msvc redist
Function redistPageEnter
  File redist\vcredist_x64.exe
  File redist\vcredist_x86.exe

  ExecWait 'vcredist_x64.exe /install /passive'
  ExecWait 'vcredist_x86.exe /install /passive'
FunctionEnd


!define PRODUCT "Trace Server"
!define FILE "trace_server"
!define VERSION "0.9.10-4"
!define BRANDINGTEXT "Trace Server"
CRCCheck On

!include "${NSISDIR}\Contrib\Modern UI\System.nsh"

;---------------------------------
;General

ShowInstDetails "nevershow"
ShowUninstDetails "nevershow"
;SetCompressor "bzip2"

!define ICON "Icon.ico"
!define UNICON "Icon.ico"
#!define SPECIALBITMAP "Bitmap.bmp"


;--------------------------------
;Folder selection page

InstallDir "$PROGRAMFILES64\${PRODUCT}"


;--------------------------------
;Modern UI Configuration

!define WELCOMEPAGE
!define LICENSEPAGE
!define DIRECTORYPAGE
!define ABORTWARNING
!define UNINSTALLER
!define UNCONFIRMPAGE
!define FINISHPAGE


;--------------------------------
;Language

#!insertmacro LANGUAGE "English"


;--------------------------------
;Modern UI System

#!insertmacro SYSTEM


;--------------------------------
;Data

LicenseData "LICENSE"


;--------------------------------
;Installer Sections
Section "install"

  ;Add files
  SetOutPath "$INSTDIR"
  File "${FILE}.exe"
  File "VisualStudioFileOpenTool.all.exe"

  ;create desktop shortcut
  CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\${FILE}.exe" ""

  ;create start-menu items
  CreateDirectory "$SMPROGRAMS\${PRODUCT}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\${PRODUCT}\${PRODUCT}.lnk" "$INSTDIR\${FILE}.exe" "" "$INSTDIR\${FILE}.exe" 0

  ;write uninstall information to the registry
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  call redistPageEnter
SectionEnd


;--------------------------------
;Uninstaller Section
Section "Uninstall"
  ;Delete Files
  RMDir /r "$INSTDIR\*.*"
  ;Remove the installation directory
  RMDir "$INSTDIR"
  ;Delete Start Menu Shortcuts
  Delete "$DESKTOP\${PRODUCT}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT}\*.*"
  RmDir  "$SMPROGRAMS\${PRODUCT}"
  ;Delete Uninstaller And Unistall Registry Entries
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT}"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
SectionEnd

;--------------------------------
;MessageBox Section

;Function that calls a messagebox when installation finished correctly
Function .onInstSuccess
  MessageBox MB_OK "You have successfully installed ${PRODUCT}. Use the desktop icon to start the program."
FunctionEnd

Function un.onUninstSuccess
  MessageBox MB_OK "You have successfully uninstalled ${PRODUCT}."
FunctionEnd

