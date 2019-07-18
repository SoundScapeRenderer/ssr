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

git clone git://github.com/zaphoyd/websocketpp.git
cd websocketpp
mkdir build
cd build
cmake ..
make
sudo make install
cd ..
cd ..


if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  git clone https://github.com/vrpn/vrpn.git
  cd vrpn
  mkdir build
  cd build
  cmake -DVRPN_BUILD_JAVA=OFF ..
  make
  sudo make install
  cd ..
  cd ..
fi


exit 0
