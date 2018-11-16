## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

## See configure.ac
bin_PROGRAMS = $(SSR_executables)

## All possible optional programs must be listed here
EXTRA_PROGRAMS = ssr-binaural ssr-wfs ssr-generic ssr-brs ssr-dca ssr-vbap ssr-aap

## CPPFLAGS: preprocessor flags, e.g. -I and -D
## -I., -I$(srcdir), and a -I pointing to the directory holding config.h
## are separately provided by Automake (disable with "nostdinc")

## for CFLAGS vs. AM_CFLAGS vs. mumble_CFLAGS read this (highly recommended!):
## http://www.gnu.org/software/automake/manual/html_node/Flag-Variables-Ordering.html

AM_CPPFLAGS =

## Add Audio Processing Framework (APF) path
AM_CPPFLAGS += -I$(srcdir)/../apf

if ENABLE_APP_BUNDLE
AM_CPPFLAGS += -DSSR_DATA_DIR=\"SoundScapeRenderer.app/Contents/Resources\"
else
AM_CPPFLAGS += -DSSR_DATA_DIR=\"$(pkgdatadir)\"
endif

## this is somehow needed for Solaris (or not?)
AM_CPPFLAGS += -D_POSIX_PTHREAD_SEMANTICS

## C++ compiler flags which are used for everything
AM_CXXFLAGS = $(WARNING_FLAGS) $(PKG_FLAGS) $(OPT_FLAGS) $(DEBUGGING_FLAGS)

# This is also passed to doxygen
DOXYGEN_DOC_DIR = ../doc/doxygen

# files which should be distributed but not installed
dist_noinst_DATA = Doxyfile coding_style.txt \
	../apf/misc/Makefile.dependencies \
	../apf/apf/mextools.h

ssr_binaural_SOURCES = ssr_binaural.cpp binauralrenderer.h \
	$(SSRSOURCES)

nodist_ssr_binaural_SOURCES = $(SSRMOCFILES)

ssr_wfs_SOURCES = ssr_wfs.cpp wfsrenderer.h \
	$(LOUDSPEAKERSOURCES) \
	$(SSRSOURCES)

nodist_ssr_wfs_SOURCES = $(SSRMOCFILES)

ssr_generic_SOURCES = ssr_generic.cpp genericrenderer.h \
	$(LOUDSPEAKERSOURCES) \
	$(SSRSOURCES)

nodist_ssr_generic_SOURCES = $(SSRMOCFILES)

ssr_vbap_SOURCES = ssr_vbap.cpp vbaprenderer.h \
	$(LOUDSPEAKERSOURCES) \
	$(SSRSOURCES)

nodist_ssr_vbap_SOURCES = $(SSRMOCFILES)

ssr_aap_SOURCES = ssr_aap.cpp aaprenderer.h \
	$(LOUDSPEAKERSOURCES) \
	$(SSRSOURCES)

nodist_ssr_aap_SOURCES = $(SSRMOCFILES)

ssr_brs_SOURCES = ssr_brs.cpp brsrenderer.h \
	$(SSRSOURCES)

nodist_ssr_brs_SOURCES = $(SSRMOCFILES)

ssr_dca_SOURCES = ssr_dca.cpp dcarenderer.h \
	dcacoefficients.h laplace_coeffs_double.h laplace_coeffs_float.h \
	../apf/apf/biquad.h \
	../apf/apf/denormalprevention.h \
	$(LOUDSPEAKERSOURCES) \
	$(SSRSOURCES)

nodist_ssr_dca_SOURCES = $(SSRMOCFILES)

LOUDSPEAKERSOURCES = \
	loudspeakerrenderer.h \
	loudspeaker.h

SSRSOURCES = \
	../apf/apf/blockdelayline.h \
	../apf/apf/combine_channels.h \
	../apf/apf/commandqueue.h \
	../apf/apf/container.h \
	../apf/apf/convolver.h \
	../apf/apf/cxx_thread_policy.h \
	../apf/apf/dummy_thread_policy.h \
	../apf/apf/fftwtools.h \
	../apf/apf/iterator.h \
	../apf/apf/jackclient.h \
	../apf/apf/jack_policy.h \
	../apf/apf/lockfreefifo.h \
	../apf/apf/math.h \
	../apf/apf/mimoprocessor_file_io.h \
	../apf/apf/mimoprocessor.h \
	../apf/apf/misc.h \
	../apf/apf/parameter_map.h \
	../apf/apf/pointer_policy.h \
	../apf/apf/posix_thread_policy.h \
	../apf/apf/rtlist.h \
	../apf/apf/shareddata.h \
	../apf/apf/sndfiletools.h \
	../apf/apf/stringtools.h \
	configuration.cpp \
	configuration.h \
	controller.h \
	directionalpoint.cpp \
	directionalpoint.h \
	maptools.h \
	orientation.cpp \
	orientation.h \
	position.cpp \
	position.h \
	posixpathtools.h \
	publisher.h \
	rendererbase.h \
	scene.cpp \
	scene.h \
	source.h \
	ssr_global.cpp \
	ssr_global.h \
	subscriber.h \
	timetools.h \
	tracker.h \
	xmlparser.cpp \
	xmlparser.h \
	rendersubscriber.h

