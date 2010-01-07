; DESCRIPTION: Blur Maxscript Extension installer script
; Diego Garcia
; (C) Blur Studios 2007

!include blurdlx-svnrev.nsi

!define MUI_PRODUCT "Blur Maxscript Extension"
!define MUI_VERSION "v1.5"
Name "${MUI_PRODUCT} ${MUI_VERSION}"

# These will change with different releases.
!define MAX_VERSION "8"

# These are all derived from the above.
!define DLL_INSTALLDIR  "C:\max${MAX_VERSION}\plugins\blurbeta\BlurLib_7\"

; Name of resulting executable installer
OutFile "blurdlx_${MUI_SVNREV}.exe"
InstallDir "${DLL_INSTALLDIR}"

SilentInstall silent

Section "install"
	IfFileExists $INSTDIR MaxPathExists
		Abort ;only install this in the computers that have the related version of max installed

	MaxPathExists:
	DetailPrint "${DLL_INSTALLDIR}"
	SetOverwrite on
	SetOutPath $INSTDIR
	File ".\release\blurdlx.dlx"
	
	ClearErrors
	FileOpen $0 "blurdlx_Version.txt" w
	IfErrors done
	FileWrite $0 "${MUI_SVNREV}"
	FileClose $0
	done:	
	
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayName" "${MUI_PRODUCT} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "UninstallString" "$INSTDIR\BlurDLXUninstall.exe"
	WriteUninstaller "$INSTDIR\BlurDLXUninstall.exe"
SectionEnd

Section "Uninstall"
;	;Delete Files
    SetOutPath $INSTDIR
	Delete $INSTDIR\blurdlx.dlx
	Delete $INSTDIR\blurdlx_Version.txt
	Delete "$INSTDIR\BlurDLXUninstall.exe"

	;Delete Uninstaller And Unistall Registry Entries
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${MUI_PRODUCT}"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}"  
SectionEnd

