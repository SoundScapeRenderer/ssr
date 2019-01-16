# Makefile for building MEX-files

# Makefile example:
# http://www.klab.caltech.edu/~harel/share/gbvs_scale/sift/Makefile

MEXFILES ?= ssr_dca ssr_binaural ssr_vbap ssr_aap ssr_wfs ssr_generic \
  ssr_brs

OBJECTS := ssr_global legacy_position legacy_orientation \
  legacy_directionalpoint xmlparser

LIBRARIES += libxml-2.0 fftw3f sndfile

APF_DIR ?= ../apf
SRC_DIR ?= ../src
MEX ?= mex -v
OCT ?= mkoctfile --mex --verbose

CXXFLAGS += -std=c++17

# optimization:
CXXFLAGS_OPT += -O3
#CXXFLAGS_OPT += -fomit-frame-pointer -ffast-math -funroll-loops
#CXXFLAGS_OPT += -march=native
CPPFLAGS += -DNDEBUG
# TODO: check for SSE?

CXXFLAGS += -pthread

# TODO: this works for 64bit systems, check if it also works on 32bit
CXXFLAGS += -fPIC

# maybe for Mac OS X:
#CFLAGS += -fno-common -no-cpp-precomp -fexceptions

#CPPFLAGS += -DSSR_MEX_USE_DOUBLE

# show many warnings
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -pedantic
# warnings are errors
CXXFLAGS += -pedantic-errors
# TODO: re-enable when deprecated dynamic exception specifications are removed:
#CXXFLAGS += -Werror
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
LDLIBS += `$(PKG_CONFIG) --libs $(LIBRARIES)`
CPPFLAGS += `$(PKG_CONFIG) --cflags $(LIBRARIES)`

CPPFLAGS += -I$(APF_DIR) -I$(SRC_DIR)

OCTAVE_TARGETS := $(MEXFILES:%=%.octave)
MATLAB_TARGETS := $(MEXFILES:%=%.matlab)

OBJECTS := $(OBJECTS:%=%.o)

CXXFLAGS += $(CXXFLAGS_OPT)

# prevent make from deleting intermediate files:
.SECONDARY:

vpath %.cpp $(SRC_DIR)

all:
	$(MAKE) octave
	$(MAKE) matlab

octave: $(OCTAVE_TARGETS)

matlab: $(MATLAB_TARGETS)

# Rebuild objects if Makefile has changed:
$(OBJECTS): Makefile

# Targets have different extension depending on the platform.
# To avoid finding out the extension, we use empty *.stamp files.

$(OCTAVE_TARGETS) $(MATLAB_TARGETS): %: %.stamp

STAMPFILES := $(OCTAVE_TARGETS:%=%.stamp) $(MATLAB_TARGETS:%=%.stamp)

%.octave.stamp: %.cpp $(OBJECTS) | Makefile
	CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" $(OCT) $(CPPFLAGS) $(LDLIBS) $< \
	    $(filter %.o, $^)
	@touch $@

# TODO: somehow add -O3 to linking step

%.matlab.stamp: %.cpp $(OBJECTS) | Makefile
	$(MEX) $(CPPFLAGS) $(LDLIBS) CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" \
	    CXXOPTIMFLAGS="$(CXXFLAGS_OPT)" LDOPTIMFLAGS="$(CXXFLAGS_OPT)" $< \
	    $(filter %.o, $^)
	@touch $@

# Reminder: to specify MEX file name: -output bla

clean:
	$(RM) $(MEXFILES:%=%.mexa64)
	$(RM) $(MEXFILES:%=%.mexglx)
	$(RM) $(MEXFILES:%=%.mexmaci)
	$(RM) $(MEXFILES:%=%.mexmaci64)
	$(RM) $(MEXFILES:%=%.mex)
	$(RM) $(MEXFILES:%=%.o)  # mkoctfile creates these
	$(RM) $(OBJECTS)
	$(RM) $(STAMPFILES)

.PHONY: all octave matlab clean $(OCTAVE_TARGETS) $(MATLAB_TARGETS)

.DELETE_ON_ERROR:

DEPENDENCIES := $(OBJECTS) $(STAMPFILES)
BINARY_EXTENSIONS := .o .octave.stamp .matlab.stamp
include ../apf/misc/Makefile.dependencies

