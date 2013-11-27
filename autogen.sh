#!/bin/sh

# Shell script for automation of autotools.

run_command()
{
  # show message; run command; if unsuccessful, show error message and exit
  echo $0: Running \"$@\" ...
  "$@" || { status=$?; echo $0: Error in \"$@\"!; exit $status; }
}

# change to the directory where the script is located
# (in case it was started from somewhere else)
cd $(dirname $0)

# TODO: find a way to move aclocal.m4 from the root directory to somewhere else
#ACLOCAL_FLAGS="--output=autotools/aclocal.m4"

# Preferring autoreconf for a more transparent configuration/debugging process.
if test -f autotools/config/depcomp; then
  if test x$1 != x--no-auto; then
    # TODO: check if autoreconf is available?
    echo $0: Using \"autoreconf\" from now on.
    echo Use \"$0 --no-auto\" to override this behaviour.
    run_command autoreconf
    echo $0: Done!
    exit
  fi
fi

# on OS X, which(1) returns 0 even when it can't find a program 
if type libtoolize >/dev/null 2>&1; then
  LIBTOOLIZE=libtoolize
else
  if which glibtoolize >/dev/null; then
    # on the Mac it's called glibtoolize for some reason
    LIBTOOLIZE=glibtoolize
  else
    echo $0: Error: libtoolize not found!
    exit 127
  fi
fi

run_command $LIBTOOLIZE --force

# in most cases, "libtoolize" is used like this:
#
# libtoolize --force 2>&1 | sed '/^You should/d' || {
#    echo "libtool failed, exiting..."
#    exit 1
# }
#
# This just removes all lines starting with "You should" from the output

run_command aclocal $ACLOCAL_FLAGS
run_command autoheader
run_command automake --add-missing --foreign
run_command autoconf

echo $0: Done!

# Settings for Vim (http://www.vim.org/), please do not remove:
# vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80
