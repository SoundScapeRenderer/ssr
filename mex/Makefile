# Makefile for building MEX-files

# Makefile example:
# http://www.klab.caltech.edu/~harel/share/gbvs_scale/sift/Makefile

MEXFILES ?= ssr_nfc_hoa ssr_binaural ssr_vbap ssr_aap ssr_wfs

SOURCES := ssr_global position orientation directionalpoint xmlparser

# TODO: separate libraries for different renderers?
LIBRARIES += libxml-2.0 fftw3f sndfile

APF_DIR ?= ../apf
SRC_DIR ?= ../src
MEX ?= mex -v
OCT ?= mkoctfile --mex --verbose

CXXFLAGS += -std=c++11

# optimization:
CXXFLAGS += -O3
#CXXFLAGS += -fomit-frame-pointer -ffast-math -funroll-loops
#CXXFLAGS += -march=native
CPPFLAGS += -DNDEBUG
# TODO: check for SSE?

OCTAVE_TARGETS := $(MEXFILES:%=%.octave)
MATLAB_TARGETS := $(MEXFILES:%=%.matlab)

SOURCES := $(SOURCES:%=$(SRC_DIR)/%.cpp)

CXXFLAGS += -pthread

# maybe for Mac OS X:
#CFLAGS += -fno-common -no-cpp-precomp -fexceptions

#MEX_CPPFLAGS += -DSSR_MEX_USE_DOUBLE

MEX_CPPFLAGS += -I$(APF_DIR) -I$(SRC_DIR)

# show many warnings
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -pedantic
# warnings are errors
CXXFLAGS += -pedantic-errors
CXXFLAGS += -Werror
# even more warnings:
CXXFLAGS += -Wpointer-arith
CXXFLAGS += -Wcast-align
CXXFLAGS += -Wwrite-strings
CXXFLAGS += -Wredundant-decls
#CXXFLAGS += -Wlong-long
#CXXFLAGS += -Wconversion
#CXXFLAGS += -Wsign-conversion
#CXXFLAGS += -Wshadow
#CXXFLAGS += -Wold-style-cast
#CXXFLAGS += -Winline

PKG_CONFIG ?= pkg-config
MEX_CPPFLAGS += `$(PKG_CONFIG) --cflags $(LIBRARIES)`
MEX_LDLIBS += `$(PKG_CONFIG) --libs $(LIBRARIES)`

CPPFLAGS += $(MEX_CPPFLAGS)
CFLAGS += $(MEX_CFLAGS)
CXXFLAGS += $(MEX_CXXFLAGS)
LDLIBS += $(MEX_LDLIBS)

# don't use implicit rules:
.SUFFIXES:

all: octave matlab

octave: $(OCTAVE_TARGETS)

matlab: $(MATLAB_TARGETS)

# Targets have different extension depending on the platform.
# To avoid finding out the extension, we use empty *.stamp files.

$(OCTAVE_TARGETS) $(MATLAB_TARGETS): %: .%.stamp

.%.octave.stamp: %.cpp $(SOURCES)
	CC="$(CXX)" CXX="$(CXX)" LD="$(CXX)" \
	   CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" \
	   $(OCT) $(MEX_CPPFLAGS) $(MEX_CFLAGS) $(MEX_CXXFLAGS) $(MEX_LDLIBS) $^
	@touch $@

# TODO: this works for 64bit systems, check if it also works on 32bit
.%.matlab.stamp: CXXFLAGS += -fPIC

.%.matlab.stamp: %.cpp $(SOURCES)
	$(MEX) $(MEX_CPPFLAGS) $(MEX_CFLAGS) $(MEX_CXXFLAGS) $(MEX_LDLIBS) \
		CC="$(CXX)" CXX="$(CXX)" LD="$(CXX)" \
		CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" $^
	@touch $@

# To specify MEX file name: -output bla

clean:
	$(RM) $(MEXFILES:%=%.mexa64)
	$(RM) $(MEXFILES:%=%.mexglx)
	$(RM) $(MEXFILES:%=%.mexmaci)
	$(RM) $(MEXFILES:%=%.mexmaci64)
	$(RM) $(MEXFILES:%=%.mex)
	$(RM) *.o
	$(RM) .*.stamp

.PHONY: all octave matlab clean $(OCTAVE_TARGETS) $(MATLAB_TARGETS)

.DELETE_ON_ERROR:

