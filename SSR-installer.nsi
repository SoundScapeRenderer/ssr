# define name of installer
OutFile "SSR-installer.exe"
 
# The default installation directory
InstallDir $PROGRAMFILES64\SoundScapeRenderer
 
RequestExecutionLevel admin

;--------------------------------
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles
;-------------------------------

 
# start default section
Section "Installer Section"

    # set the installation directory as the destination for the following actions
    SetOutPath $INSTDIR

    # specify file to go in output path
    File ssr-win-bin\*.exe

    # add dependencies
    File /r /x libjack*.dll ssr-win-bin\deps\*.*
 
    # add resources
    File /r ssr-win-bin\data\

    File /oname=data\default_hrirs.wav ssr-win-bin\data\impulse_responses\hrirs\hrirs_fabian_min_phase_eq.wav

    # create the uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts"
    # create a shortcut named "SoundScapeRenderer" in the start menu programs directory
    # point the new shortcut at the program uninstaller
    CreateDirectory "$SMPROGRAMS\SoundScapeRenderer"
    CreateShortcut "$SMPROGRAMS\SoundScapeRenderer\uninstall.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

 
# uninstaller section start
Section "un.Uninstaller Section"
    # TODO

    # Remove the link from the start menu
    Delete "$SMPROGRAMS\SoundScapeRenderer\uninstall.lnk"

    RMDir "$SMPROGRAMS\SoundScapeRenderer"
    RMDir $INSTDIR

    MessageBox MB_OK "You need to manually remove the files for now!"

# uninstaller section end
SectionEnd