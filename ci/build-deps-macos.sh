#!/usr/bin/env bash

set -euo pipefail

git clone https://github.com/zaphoyd/websocketpp.git
cd websocketpp
mkdir build
cd build
cmake ..
make
sudo make install
cd ..
cd ..

git clone https://github.com/hoene/libmysofa.git
cd libmysofa
cd build
cmake ..
make
sudo make install
cd ..
cd ..
