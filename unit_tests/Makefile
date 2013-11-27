# Makefile for unit tests

TESTS += test_math
TESTS += test_stringtools
TESTS += test_trivial_iterator
TESTS += test_iterator
TESTS += test_accumulating_iterator
TESTS += test_cast_iterator
TESTS += test_circular_iterator
TESTS += test_transform_iterator
TESTS += test_index_iterator
TESTS += test_stride_iterator
TESTS += test_dual_iterator
TESTS += test_discard_iterator
TESTS += test_iterator_combinations
TESTS += test_biquad
TESTS += test_blockdelayline
TESTS += test_container
TESTS += test_mimoprocessor
TESTS += test_combine_channels
TESTS += test_misc

ifneq (,$(findstring $(MAKECMDGOALS), fftw clean))
TESTS += test_fftwtools
TESTS += test_convolver
endif

OBJECTS = $(TESTS:=.o)

CXXFLAGS += -std=c++11

CXXFLAGS += -g

# show all warnings
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -pedantic
# warnings are errors
CXXFLAGS += -pedantic-errors
#CXXFLAGS += -Werror
# even more warnings:
CXXFLAGS += -Wpointer-arith
CXXFLAGS += -Wcast-align
CXXFLAGS += -Wwrite-strings
CXXFLAGS += -Wredundant-decls
CXXFLAGS += -Wconversion
CXXFLAGS += -Wshadow
CXXFLAGS += -Wold-style-cast
CXXFLAGS += -Wlong-long
CXXFLAGS += -Winline

#CXXFLAGS += -Wno-sign-conversion

CPPFLAGS += -I..

# this adds (very slow) runtime checks for many STL functions:
CPPFLAGS += -D_GLIBCXX_DEBUG

all:
	$(MAKE) fftw

run_tests: build_tests
	./main

build_tests: main

fftw: LDLIBS += -lfftw3f -lfftw3 -lfftw3l -lm
fftw: run_tests

main: $(OBJECTS)

# TODO: check why this gives false(?) positives in test_blockdelayline.h
test_blockdelayline.o: CPPFLAGS := $(filter-out -D_GLIBCXX_DEBUG,$(CPPFLAGS))

DEPENDENCIES = main $(OBJECTS)

clean:
	$(RM) $(DEPENDENCIES)

.PHONY: all build_tests run_tests clean fftw

# rebuild everything when Makefile changes
$(DEPENDENCIES): Makefile

include ../misc/Makefile.dependencies
