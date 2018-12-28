#!/bin/sh

# Shell-Script for starting the SSR locally in the tarball directory.
# This is not needed for the installed version!

# find the directory where the script is located
# (in case it was started from somewhere else)
DATA_DIR="$(dirname "$0")"
SCRIPTNAME="$(basename "$0")"

DEFAULT_EXECUTABLE=ssr-binaural

if [ "$SCRIPTNAME" = local_ssr.sh ]
then
  echo -n "Which SSR binary do you want to start? [$DEFAULT_EXECUTABLE] "
  read SCRIPTNAME
fi

SSR="$DATA_DIR/../src/${SCRIPTNAME:=$DEFAULT_EXECUTABLE}"
CONF="$DATA_DIR/ssr.conf.local"

if [ -x "$SSR" ]
then
  "$SSR" -c "$CONF" "$@"
else
  echo $0: Error: $SSR not found. Did you forget to \"make\" it?
  exit 1
fi
