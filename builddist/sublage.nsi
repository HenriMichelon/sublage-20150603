Name "Sublage"
OutFile "sublage-r0-setup_x86_64.exe"
InstallDir "$PROGRAMFILES64\Sublage"
InstallDirRegKey HKLM "Software\Sublage" "Install_Dir"
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Sublage (required)"
	SectionIn RO    
	SetOutPath $INSTDIR\bin  
	File "..\sublage\sublage.exe"  
	File "..\sublage\sublagec.exe"  
	File "..\sublage\sublage.exe" 
	File "..\sublage\sublagert.dll"    

	SetOutPath $INSTDIR\lib\sublage\stdlib  
	File "..\stdlib\binaries\console.library"  
	File "..\stdlib\binaries\console.library.native"  
	File "..\stdlib\binaries\socket.library"  
	File "..\stdlib\binaries\socket.library.native"  
	File "..\stdlib\binaries\string.library"  
	File "..\stdlib\binaries\string.library.native"  
	File "..\stdlib\binaries\thread.library"  
	File "..\stdlib\binaries\thread.library.native"  
	File "..\stdlib\binaries\array.library"  
	File "..\stdlib\binaries\array.library.native"  
	File "..\stdlib\binaries\io.library"  
	File "..\stdlib\binaries\io.library.native"  
	File "..\stdlib\binaries\mutex.library"  
	File "..\stdlib\binaries\mutex.library.native"  
	File "..\stdlib\binaries\math.library"  
	File "..\stdlib\binaries\math.library.native"  
	File "..\stdlib\binaries\appserver.library" 
   
	WriteRegStr HKLM SOFTWARE\Sublage "Install_Dir" "$INSTDIR"    
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sublage" "DisplayName" "Sublage"  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sublage" "UninstallString" '"$INSTDIR\uninstall.exe"' 
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sublage" "NoModify" 1 
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sublage" "NoRepair" 1  

	WriteUninstaller "uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts"  
	CreateDirectory "$SMPROGRAMS\Sublage"  
	CreateShortCut "$SMPROGRAMS\Sublage\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Uninstall"  
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sublage"  
	DeleteRegKey HKLM SOFTWARE\Sublage  
	Delete $INSTDIR\lib\sublage\stdlib\*.*  
	Delete $INSTDIR\lib\sublage\bin\*.*  
	Delete $SYSDIR\sublagert.dll  
	Delete "$SMPROGRAMS\Sublage\*.*"  
	RMDir "$SMPROGRAMS\Sublage"  
	RMDir "$INSTDIR\lib\sublage\stdlib"  
	RMDir "$INSTDIR\lib\sublage"  
	RMDir "$INSTDIR\lib"  
	RMDir "$INSTDIR\bin"  
	RMDir "$INSTDIR"
SectionEnd