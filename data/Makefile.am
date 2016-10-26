## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

dist_bin_SCRIPTS = ssr

if ENABLE_APP_BUNDLE
SUBDIRS = MacOSX
extrasdir = $(bundledir)/Extras
scenesdir = $(bundledir)/Scenes

## these are always distributed, but only installed if ENABLE_APP_BUNDLE
dist_scenes_DATA = \
	scenes/asdf2html.xsl \
	scenes/live_input.asd
else
extrasdir = $(pkgdatadir)
endif

## stuff that will end up in $prefix/share/ssr/
nobase_dist_pkgdata_DATA = \
	asdf.xsd \
	images/listener_background.png \
	images/listener.png \
	images/listener_shadow.png \
	images/pause_button.png \
	images/pause_button_pressed.png \
	images/play_button.png \
	images/play_button_pressed.png \
	images/processing_button.png \
	images/processing_button_pressed.png \
	images/scene_menu_item.png \
	images/scene_menu_item_selected.png \
	images/skip_back_button.png \
	images/skip_back_button_pressed.png \
	images/source_shadow.png \
	images/ssr_logo_large.png \
	images/ssr_logo.png

## stuff that will end up either in $prefix/share/ssr/ or in $bundledir/Extras
nobase_dist_extras_DATA = \
	reproduction_setups/2.0.asd \
	reproduction_setups/2.1.asd \
	reproduction_setups/5.1.asd \
	reproduction_setups/rounded_rectangle.asd \
	reproduction_setups/circle.asd \
	reproduction_setups/loudspeaker_setup_with_nearly_all_features.asd \
	reproduction_setups/asdf2html.xsl \
	impulse_responses/hrirs/hrirs_fabian.wav \
	impulse_responses/hrirs/hrirs_fabian_min_phase_eq.wav \
	impulse_responses/hrirs/eq_filter_fabian_min_phase.wav \
	impulse_responses/hrirs/hrirs_kemar.wav \
	impulse_responses/hrirs/hrirs_kemar_min_phase_eq.wav \
	impulse_responses/hrirs/eq_filter_kemar_min_phase.wav \
	impulse_responses/hrirs/hrirs_fabian_documentation.pdf \
	impulse_responses/wfs_prefilters/wfs_prefilter_120_1500_44100.wav \
	impulse_responses/wfs_prefilters/wfs_prefilter_100_1300_44100.wav \
	impulse_responses/wfs_prefilters/wfs_prefilter_100_1300_48000.wav \
	impulse_responses/wfs_prefilters/wfs_prefilter_100_1800_44100.wav \
	impulse_responses/wfs_prefilters/wfs_prefilter_100_1800_48000.wav \
	matlab_scripts/prepare_hrirs_cipic.m \
	matlab_scripts/make_wfs_prefilter.m

# these will be removed on "make uninstall":
UNINSTALL_PKGDATA = \
	images

UNINSTALL_EXTRASDATA = \
	reproduction_setups \
	impulse_responses/hrirs \
	impulse_responses/wfs_prefilters \
	impulse_responses \
	matlab_scripts

## this is installed in $prefix/share/doc/ssr/
dist_doc_DATA = ssr.conf.example

## those files are thrown into the tarball but not installed
dist_noinst_DATA = \
	ssr.conf.local

dist_noinst_SCRIPTS = local_ssr.sh

## this is done after "make dist" is finished, use $(srcdir) and $(distdir)
##dist-hook:
##	...

# compute relative path between $(pkgdatadir) and $(extrasdir)
REL_LINK_DIR = $(shell \
	  target="${extrasdir}"; \
	  common_part="${pkgdatadir}"; \
	  if test "x$$target" = "x$$common_part"; then echo .; exit; fi; \
	  back= ; \
	  while test "x$${target\#$$common_part}" = "x$$target"; do \
	    common_part="`dirname "$$common_part"`"; \
	    back="../$$back" ; \
	  done; \
	  echo "$${back}$${target\#$$common_part/}")

# create symbolic links in the install directory (for default files) and
# before that, remove old symbolic links
install-data-hook:
	-cd $(DESTDIR)$(pkgdatadir) \
	  && test '!' -L default_hrirs.wav || $(RM) default_hrirs.wav \
	  && $(LN_S) $(REL_LINK_DIR)/impulse_responses/hrirs/hrirs_fabian_min_phase_eq.wav default_hrirs.wav
	-cd $(DESTDIR)$(pkgdatadir) \
	  && test '!' -L default_setup.asd || $(RM) default_setup.asd \
	  && $(LN_S) $(REL_LINK_DIR)/reproduction_setups/rounded_rectangle.asd default_setup.asd
	-cd $(DESTDIR)$(pkgdatadir) \
	  && test '!' -L default_wfs_prefilter.wav || $(RM) default_wfs_prefilter.wav \
	  && $(LN_S) $(REL_LINK_DIR)/impulse_responses/wfs_prefilters/wfs_prefilter_120_1500_44100.wav default_wfs_prefilter.wav

# remove the symlinks, UNINSTALL_DIRS (if empty)
uninstall-hook:
	-cd $(DESTDIR)$(pkgdatadir) \
		&& test -L default_hrirs.wav \
		&& $(RM) default_hrirs.wav
	-cd $(DESTDIR)$(pkgdatadir) \
		&& test -L default_setup.asd \
		&& $(RM) default_setup.asd
	-cd $(DESTDIR)$(pkgdatadir) \
		&& test -L default_wfs_prefilter.wav \
		&& $(RM) default_wfs_prefilter.wav
	-cd $(DESTDIR)$(pkgdatadir) \
		&& for dir in $(UNINSTALL_PKGDATA) ; do rmdir $$dir ; done
	-cd $(DESTDIR)$(extrasdir) \
		&& for dir in $(UNINSTALL_EXTRASDATA) ; do rmdir $$dir ; done

## Settings for Vim (http://www.vim.org/), please do not remove:
## vim:textwidth=80:comments+=bO\:##
