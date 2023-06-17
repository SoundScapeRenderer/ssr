## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

## This file is only taken into account if --enable-windows-installer was chosen!
if ENABLE_WINDOWS_INSTALLER

# this is done after installing the executables
install-exec-hook:
	for exec in $(SSR_executables); do \
	  windeployqt --no-compiler-runtime $(DESTDIR)$(bindir)/$$exec ; \
	done
	$(RM) -r $(DESTDIR)$(bindir)/styles
	$(RM) -r $(DESTDIR)$(bindir)/imageformats

# create Windows installer
nsis:
	@echo "TODO: run NSIS!"

.PHONY: nsis

endif
