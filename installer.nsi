# define name of installer
OutFile "SSR-installer.exe"
 
# The default installation directory
InstallDir $PROGRAMFILES64\SSR
 
RequestExecutionLevel admin

;--------------------------------
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

Section "Start Menu Shortcuts (required)"
    # create a shortcut named "SSR" in the start menu programs directory
    # point the new shortcut at the program uninstaller
    CreateShortcut "$SMPROGRAMS\SoundScapeRenderer(SSR)\uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

 
# uninstaller section start
Section "un.Uninstaller Section"
    # TODO

    # Remove the link from the start menu
    Delete "$SMPROGRAMS\SoundScapeRenderer(SSR)\uninstall.lnk"

    RMDir "$SMPROGRAMS\SoundScapeRenderer(SSR)"
    RMDir $INSTDIR
# uninstaller section end
SectionEnd