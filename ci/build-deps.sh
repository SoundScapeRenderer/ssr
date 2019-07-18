#!/usr/bin/env bash

set -euo pipefail

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # liblo-dev is still at 0.28 in trusty, but we require 0.29
  wget https://downloads.sourceforge.net/liblo/liblo-0.29.tar.gz
  tar xvf liblo-0.29.tar.gz && cd liblo-0.29
  ./configure && make && sudo make install
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
    cd ..
  fi

  wget https://www.intersense.com/wp-content/uploads/2018/12/InterSense_SDK_4.2381.zip
  unzip InterSense_SDK_4.2381.zip
  sudo cp SDK/Linux/Sample/*.h /usr/local/include
  sudo cp SDK/Linux/x86_64/libisense.so /usr/local/lib

  sudo ldconfig
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

exit 0
