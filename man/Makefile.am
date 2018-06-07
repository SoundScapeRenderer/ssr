## This file will be processed by automake (which is called by autogen.sh) to
## generate Makefile.in, which in turn will be processed by configure to
## generate Makefile.

## comments starting with a single # are copied to Makefile.in (and afterwards
## to Makefile), comments with ## are dropped.

if HAVE_HELP2MAN
dist_man_MANS = $(SSR_executables:=.1)
endif

MAINTAINERCLEANFILES = $(dist_man_MANS)

RENDERER = $(@:ssr-%.1=%)
RENDERER_TEXT = "manual page for the \
		`test $(RENDERER) = binaural -o $(RENDERER) = generic \
		&& echo $(RENDERER) \
		|| echo $(RENDERER) | tr 'a-z' 'A-Z'` renderer"

# help2man must be called after the binaries are built in the src/ folder!
# Note the order of SUBDIRS in the top-level Makefile.
#
# "make" must be called before "make dist", there is no automatic dependency!
HELP2MAN_LOCALE ?= C.UTF-8

$(dist_man_MANS): $(top_srcdir)/src/configuration.cpp $(top_srcdir)/configure.ac 
	help2man \
		--no-info \
                --name=$(RENDERER_TEXT) \
                --output=$@ \
                --locale=$(HELP2MAN_LOCALE) $(top_builddir)/src/$(@:.1=$(EXEEXT))

## Settings for Vim (http://www.vim.org/), please do not remove:
## vim:textwidth=80:comments+=bO\:##
