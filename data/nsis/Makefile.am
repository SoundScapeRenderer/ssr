## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

## This file is only taken into account if --enable-windows-installer was chosen!
if ENABLE_WINDOWS_INSTALLER

# this is done after installing the executables
install-exec-hook:
## the line starting with "ldd" was adapted from
## https://github.com/msys2/MINGW-packages/issues/5204#issuecomment-1013818547
	for exec in $(SSR_executables); do \
	  windeployqt --no-compiler-runtime $(DESTDIR)$(bindir)/$$exec ; \
	  ldd $(DESTDIR)$(bindir)/$$exec | grep -iv system32 | cut -f2 -d\> | cut -f1 -d\( | while read a; do ! [ -e "$(DESTDIR)$(bindir)/`basename $$a`" ] && cp -v "$$a" $(DESTDIR)$(bindir)/ || true; done \
	done
## those are copied by windeployqt, but they are not needed:
	$(RM) -r $(DESTDIR)$(bindir)/styles
	$(RM) -r $(DESTDIR)$(bindir)/imageformats

# create Windows installer
nsis:
	@echo "TODO: run NSIS!"

.PHONY: nsis

endif
