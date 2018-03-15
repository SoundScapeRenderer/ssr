#!/usr/bin/env bash

set -euo pipefail

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # liblo-dev is still at 0.28 in trusty, but we require 0.29
  wget https://downloads.sourceforge.net/liblo/liblo-0.29.tar.gz
  tar xvf liblo-0.29.tar.gz && cd liblo-0.29
  ./configure && make && sudo make install
  sudo ldconfig
  cd ..
fi

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # help2man requires perl's Locale::gettext
  cpanm Locale:gettext
fi
exit 0
