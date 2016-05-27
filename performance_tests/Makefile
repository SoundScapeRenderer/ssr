EXECUTABLES += crossfade
EXECUTABLES += interpolation
EXECUTABLES += biquad_denormals
EXECUTABLES += biquad_count_denormals

OPT ?= -O3

# TODO: automatic tests of different combinations for biquad_denormals:
OPT += -march=native
#OPT += -mfpmath=sse
#OPT += -msse
#OPT += -msse3

CXXFLAGS += $(OPT)
CXXFLAGS += -g

CXXFLAGS += -std=c++11 -pthread

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
CXXFLAGS += -Wconversion
CXXFLAGS += -Wshadow
CXXFLAGS += -Wold-style-cast
CXXFLAGS += -Wlong-long
CXXFLAGS += -Wconversion
#CXXFLAGS += -Winline
#CXXFLAGS += -Weffc++

CPPFLAGS += -I..
CPPFLAGS += -DNDEBUG

#### no more setting below here ####

# without this, intermediate .o files are generated:
.SUFFIXES:
.SUFFIXES: .cpp .o

all: $(EXECUTABLES)

.PHONY: all

clean:
	$(RM) $(EXECUTABLES) $(OBJECTS)

.PHONY: clean

# rebuild everything when Makefile changes
$(OBJECTS) $(EXECUTABLES): Makefile

DEPENDENCIES = $(EXECUTABLES) $(OBJECTS)

include ../misc/Makefile.dependencies