if ENABLE_INTERSENSE
SSRSOURCES += trackerintersense.cpp trackerintersense.h
endif

if ENABLE_POLHEMUS
SSRSOURCES += trackerpolhemus.cpp trackerpolhemus.h
endif

if ENABLE_RAZOR
SSRSOURCES += trackerrazor.cpp trackerrazor.h
SSRSOURCES += razor-ahrs/RazorAHRS.cpp razor-ahrs/RazorAHRS.h
dist_noinst_DATA += \
	razor-ahrs/README.txt \
	razor-ahrs/GPL.txt \
	razor-ahrs/Example.cpp
endif

if ENABLE_VRPN
SSRSOURCES += trackervrpn.cpp trackervrpn.h
endif

if ENABLE_ECASOUND
SSRSOURCES += \
	audioplayer.cpp \
	audioplayer.h \
	audiorecorder.cpp \
	audiorecorder.h
endif

if ENABLE_IP_INTERFACE
AM_CPPFLAGS += -I$(srcdir)/network

SSRSOURCES += \
	network/commandparser.cpp \
	network/commandparser.h \
	network/connection.cpp \
	network/connection.h \
	network/networksubscriber.cpp \
	network/networksubscriber.h \
	network/server.cpp \
	network/server.h
endif

if ENABLE_GUI
AM_CPPFLAGS += -I$(srcdir)/gui

SSRSOURCES += \
	gui/qclicktextlabel.cpp \
	gui/qclicktextlabel.h \
	gui/qcpulabel.cpp \
	gui/qcpulabel.h \
	gui/qfilemenulabel.cpp \
	gui/qfilemenulabel.h \
	gui/qgui.cpp \
	gui/qguiframe.cpp \
	gui/qguiframe.h \
	gui/qgui.h \
	gui/qopenglplotter.cpp \
	gui/qopenglplotter.h \
	gui/qscenebutton.cpp \
	gui/qscenebutton.h \
	gui/qsourceproperties.cpp \
	gui/qsourceproperties.h \
	gui/qssrtimeline.cpp \
	gui/qssrtimeline.h \
	gui/qtimeedit.cpp \
	gui/qtimeedit.h \
	gui/quserinterface.cpp \
	gui/quserinterface.h \
	gui/qvolumeslider.cpp \
	gui/qvolumeslider.h \
	gui/qzoomlabel.cpp \
	gui/qzoomlabel.h

## moc-files should not go to the tarball because of possible version
## incompatibilities of Qt.
SSRMOCFILES = \
	gui/qclicktextlabel_moc.cpp \
	gui/qcpulabel_moc.cpp \
	gui/qfilemenulabel_moc.cpp \
	gui/qgui_moc.cpp \
	gui/qguiframe_moc.cpp \
	gui/qopenglplotter_moc.cpp \
	gui/qscenebutton_moc.cpp \
	gui/qsourceproperties_moc.cpp \
	gui/qssrtimeline_moc.cpp \
	gui/qtimeedit_moc.cpp \
	gui/quserinterface_moc.cpp \
	gui/qvolumeslider_moc.cpp \
	gui/qzoomlabel_moc.cpp

SUFFIXES = _moc.cpp

.h_moc.cpp:
	@test -d gui || $(MKDIR_P) gui
	@QTMOC@ @MOCFLAGS@ -I$(srcdir)/gui -o $@ $<
endif

doc:
	@echo "Running doxygen..." 
	@test -f Doxyfile || (echo \"Doxyfile\" not found! && false)
	@( cat Doxyfile \
		; echo PROJECT_NUMBER = '"Version @PACKAGE_VERSION@"'\
		; echo OUTPUT_DIRECTORY = $(DOXYGEN_DOC_DIR)\
	       	) \
		| doxygen - > /dev/null # discard stdout, only display errors

.PHONY: doc

## these links won't work on VPATH builds, but we don't care
all-local:
	cd ../data && for prog in $(bin_PROGRAMS) ; do \
	  $(RM) $$prog ; $(LN_S) local_ssr.sh $$prog ; done

clean-local:
	$(RM) gui/*_moc.cpp
	$(RM) -r $(DOXYGEN_DOC_DIR)
	cd ../data && for prog in $(bin_PROGRAMS) ; do $(RM) $$prog ; done

## Settings for Vim (http://www.vim.org/), please do not remove:
## vim:textwidth=80:comments+=bO\:##
