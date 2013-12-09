## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

## This file is only taken into account if --enable-app-bundle was chosen!
if ENABLE_APP_BUNDLE

dist_pkgdata_DATA = SSRIcon.icns qt.conf
contentsdir = $(prefix)
contents_DATA = Info.plist
dist_bin_SCRIPTS = run-ssr.sh run-ssr.scpt

nobase_dist_dmgroot_DATA = .background/background.png Jack.webloc Getting-Started.txt

## the helper program dylibbundler is compiled but not installed
noinst_PROGRAMS = dylibbundler/dylibbundler

dylibbundler_dylibbundler_SOURCES = \
	dylibbundler/src/Dependency.cpp \
	dylibbundler/src/Dependency.h \
	dylibbundler/src/DylibBundler.cpp \
	dylibbundler/src/DylibBundler.h \
	dylibbundler/src/main.cpp \
	dylibbundler/src/Settings.cpp \
	dylibbundler/src/Settings.h \
	dylibbundler/src/Utils.cpp \
	dylibbundler/src/Utils.h

## the makefile is actually not used, we include it for the sake of completeness
dist_noinst_DATA = dylibbundler/help.html dylibbundler/makefile

# this is done after installing the executable
install-exec-hook:
	for exec in $(SSR_executables) ; do \
		$(builddir)/dylibbundler/dylibbundler \
		--overwrite-dir --bundle-deps \
		--fix-file $(DESTDIR)$(bindir)/$$exec \
		--dest-dir $(DESTDIR)$(contentsdir)/Libraries \
		--install-path @executable_path/../Libraries \
	; done
	cp `which ecasound` $(DESTDIR)$(bindir)

install-data-hook:
	cp -r $(QTLIBDIR)/Resources/qt_menu.nib $(DESTDIR)$(pkgdatadir)/
	-cd $(DESTDIR)$(dmgrootdir) && $(LN_S) /Applications

# create a DMG image
dmg:
	@test -d "$(dmgrootdir)" || ( echo; echo; \
		echo "MacOSX App-Bundle directory not found!"; \
		echo "run 'make install' first!"; echo; false )
	hdiutil create -ov -attach -format UDRW \
		-volname "SoundScape Renderer @PACKAGE_VERSION@" \
		-srcfolder "$(dmgrootdir)" \
		"$(top_builddir)/SSRTemp.dmg"
	osascript DMG-Layout.applescript
	hdiutil detach "/Volumes/SoundScape Renderer @PACKAGE_VERSION@"
	hdiutil convert -ov -format UDZO -imagekey zlib-level=9 \
		-o "$(top_builddir)/$(DMG_NAME)" \
		"$(top_builddir)/SSRTemp.dmg"
	$(RM) "$(top_builddir)/SSRTemp.dmg"
	$(RM) -r "$(dmgrootdir)"

.PHONY: dmg

endif
