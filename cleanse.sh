#!/bin/sh

# Shell-Script for removing all files that don't belong into the Git repo.
# See also http://www.gnu.org/software/hello/manual/automake/Clean.html

# always exit on error
set -e

# change to the directory where the script is located
# (in case it was started from somewhere else)
cd "$(dirname "$0")"

if test -f Makefile
then
  echo $0: Running \"make maintainer-clean\" ...
  make maintainer-clean
elif test -x configure
then
  echo $0: Error: Run the configure script first, then \"$0\"!
  exit 1
fi

echo $0: Removing miscellaneous files ...
rm -rf autotools/
rm -f src/config.h.in configure Makefile.in src/Makefile.in data/Makefile.in
rm -f data/MacOSX/Makefile.in man/Makefile.in

SSR_TARBALL=ssr-*.*.*.tar.gz
SSR_USERMANUAL=doc/SoundScapeRenderer-*.*.*-manual.pdf
# BTW, the user manual is copied and renamed on "make dist"

# if several files match, all of them are deleted
for i in $SSR_TARBALL $SSR_USERMANUAL
do
  test -f $i || continue
  echo $0: Removing \"$i\" ...
  rm $i
done

if test -d doc/doxygen
then
  echo $0: Removing Doxygen documentation ...
  rm -r doc/doxygen
fi

echo $0: Cleansing user manual ...
(cd doc/manual && make clean)

# this is only shown if everything went smoothly
echo $0: Done!
