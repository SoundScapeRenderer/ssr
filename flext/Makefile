# Makefile to generate externals for Puredata

PKG_CONFIG ?= pkg-config

cflags += \
       -DFLEXT_INLINE \
       -Iflext/source \
       -I../apf \
       -I../src \
       -I../gml/include \
       `$(PKG_CONFIG) --cflags libxml-2.0` \
       -std=c++17 \
       -DENABLE_SOFA \
       -DNDEBUG \

# At least version 10.15 is needed for std::filesystem
define forDarwin
  cflags += -mmacosx-version-min=10.15
endef

ldlibs = -lfftw3f -lsndfile -lmysofa `$(PKG_CONFIG) --libs libxml-2.0`

class.sources = \
	ssr_aap~.cpp \
	ssr_binaural~.cpp \
	ssr_brs~.cpp \
	ssr_dca~.cpp \
	ssr_wfs~.cpp \
	ssr_vbap~.cpp \

common.sources = \
	legacy_position.cpp \
	legacy_orientation.cpp \
	legacy_directionalpoint.cpp \
	ssr_global.cpp \
	xmlparser.cpp \

datafiles = \
	$(wildcard *-help.pd) \
	ssr_messages.pd \
	8channels.asd \
	circle.asd \
	hrirs_fabian_min_phase_eq.wav \
	wfs_prefilter_120_1500_44100.wav \
	brirs_1.wav \
	brirs_2.wav \

include pd-lib-builder/Makefile.pdlibbuilder
