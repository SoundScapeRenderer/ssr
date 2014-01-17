JACK_STUFF += jack_simpleprocessor jack_dynamic_inputs jack_dynamic_outputs
JACK_STUFF += jack_minimal jack_matrix jack_query_thread
JACK_STUFF += jack_convolver
JACK_STUFF += jack_change_volume
JACK_STUFF += jack_connections

PORTAUDIO_STUFF += portaudio_simpleprocessor

SNDFILE_STUFF += audiofile_simpleprocessor
SNDFILE_STUFF += jack_convolver

FFTW_STUFF += jack_convolver

EXECUTABLES += $(SNDFILE_STUFF) $(JACK_STUFF) $(PORTAUDIO_STUFF) $(FFTW_STUFF)
EXECUTABLES += dummy_example

MEX_FILES = mex_simpleprocessor.mex

OPT ?= -O3

CXXFLAGS += $(OPT)
CXXFLAGS += -g

CXXFLAGS += -std=c++11

LDLIBS += -lpthread

# show all warnings
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
CXXFLAGS += -Wshadow
CXXFLAGS += -Wold-style-cast
CXXFLAGS += -Wlong-long
CXXFLAGS += -Wconversion
#CXXFLAGS += -Winline
#CXXFLAGS += -Weffc++

CPPFLAGS += -I.. -D_REENTRANT

#### no more setting below here ####

FLEXTPATH ?= /usr/local/src/flext

# without this, intermediate .o files are generated:
.SUFFIXES:
.SUFFIXES: .cpp .o

# this adds (very slow) runtime checks for many STL functions:
debug: CPPFLAGS += -D_GLIBCXX_DEBUG
debug: all

no-debug: CPPFLAGS += -DNDEBUG
no-debug: all

all: $(EXECUTABLES)

.PHONY: debug no-debug all

$(JACK_STUFF): LDLIBS += -ljack
$(JACK_STUFF): CPPFLAGS += -DAPF_JACK_POLICY_DEBUG

$(SNDFILE_STUFF): LDLIBS += -lsndfile

$(PORTAUDIO_STUFF): LDLIBS += -lportaudio

$(FFTW_STUFF): LDLIBS += -lfftw3f

# For Puredata stuff see also package.txt

pd: $(FLEXTPATH) package.txt
	$(FLEXTPATH)/build.sh pd gcc

.PHONY: pd

$(FLEXTPATH):
	@test -d $(FLEXTPATH) || \
		( echo \"$(FLEXTPATH)\" not found! Set FLEXTPATH! ; false )

mex: CPPFLAGS += -DNDEBUG
mex: $(MEX_FILES)

mex-double: CPPFLAGS += -DMEX_USE_DOUBLE
mex-double: mex

.PHONY: mex mex-double

%.mex: %.cpp
	CXXFLAGS="$(CXXFLAGS)" mkoctfile --mex $< $(CPPFLAGS)
	$(RM) $*.o

clean:
	$(RM) $(EXECUTABLES) $(OBJECTS)
	test -d $(FLEXTPATH) && $(FLEXTPATH)/build.sh pd gcc clean || true
	rmdir pd-linux || true
	$(RM) $(MEX_FILES)

.PHONY: clean

# rebuild everything when Makefile changes
$(OBJECTS) $(EXECUTABLES) $(PD_EXTERNALS) $(MEX_FILES): Makefile

DEPENDENCIES = $(EXECUTABLES) $(OBJECTS)

include ../misc/Makefile.dependencies
