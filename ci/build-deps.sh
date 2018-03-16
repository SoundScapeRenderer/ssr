#!/usr/bin/env bash

set -euo pipefail

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # liblo-dev is still at 0.28 in trusty, but we require 0.29
  wget https://downloads.sourceforge.net/liblo/liblo-0.29.tar.gz
  tar xvf liblo-0.29.tar.gz && cd liblo-0.29
  ./configure && make && sudo make install
  sudo ldconfig
  cd ..
  if [ $COMPILE_ASIO -eq 1 ]; then
    # remove previously installed asio, as we're trying a newer version
    sudo apt-get remove libasio-dev
    # compile and install asio 1.12.0
    wget https://github.com/chriskohlhoff/asio/archive/asio-1-12-0.tar.gz
    tar xvf asio-1-12-0.tar.gz
    cd asio-asio-1-12-0/asio
    autoreconf -vfi
    ./configure --with-boost=no --prefix=/usr
    make
    sudo make install
  fi
fi

exit 0
