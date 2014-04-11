#!/bin/sh

# if no patch is specified on the command line, this one is opened:
PD_PATCH=ssr_binaural~-help.pd

if [ $# -gt 0 ]
then
  PD_PATCH="$@"
fi

cd "$(dirname "$0")"

pd -path pd-linux/release-single -helppath . "$PD_PATCH" &
