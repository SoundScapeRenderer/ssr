## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

check_PROGRAMS = catch2

catch2_SOURCES = main.cpp pathtools.cpp

catch2_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/apf/unit_tests

check-local:
	./catch2
